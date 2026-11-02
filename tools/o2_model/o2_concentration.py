# -*- coding: utf-8 -*-
"""
7500HA-YY 超声波氧气传感器 —— 氧气浓度算法（Python 版）
=====================================================================

本文件是固件 C 代码的等价 Python 移植，只保留"氧气浓度"这条链路，
不包含流量、压力、报警、串口等任何与流量相关的内容。

--------------------------------------------------------------------
一、算法出处（C 源码位置）
--------------------------------------------------------------------
1) 标定系数计算：
   MyDriver/src/Linearization.c  ->  FnLinearization1()
                                  ->  stdFf()（静态函数）
   依赖库函数：
   CoeffCountMod/FnCoeffCount3P()  三点二次曲线拟合  y = C0*x^2 + C1*x + C2
2) 浓度实时计算：
   user/src/ResultCount.c        ->  ResultCount5P_O2()
3) 飞行计数值的 10 倍增益：
   user/src/main.c               ->  lFlowRefCountFf = nT1CountLeft * 10;
   （固件送给算法的 Ff/Fb 是原始捕获计数的 10 倍，出厂标定存入 EEPROM 的
     也是 10 倍后的值；而检验报告里打印的 Ff/Fb 是原始计数，两者相差 10 倍）

--------------------------------------------------------------------
二、算法在做什么（一句话）
--------------------------------------------------------------------
超声波在气体中的飞行时间只取决于气体分子量（氧气比氮气重，氧越多飞得越慢）。
算法用"空气点(20.9%)"和"氧气点(94.6%)"这两个已知浓度把飞行计数值标成浓度，
并且这个"计数差 -> 浓度差"的比例随温度变化，所以先用 5℃/30℃/50℃ 三点分别拟合
空气计数值-温度 和 氧气计数值-温度 两条二次曲线，运行时按当前温度求两条曲线，
再在两者之间做线性插值。

        O2(%) = 209 + (CalO2 - 209) * (S - S_air(T)) / (S_o2(T) - S_air(T))
        其中 S = k*(Ff + Fb)，k = 10（固件增益）

--------------------------------------------------------------------
三、单位约定（非常重要）
--------------------------------------------------------------------
* 温度 T      : 0.01 ℃ 的整数。例如 647 表示 6.47 ℃，3083 表示 30.83 ℃。
                （MyDriver/src/ntc.c -> test_tab() 返回 temp*100）
* 浓度        : 0.1 % 的整数。209 = 20.9 %，946 = 94.6 %。
* 飞行计数 Ff : 固件算法内部使用的计数（= 原始捕获计数 * 10，约 14 万）。
                若手上是检验报告里的原始计数（约 1.4 万），传 raw_counts=True
                或先自己乘 10。
--------------------------------------------------------------------

作者：由 7500HA-YY V3.08.032 固件逆向移植
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Iterable, Sequence

# ----------------------------------------------------------------------------
# 常量（与固件保持一致）
# ----------------------------------------------------------------------------
CONC_AIR = 209          # 空气点浓度 20.9%，固件里写死 209（RXExecute.c -> FnPointSaveT(?, 209)）
CONC_O2_DEFAULT = 946   # 氧气点浓度 94.6%，标定时由上位机下发（必须 > 500 才被接收）
COUNT_GAIN = 10.0       # main.c: lFlowRefCountFf = nT1CountLeft * 10
O2_RESULT_MAX = 1200    # 输出上限 120.0%
O2_RESULT_MIN = 0       # 输出下限 0%


# ----------------------------------------------------------------------------
# 基础数学：三点二次曲线拟合（对应 CoeffCountMod V3.0 USF.lib 的 fnCoeffCount3P）
# ----------------------------------------------------------------------------
def fn_coeff_count_3p(x: Sequence[float], y: Sequence[float]) -> tuple[float, float, float]:
    """过三点的二次曲线 y = C0*x^2 + C1*x + C2，返回 (C0, C1, C2)。

    与固件库函数 fnCoeffCount3P() 等价：三个点唯一确定一条抛物线，
    因此这里用拉格朗日插值求解，结果与最小二乘三点拟合完全相同。
    """
    x0, x1, x2 = float(x[0]), float(x[1]), float(x[2])
    y0, y1, y2 = float(y[0]), float(y[1]), float(y[2])

    d0 = (x0 - x1) * (x0 - x2)
    d1 = (x1 - x0) * (x1 - x2)
    d2 = (x2 - x0) * (x2 - x1)
    if d0 == 0 or d1 == 0 or d2 == 0:
        raise ValueError("三点二次拟合失败：存在相同温度点 %r" % (list(x),))

    # y = a2*x^2 + a1*x + a0
    a2 = y0 / d0 + y1 / d1 + y2 / d2
    a1 = -(y0 * (x1 + x2) / d0 + y1 * (x0 + x2) / d1 + y2 * (x0 + x1) / d2)
    a0 = y0 * x1 * x2 / d0 + y1 * x0 * x2 / d1 + y2 * x0 * x1 / d2
    return a2, a1, a0


def poly_eval(coef: Sequence[float], x: float) -> float:
    """y = C0*x^2 + C1*x + C2（与固件 fSlopeFsumX[0]*x*x + fSlopeFsumX[1]*x + fSlopeFsumX[2] 一致）"""
    return coef[0] * x * x + coef[1] * x + coef[2]


# ----------------------------------------------------------------------------
# 标定 + 测量
# ----------------------------------------------------------------------------
@dataclass
class CalPoint:
    """单个标定点：温度(0.01℃)、飞行计数(Ff, Fb，算法单位)"""
    temp: float
    ff: float
    fb: float

    @property
    def total(self) -> float:
        """飞行时间 = Ff + Fb（固件里 CountLong）"""
        return self.ff + self.fb


@dataclass
class O2Calibration:
    """一台传感器的标定数据：空气 3 点 + 氧气 3 点。"""
    air_points: list[CalPoint]
    o2_points: list[CalPoint]
    # 标定时上位机下发的氧气标准浓度（0.1%），默认三点都是 94.6%
    o2_conc: list[float] = field(default_factory=lambda: [CONC_O2_DEFAULT] * 3)

    # 计算结果（标定完成后填充）
    air_coef: tuple[float, float, float] = (0.0, 0.0, 0.0)
    o2_coef: tuple[float, float, float] = (0.0, 0.0, 0.0)
    o2_total_normalized: list[float] = field(default_factory=list)

    def __post_init__(self) -> None:
        if len(self.air_points) != 3 or len(self.o2_points) != 3:
            raise ValueError("空气点和氧气点都必须各 3 个（5℃ / 30℃ / 50℃）")
        if len(self.o2_conc) != 3:
            raise ValueError("o2_conc 需要 3 个温度点对应的氧气标准浓度")
        if any(c <= 500 for c in self.o2_conc):
            # 固件 RXExecute.c 中 "if(tempint > 500)" 才接收氧气点浓度
            raise ValueError("氧气点浓度必须 > 500（即 > 50.0%），当前 %r" % (self.o2_conc,))

    # -- 标定流程：FnLinearization1() -------------------------------------
    def build(self) -> "O2Calibration":
        """等价于固件 FnLinearization1()：算出 fSlopeFsumAir / fSlopeFsumO2。"""
        t_air = [p.temp for p in self.air_points]
        s_air = [p.total for p in self.air_points]
        # 1) 空气点温度曲线
        self.air_coef = fn_coeff_count_3p(t_air, s_air)

        # 2) 氧气点温度曲线（先做 stdFf 归一化）
        t_o2 = [p.temp for p in self.o2_points]
        s_o2 = [p.total for p in self.o2_points]
        self.o2_total_normalized = self._std_ff(s_air, self.o2_conc, s_o2)
        self.o2_coef = fn_coeff_count_3p(t_o2, self.o2_total_normalized)
        return self

    @staticmethod
    def _std_ff(air_sum: Sequence[float], conc: Sequence[float], o2_sum: Sequence[float]) -> list[float]:
        """Linearization.c -> stdFf()：把 5℃ / 50℃ 的氧气点计数折算到 30℃ 的氧气浓度上。

        固件原逻辑（以第 0 点为例）：
            kk = (O2Ff[0]-AirFf[0]) / (Cn[0]-209)
            bb = AirFf[0] - kk*209
            O2Ff[0] = kk*Cn[1] + bb        // 用常温(30℃)的氧浓度反算 5℃ 应有的计数值
        即"假设跨导(counts per 0.1%O2)在该温度下线性"，把不同温度下标准气的浓度
        差异折算掉。若三点标准气浓度相同（本批数据均为 94.6%），该步骤为恒等变换。
        """
        out = [float(o2_sum[0]), float(o2_sum[1]), float(o2_sum[2])]
        for i in (0, 2):
            kk = (o2_sum[i] - air_sum[i]) / (conc[i] - CONC_AIR)
            bb = air_sum[i] - kk * CONC_AIR
            out[i] = kk * conc[1] + bb
        return out

    # -- 实时计算：ResultCount5P_O2() -------------------------------------
    def measure_raw(self, temp: float, ff: float, fb: float, raw_counts: bool = False) -> float:
        """同上，但返回未取整、未限幅的浓度（0.1% 单位），用于灵敏度分析。"""
        if not self.o2_coef or not self.air_coef:
            self.build()
        s = ff + fb
        if raw_counts:
            s *= COUNT_GAIN

        temp_air = poly_eval(self.air_coef, temp)     # 该温度下空气应有的计数
        temp_o2 = poly_eval(self.o2_coef, temp)       # 该温度下氧气点应有的计数

        # 一次标定：把实测计数线性映射到 209 ~ CalO2 之间
        cal_o2 = self.o2_conc[1]                      # calibrationO2（30℃ 那个点）
        cal_o2_fac = self.o2_conc[2]                  # calibrationO2Fac（50℃ 那个点）
        span = temp_o2 - temp_air
        if span == 0:
            raise ZeroDivisionError("空气点与氧气点温度曲线重合，标定数据异常")
        k = (cal_o2 - CONC_AIR) / span
        b = CONC_AIR - k * temp_air
        conc = k * s + b

        # 二次标定：校正标准气源本身的误差（三点浓度相同则为恒等变换）
        k2 = (cal_o2_fac - CONC_AIR) / (cal_o2 - CONC_AIR)
        b2 = CONC_AIR - k2 * CONC_AIR
        return k2 * conc + b2

    def measure(self, temp: float, ff: float, fb: float, raw_counts: bool = False) -> float:
        """给定温度(0.01℃)和 Ff/Fb，返回氧气浓度（0.1% 单位）。

        raw_counts=True 时，认为传入的是检验报告里的原始计数（约 1.4 万），
        内部自动乘 COUNT_GAIN。
        """
        conc = self.measure_raw(temp, ff, fb, raw_counts)

        # 固件：tempO2 = (int)(tempvalue + 0.5); 限幅 0 ~ 1200
        conc = int(conc + 0.5)
        if conc < O2_RESULT_MIN:
            conc = O2_RESULT_MIN
        if conc > O2_RESULT_MAX:
            conc = O2_RESULT_MAX
        return float(conc)

    def measure_percent(self, temp: float, ff: float, fb: float, raw_counts: bool = False) -> float:
        """同上，但返回百分数（例如 94.6 表示 94.6%）。"""
        return self.measure(temp, ff, fb, raw_counts) / 10.0

    # -- 便于导出/对比 -----------------------------------------------------
    def report(self) -> dict:
        if not self.o2_coef or not self.air_coef:
            self.build()
        return {
            "air_coef": self.air_coef,          # (a2, a1, a0)
            "o2_coef": self.o2_coef,            # (b2, b1, b0)
            "o2_total_normalized": self.o2_total_normalized,
        }


# ----------------------------------------------------------------------------
# 便捷构造函数：直接用 6 个标定点建好模型
# ----------------------------------------------------------------------------
def build_from_points(
    air: Iterable[tuple[float, float, float]],
    o2: Iterable[tuple[float, float, float]],
    o2_conc: float | Sequence[float] = CONC_O2_DEFAULT,
    raw_counts: bool = False,
) -> O2Calibration:
    """air / o2 均为 [(T, Ff, Fb), ...] 三点；raw_counts=True 表示计数值是原始计数。"""
    gain = COUNT_GAIN if raw_counts else 1.0
    air_pts = [CalPoint(t, ff * gain, fb * gain) for t, ff, fb in air]
    o2_pts = [CalPoint(t, ff * gain, fb * gain) for t, ff, fb in o2]
    if isinstance(o2_conc, (int, float)):
        conc = [float(o2_conc)] * 3
    else:
        conc = [float(c) for c in o2_conc]
    return O2Calibration(air_pts, o2_pts, conc).build()


# ----------------------------------------------------------------------------
# 自检：用一组真实标定数据跑一遍
# ----------------------------------------------------------------------------
if __name__ == "__main__":
    # 仪器编号 1798260714010911 的标定原始数据（来自 标定提取结果.xlsx）
    air = [(647, 145140, 145120), (3083, 141550, 141560), (4871, 139200, 139190)]
    o2 = [(632, 150550, 150560), (3072, 146790, 146800), (4896, 144230, 144250)]

    algo = build_from_points(air, o2)
    print("空气曲线 a2/T^2 + a1/T + a0 =", algo.air_coef)
    print("氧气曲线 b2/T^2 + b1/T + b0 =", algo.o2_coef)

    # 30℃ 检验报告（20260920 7500H（982）表 '30' 页）：
    #   零点：T=30.7℃，Ff=14157，Fb=14158 -> 表中值 20.5%
    #   量程：T=30.7℃，Ff=14681，Fb=14681 -> 表中值 94.7%
    print("空气点 ->", algo.measure(3070, 14157, 14158, raw_counts=True), " (0.1%)")
    print("氧气点 ->", algo.measure(3070, 14681, 14681, raw_counts=True), " (0.1%)")

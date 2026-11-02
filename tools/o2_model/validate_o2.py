# -*- coding: utf-8 -*-
"""
验证脚本
========
用 标定提取结果.xlsx 的标定数据建模型 ->
用 20260920 7500H（982）V3.08.032架25.xlsx 的 20 / 30 / 45 三个检验页做验证 ->
把算法算出的氧气浓度和检验报告里记录的值逐条对比。

输出：
  1) 控制台摘要
  2) 验证结果.xlsx（10台明细 / 全量统计 / 全量明细）
"""
from __future__ import annotations

import os
import statistics as st

import openpyxl
from openpyxl.styles import Alignment, Border, Font, PatternFill, Side
from openpyxl.utils import get_column_letter

from o2_concentration import build_from_points

HERE = os.path.dirname(os.path.abspath(__file__))
DATA_DIR = os.environ.get("O2_DATA_DIR", os.path.join(HERE, "data"))


def _locate(name: str) -> str:
    """按 data/ -> 原工作目录 的顺序找数据文件，方便仓库内外都能跑。"""
    for d in (DATA_DIR, r"D:\sensors"):
        p = os.path.join(d, name)
        if os.path.exists(p):
            return p
    raise FileNotFoundError(f"找不到 {name}，请放到 {DATA_DIR} 或用环境变量 O2_DATA_DIR 指定")


CAL_FILE = _locate("标定提取结果.xlsx")
REP_FILE = _locate("20260920 7500H（982）V3.08.032架25.xlsx")
OUT_FILE = os.path.join(HERE, "验证结果.xlsx")

PAGES = ("20", "30", "45")     # 检验报告里的三个温度页
TOL = 0.5                      # 判定"一致"的阈值，单位：氧浓度百分点
N_SHOW = 10                    # 明细展示的传感器台数

CAL_KEYS = ["Ff_air_5", "Fb_air_5", "T_air_5", "Ff_air_30", "Fb_air_30", "T_air_30",
            "Ff_air_50", "Fb_air_50", "T_air_50", "Ff_o2_5", "Fb_o2_5", "T_o2_5",
            "Ff_o2_30", "Fb_o2_30", "T_o2_30", "Ff_o2_50", "Fb_o2_50", "T_o2_50"]


# ---------------------------------------------------------------- 数据读取
def load_calib() -> dict[str, dict]:
    """读 标定提取结果.xlsx；同一编号重复出现（表内有重复块）时取第一条。"""
    wb = openpyxl.load_workbook(CAL_FILE, read_only=True, data_only=True)
    ws = wb["提取结果"]
    rows = list(ws.iter_rows(min_row=1, values_only=True))
    hdr = rows[0]
    out: dict[str, dict] = {}
    for r in rows[1:]:
        if not r[0]:
            continue
        d = dict(zip(hdr, r))
        sid = str(r[0]).strip()
        if sid in out:
            continue
        if any(d.get(k) in (None, "") for k in CAL_KEYS):
            continue                      # 标定不全的跳过
        out[sid] = d
    return out


def load_report(page: str) -> list[dict]:
    wb = openpyxl.load_workbook(REP_FILE, read_only=True, data_only=True)
    ws = wb[page]
    rows = list(ws.iter_rows(min_row=3, values_only=True))
    hdr = rows[0]
    return [dict(zip(hdr, r)) for r in rows[1:] if r[0]]


def build_algo(c: dict, o2_conc: float = 946):
    """从一行标定数据建立氧气模型（标定计数已是算法单位，无需再放大 10 倍）。"""
    air = [(float(c["T_air_5"]), float(c["Ff_air_5"]), float(c["Fb_air_5"])),
           (float(c["T_air_30"]), float(c["Ff_air_30"]), float(c["Fb_air_30"])),
           (float(c["T_air_50"]), float(c["Ff_air_50"]), float(c["Fb_air_50"]))]
    o2 = [(float(c["T_o2_5"]), float(c["Ff_o2_5"]), float(c["Fb_o2_5"])),
          (float(c["T_o2_30"]), float(c["Ff_o2_30"]), float(c["Fb_o2_30"])),
          (float(c["T_o2_50"]), float(c["Ff_o2_50"]), float(c["Fb_o2_50"]))]
    return build_from_points(air, o2, o2_conc=o2_conc)


# ---------------------------------------------------------------- 单条评估
def infer_temp(algo, ff: float, fb: float, target: float, t_guess: float) -> float:
    """反推"等效温度"：要让空气点算出来正好等于 target，温度该是多少（0.01℃）。

    空气点对温度极敏感（约 2 %O2/℃），报告里的温度只记到 0.1℃；
    万一某行偏差较大，看这个反推温度就知道是"温度没记准"还是"算法不对"。
    """
    def f(t):
        return algo.measure_raw(t, ff, fb, raw_counts=True) / 10.0 - target

    lo, hi = t_guess - 500, t_guess + 500
    flo = f(lo)
    if flo * f(hi) > 0:
        return float("nan")
    for _ in range(60):
        mid = (lo + hi) / 2
        fm = f(mid)
        if fm * flo <= 0:
            hi = mid
        else:
            lo, flo = mid, fm
    return (lo + hi) / 2


def evaluate(algo, d: dict) -> dict:
    """对检验报告的一行：算空气点和氧气点的浓度。"""
    t_air = float(d["零点检验温度(℃)"]) * 100        # 0.1℃ -> 0.01℃
    t_o2 = float(d["量程点检验温度(℃)"]) * 100
    ff_a, fb_a = float(d["零点检验Ff"]), float(d["零点检验Fb"])
    ff_o, fb_o = float(d["量程点检验Ff"]), float(d["量程点检验Fb"])

    a_calc = algo.measure(t_air, ff_a, fb_a, raw_counts=True) / 10
    o_calc = algo.measure(t_o2, ff_o, fb_o, raw_counts=True) / 10
    a_tab = float(d["零点检验O2(%)"])
    o_tab = float(d["量程点检验O2(%)"])

    return {
        "sid": str(d["仪器编号"]).strip(),
        "pos": d.get("位置"),
        "t_air": t_air / 100,
        "t_o2": t_o2 / 100,
        "t_eq": infer_temp(algo, ff_a, fb_a, a_tab, t_air) / 100,
        "ff_a": ff_a, "fb_a": fb_a,
        "ff_o": ff_o, "fb_o": fb_o,
        "air_tab": a_tab, "air_calc": a_calc, "air_diff": a_calc - a_tab,
        "o2_tab": o_tab, "o2_calc": o_calc, "o2_diff": o_calc - o_tab,
    }


# ---------------------------------------------------------------- Excel 输出
HDR_FILL = PatternFill("solid", fgColor="1F4E79")
HDR_FONT = Font(color="FFFFFF", bold=True, size=10)
SUB_FILL = PatternFill("solid", fgColor="DDEBF7")
THIN = Side(style="thin", color="B0B0B0")
BORDER = Border(left=THIN, right=THIN, top=THIN, bottom=THIN)


def write_sheet(ws, headers, rows, widths, title_rows=()):
    r = 1
    for text in title_rows:
        ws.cell(row=r, column=1, value=text).font = Font(bold=True, size=11, color="1F4E79")
        r += 1
    start = r
    for j, h in enumerate(headers, 1):
        c = ws.cell(row=r, column=j, value=h)
        c.fill = HDR_FILL
        c.font = HDR_FONT
        c.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
        c.border = BORDER
    for row in rows:
        r += 1
        for j, v in enumerate(row, 1):
            c = ws.cell(row=r, column=j, value=v)
            c.border = BORDER
            c.alignment = Alignment(horizontal="center" if j > 2 else "left")
    for j, w in enumerate(widths, 1):
        ws.column_dimensions[get_column_letter(j)].width = w
    ws.freeze_panes = ws.cell(row=start + 1, column=1)


# ---------------------------------------------------------------- 主流程
def main() -> None:
    cal = load_calib()
    reports = {p: load_report(p) for p in PAGES}

    # 1) 挑 10 台：三页都合格、标定完整，按 30℃ 页的板卡顺序取前 10 台
    def pick() -> list[str]:
        by_page = {}
        for p in PAGES:
            m = {}
            for d in reports[p]:
                m[str(d["仪器编号"]).strip()] = d
            by_page[p] = m
        sel = []
        for d in reports["30"]:
            sid = str(d["仪器编号"]).strip()
            if sid not in cal:
                continue
            if all(by_page[p].get(sid, {}).get("数据是否合格") == "合格" for p in PAGES):
                sel.append(sid)
            if len(sel) >= N_SHOW:
                break
        return sel

    picked = pick()

    # 2) 10 台明细
    detail = []
    print("=" * 100)
    print("一、10 台传感器验证明细（表内 = 检验报告记录值，算法 = Python 复算值）")
    print("=" * 100)
    for sid in picked:
        algo = build_algo(cal[sid])
        for p in PAGES:
            d = next(x for x in reports[p] if str(x["仪器编号"]).strip() == sid)
            e = evaluate(algo, d)
            ok = abs(e["air_diff"]) <= TOL and abs(e["o2_diff"]) <= TOL
            detail.append([e["sid"], e["pos"], p + "℃", e["t_air"],
                           e["air_tab"], round(e["air_calc"], 1), round(e["air_diff"], 1),
                           round(e["t_eq"], 1),
                           e["o2_tab"], round(e["o2_calc"], 1), round(e["o2_diff"], 1),
                           "一致" if ok else "偏离"])
    hdr = (f"{'仪器编号':<17}{'位置':<9}{'页':>5}{'T(℃)':>7}|{'空气表':>7}{'空气算':>7}{'Δ':>6}"
           f"|{'氧表':>7}{'氧算':>7}{'Δ':>6}|{'判定':>6}")
    print(hdr)
    print("-" * len(hdr) + "   （另见 验证结果.xlsx 中的等效温度列）")
    for row in detail:
        print(f"{row[0]:<17}{str(row[1]):<9}{row[2]:>5}{row[3]:>7.1f}|{row[4]:>7.1f}{row[5]:>7.1f}{row[6]:>+6.1f}|"
              f"{row[8]:>7.1f}{row[9]:>7.1f}{row[10]:>+6.1f}|{row[11]:>6}")

    # 3) 全量统计（仅"合格"行，排除 -59.8 / 110.0 这类故障码）
    print()
    print("=" * 100)
    print("二、全量统计（20/30/45 三页所有判定为“合格”的记录）")
    print("=" * 100)
    stats_rows = []
    all_detail = []
    algo_cache: dict[str, object] = {}
    for p in PAGES:
        da, do = [], []
        for d in reports[p]:
            if d["数据是否合格"] != "合格":
                continue
            sid = str(d["仪器编号"]).strip()
            if sid not in cal:
                continue
            algo = algo_cache.get(sid) or build_algo(cal[sid])
            algo_cache[sid] = algo
            e = evaluate(algo, d)
            da.append(e["air_diff"])
            do.append(e["o2_diff"])
            all_detail.append([p, e["sid"], e["pos"], e["t_air"], e["t_o2"],
                               e["ff_a"], e["fb_a"], e["air_tab"], round(e["air_calc"], 1), round(e["air_diff"], 1),
                               e["ff_o"], e["fb_o"], e["o2_tab"], round(e["o2_calc"], 1), round(e["o2_diff"], 1)])

        def pack(x):
            ax = sorted(abs(v) for v in x)
            return [len(x), round(st.median(ax), 2), round(ax[int(0.9 * len(ax)) - 1], 2),
                    round(sum(1 for v in ax if v <= 0.31) / len(ax) * 100, 1),
                    round(sum(1 for v in ax if v <= 0.51) / len(ax) * 100, 1),
                    round(st.mean(x), 2)]

        pa, po = pack(da), pack(do)
        stats_rows.append([p + "℃", "空气点(20.9%)", *pa])
        stats_rows.append([p + "℃", "氧气点(94.6%)", *po])
        print(f"{p}℃ 空气点 n={pa[0]:<4} 中位|Δ|={pa[1]:.2f} P90={pa[2]:.2f} "
              f"|Δ|≤0.3 占 {pa[3]:.0f}%  |Δ|≤0.5 占 {pa[4]:.0f}%  平均Δ={pa[5]:+.2f}")
        print(f"{p}℃ 氧气点 n={po[0]:<4} 中位|Δ|={po[1]:.2f} P90={po[2]:.2f} "
              f"|Δ|≤0.3 占 {po[3]:.0f}%  |Δ|≤0.5 占 {po[4]:.0f}%  平均Δ={po[5]:+.2f}")

    # 4) 写 Excel
    wb = openpyxl.Workbook()
    ws = wb.active
    ws.title = "10台明细"
    write_sheet(
        ws,
        ["仪器编号", "板卡位置", "检验页", "温度(℃)", "空气点 表内(%)", "空气点 算法(%)", "Δ",
         "等效温度(℃)", "氧气点 表内(%)", "氧气点 算法(%)", "Δ", "判定(±0.5)"],
        detail,
        [19, 12, 8, 9, 13, 13, 7, 11, 13, 13, 7, 11],
        title_rows=("10 台传感器：Python 算法复算值 vs 检验报告记录值（Δ = 算法 - 表内）",
                    "标定数据来自 标定提取结果.xlsx；验证数据来自 20260920 7500H（982）V3.08.032架25.xlsx 的 20/30/45 页",
                    "等效温度 = 要让空气点算成表内值所需的温度；与“温度”列差得越多，说明该行温度记录越不准（空气点约 2 %O2/℃）",),
    )

    ws2 = wb.create_sheet("全量统计")
    write_sheet(
        ws2,
        ["检验页", "检验点", "样本数", "中位|Δ|", "P90|Δ|", "|Δ|≤0.3 占比%", "|Δ|≤0.5 占比%", "平均Δ"],
        stats_rows,
        [10, 16, 9, 10, 10, 14, 14, 9],
        title_rows=("三页全部“合格”记录的误差统计（单位：氧浓度百分点）",),
    )

    ws3 = wb.create_sheet("全量明细")
    write_sheet(
        ws3,
        ["检验页", "仪器编号", "板卡位置", "零点温度(℃)", "量程温度(℃)", "零点Ff", "零点Fb",
         "空气 表内", "空气 算法", "Δ", "量程Ff", "量程Fb", "氧气 表内", "氧气 算法", "Δ"],
        all_detail,
        [8, 19, 12, 12, 12, 9, 9, 9, 9, 7, 9, 9, 9, 9, 7],
        title_rows=("全量明细（仅合格记录）",),
    )

    wb.save(OUT_FILE)
    print()
    print("已保存：", OUT_FILE)

    make_plot(all_detail)

    n_ok = sum(1 for r in detail if r[11] == "一致")
    print(f"10 台 × 3 温度页 = {len(detail)} 条记录，其中 {n_ok} 条与表格一致（判定阈值 ±{TOL}%）")


def make_plot(all_detail) -> None:
    """画一张"算法算出来 vs 表格记录"的散点图，直观看出两条线是否压在对角线上。"""
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except Exception as exc:                                   # pragma: no cover
        print("（绘图跳过：%s）" % exc)
        return

    plt.rcParams["font.sans-serif"] = ["Microsoft YaHei", "SimHei", "DejaVu Sans"]
    plt.rcParams["axes.unicode_minus"] = False

    fig, axes = plt.subplots(1, 3, figsize=(13, 4.2))
    colors = {"20": "#2E86C1", "30": "#28B463", "45": "#E67E22"}
    for ax, (name, lo, hi, xi_tab, xi_calc) in zip(
        axes,
        [("空气点 (标准 20.9%)", 19.5, 22.5, 7, 8), ("氧气点 (标准 94.6%)", 93.0, 96.5, 12, 13)],
    ):
        for page in PAGES:
            pts = [(r[xi_tab], r[xi_calc]) for r in all_detail if r[0] == page]
            ax.scatter([p[0] for p in pts], [p[1] for p in pts], s=9, alpha=0.55,
                       label=f"{page}℃页", color=colors[page], linewidths=0)
        ax.plot([lo, hi], [lo, hi], "k--", lw=1, label="y = x")
        ax.set_xlim(lo, hi)
        ax.set_ylim(lo, hi)
        ax.set_xlabel("检验报告记录值 (%)")
        ax.set_ylabel("Python 算法复算值 (%)")
        ax.set_title(name, fontsize=11)
        ax.grid(alpha=0.25)
        ax.legend(fontsize=8, loc="upper left")

    # 第三张：误差直方图
    ax = axes[2]
    for page in PAGES:
        d = [r[9] for r in all_detail if r[0] == page]          # 空气点 Δ
        ax.hist(d, bins=[-3 + 0.2 * i for i in range(31)], alpha=0.55, label=f"{page}℃ 空气点", color=colors[page])
    ax.set_xlim(-3, 3)
    ax.set_xlabel("Δ = 算法 - 表内 (%)")
    ax.set_ylabel("台数")
    ax.set_title("空气点误差分布", fontsize=11)
    ax.grid(alpha=0.25)
    ax.legend(fontsize=8)

    fig.suptitle("7500HA-YY 氧气算法复算 vs 检验报告（20/30/45℃ 三页全部合格记录）", fontsize=12)
    fig.tight_layout(rect=(0, 0, 1, 0.94))
    out = os.path.join(HERE, "验证散点图.png")
    fig.savefig(out, dpi=160)
    print("已保存：", out)


if __name__ == "__main__":
    main()

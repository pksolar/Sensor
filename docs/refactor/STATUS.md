# 重构状态

最后更新：2026-11-03

## 当前状态

行为保持型重构的所有不依赖硬件的可执行项已完成。ARMCC 5.06u7 目标构建因本机工具链缺失未达 `BUILD_VERIFIED`；该限制已记录，未被写成通过。

## 已完成

- 外部备份、哈希清单、Git 基线和可恢复提交。
- 重构前架构、数据流、契约、历史问题及硬件检查表。
- 公共业务类型、测量算法、UART 编码/校验、Flash 布局/编码的模块边界。
- 主循环 20/100/500 ms 任务和 UART 后处理职责拆分。
- Keil 工程已包含 3 个新生产 `.c` 文件，无缺失或重复项。
- 61 项生产 C 特征断言、103 项静态契约、35 个 C 源文件补充语法检查通过。
- ARM Compiler 6 基线/当前可链接补充构建和资源比较完成。
- 代码批次提交：`9de65e3` (`refactor: establish behavior-preserving module boundaries`)。
- 已增加独立 Python 氧浓度模型和 10 传感器验证报告；未修改生产固件，也未实现流量计算。

## 未完成/外部阻塞

- ARMCC 5.06u7 未安装，因此无当前 ARMCC HEX/BIN、同编译器 ROM/RAM 差值或 ARMCC 关键函数反汇编。
- 目标板、传感器、逻辑分析仪和标准气源不可用，所有实物时序与精度项均为 `HARDWARE_VALIDATION_PENDING`。

## 证据等级

- 特征测试：`HOST_VERIFIED`
- 工程/配置/关键函数/厂商目录：`STATICALLY_REVIEWED`
- ARM Compiler 6 补充构建：主机工具链烟雾验证，不等于 `BUILD_VERIFIED`
- ARMCC 5 当前工程：未验证（工具缺失）
- 硬件行为：`HARDWARE_VALIDATION_PENDING`

## 氧浓度离线模型

- 交付：`tools/oxygen_calibration_model.py`、`docs/oxygen_calibration_validation_10_sensors.md`。
- 10 个传感器共 60 个空气/氧气点：严格 ±0.1 个百分点一致 43/60，显示/状态容差 ±0.5 内 57/60。
- 30 个氧气点：严格一致 28/30，±0.5 内 30/30。
- 等级：`HOST_VERIFIED`、`STATICALLY_REVIEWED`；实物输入历史复现仍为 `HARDWARE_VALIDATION_PENDING`。

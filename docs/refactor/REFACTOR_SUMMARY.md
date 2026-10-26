# 重构交付摘要

## 修改范围

修改了 Keil 工程、`main.c`、`ResultCount.c`、`SubFun.c`、`RXExecute.c`、`Linearization.c`、`flash.c`、`pwm.c` 及其相关自有头文件。CMSIS 和 `HK32F0xxA_StdPeriphDriver` 无修改，闭源库无修改。

新增生产文件：

- `user/inc/app_types.h`
- `user/inc/measurement_core.h`
- `user/src/measurement_core.c`
- `MyDriver/inc/protocol_codec.h`
- `MyDriver/src/protocol_codec.c`
- `MyDriver/inc/persistent_layout.h`
- `MyDriver/inc/persistent_codec.h`
- `MyDriver/src/persistent_codec.c`

新增测试/检查工具：`tests/host/*`、`tools/refactor/*`。新增本目录持久文档和根目录 `AGENTS.md`。没有删除生产源文件。

## 主要结果

- 公共类型不再由 `main.h`/`SubFun.h` 重复承载。
- 滤波、O2、流量、窗口和 PWM 算法拥有可测纯 C 核心。
- UART 的 checksum、错误应答和实时帧编码与命令执行分开。
- Flash 物理地址与字段偏移集中定义，物理擦写体保留。
- 主循环的测量、输出、上报和 UART 后处理拆分为静态职责函数。
- 原对外函数、必要全局符号、闭源库 ABI、引脚/中断/时序和历史异常行为保留。

## 验证摘要

- `HOST_VERIFIED`：61 项特征断言通过。
- `STATICALLY_REVIEWED`：103 项契约检查通过，35 个 Keil C 文件通过 ARM Compiler 6 语法检查。
- 补充链接构建：当前 ROM 24,452 B / 64 KiB，RAM 3,128 B / 10 KiB；相同补充编译器相对基线 ROM +336 B、RAM +0 B。
- ARMCC 5 构建：本机未安装所需编译器，退出 2，未达 `BUILD_VERIFIED`。
- 硬件时序、测量精度和产品级验证：`HARDWARE_VALIDATION_PENDING`。

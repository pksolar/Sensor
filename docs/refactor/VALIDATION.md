# 验证记录

## 等级定义

- `HOST_VERIFIED`：当前机器上的自动测试已执行通过。
- `BUILD_VERIFIED`：ARMCC 5 目标工程在当前源码上构建通过。
- `STATICALLY_REVIEWED`：通过源码、工程 XML、哈希、补充编译器或静态脚本检查。
- `HARDWARE_VALIDATION_PENDING`：需要目标板/仪器。

## 安全基线

- 完整备份：`D:\project\7500HA-YY_backup_20261026-203647`。
- 核对：271/271 文件，37,266,134/37,266,134 字节；关注文件 SHA-256 清单 156 条。
- Git 基线：`a846929c98653813d88705eca94e20e7eefdfb16`。
- 历史 ARMCC 5.06u7 日志：0 error、2 warning（`LQTP.c` 两个 set-but-unused）；ROM 25,040 B，RAM 3,704 B。这是历史产物，非本次构建。
- 等级：`STATICALLY_REVIEWED`。

## 主机特征测试

- 命令：`powershell -NoProfile -ExecutionPolicy Bypass -File tests/host/run_tests.ps1`
- 结果：`PASS: 61 characterization assertions`。
- 覆盖：UART checksum、正常/错误帧和实时数据逐字节编码；Flash int/long 小端序与默认恢复；中值/平均/去极值；窗口启动/稳定/异常；O2 标定点/限幅；流量严格分段边界；PWM 映射。
- 实现：受限 C99 AST 执行器直接运行 `measurement_core.c`、`protocol_codec.c`、`persistent_codec.c` 函数体；滤波器与 Git 基线函数动态对比。
- 等级：`HOST_VERIFIED`。

## 静态兼容契约

- 命令：`powershell -NoProfile -ExecutionPolicy Bypass -File tools/refactor/check_contracts.ps1`
- 结果：`PASS: 103 static contract checks`。
- 检查：26 个关键硬件函数源文本与基线相同；CMSIS/标准外设库无修改；MCU、IROM/IRAM、宏、工程成员、Flash 布局、SysTick 阈值、UART 常量不变；无动态内存和 RTOS。
- 等级：`STATICALLY_REVIEWED`。

## 全工程 C 语法检查

- 命令：`powershell -NoProfile -ExecutionPolicy Bypass -File tools/refactor/check_armclang_syntax.ps1`
- 工具：ARM Compiler 6.22，Cortex-M0/C99，工程宏和 include 路径与 Keil 配置一致。
- 结果：`PASS: 35 Keil C sources accepted`。
- 诊断处理：仅屏蔽 ARMCC5 专有 `#pragma import`、历史多余括号、空 while 和旧 printf 格式诊断。新纯模块在未屏蔽这些诊断时无警告。
- 等级：`STATICALLY_REVIEWED`，不是 ARMCC 5 `BUILD_VERIFIED`。

## ARM Compiler 6 补充链接/资源对比

两份源码均使用 ARM Compiler 6.22、`-Oz -fshort-enums -fshort-wchar`、原 Cortex-M0 scatter、MicroLIB 和原闭源库。基线来自外部备份，当前来自工作区。

| 项目 | 基线 | 当前 | 差值 |
|---|---:|---:|---:|
| Code | 23,240 B | 23,576 B | +336 B |
| RO-data | 860 B | 860 B | 0 B |
| RW-data | 16 B | 16 B | 0 B |
| ZI-data | 3,112 B | 3,112 B | 0 B |
| ROM (Code+RO+RW) | 24,116 B | 24,452 B | +336 B |
| RAM (RW+ZI) | 3,128 B | 3,128 B | 0 B |

- 当前 BIN SHA-256：`93F3A2456EE852095CA026727650F95132FDF80C9A15CEFF9A9745B7F9674C99`。
- 结论：补充构建下 ROM < 64 KiB，RAM < 10 KiB。该差值是同一补充编译器下的可比较数据，不可与历史 ARMCC 数值混用。
- 等级：`STATICALLY_REVIEWED`（补充构建证据）。

## 关键函数指令对比

- 方法：对上述基线/当前 ARM Compiler 6 对象用 `fromelf --text -c` 提取 26 个保护函数段字节。
- 结果：25/26 完全相同，包括 `StartCaiyangLeft/Right`、`TimerDelay12_5us`、`TIM14_IRQHandler`、`USART1_IRQHandler`、`WriteFlashAll`、GPIO/PWM/ADC/I2C/IWDG 关键函数。
- `SysTick_Handler`：前后均 136 B，计数、比较、分支及赋值序列一致；6 条 LDRB/STRB 的 TU 私有变量偏移因 `main.c` 数据布局变化而不同。源函数由静态契约确认逐字符相同。
- 等级：`STATICALLY_REVIEWED`；仍需 ARMCC 5 反汇编和硬件时序验证。

## 最终 ARMCC 5 构建尝试

- 命令：`D:\software\keil\UV4\UV4.exe -r D:\project\7500HA-YY\7500HA-YY.uvprojx -j0 -o D:\project\7500HA-YY_final_build_20261026.log`
- 结果：退出码 2；`Default Compiler Version 5 is not available`；未进入编译。
- 当前 ARMCC 错误/警告、HEX/BIN 哈希、同编译器 ROM/RAM 差值：无法产生。
- 等级：未达 `BUILD_VERIFIED`，`BLOCKED_TOOLING`。

## 其他环境限制

- FVP Cortex-M0 可执行文件存在，但实际启动因 Flex 许可证缺失失败，未作为测试通过证据。
- 无目标板和仪器；实物时序、精度、功耗、Flash 断电可靠性和真实 UART 时序均为 `HARDWARE_VALIDATION_PENDING`。

## 一键软件侧回归

- 命令：`powershell -NoProfile -ExecutionPolicy Bypass -File tools/refactor/run_validation.ps1`
- 最终结果：61 特征断言、103 静态契约、35 源文件语法检查和 `git diff --check` 全部通过。

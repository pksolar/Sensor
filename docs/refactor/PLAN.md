# 行为保持型重构计划

最后更新：2026-10-26。状态：软件侧可执行项已完成；ARMCC 5 验收受本机工具链限制。

## 阶段 0：安全基线 — DONE

- 在项目外创建完整备份和 SHA-256 清单。
- 初始化 Git，基线提交为 `a846929`。
- 保存历史 log/map/hex/bin 证据，实际尝试 ARMCC 5 构建。

## 阶段 1：理解和文档 — DONE

- 完成启动、调度、中断、超声波、测量算法、UART、Flash、全局状态和模块依赖梳理。
- 建立持久任务文档和 `AGENTS.md`。

## 阶段 2：行为特征测试 — DONE

- 建立受限 C99 AST 执行器，直接执行生产 `.c` 中的纯函数体。
- 61 项断言覆盖 UART 正常/错误帧、checksum、应答字节，Flash 小端序/默认值，排序/平均/去极值，窗口启动/稳定/异常，O2、流量和 PWM。
- 滤波函数还会动态读取 Git 基线函数并对比新实现。

## 阶段 3：基础结构 — DONE

- 新建 `app_types.h`，从 `main.h`/`SubFun.h` 解耦公共业务类型。
- 规范本次触及的自有头文件 guard，缩小 `RXExecute.h` 依赖。
- 新增状态优先放在模块私有范围；可能属于旧 ABI 的已有全局符号不冒然隐藏。

## 阶段 4：算法边界 — DONE

- 新建 `measurement_core` 集中纯算法，保持比较边界、求值顺序、整数除法、`+0.5` 转换和限幅。
- `ResultCount.c` 保留 O2/流量共享的异常波动历史。

## 阶段 5：UART 协议 — DONE

- `protocol_codec` 分离 checksum、长度错误应答、YY/MT 实时数据编码。
- 命令 1 和自动上报共用同一生产编码器；应答发送统一经 `SendResponse`。
- USART ISR 和 `RXExecute` 的历史缓冲区长度依赖保留。

## 阶段 6：Flash 配置 — DONE

- `persistent_layout` 集中物理页、镜像大小和字段偏移。
- `persistent_codec` 承载可测小端序和默认值规则。
- Flash 擦写/回读函数本体保持；为保持关键函数指令，`WriteEepromInt/Long` 的历史赋值顺序不替换为跨翻译单元调用。

## 阶段 7：应用状态和主循环 — DONE

- 抽取 20/100/500 ms 任务和 UART 后处理的静态函数。
- 保持主循环任务检查顺序、标志清零位置和单比特合并语义。
- 10 ms 超声波及 1 s 压力/I2C 路径原地保留。

## 阶段 8：硬件接口边界 — DONE

- 现有 `gpio/adc/pwm/usart/cap/IIC` 作为 Board/Platform 边界，不改寄存器和中断时序体。
- 103 项静态契约检查包含 26 个关键函数源文本、厂商目录、Keil 配置、初始化顺序、Flash/UART/周期常量。
- 同一 ARM Compiler 6 补充构建下，26 个关键函数中 25 个机器码完全相同；`SysTick_Handler` 长度与控制流相同，只有 TU 私有字节偏移改变。

## 阶段 9：最终验证 — DONE_WITH_TOOLING_LIMIT

- `run_validation.ps1`：61 主机断言、103 静态契约、35 个 Keil C 源文件 ARM Compiler 6 语法检查全部通过。
- 同一 ARM Compiler 6 `-Oz` 补充构建比较：ROM +336 B，RAM +0 B，当前 ROM 24,452 B，RAM 3,128 B。
- 最终 ARMCC 5 工程构建已重试，因本机未安装 Compiler 5 退出 2，未达 `BUILD_VERIFIED`。
- 硬件验收全部保留在 `HARDWARE_VALIDATION_CHECKLIST.md`。

## 验证命令

1. `powershell -ExecutionPolicy Bypass -File tools/refactor/run_validation.ps1`
2. `powershell -ExecutionPolicy Bypass -File tests/host/run_tests.ps1`
3. `powershell -ExecutionPolicy Bypass -File tools/refactor/check_contracts.ps1`
4. `powershell -ExecutionPolicy Bypass -File tools/refactor/check_armclang_syntax.ps1`
5. `D:\software\keil\UV4\UV4.exe -r D:\project\7500HA-YY\7500HA-YY.uvprojx -j0 -o <log>`

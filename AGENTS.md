# 7500HA-YY 行为保持型重构约束

本仓库是 HK32F030K6TxA（ARM Cortex-M0）固件，目标工程为 `7500HA-YY.uvprojx`，目标编译器为 ARMCC 5.06。当前已在外部建立完整备份，并在 Git 提交 `a846929` 保存重构前源码基线。

## 不可破坏的兼容契约

- 当前实际行为是兼容基准；重构不得夹带 bug 修复或功能升级。
- 保持启动文件、中断向量、IROM `0x08000000/0x10000`、IRAM `0x20000000/0x2800`、闭源库 ABI 和 Keil/ARMCC 5 兼容性。
- 保持 GPIO、电平极性、ADC、PWM、USART、TIM14、TIM16、软件 I2C、Flash 地址和初始化顺序。
- 保持超声波正反激励序列、延时、寄存器访问顺序、关/开中断顺序和忙等行为。
- 保持实际任务周期和调用顺序；名称或注释错误不得通过改行为“修正”。
- 保持公式求值顺序、整数除法、浮点转整数、舍入、阈值、滤波窗口、启动输出、跳变输出和限幅。
- 保持 UART 帧、命令号、字段顺序、长度、校验、错误响应、自动上报以及 ISR 组帧行为。
- 保持 Flash 页 30/31、偏移、字节序、轮换策略、默认恢复和现有设备标定数据兼容性。
- 不引入 RTOS、动态内存、递归、C++ 运行时、新固件依赖或 ARMCC 5 不支持的语言特性。
- 不修改 `CMSIS/` 和 `HK32F0xxA_StdPeriphDriver/`。

## 高风险保护区

以下函数原则上保持函数体原样；若无 ARMCC 5 汇编对比，不进行实质改写：

- `StartCaiyangLeft`、`StartCaiyangRight`、`TimerDelay12_5us`
- `TIM14_IRQHandler`、`TimerCountStart`、输入捕获配置
- `SysTick_Handler`、`USART1_IRQHandler`
- 软件 I2C 位操作和延时
- `WriteFlashAll` 的擦写、轮换与校验流程

## 工作方式与验证

- 每批只做一个逻辑主题；批后运行主机测试、静态契约检查，并尝试 Keil 构建。
- 失败时只撤销当前批，不扩大修改掩盖失败。
- 验证结论只使用 `HOST_VERIFIED`、`BUILD_VERIFIED`、`STATICALLY_REVIEWED`、`HARDWARE_VALIDATION_PENDING`。
- 当前机器缺少 ARMCC 5；µVision 5.40 的失败日志不能算构建通过。安装 ARMCC 5 后必须重新执行目标构建。
- 不把历史 `Objects/`、`Listings/` 产物冒充当前构建结果。
- 任何变更都要同步维护 `docs/refactor/PLAN.md`、`STATUS.md`、`DECISIONS.md` 和 `VALIDATION.md`。

## 已知但本次不修复

详见 `docs/refactor/KNOWN_BEHAVIOR.md`。特别包括 `Flag20ms` 实际 100 ms、未初始化的 `cReadTCount`、压力有效前可能除零、氧浓度与流量共享异常波动静态历史、GXHT40 路径禁用、水汽修正固定为零，以及所有疑似冗余的延时/volatile/寄存器直写/临界区。

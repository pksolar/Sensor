# 重构后架构

## 依赖方向

`main.c` 作为 Application 协调层，按原有顺序调用超声波采集、Measurement、Protocol、Persistent Configuration 和 Board/Platform。本次不移动硬件驱动文件，以避免 Keil 工程和时序风险。

| 边界 | 生产文件 | 职责 |
|---|---|---|
| Application | `user/src/main.c` | 原有初始化顺序、周期标志消费、报警/输出/上报协调 |
| Shared model | `user/inc/app_types.h` | 传感器值、Flash 配置、标定系数和结果类型 |
| Measurement | `measurement_core.*`, `ResultCount.c` | 纯滤波、O2、流量、窗口和 PWM 映射；历史状态保留在包装层 |
| Protocol | `protocol_codec.*`, `RXExecute.c`, `usart.c` | 纯校验/编码、命令执行、ISR 字节组帧 |
| Persistent config | `persistent_layout.h`, `persistent_codec.*`, `flash.c` | 地址契约、小端序/默认值、物理页擦写 |
| Calibration | `Linearization.c`, 闭源 `.lib` | 标定点读写与系数计算；ABI 不变 |
| Board/Platform | `gpio/adc/pwm/tim/cap/usart/IIC/iwdg` | 原有引脚、寄存器、中断、延时和低功耗实现 |

## 主循环

- 10 ms：保留原地的左/右激励和 Ff/Fb 原始窗口。
- 名义 20 ms（实际阈值 100 ms）：`ApplicationRunMeasurementTask`。
- 100 ms：`ApplicationRunOutputTask`。
- 500 ms：`ApplicationRunReportTask`。
- 1 s：保留原地的 NTC/压力/I2C 路径，包括未初始化计数器的历史行为。
- UART：`ApplicationProcessSerial`，保持标定后系数重算顺序。

## 兼容层

`ResultCount.c`、`RXExecute.c`、`flash.c` 的旧对外函数名和必要全局符号保留。新纯函数是可测内核，旧函数是 ABI/状态兼容包装层。硬件敏感函数未改写。

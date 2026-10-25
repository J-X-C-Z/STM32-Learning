# 第二次作业：外部中断控制 LED 状态机

## 功能

上电后 LED 熄灭。每按下一次按键，状态按以下顺序循环：

```text
熄灭 --按键中断--> 常亮 --按键中断--> 呼吸灯 --按键中断--> 熄灭
```

PA10 按键使用下降沿 EXTI，主循环读取中断事件并推进状态机；中断回调只做 200 ms 消抖和事件记录。呼吸灯由主循环软件 PWM 实现，亮度每 20 ms 变化一级。

## 引脚

- 红、绿、蓝 LED：PA7、PA5、PA3，低电平点亮。
- 按键：PA10，上拉输入，按下接地；PA9 配置为按键地端输出低电平。
- MCU：STM32F103C8T6，系统时钟使用内部 8 MHz HSI。

## 构建

使用 STM32CubeIDE 导入本目录，或在已安装 CMake、Ninja 和 `arm-none-eabi-gcc` 的环境中执行：

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

CubeMX 配置文件为 `Homework2.ioc`。

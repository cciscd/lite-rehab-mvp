# MVP components / MVP 元件表

## Required / 必需

| Qty. | Component / 元件 | Use / 用途 |
|---:|---|---|
| 1 | MYOSA ESP32 WROOM-32E motherboard / MYOSA 主板 | Wearable sensing and BLE / 佩戴端采集与 BLE |
| 1 | ESP32-S3-DevKitC-1 N16R8 | BLE receiver and USB gateway / BLE 接收与 USB 网关 |
| 1 | MYOSA MPU6050 board | Six-axis arm motion / 六轴手臂运动 |
| 1 | MYOSA SSD1306 OLED board | Local state and repetition display / 本地状态与次数显示 |
| 2 | Four-wire JST female-to-female cables / 四线 JST 母对母线 | MYOSA → MPU6050 → OLED cascade / 级联连接 |
| 1 | Small passive buzzer / 小型无源蜂鸣器 | Audio feedback / 声音反馈 |
| 1 | Ordinary LED / 普通 LED | Receiver status / 接收端状态 |
| 1 | 220–330 Ω resistor / 电阻 | LED current limiting / LED 限流 |
| 1 | 100–330 Ω resistor / 电阻 | Buzzer series protection / 蜂鸣器串联保护 |
| 1 | Breadboard / 面包板 | Receiver output wiring / 接收端输出接线 |
| Several | Jumper wires / 杜邦线 | LED and buzzer wiring / LED 与蜂鸣器接线 |
| 2 | USB data cables / USB 数据线 | Power, flashing, and serial / 供电、烧录和串口 |
| 1 | Elastic strap or wrist support / 弹性绑带或护腕 | Fix MPU6050 to forearm / 固定 MPU6050 |
| 1 | Laptop with webcam / 带摄像头笔记本 | Vision and Python application / 视觉与 Python 程序 |

## Recommended but not required / 推荐但非必需

| Component / 元件 | Use / 用途 |
|---|---|
| Small USB power bank / 小型移动电源 | Wireless power for the MYOSA wearable / 为佩戴端独立供电 |
| NPN transistor (S8050 or 2N2222) + 1 kΩ resistor | Drive a buzzer whose current is unknown / 驱动电流未知的蜂鸣器 |
| Rigid plastic plate or small enclosure / 塑料固定板或小外壳 | Prevent cable movement / 防止线缆晃动 |

BMP180 and APDS9960 are supplied in the kit but are not used in the MVP. / 套件中的 BMP180 和 APDS9960 不用于 MVP。

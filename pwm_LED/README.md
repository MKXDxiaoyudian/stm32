# pwm_LED —— 旋转编码器调 PWM 亮度（STM32F103C8T6）

用 **EC11 旋转编码器**调节 **TIM2 的 PWM 占空比**，控制 LED 亮度。

这是本合集里第一个用上**定时器高级功能**的工程：TIM3 走**硬件编码器接口模式**（不是 GPIO 轮询），
TIM2 走 **PWM 输出**。全部配置都在 `.ioc` 里登记过，`main.c` 里只剩逻辑代码。

当前状态：**🔄 转编码器 → LED 亮度跟手；上电 50%；全程 20 档。**

---

## 硬件

| 件 | 型号 / 说明 |
|---|---|
| MCU 板 | STM32F103C8T6（Blue Pill，**64 KB Flash / 20 KB RAM**） |
| 调试器 | DAP 仿真器（CMSIS-DAP，VID:PID `0483:572a`） |
| 编码器 | **EC11 型旋转编码器模块，无按压开关**（5 个引脚：一侧 `VCC`/`GND`，另一侧 `A`/`C`/`B`） |
| LED | 普通 5 mm LED + 限流电阻（220 Ω ~ 1 kΩ），**高电平点亮** |
| 预留 | 0.96" OLED（SSD1306，I2C，地址一般 `0x3C`）—— I2C1 已配好，代码待写 |

## 接线

| 模块 | 引脚 | 说明 |
|---|---|---|
| 编码器 `VCC` | **3.3 V** | ⚠️ **绝对不能接 5 V** —— 它给模块自带的上拉供电，接 5 V 会让 `A`/`B` 空闲时输出 5 V，而 **PA6/PA7 在 STM32F103 上是 TTa 结构，不耐 5 V**，会打穿引脚 |
| 编码器 `GND` | GND | |
| 编码器 `A` | **PA6**（TIM3_CH1） | 正交 A 相 |
| 编码器 `B` | **PA7**（TIM3_CH2） | 正交 B 相 |
| 编码器 `C` | **GND** ⭐ | **公共端，必须接地**。模块内部不一定已经接好，漏了这根线的话 `A`/`B` 永远是断开的，一点信号都没有 |
| LED 正极（经限流电阻） | **PA1**（TIM2_CH2） | **高电平点亮** |
| LED 负极 | GND | |
| 按键 1 | **PB1** | 上拉输入，按下接 GND |
| 按键 2 | **PB11** | 同上 |
| OLED `SCL` | **PB8**（I2C1_SCL） | 走 AFIO 重映射 |
| OLED `SDA` | **PB9**（I2C1_SDA） | |
| DAP `SWDIO` / `SWCLK` | **PA13 / PA14** | 加 GND 与 3.3 V |
| DAP `nRESET` | *(未接)* | 所以 `openocd.cfg` 里必须是 `reset_config none` |

> **接线自查**：万用表直流电压档，黑表笔接 GND，红表笔量 **PA6**，慢慢转旋钮 ——
> 应该看到电压在 **0 V ↔ 3.3 V 之间跳变**。一直不动就是 `C` 没接 GND、或者模块 `VCC` 没接。

## 外设配置（`.ioc` 里登记的）

| 外设 | 引脚 | 关键参数 | 换算 |
|---|---|---|---|
| **TIM2** | PA1 = CH2 | Prescaler = **7**，Counter Period = **999**，PWM mode 1，CH Polarity = High | `8 MHz ÷ 8 ÷ 1000` = **1 kHz** |
| **TIM3** | PA6/PA7 = CH1/CH2 | **Combined Channels = Encoder Mode**，Encoder Mode = **TI1**，Prescaler = 0，Period = 65535，Input Filter = **10** | 一格 = **2 个计数** |
| **I2C1** | PB8/PB9（重映射）| Clock Speed = 100 kHz，Standard Mode | 留给 OLED |
| GPIO | PB1 / PB11 | 输入 + **上拉** | 按键 |
| SYS | PA13/PA14 | Serial Wire | ⚠️ 一定要选 SWD，选错会锁死芯片 |

## 亮度分级（`Core/Inc/main.h` 里的宏）

| 宏 | 值 | 含义 |
|---|---|---|
| `BRIGHT_MAX` | `1000` | 亮度上限，正好对应 100% 占空比（`CCR = 1000 > ARR = 999` → 恒高） |
| `BRIGHT_STEP` | `50` | 每转一格改变 **5%** |
| `ENC_COUNTS_PER_DETENT` | `2` | 编码器一格产生几个硬件计数 |

- 上电默认 **50%**（`s_bright = 500`）
- 从全灭到全亮 **20 格**（往上 10 格到顶、往下 10 格到底）
- 到顶/到底自动 clamp，不会绕回

> ⚠️ **`ENC_COUNTS_PER_DETENT` 必须和 CubeMX 里的 Encoder Mode 配套**：
>
> | Encoder Mode | 一格计数 | `ENC_COUNTS_PER_DETENT` |
> |---|---|---|
> | `TI1`（本工程）| 2 | **2** |
> | `TI1 and TI2` | 4 | **4** |
>
> 改了模式不改这个宏 → 要么一格跳两档，要么两格才动一档。

## 构建 / 烧录

和本合集其它工程一样，全程在 VS Code 里做（打开**本工程目录**，不是仓库根目录）：

| 操作 | 怎么按 |
|---|---|
| 编译 Release | `⌘⇧B` |
| 编译 + 烧录 | `⇧⌘P → Tasks: Run Task → DAPLink Flash` |
| 调试 | `F5`（自动先编 Debug） |

命令行等价：

```bash
cd ~/Documents/stm32/pwm_LED
cmake --preset Release && cmake --build build/Release   # 产物 build/Release/pwm_LED.elf
openocd -f openocd.cfg -c "program build/Release/pwm_LED.elf verify reset exit"
```

## 代码结构（`USER CODE` 区块）

| 位置 | 内容 |
|---|---|
| `main.h` → `USER CODE BEGIN EM` | 亮度/PWM/编码器参数宏 |
| `main.c` → `USER CODE BEGIN PV` | `s_bright`（当前亮度）、`s_enc_last`（上次计数）、`s_enc_acc`（累计器） |
| `main.c` → `USER CODE BEGIN 2` | `HAL_TIM_PWM_Start()` + `HAL_TIM_Encoder_Start()` |
| `main.c` → `while(1)` 里（`USER CODE END WHILE` 与 `USER CODE BEGIN 3` 之间）| 读 `TIM3->CNT` → 累加 → 满一格改一次亮度 |

**核心思想**：编码器模式和计数**全在硬件里做**，软件只读 `TIM3->CNT` 的增量。
`delta` 先累加进 `s_enc_acc`，**攒够整格才动作一次**，这样无论主循环跑多快、你转得多快，
**一格永远等于一档**。

---

## 踩坑记录（这一节的每一条都是真踩过的）

### 1. CubeMX 只拷「用到的」驱动，光指派引脚不够

`.ioc` 里 `ProjectManager.LibraryCopy=1` 时，CubeMX **只把 `.ioc` 里正式启用的外设**对应的 HAL 驱动拷进 `Drivers/`。

只在 Pinout 图上点引脚选信号、**没有进 `Categories → Timers → TIMx` 设置 Mode**，会得到一堆诡异现象：

- `HAL_TIM_MODULE_ENABLED` 仍是注释状态
- `Drivers/STM32F1xx_HAL_Driver/Src/` 里**根本没有** `stm32f1xx_hal_tim.c`
- `cmake/stm32cubemx/CMakeLists.txt` 里没有 tim 源文件
- 但 `MX_GPIO_Init()` **已经**把引脚配成了 `GPIO_MODE_AF_PP` → **"悬空 AF"**：引脚进了复用模式，却没有外设在驱动它

**判定方法**（生成后一定要跑）：

```bash
grep -n "HAL_TIM_MODULE_ENABLED" Core/Inc/stm32f1xx_hal_conf.h    # 不能是注释
ls Drivers/STM32F1xx_HAL_Driver/Src/ | grep tim                    # 要有 stm32f1xx_hal_tim.c
grep -in tim cmake/stm32cubemx/CMakeLists.txt                      # 要编进去
grep -n "MX_TIM2_Init\|MX_TIM3_Init" Core/Src/main.c               # 要生成
```

### 2. 启动代码必须放 `USER CODE BEGIN 2`，不能放 `USER CODE BEGIN Init`

`USER CODE BEGIN Init` 在 **`HAL_Init()` 之后、`SystemClock_Config()` 和所有 `MX_xxx_Init()` 之前**。

在那儿调 `HAL_TIM_PWM_Start(&htim2, ...)` 时：

| 依赖 | 当时状态 |
|---|---|
| `htim2.Instance` | `NULL`（全局变量零初始化）|
| `htim2.State` | `HAL_TIM_STATE_RESET` |
| TIM2 的 APB1 时钟 | **没开**（在 `MX_TIM2_Init` 里才开）|
| PA1 的 AF_PP 复用 | **还没配** |
| PSC / ARR | 还是 0 |

结果 `HAL_TIM_PWM_Start` 直接返回 `HAL_ERROR`，**PWM 从来没启动过**，而且不报错、不警告。

**正确位置是 `USER CODE BEGIN 2`** —— 它在所有 `MX_xxx_Init()` **之后**。

### 3. 定时器做编码器要选 `Encoder Mode`，不是 `Output Compare`

在 `Timers → TIM3` 里，**`Combined Channels` 下拉选 `Encoder Mode`**。
如果把 `Channel1`/`Channel2` 选成 `Output Compare`，CubeMX 会生成 `HAL_TIM_OC_Init()`（**输出**模式），
此时调 `HAL_TIM_Encoder_Start()` 会把 CC1E/CC2E 打开，**让 PA6/PA7 变成输出**，
和编码器对推 —— 读数全乱，还可能伤引脚。

**判定**：

```bash
grep -n "HAL_TIM_Encoder_Init\|HAL_TIM_OC_Init" Core/Src/main.c
# 要看到 HAL_TIM_Encoder_Init，不能看到 HAL_TIM_OC_Init
```

### 4. 引脚必须和定时器通道对得上

| 通道 | 默认引脚 | | 通道 | 默认引脚 |
|---|---|---|---|---|
| TIM2_CH1 | **PA0** | | TIM3_CH1 | **PA6** |
| TIM2_CH2 | **PA1** | | TIM3_CH2 | **PA7** |
| TIM2_CH3 | PA2 | | TIM3_CH3 | PB0 |
| TIM2_CH4 | PA3 | | TIM3_CH4 | PB1 |

配 CubeMX 时**先想清楚 LED 接哪个脚**，再去选对应的通道。选错了不会报错，就是灯不亮。

### 5. 一格跳两档 —— 累加器方案（本项目最值得记的一条）

最初的写法是：

```c
if (delta != 0) { s_bright += BRIGHT_STEP; }   /* ❌ 只看符号，不看大小 */
```

**症状**：一格实际改变了 **100**（两倍），从 50% 到顶只有 5 步。

**原因**：`TI1` 模式下转一格产生 **2 个计数**，而主循环里没有任何延时、跑得极快，
这 2 个计数被**分两次循环**读到，于是加了两次。

**更糟的是**：加几次**取决于主循环跑多快** —— 加个 `HAL_Delay(1)` 或别的任务，手感就会变，极难排查。

**修法**（现在代码里的做法）：把 `delta` 累加进 `s_enc_acc`，用 `while` 攒够整格才动作一次：

```c
s_enc_acc += delta;
while (s_enc_acc >= (int32_t)ENC_COUNTS_PER_DETENT) { ... }   /* 顺时针 */
while (s_enc_acc <= -(int32_t)ENC_COUNTS_PER_DETENT) { ... }  /* 逆时针 */
```

用 `while` 而不是 `if`：万一某次 `delta` 一次就是好几格（主循环被卡过、或者手转得飞快），
`while` 能全部补上，**不丢步**。

### 6. `Input Filter` 一定要开

机械编码器的触点在每次通断时都会抖动，抖出一个多余计数、攒够 2 个就**误跳一档**。
CubeMX 里把 `Channel1/Channel2 Input Filter` 设成 **10**，让硬件去抖。

### 7. 从别的工程抄 `.vscode/` 时，务必改 `.elf` 名字

`tasks.json` 的 `DAPLink Flash` 任务里写死了 `program .../<工程名>.elf`。
抄过来不改，烧录会报找不到文件。**`tasks.json` 和 `launch.json` 两个都要查**：

```bash
grep -rn "\.elf" .vscode/
```

### 8. `USER CODE END WHILE` 和 `USER CODE BEGIN 3` 之间不保证保留

本工程的 `while` 循环代码就放在那个位置，**重新生成过 PSC/ARR 后活下来了**，
但曾经有过一次同样的位置被清空的经历（原因未查明）。
**更保险的位置是 `USER CODE BEGIN 3` 和 `USER CODE END 3` 之间**（成对的保护标记）。
无论放哪儿，重新生成后都跑一下：

```bash
grep -n "s_enc_acc" Core/Src/main.c    # 循环里的代码还在不在
```

---

## 已知问题 / TODO

- [ ] **`PA2` 还配着 `GPIO_Output`**，但没接任何东西（从别的工程抄配置时的残留）。清掉的话在 CubeMX 里把它设成 `Reset_State`。
- [ ] **`main.h` 里有几个未被使用的残留宏**：`LED_INIT` / `LED_ON` / `LED_OFF` / `LED_SWITCH`（写 `GPIOA` 的，但 PA1 现在归 TIM2 管，写 ODR 无效）、`PWM_ARR`（ARR 实际由 CubeMX 生成）、`KEY_IsPressed`。可以删掉，也可以留着以后接普通 LED 用。
- [ ] **`main.c:49` 的注释 `/* 上电微亮 */` 已过时** —— 现在的 `ARR=999`，`s_bright=500` 就是正正经经的 **50%** 亮度（那是按早期 `ARR=65535` 算的）。
- [ ] **系统时钟还是 HSI 8 MHz、PLL 没开**。`HAL_Delay` 时基和 I2C 时序都按 SYSCLK 算，上 I2C 屏之前建议先在 Clock Configuration 里配成 **HSE + PLL ×9 = 72 MHz，APB1 /2**。
- [ ] **编码器没有按压开关**（这版 EC11 是无开关型）。以后换成带开关的型号，把开关一脚接 GND、另一脚接任意空闲 GPIO（记得开上拉），再复用 `button_LED` 里那套消抖逻辑。
- [ ] **长时间阻塞后，亮度变化会"一次性生效"（视觉上是跳变）**。这**不是丢步** —— 硬件计数器一直在加，`s_enc_acc` 的 `while` 循环会把积压的档位**全部补上**，一格都不会少。
      只是如果主循环被卡了 200 ms、期间你转了 5 格，回到循环时会一次性加 5 档。
      想要变化更平滑，就把 `s_enc_acc` 的累加搬进 **1 ms 定时器中断**。
- [ ] **I2C 屏的代码还没写**（I2C1 已经在 `.ioc` 里配好，100 kHz）。

## 目录结构（CubeMX 标准布局）

```
Core/          # main.c / main.h / stm32f1xx_it.c / stm32f1xx_hal_msp.c
Drivers/       # HAL + CMSIS（4.5 MB，保留在仓库里便于离线自包含构建）
cmake/         # 工具链与 CubeMX 的 CMake 胶水
.vscode/       # Build / DAPLink Flash / cortex-debug 配置
pwm_LED.ioc    # CubeMX 工程文件，改配置后要重新 GENERATE CODE
```

`build/` 是编译产物，已在 `.gitignore` 里忽略。

> ⚠️ **只改 `USER CODE` 区块** —— 区块外的代码会在 CubeMX 重新生成时被覆盖。

## 提交与推送（git）

**在仓库根目录做，不要在本工程目录里做**：

```bash
cd ~/Documents/stm32        # ← 仓库根，不是 pwm_LED/
git add pwm_LED             # 只交本工程
git commit -m "pwm_LED: …"
git push origin main        # 推 GitHub
git push backup main        # 推本地裸仓库备份
```

详见仓库根目录的 [`../README.md`](../README.md) 和 [`../新建工程指南.md`](../新建工程指南.md)。

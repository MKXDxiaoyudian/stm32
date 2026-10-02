# STM32F103C8T6 bring-up（CubeMX + CMake/Ninja + DAP）

一个从零搭起来的 STM32F103C8T6（Blue Pill）工程：**CubeMX 生成 + CMake/Ninja 构建 + DAP（CMSIS-DAP）烧录**，
全部在 VS Code 里完成（写代码、编译、烧录、调试、看串口）。

当前状态：**能编译、能烧录、板载 LED 会亮**（PC13，低电平点亮）。
下一步：接 MPU6050，用 I2C 读 `WHO_AM_I` → 读六轴原始数据 → 定时器中断采样。

---

## 硬件

| 件 | 型号 / 说明 |
|---|---|
| MCU 板 | STM32F103C8T6（Blue Pill，**64 KB Flash / 20 KB RAM**，8 MHz 晶振） |
| 调试器 | DAP 仿真器（CMSIS-DAP，VID:PID `0483:572a`） |
| 传感器 | MPU6050（GY-521，I2C 地址 `0x68`） |
| 显示 | 0.96" OLED（SSD1306，I2C 地址一般 `0x3C`） |
| 其它 | 面包板、按键、蜂鸣器、舵机、电位器、光敏/热敏、红外 |

## 接线

| 模块 | 引脚 | 说明 |
|---|---|---|
| MPU6050 `SCL` | **PB6** | I2C1_SCL（3.3 V，**不要接 5 V**） |
| MPU6050 `SDA` | **PB7** | I2C1_SDA |
| OLED `SCL` / `SDA` | **PB6 / PB7** | 与 MPU6050 共用同一条 I2C1 |
| DAP `SWDIO` | **PA13** | 加 GND 与 3.3 V |
| DAP `SWCLK` | **PA14** | |
| 板载 LED | **PC13** | **低电平点亮**（板子自带，无需外接） |

> GY-521 与 OLED 模块板上自带 4.7 k 上拉，不用另加电阻。

## 构建 / 烧录（VS Code，推荐）

工程里已经配好 `.vscode/tasks.json`，四个任务：

| 任务 | 作用 |
|---|---|
| `Build` | 编译 Release（默认构建，`⌘⇧B`） |
| `Build Debug` | 编译 Debug（F5 调试前会自动跑） |
| `DAPLink Flash` | ⭐ **先编译、再用 DAP 写入**（一键） |
| `DAPLink 探测（只连不烧）` | 只连接芯片，确认调试器和线都没问题 |

- 烧录：`⇧⌘P` → `Tasks: Run Task` → `DAPLink Flash`；成功时最后会打印 `** Verified OK **` 与 `** Resetting **`。
- 调试：按 `F5`（`launch.json` 用 cortex-debug + openocd，`runToEntryPoint: main`）。
- 看串口：`⇧⌘P` → `Serial Monitor`（115200-8-N-1）。
- 看寄存器：调试时的 **Peripherals** 视图。

## 构建 / 烧录（命令行等价）

```bash
# 编译（需要 arm-none-eabi-gcc 在 PATH，或直接用 VS Code 任务）
cmake --build build/Release

# 烧录（openocd 0.12 + CMSIS-DAP）
openocd -f interface/cmsis-dap.cfg -f target/stm32f1x.cfg \
        -c "program build/Release/new.elf verify reset exit"

# 只连不烧（注意：exit 必须在 init 之后，单写 -c "exit" 一定报错）
openocd -f interface/cmsis-dap.cfg -f target/stm32f1x.cfg -c "init; exit"
```

## 工具链（本工程实际使用的版本）

- **STM32CubeMX**（生成初始化代码，改配置后要重新 GENERATE CODE）
- **arm-none-eabi-gcc**（STM32CubeCLT 自带）
- **CMake + Ninja**
- **openocd 0.12**（Homebrew）＋ `interface/cmsis-dap.cfg` / `target/stm32f1x.cfg`
- **VS Code**：STM32Cube for VS Code 扩展、cortex-debug、C/C++、Serial Monitor

> ⚠️ **只改 `USER CODE` 区块** —— 区块外的代码会在 CubeMX 重新生成时被覆盖。

## 已知问题 / TODO

- [ ] **系统时钟还是 HSI 8 MHz**（`PLLState = RCC_PLL_NONE`、`SYSCLKSource = HSI`、APB1/APB2 都是 `DIV1`）。
      计划改成 **HSE + PLL ×9 = 72 MHz、APB1 /2**；否则定时器算出来的频率和串口波特率都不对。
- [ ] **`printf` 打不出小数**：工程用 `--specs=nano.specs`（newlib-nano），`%f` 默认被裁掉（`_printf_float` 是弱符号）。
      需要加链接选项 `-u _printf_float`，或在 CubeMX 里勾 "Use float with printf from newlib-nano"。
- [ ] **栈只有 1 KB**（链接脚本 `_Min_Stack_Size = 0x400`）→ 中断里的缓冲区要写成 `static`。
- [ ] I2C / MPU6050 的代码还没进仓库（下一步）。
- [ ] 无电机驱动模块，**不接电机**（会烧 IO）。

## 目录结构（CubeMX 标准布局）

```
Core/          # 应用代码（main.c / gpio.c / stm32f1xx_it.c …）
Drivers/       # HAL + CMSIS（保留在仓库里，便于离线自包含构建）
cmake/         # 工具链与 CubeMX 的 CMake 胶水
.vscode/       # 任务与调试配置（Build / DAPLink Flash / cortex-debug）
```

`build/` 是编译产物，已在 `.gitignore` 里忽略。

## 提交与推送（git）

> ⚠️ **2026-10-02 变更**：本工程不再是独立仓库，已经并入 `~/Documents/stm32` 这个「合集仓库」
> （和 `button_LED` / `firstdemo` / `firstdemo2` 放在一起）。原来的独立历史仍保存在
> GitHub 的 `MKXDxiaoyudian/stm32f103-bringup` 和本地 `~/stm32-backup.git` 里。

**git 操作在仓库根目录做，不要在 `new/` 里做**：

```bash
cd ~/Documents/stm32        # ← 仓库根，不是 new/

git status --short          # 1. 看整个仓库改了哪些文件
git add -A                  # 2. 暂存（只想交本工程的改动：git add new/）
git commit -m "点灯：…"      # 3. 提交
git push origin main        # 4a. 推 GitHub（MKXDxiaoyudian/stm32）
git push backup main        # 4b. 推本地裸仓库备份
```

几个注意点：

- **新机器先验权**：`ssh -T git@github.com`，看到 `Hi MKXDxiaoyudian!` 才说明 key 配好了。
- **`git status` 里的 `??` 是未跟踪文件**，`git add -A` 会一起收进去 —— 加之前扫一眼，别把本机绝对路径或烧录日志传上去。
- **`.gitignore` 已经挡掉** `build/`、`mx.scratch`、`.DS_Store`、`CMakeUserPresets.json`，所以 CubeMX 重新 GENERATE CODE 后不会污染仓库；新建工程记得抄一份。
- **提交前先编译一次**：`cmake --build build/Release` 通过再提交，避免把编不过的代码推上去。
- **推错了想撤**：`git reset --soft HEAD~1` 撤回提交但保留改动，改完重新提交；已经推上去的话再 `git push --force-with-lease origin main`（只在自己一个人的分支上用）。
- **`Drivers/` 是要进仓库的**，别一时手快把它 ignore 掉 —— 留着才能离线自包含构建。
  本工程的 `Drivers/` 是 **3.6 MB**，这是正常值（CubeMX 勾了「Copy only the necessary library files」的结果）。
  如果哪天膨胀到几十 MB，说明 Code Generator 被改成了 copy-all，改回 **Copy only the necessary library files**
  再 GENERATE（`button_LED` 就踩过这个坑：67 MB → 瘦身到 3.5 MB）。

# stm32 练手工程合集（STM32F103C8T6）

一个仓库放多个 CubeMX + CMake/Ninja 工程。芯片统一是 **STM32F103C8T6**（Blue Pill，64 KB Flash / 20 KB RAM），
调试器统一是 **DAP（CMSIS-DAP，VID:PID `0483:572a`）**，只走 SWD。

## 工程一览

| 目录 | 内容 | 状态 |
|---|---|---|
| `new/` | 点灯：PC13 + PA0，`HAL_GPIO_TogglePin` + `HAL_Delay(500)` | ✅ 编译 / 烧录 / 点亮全部验证过；工程自带 `README.md`、`.vscode`、`openocd.cfg`，是最完整的一份 |
| `button_LED/` | 按键控制 LED（`button_LED.ioc`） | 🚧 只把 `new` 的点灯代码搬了过来，按键逻辑还没写；缺 `.clangd` 和 `.gitignore` |
| `firstdemo/` | 最早期的练手工程 | 📦 归档，能编译，没配 VS Code 任务 |
| `firstdemo2/` | 早期练手（`Inc/`+`Src/` 布局，**没有 `.ioc`**，不是 CubeMX 生成的） | 📦 归档 |

`new/README.md` 里有完整的接线表、构建/烧录命令、工具链版本和已知问题，新工程照着它抄。

## 每个工程怎么构建

```bash
cd <工程目录>
cmake --preset Debug         # 首次必须先 configure，否则 build/ 不存在
cmake --preset Release
cmake --build build/Release  # 产物：build/Release/<工程名>.elf
```

VS Code 里打开**单个工程目录**（不是这个仓库根目录），然后：`⌘⇧B` 编译、`⇧⌘P → Tasks: Run Task → DAPLink Flash` 烧录、`F5` 调试。

## 新增一个工程时

📖 **完整步骤看 [`新建工程指南.md`](新建工程指南.md)** —— 那里把 CubeMX 生成的文件分成三类（每次重写的 / 只生成一次的 / 完全不生成的），
逐条讲清哪些要抄、哪些千万别抄。这里只放最要命的几条：

1. CubeMX 新建工程，**Toolchain/IDE 必须选 `CMake`**，Project Name 就是工程名（它决定 `.elf` 的名字）。
2. 生成后**第一件事**：`grep -n CMAKE_PROJECT_NAME CMakeLists.txt`，确认是新工程名。
   这个文件 CubeMX 只在**第一次**生成，之后 `GENERATE CODE` 永不覆盖 —— 抄错只能手改。
3. **不要**复制 `.git/`、`.settings/`、`build/`、`CMakeLists.txt`。
4. 从 `new/` 抄这些（CubeMX 完全不生成）：`.vscode/`、`.clangd`、`.gitignore`、`openocd.cfg`，
   然后 `grep -rn "\.elf" .vscode/` 把旧工程名的 `.elf` 全换掉（`tasks.json` 和 `launch.json` **两个都要改**）。
5. Code Generator 里保持 **`Copy only the necessary library files`**（`.ioc` 中 `ProjectManager.LibraryCopy=1`），
   选成 `0` 会让 `Drivers/` 从 3.6 MB 膨胀到 **67 MB**。
6. SYS → Debug 选 **Serial Wire**（默认 `No Debug` 时 PA13/PA14 不登记，误配成 GPIO 就再也连不上调试器）。

## 注意点

- **`.elf` 名字由 `CMakeLists.txt` 的 `set(CMAKE_PROJECT_NAME ...)` 决定**，改完名字记得删掉 `build/` 重新 configure ——
  旧的 `CMakeCache.txt` 会把老名字缓存住。
- **`build/` 不进仓库**（根 `.gitignore` 已挡），`Drivers/` 是**要进仓库**的，这样 clone 下来不用装 CubeMX 就能编译。
- 每个工程需要**自己的** `.clangd`（内容是 `CompilationDatabase: build/Release`，路径相对工程目录）。
  放在仓库根目录没用，反而会让子工程的代码补全失效。
- 引脚目前**基本都是手写在 `main.c` 的 `USER CODE` 区块里**（PC13 / PA0 没在 `.ioc` 里登记）。
  以后改引脚建议先在 CubeMX 的 Pinout 里配好再生成，比手写寄存器/HAL 初始化靠谱。
- **系统时钟还是 HSI 8 MHz、PLL 没开**：`HAL_Delay` 的时基、I2C 时序、串口波特率都按 SYSCLK 算，
  上 I2C/串口之前要先在 Clock Configuration 里配成 **HSE + PLL ×9 = 72 MHz，APB1 /2**。

## 关于 git 历史

`new/` 原本是一个独立仓库，推在 GitHub 的
[`MKXDxiaoyudian/stm32f103-bringup`](https://github.com/MKXDxiaoyudian/stm32f103-bringup)，
本地备份在 `~/stm32-backup.git`。改成这个合集仓库后，它那 4 条历史提交不再挂在本地了，但两个远程都还留着：

```
4f05e6d 恢复可用的 STM32F103C8T6 点灯工程（CubeMX + CMake/Ninja + DAP 烧录）
5077345 docs: 加 README（接线表 / 构建 / 烧录 / 调试 / 工具链 / 已知问题）
652c9d3 点灯：增加 PA0 翻转电平，PC13 改为 Toggle + HAL_Delay(500)
da55fbf docs: README 补「提交与推送」一节（双远程 / 三步流程 / 注意点）
```

需要的时候 `git -C ~/Documents/stm32/new remote add old git@github.com:MKXDxiaoyudian/stm32f103-bringup.git` 就能翻回来。

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

1. CubeMX 新建工程，**Toolchain/IDE 选 `CMake`**，Project Name 就是工程名（它决定 `.elf` 的名字）。
2. 生成后把这三样从 `new/` 抄进新工程（CubeMX 不生成它们）：
   `.vscode/`、`.clangd`、`.gitignore`、`openocd.cfg`。
3. **不要**复制 `.git/` 和 `.settings/`：前者会让新工程挂到旧仓库的历史上，后者是本机 STM32Cube 扩展的状态文件。
4. **不要**直接复制 `CMakeLists.txt` —— 它里面 `set(CMAKE_PROJECT_NAME <名字>)` 写死了工程名，抄过来 `.elf` 就是别人的名字。
   而且 CubeMX 只在**第一次**生成这个文件，之后 `GENERATE CODE` 不会覆盖它，改错了只能手改。
5. 抄完 `.vscode/` 后，把 `tasks.json` 和 `launch.json` 里所有 `<旧工程名>.elf` 换成新工程名（**两个文件都要改**，
   `tasks.json` 那行漏改的话 `DAPLink Flash` 会报找不到文件）。
6. `LibraryCopy` 建议保持 **`Copy only the necessary library files`**（`.ioc` 里 `ProjectManager.LibraryCopy=1`）。
   选成 `0`（copy all）会让 `Drivers/` 从 3.6 MB 膨胀到 **67 MB**。

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

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

**推荐：全程在 VS Code 里做，不用开终端。** 打开**单个工程目录**（不是这个仓库根目录），然后：

| 操作 | 怎么按 |
|---|---|
| 编译 Release | `⌘⇧B`（任务 `Build`，已经自动带 configure） |
| 编译 + 烧录 | `⇧⌘P → Tasks: Run Task → DAPLink Flash` |
| 调试 | `F5`（自动先编 Debug） |
| 只探测芯片连没连上 | `Tasks: Run Task → DAPLink 探测（只连不烧）` |
| 换 preset / 单独 configure | 底部状态栏点 CMake Tools 的 preset（`Debug` / `Release`） |
| CMake 缓存出邪门问题 | `Tasks: Run Task → 从零重建（删 build 重配）` |

2026-10-02 起 `Build` 任务会**先自动 Configure 再 Build**，所以新 clone 下来（还没有 `build/` 目录）直接 `⌘⇧B` 就能过，
不需要先回终端敲一遍 configure。

命令行等价（CI 或不想开 VS Code 时用）：

```bash
cd <工程目录>
cmake --preset Release       # configure（首次必须，否则 build/ 不存在）
cmake --build build/Release  # 产物：build/Release/<工程名>.elf
cmake --preset Debug && cmake --build build/Debug   # F5 调试要的那份
```

> 环境已经配好：VS Code 里装了 **CMake Tools**（`ms-vscode.cmake-tools`），
> 用户设置里把 STM32Cube 扩展自带的 `cube-cmake` 加进了 `cmake.environment.PATH`，
> 工程的 `.vscode/settings.json` 指向它。所以 CMake Tools 和上面的任务都能直接用。

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
- **`.clangd` 由 VS Code 扩展生成，但要满足两个条件才会生成**：STM32Cube 扩展
  （`stm32cube-ide-build-cmake`）会写 `.clangd` 和 `.vscode/c_cpp_properties.json`，
  内容按当时的 build 目录写（`build/Release` 或 `build/Debug`）。**光打开工程目录不够**，还要：
  ① 命令面板跑一次 **「设置 STM32Cube 项目」**（否则报 `active project is not set`）；
  ② 跑一次 **`CMake: Configure`**（否则报 `Cannot find project build information yet` ——
  扩展的 build 目录是从 CMake Tools 的 code model 拿的，用终端 `cmake --preset` 它不知道）。
  卡住就看输出面板的 `STM32Cube CMake build` 通道。
  **不要**从别的工程抄，也**不要**放在仓库根目录 —— 根目录那份会让所有子工程读到指向不存在路径的配置，
  补全反而彻底失效。详见 [`新建工程指南.md`](新建工程指南.md) 的 D 类。
- 引脚目前**基本都是手写在 `main.c` 的 `USER CODE` 区块里**（PC13 / PA0 没在 `.ioc` 里登记）。
  以后改引脚建议先在 CubeMX 的 Pinout 里配好再生成，比手写寄存器/HAL 初始化靠谱。
- **系统时钟还是 HSI 8 MHz、PLL 没开**：`HAL_Delay` 的时基、I2C 时序、串口波特率都按 SYSCLK 算，
  上 I2C/串口之前要先在 Clock Configuration 里配成 **HSE + PLL ×9 = 72 MHz，APB1 /2**。

## 提交与推送（git）

本仓库有两个远程，习惯上两边都推（一份上云，一份本地留底）：

| 远程 | 地址 | 说明 |
|---|---|---|
| `origin` | `git@github.com:MKXDxiaoyudian/stm32.git` | GitHub，走 SSH |
| `backup` | `/Users/zouminyu/stm32-workspace-backup.git` | 本机裸仓库（bare），离线备份 |

日常三步（**在仓库根目录做**）：

```bash
cd ~/Documents/stm32        # ← 仓库根，不是子工程目录

git status --short          # 1. 看改了哪些文件（?? 是未跟踪，别误提交本机路径）
git add -A                  # 2. 暂存（只交一个工程：git add button_LED/）
git commit -m "说明"         # 3. 提交
git push origin main        # 4a. 推 GitHub
git push backup main        # 4b. 推本地备份
```

注意点：

- **新机器先验权**：`ssh -T git@github.com`，看到 `Hi MKXDxiaoyudian!` 才算配好。
- **提交前先编译一次**，别把编不过的代码推上去。
- **推错了想撤**：`git reset --soft HEAD~1` 撤回提交但保留改动；已经推上去的就 `git push --force-with-lease origin main`（只在自己一个人的分支上用）。
- 改历史要小心：本仓库在首次推送前用 `git filter-branch` 剔除过一次大文件（见下），推上去之后再改就得 force push。

## 关于 git 历史

**这个仓库不是我原来的点灯工程仓库**，是 2026-10-02 新建的合集仓库。三个仓库的关系：

| 仓库 | 装什么 | 状态 |
|---|---|---|
| `MKXDxiaoyudian/stm32`（本仓库） | `new` + `button_LED` + `firstdemo` + `firstdemo2` 四个工程的合集 | ✅ 当前在用 |
| `MKXDxiaoyudian/stm32f103-bringup` | 只有原来那个点灯工程（`new`） | 📦 冻结，内容已并入本仓库 |
| `~/stm32-backup.git` | 上面那个点灯工程仓库的本地裸备份 | 📦 冻结 |

`new/` 原来那 4 条独立历史提交还在旧的 GitHub 仓库和 `~/stm32-backup.git` 里：

```
4f05e6d 恢复可用的 STM32F103C8T6 点灯工程（CubeMX + CMake/Ninja + DAP 烧录）
5077345 docs: 加 README（接线表 / 构建 / 烧录 / 调试 / 工具链 / 已知问题）
652c9d3 点灯：增加 PA0 翻转电平，PC13 改为 Toggle + HAL_Delay(500)
da55fbf docs: README 补「提交与推送」一节（双远程 / 三步流程 / 注意点）
```

需要翻旧账：

```bash
git remote add old git@github.com:MKXDxiaoyudian/stm32f103-bringup.git
git fetch old && git log --oneline old/main
```

### 首次推送前做过一次历史瘦身

`button_LED` 最早是用 CubeMX 的 copy-all 生成的，`Drivers/` 有 **67 MB**。这批文件被提交进第一个 commit 后，
用 `git filter-branch --index-filter` 从**全部历史提交**里剔除，换成了 3.5 MB 的精简集，然后 `git gc --prune=now`。
结果：仓库 `.git` 从 14 MB 降到 **740 KB**，`button_LED` 的跟踪文件从 925 个降到 87 个。

> 这也是为什么 2026-10-02 那几条提交的 hash 和最初报告的不一样（`c272aec` → `989e4be` 等）——历史被重写过了。
> 快照备份留在 `~/stm32-snapshot-20261002.tgz`（18 MB，含全部分支未被重写前的状态）。

# CLAUDE.md

本文件是 Claude Code 在本 UE5 + C++ 游戏项目中工作时的入口说明。所有具体规范（代码风格、命名、目录结构、UE5 特有规范等）都已拆分为独立技能，存放在 `.claude/skills/` 下，按场景自动加载，本文件不重复内容，只负责指路。

---

## 项目目标

48 小时 GameJam 项目，主题为 **"Anchor"**。做一款 **2D 像素风竖版跑酷/避障游戏**：玩家操控一根锚向海洋深处"落下"，视觉上通过场景与障碍整体向上滚动来模拟下落。

- 画面规格：竖屏逻辑分辨率 **180×320**（最终渲染分辨率可调整），像素画风格。
- 玩家区域：锚只能在 180×320 的可视区域内 W/A/S/D 自由移动，**完全离开区域（一个像素都不在范围内）即失败**。
- 关卡：无限生成，当前 demo 阶段包含静态障碍与在 180 宽度内左右摆动的动态障碍。
- 生成方式：以 **320 单位高度**的 Chunk 为单位向上拼接生成，出屏后销毁/回收。
- 当前阶段：demo 验证，优先跑通，细节后面再细化。

## 项目基本信息

- 引擎版本：UE 5.7（已启用 `Paper2D` 插件）
- 项目类型：2D 像素风竖版跑酷/避障 GameJam Demo（`Content/` 为空，正在搭建基础框架）
- 目标平台：Desktop（Windows 为主；`Config/DefaultEngine.ini` 中保留了 Linux/Mac RHI 配置）
- C++ 标准：C++20（跟随引擎默认）
- 构建工具：UnrealBuildTool + Visual Studio 2022 / Rider for UE
- 引擎安装路径：`G:/EPIC/Unreal Engine/UE_5.7`

## 常用命令

以下命令基于上述引擎路径。若引擎位置变化，替换路径即可。

```powershell
# 生成/刷新 Visual Studio 项目文件
"G:/EPIC/Unreal Engine/UE_5.7/Engine/Build/BatchFiles/Build.bat" -projectfiles -project="G:/Project/GameJamAnchor/GameJamAnchor.uproject" -game -engine -progress

# 编译 Editor（Development）
"G:/EPIC/Unreal Engine/UE_5.7/Engine/Build/BatchFiles/Build.bat" GameJamAnchorEditor Win64 Development "G:/Project/GameJamAnchor/GameJamAnchor.uproject" -waitmutex

# 启动 Editor
"G:/EPIC/Unreal Engine/UE_5.7/Engine/Binaries/Win64/UnrealEditor.exe" "G:/Project/GameJamAnchor/GameJamAnchor.uproject"
```

注意：若 Editor 正在运行且启用了 Live Coding，编译会失败，提示 `Unable to build while Live Coding is active`。此时需关闭 Editor 后再编译，或在 Editor 内按 `Ctrl+Alt+F11` 热重载。

## 功能模块

当前已划分 demo 阶段的核心模块与负责人：

| 模块 | 负责人 | 当前状态/说明 |
|---|---|---|
| 玩家系统（Pawn） | 另一位程序 | 负责 W/A/S/D 移动、失败判定（完全出界）。移动使用 `AddMovementInput`，失败等事件通过事件分发器/委托广播。 |
| 场景系统（Chunk / 障碍生成与滚动） | 你 | `AChunkManager`（`Source/GameJamAnchor/Chunk/`）维护 Chunk 队列；`AChunk` 为 320 高的关卡段；`AObstacle`（`Source/GameJamAnchor/Obstacle/`）为障碍基类，支持静态与动态正弦摆动。滚动速度先固定，但已暴露 `ScrollSpeed` 属性供后续调整。 |
| 渲染与相机 | 你 | 已启用 `Paper2D` 插件；`AAAnchorCamera` 静态正交相机已可工作；`AGameJamAnchorGameMode` 自动切换 ViewTarget；`AAAnchorPlayerPawn` 为临时占位 Pawn。 |

### 关键接口约定

- **场景滚动**：由 Chunk/障碍自身负责上移（同伴倾向使用 `UProjectileMovementComponent`），`AChunkManager` 控制生成节奏与滚动速度。
- **玩家失败判定**：由 Pawn 自行检测是否完全离开 180×320 区域，触发事件（Event Dispatcher / 多播委托）通知 GameMode 与场景系统。
  - 具体实现：`AGameJamAnchorGameMode` 暴露 `OnAnchorPlayerOutOfBounds` 多播委托与 `ReportAnchorPlayerOutOfBounds()` 方法。
  - Pawn 判定失败后调用 `ReportAnchorPlayerOutOfBounds()` 或绑定蓝图事件。
  - `AChunkManager` 在 `BeginPlay` 中绑定该委托，收到通知后停止 `Tick`，不再生成/移动 Chunk。
- **解耦原则**：Pawn 不直接调用 SceneManager，SceneManager 也不直接控制 Pawn，双方通过事件/属性通信。

### Paper2D / 相机 / 材质配置

- **插件**：`.uproject` 中已启用 `Paper2D`；`GameJamAnchor.Build.cs` 已添加 `Paper2D` 模块依赖。
- **相机类**：`AAAnchorCamera`（`Source/GameJamAnchor/Camera/`）。静态正交相机，不跟随玩家；`ViewportWidth` / `ViewportHeight` 可在编辑器中调整，默认 180×320。
- **推荐坐标系**：游戏区域放在 **XZ 平面**（X 水平、Z 垂直），相机沿 **-Y 轴**俯视。经编辑器验证的有效摆放：
  - `AnchorCamera` 位置 `(0, 1000, 0)`，旋转编辑器显示为 `(0, 0, -90)`（即 Yaw=-90，沿 -Y 看向原点）
  - `PlayerStart` 位置 `(0, 0, 0)`（屏幕中心）
  - 可视区域：X ∈ [-90, 90]，Z ∈ [-160, 160]
  - 屏幕上方对应 **+Z**，Chunk 向上移动即沿 **+Z** 方向。
- **GameMode**：`AGameJamAnchorGameMode`（`Source/GameJamAnchor/Framework/`）。自动查找关卡中的 `AAAnchorCamera` 并设为玩家 ViewTarget。
- **占位 Pawn**：`AAAnchorPlayerPawn`（`Source/GameJamAnchor/Player/`）。当前仅挂载 `UPaperSpriteComponent`，无移动逻辑。另一位程序实现正式 Pawn 后，在蓝图 GameMode 或 `DefaultGame.ini` 中覆盖 `DefaultPawnClass` 即可替换。
- **材质**：当前使用引擎默认 Sprite 材质。美术资产就绪后，创建 `MI_` 前缀的 Material Instance 或在蓝图子类中直接替换 `UPaperSpriteComponent::Sprite`。
- **目录**：美术资产建议放入 `Content/_Game/Sprites/`、`Content/_Game/Textures/`、`Content/_Game/Materials/`；地图放入 `Content/_Game/Maps/`。

### Chunk 生成与滚动

- **Chunk 类**：`AChunk`（`Source/GameJamAnchor/Chunk/`）。一个 320 单位高的关卡段，使用 `UPaperSpriteComponent` 作为编辑器可视化组件；默认加载引擎占位 Sprite `S_Actor`，后续在蓝图子类 `BP_Chunk` 中替换为美术像素 Sprite。
  - 新增 `LeadingGap` / `TrailingGap`：分别控制本 Chunk 与上一个、下一个 Chunk 之间的额外间距，可在不同 `BP_Chunk` 里独立调整，方便关卡节奏设计。
- **背景类**：`ABackground`（`Source/GameJamAnchor/Background/`）。通过 Details 面板指定 `BackgroundSprite`（PaperSprite 资产）或 `BackgroundTexture`（Texture 资产）。运行时会按 `BackgroundWidth × BackgroundHeight` 自动缩放，使其铺满 180×320 可视区域。
- **Chunk 管理器**：`AChunkManager`（`Source/GameJamAnchor/Chunk/`）。
  - 支持多种普通 Chunk 模板：在 Details 面板的 `Chunk Classes` 数组中添加 `BP_ChunkA`、`BP_ChunkB` 等，生成时会随机挑选；如果数组为空则回退到单个 `Chunk Class`。
  - 支持终点 Chunk：设置 `Termination Chunk Class` 与 `Termination Chunk Index`（例如 20），生成第 20 个普通 Chunk 后，下一个会生成终点 Chunk，并不再生成后续 Chunk。
  - `BeginPlay` 时生成初始 Chunk，覆盖屏幕及下方缓冲。
  - 每帧按 `ScrollSpeed` 将所有 Chunk 沿 **+Z** 移动。
  - Chunk 底部超过屏幕顶部（`ScreenTopZ = 160`）后销毁。
  - 当屏幕下方缓冲区内没有 Chunk 覆盖时，自动在缓冲区内生成新 Chunk，实现无限滚动；生成终点 Chunk 后停止补充。
  - `ChunkSpacing` 控制相邻 Chunk 中心距；默认等于 `ChunkHeight`（无缝），调大可在 Sprite 之间留出间隙。
  - **滚动速度统一由 `ChunkManager` 的 `ScrollSpeed` 控制**，生成新 Chunk 或在每帧同步时，`AChunk` 与 `AObstacle` 的 `ScrollSpeed` 会自动被覆盖，无需分别调整。
- **在关卡中使用**：拖入一个 `ChunkManager`，在 Details 面板设置 `Chunk Class` 为 `BP_Chunk`（基于 `AChunk` 的蓝图子类）或直接使用 `AChunk`，Play 即可看到 Chunk 向上滚动。

### 障碍生成

- **障碍基类**：`AObstacle`（`Source/GameJamAnchor/Obstacle/`）。
  - 每帧沿 **+Z** 以 `ScrollSpeed` 上移，模拟场景下落。
  - 支持两种模式：
    - 静态障碍：`bDynamic = false`，只随场景上移。
    - 动态障碍：`bDynamic = true`，在 `InitialX` 基础上叠加 `SwayAmplitude * Sin(SwayPhase)` 的 X 方向摆动。
  - 挂载 `UBoxComponent` 作为碰撞体，`CollisionProfile` 为 `BlockAllDynamic`，方便后续与玩家 Pawn 做物理/重叠判定。
  - 使用 `UPaperSpriteComponent` 显示 Sprite；默认加载引擎占位 Sprite `S_Actor`，后续在蓝图子类 `BP_Obstacle` 中替换为美术像素 Sprite。
- **生成配置**：`FObstacleSpawnConfig`（定义在 `AChunk` 头文件中）。
  - `ObstacleClass`：要生成的障碍类（如 `BP_Obstacle`）。
  - `RelativeLocation`：相对于 Chunk 中心的位置。
  - `bDynamic` / `SwayAmplitude` / `SwaySpeed`：动态摆动开关与参数。
- **生成流程**：`AChunk::BeginPlay()` 遍历 `ObstacleConfigs`，为每个有效配置生成一个 `AObstacle`，并把当前 Chunk 的 `ScrollSpeed` 与配置参数同步给障碍实例。
- **在关卡中使用**：
  1. 创建 `BP_Obstacle` 蓝图，继承自 `AObstacle`，替换 `SpriteComponent` 为美术 Sprite，并调整 `CollisionBox` 大小。
  2. 创建 `BP_Chunk` 蓝图，继承自 `AChunk`，在 Details 面板设置 `ObstacleConfigs`，添加若干元素并指定 `BP_Obstacle` 与相对位置。
  3. 在关卡中拖入 `ChunkManager`，设置 `Chunk Class` 为 `BP_Chunk`，Play 即可看到障碍随 Chunk 上移。

---

## 规范查阅索引

下表列出各类任务应查阅的技能（位于 `.claude/skills/<技能名>/SKILL.md`）。Claude Code 通常会根据任务自动匹配，也可在任务前主动确认已加载对应技能。

| 场景 | 应查阅的技能 |
|---|---|
| 新建 C++ 类/文件，决定放在哪个目录；给类/变量/函数/资产命名；新增跨模块依赖 | `project-structure-and-naming` |
| 编写或修改任意 C++ 函数逻辑（默认风格要求） | `cpp-guard-clause-style` |
| 修改已有代码的逻辑、参数或行为 | `code-comment-sync` |
| 涉及 `UPROPERTY`/`UFUNCTION`/GC/委托/网络同步（RPC、Replication）的代码 | `ue5-reflection-memory-network` |
| 判断某功能该用 C++ 还是蓝图；编写或评审蓝图逻辑 | `blueprint-usage-boundary` |
| 编写日志输出、设计异常处理分支 | `error-logging-standards` |
| 配置 `.gitignore`/Git LFS；编写 commit message | `git-version-control` |
| 涉及 `Tick`、逐帧逻辑、对象遍历等性能相关代码 | `performance-guidelines` |
| 为核心玩法逻辑编写 Automation 测试 | `testing-standards` |
| 提交代码前自查、审查他人代码 | `code-review-checklist` |

---
name: ue5-project-conventions
description: 本 UE5 + C++ 项目的详细工程规范手册，涵盖目录结构、命名规范、模块依赖、UE5 反射与内存管理、蓝图使用边界、错误处理与日志、版本控制、性能优化、测试规范、Code Review 清单。触发时机：新建 C++ 类或决定文件/资产该放哪个目录时；给类/变量/函数/资产命名时；写涉及 UPROPERTY/UFUNCTION/GC/委托/网络同步的代码时；判断某功能该用 C++ 还是蓝图、或编写蓝图逻辑时；配置 .gitignore/Git LFS、写 commit message 时；关注性能优化或编写 Automation 测试时；做 Code Review 自查时。只要涉及"具体怎么组织/命名/落地"而不只是"要不要用卫语句、要不要同步注释"这类核心原则，就应查阅本技能。
---

# UE5 + C++ 项目规范手册

本手册是 CLAUDE.md 中三条核心强制规则（卫语句风格、注释同步修改、蓝图 C++ 优先边界）的详细展开与配套工程规范。CLAUDE.md 负责"必须做什么"，本手册负责"具体怎么做"。

## 一、项目目录结构规范

### 1.1 工程根目录（Root）

```
ProjectRoot/
├── ProjectName.uproject
├── Config/                 # 全局配置文件（.ini）
├── Content/                # 游戏内容资产（蓝图、地图、材质等）
├── Source/                 # C++ 源代码（唯一放置手写代码的位置）
├── Plugins/                # 自研插件与第三方插件
├── Binaries/                # 编译产物（不入库）
├── Intermediate/            # 中间文件（不入库）
├── Saved/                   # 本地存档/日志/自动保存（不入库）
├── DerivedDataCache/        # 派生数据缓存（不入库）
├── .gitignore
├── .gitattributes           # 配置 Git LFS 跟踪规则
└── CLAUDE.md
```

### 1.2 `Source/` 目录结构（按模块划分）

UE5 项目应遵循**模块化（Module）**组织方式，禁止把所有代码堆在一个模块里。典型结构：

```
Source/
├── ProjectName/                     # 主游戏模块（Primary Game Module）
│   ├── ProjectName.Build.cs
│   ├── ProjectName.cpp / .h
│   ├── Public/                      # 对外暴露的头文件
│   │   ├── Characters/
│   │   ├── Player/
│   │   ├── AI/
│   │   ├── Gameplay/
│   │   │   ├── Abilities/
│   │   │   ├── Components/
│   │   │   └── Interfaces/
│   │   ├── UI/
│   │   ├── Items/
│   │   ├── Framework/                # GameMode / GameState / PlayerState 等
│   │   ├── Subsystems/
│   │   └── Utils/
│   └── Private/                     # 对应实现文件，目录结构与 Public 一一对应
│       ├── Characters/
│       ├── Player/
│       ├── AI/
│       ├── Gameplay/
│       ├── UI/
│       ├── Items/
│       ├── Framework/
│       ├── Subsystems/
│       └── Utils/
├── ProjectNameEditor/               # 编辑器专用模块（仅编辑器加载）
├── ProjectNameTests/                # 自动化测试模块
└── ProjectName.Target.cs
└── ProjectNameEditor.Target.cs
```

**规则：**
1. 每新增一个较大的功能域（如联机同步、战斗系统），优先考虑拆分为独立模块或 Plugin，避免主模块过度膨胀。
2. `Public/` 与 `Private/` 目录结构必须一一对应，`.h` 放 `Public`，`.cpp` 放 `Private`（除非该类完全内部使用，可整体放入 `Private`）。
3. 每个子系统一个子文件夹，文件夹命名使用 PascalCase，与命名空间概念对齐（即使 UE5 通常不使用 C++ namespace）。

### 1.3 `Content/` 目录结构（资产命名与分区）

```
Content/
├── _Game/                  # 项目自有内容根目录（下划线保证排序置顶）
│   ├── Characters/
│   ├── Blueprints/
│   ├── UI/
│   ├── Maps/
│   ├── VFX/
│   ├── Audio/
│   ├── Materials/
│   └── DataAssets/
├── ThirdParty/              # 商城资产/外部美术资源，禁止修改内部结构
└── Developers/              # 个人临时测试内容，禁止提交非本人内容
```

### 1.4 `Config/` 目录

- `DefaultEngine.ini`、`DefaultGame.ini`、`DefaultInput.ini` 等按 UE5 标准命名，禁止手动新建非标准配置文件名。
- 平台专属配置放入 `Config/Windows/`、`Config/PS5/` 等子目录，遵循引擎默认查找规则。

---

## 二、命名规范（Naming Convention）

遵循 **Epic 官方 C++ 编码规范** 作为基线，结合团队习惯做以下强制约定：

### 2.1 类型前缀

| 类型 | 前缀 | 示例 |
|---|---|---|
| `UObject` 派生类 | `U` | `UInventoryComponent` |
| `AActor` 派生类 | `A` | `APlayerCharacter` |
| 结构体 `struct` | `F` | `FDamageInfo` |
| 枚举 `enum class` | `E` | `EWeaponType` |
| 接口（继承 `UInterface`） | `I` | `IInteractable` |
| 模板类 | `T` | `TWeakObjectPtr` |
| 静态/自由函数库类 | `U`+`Library` 后缀 | `UMathLibraryHelpers` |
| Slate 控件 | `S` | `SHealthBarWidget` |

### 2.2 变量与函数命名

- 类、函数：`PascalCase`，如 `CalculateDamage()`。
- 布尔变量/函数：`b` 前缀，如 `bIsAlive`、`IsDead()` 返回值语义清晰即可不强制 `b` 前缀于函数名，但**变量必须加 `b`**。
- 成员变量：`PascalCase`，不使用 `m_` 前缀（遵循 Epic 规范）；如团队另有约定需在此处明确写出并保持项目内统一。
- 局部变量：`PascalCase` 或 `camelCase` 二选一，但**全项目必须统一**，默认推荐 `PascalCase` 与 UE5 源码保持一致。
- 常量 / `static const`：`PascalCase`，不使用全大写下划线风格（区别于纯 C 项目习惯）。
- 枚举值：`PascalCase`，如 `EWeaponType::Melee`。
- 委托类型：`F` + `On` + 事件名 + `Signature`（多播委托），如 `FOnHealthChangedSignature`。

### 2.3 文件命名

- 头文件/源文件名与类名去除前缀后一致：`APlayerCharacter` → `PlayerCharacter.h` / `PlayerCharacter.cpp`。
- 一个文件只放一个主要类，禁止在一个 `.h` 中塞多个不相关大类。

### 2.4 资产命名（Content Browser）

沿用行业通用的 **前缀_资产名_后缀** 规则，例如：

| 资产类型 | 前缀 |
|---|---|
| Blueprint | `BP_` |
| Widget Blueprint | `WBP_` |
| Material | `M_` |
| Material Instance | `MI_` |
| Static Mesh | `SM_` |
| Skeletal Mesh | `SK_` |
| Animation Blueprint | `ABP_` |
| Data Table | `DT_` |
| Data Asset | `DA_` |
| Gameplay Ability | `GA_` |
| Gameplay Effect | `GE_` |
| Niagara System | `NS_` |

---

## 三、模块与依赖规范

1. **单向依赖原则**：底层模块（Framework/Utils）不得反向依赖上层模块（UI/Gameplay）。
2. 模块间通信优先使用 **接口（`UInterface`）**、**委托** 或 **GameplayTag** 解耦，禁止跨模块直接 `#include` 大量具体实现类。
3. `.Build.cs` 中 `PublicDependencyModuleNames` 与 `PrivateDependencyModuleNames` 必须按最小依赖原则填写：仅在头文件中暴露的依赖放 Public，仅 `.cpp` 内部使用的放 Private。
4. 禁止循环依赖（Circular Dependency），发现循环依赖须通过接口抽象或事件系统解除。

---

## 四、代码风格规范——卫语句（Guard Clause）【强制】

本项目**强制使用卫语句风格**编写所有 C++ 逻辑代码，禁止深层嵌套的 `if-else` 结构。

### 4.1 核心原则

> **尽早返回（Return Early），拒绝嵌套（Avoid Nesting）。**
> 函数开头集中处理所有非法/边界/提前退出条件，主干逻辑保持在最外层缩进，避免出现超过 2 层的条件嵌套。

### 4.2 反例（禁止的写法）

```cpp
void APlayerCharacter::TryAttack()
{
    if (IsAlive())
    {
        if (!bIsAttacking)
        {
            if (EquippedWeapon != nullptr)
            {
                if (EquippedWeapon->HasAmmo())
                {
                    EquippedWeapon->Fire();
                }
                else
                {
                    UE_LOG(LogGame, Warning, TEXT("No ammo"));
                }
            }
        }
    }
}
```

### 4.3 正例（卫语句写法）

```cpp
void APlayerCharacter::TryAttack()
{
    if (!IsAlive())
    {
        return;
    }

    if (bIsAttacking)
    {
        return;
    }

    if (EquippedWeapon == nullptr)
    {
        return;
    }

    if (!EquippedWeapon->HasAmmo())
    {
        UE_LOG(LogGame, Warning, TEXT("No ammo"));
        return;
    }

    EquippedWeapon->Fire();
}
```

### 4.4 适用场景

- **函数入口参数/状态校验**：所有空指针检查（`IsValid()` / `nullptr` 判断）、权限校验、状态校验一律使用卫语句前置。
- **循环内部**：使用 `continue` 替代 `if (cond) { ... }` 包裹整个循环体。
- **`switch` 语句**：每个 `case` 尽量以 `break` / `return` 收尾，避免在 `case` 内再嵌套多层判断；复杂分支考虑拆成独立函数后用卫语句处理。
- **网络 RPC 函数、Server/Client 校验**：权限与合法性校验必须在函数最前面用卫语句短路返回，而非包裹主逻辑。

### 4.5 禁止事项

1. 禁止超过 **2 层** 的 `if` 嵌套（卫语句本身产生的顺序 `if` 不计入嵌套层数）。
2. 禁止 `if (cond) { 大段逻辑 } else { 大段逻辑 }` 这种双大段对称结构，应改写成：先卫语句排除一侧，再执行主逻辑。
3. 禁止用嵌套三元表达式代替卫语句来"取巧"降低 `if` 层数。
4. 函数应尽量保持单一职责，若卫语句判断本身超过 5～6 个，考虑拆分成多个小函数。

### 4.6 与 `check` / `ensure` 的配合

- 对**不可能发生、发生即视为编程错误**的情况使用 `check()` / `checkf()` 直接断言中断。
- 对**可能发生但应记录并安全退出**的情况，使用卫语句 + `UE_LOG` 记录后 `return`。
- 对**非致命但需要提示**的情况，使用 `ensure()` / `ensureAlways()` 并配合卫语句提前返回，保证后续代码不会在非法状态下继续执行。

---

## 五、注释规范——代码与注释同步修改【强制】

### 5.1 基本原则

> **代码与注释视为同一个修改单元，任何一方发生变化，另一方必须在同一次提交（commit）内同步更新。**
> 严禁出现"改了逻辑但没改注释"或"改了注释但代码逻辑未变"的不一致状态。发现两者不一致，**视为 Bug**，必须立即修正，不得以"后续再补"为由推迟。

### 5.2 Claude Code 执行要求（针对 AI 辅助编码）

在每一次代码修改任务中，必须执行以下检查清单：

1. **修改前**：阅读目标函数/类原有注释，理解其描述的行为契约（前置条件、后置条件、副作用）。
2. **修改中**：若逻辑、参数、返回值、异常/边界行为发生变化，**必须在同一次编辑中**同步修改：
   - 函数头部的 Doxygen 注释（描述、`@param`、`@return`）。
   - 行内注释中对该段逻辑的解释。
   - 类头部的整体职责说明（若类职责发生变化）。
3. **修改后**：自查一遍，确认注释中不存在指向"已删除的变量/已废弃的分支/旧的调用关系"的过期描述。
4. 如果一次改动只是重命名、格式化、纯粹搬移代码位置而不改变语义，也需检查注释中是否引用了旧名称，并同步更新。

### 5.3 头文件/类注释模板（Doxygen 风格）

```cpp
/**
 * 玩家角色主控制类，负责处理移动、战斗、装备切换等核心玩法逻辑。
 * 依赖 UHealthComponent 处理生命值，依赖 UInventoryComponent 处理背包。
 */
UCLASS()
class PROJECTNAME_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    /**
     * 尝试发起一次攻击，若角色状态不允许（死亡/正在攻击/无武器/无弹药）则直接返回。
     *
     * @return 无返回值；攻击是否成功可通过 OnAttackPerformed 委托监听。
     */
    void TryAttack();
};
```

### 5.4 函数级注释要求

- **每个非 trivial 的 public/protected 函数**必须有 Doxygen 注释，说明其**职责、前置条件（卫语句校验的内容）、副作用**。
- 卫语句本身建议配合**简短行内注释**说明"为什么在此处提前返回"，尤其是校验条件不直观时：

```cpp
if (!IsValid(EquippedWeapon))
{
    // 未装备武器时禁止进入攻击流程，避免后续空指针访问
    return;
}
```

- 纯粹自解释的卫语句（如 `if (!IsAlive()) { return; }`）可不加注释，避免过度注释造成噪音；**注释应解释"为什么"，而非重复"是什么"**。

### 5.5 禁止事项

1. 禁止保留被注释掉的旧代码块超过一次提交周期（`// old logic ...`），应通过 Git 历史查阅旧逻辑，而非注释保留死代码。
2. 禁止 TODO/FIXME 注释无归属、无追踪：格式统一为 `// TODO(负责人/工单号): 描述`。
3. 禁止注释与函数名义重复且毫无信息增量（如 `// 设置生命值` 写在 `void SetHealth(float NewHealth)` 上方却不解释边界处理）。

---

## 六、UE5 特有编码规范

### 6.1 反射宏使用规范

- `UPROPERTY()` 必须为需要被蓝图访问、序列化、GC 追踪的成员变量添加，并明确指定合适的 Specifier（`EditAnywhere` / `BlueprintReadOnly` / `VisibleAnywhere` 等），禁止无脑全部开放为 `EditAnywhere, BlueprintReadWrite`。
- 所有 `UObject*` 类型的成员变量**必须**使用 `UPROPERTY()` 标记，否则会被 GC 提前回收，产生悬空指针（这是常见且严重的 Bug 来源）。
- `UFUNCTION()` 用于需要暴露给蓝图、反射调用、RPC（`Server`/`Client`/`NetMulticast`）、或需要 `BindDelegate` 的函数。

### 6.2 内存与生命周期管理

- 引用其他 `UObject` 时优先使用 `UPROPERTY()` 标记的裸指针，由 GC 统一管理生命周期。
- 弱引用场景（如缓存跨系统的 Actor 引用、避免强制保活）使用 `TWeakObjectPtr<T>`，使用前必须 `IsValid()` 校验（卫语句前置）。
- 大型资源（贴图、Mesh 等）延迟加载场景使用 `TSoftObjectPtr<T>` / `TSoftClassPtr<T>`，避免同步硬引用拖慢加载。
- 非 `UObject` 的普通 C++ 对象生命周期使用标准 `TUniquePtr` / `TSharedPtr`（UE 容器版本），禁止裸 `new`/`delete` 管理 `UObject`。

### 6.3 蓝图使用规范（C++ 优先，蓝图仅在必要场景使用）

#### 6.3.1 基本原则

> **本项目为 C++ 优先（C++-First）项目，蓝图不是默认实现手段，而是"例外"。**
> 任何功能默认先考虑用 C++ 实现；只有落在下方"允许使用蓝图的场景"清单中，才可以用蓝图，且应尽量保持蓝图逻辑"薄"——蓝图只做胶水/表现层，不承载核心规则判断。

#### 6.3.2 允许使用蓝图的场景

| 场景 | 说明 |
|---|---|
| UMG 界面布局与动画 | Widget 的排版、过场动画、简单的显隐切换 |
| 关卡脚本化事件（Level Blueprint 少量触发） | 仅用于关卡内一次性、非复用的触发逻辑（如开门、播放过场） |
| 数值/曲线调优资产 | Data Table、Data Asset、Curve Asset，供策划直接调数值，不涉及逻辑分支 |
| VFX / 动画蓝图的表现层混合 | Blend Space、AnimGraph 中的姿态混合、状态机可视化 |
| 美术/特效人员独立迭代的表现效果 | 不影响 Gameplay 判定结果的纯表现反馈（如受击闪光、震屏参数） |
| 策划快速原型验证 | 临时性验证，验证通过后必须**迁移为 C++ 实现**方可合入主干 |

#### 6.3.3 禁止使用蓝图实现的场景

- 核心玩法规则判定（伤害计算、命中判定、状态机切换条件、连招逻辑）。
- 任何性能敏感路径（`Tick` 高频调用、大量对象遍历、AI 决策核心逻辑）。
- 网络同步相关逻辑、RPC 调用、权限/合法性校验。
- 存档、配置读写、跨系统状态管理等基础设施逻辑。
- 会被多处复用的通用算法/工具逻辑（应写成 `UBlueprintFunctionLibrary` 的 C++ 静态函数，蓝图只负责调用）。

#### 6.3.4 C++ 与蓝图的扩展点约定

- C++ 基类只暴露**清晰、单一职责**的可覆写点，统一使用 `BlueprintNativeEvent`（需要 C++ 默认实现）或 `BlueprintImplementableEvent`（纯蓝图实现的表现钩子），禁止暴露内部实现细节或大范围可读写的内部状态。
- 命名统一以 `On` 开头描述"时机"，如 `OnDamageReceived`、`OnAbilityActivated`，让美术/策划一眼看出这是一个"响应式挂点"而非可随意调用的内部函数。
- 蓝图侧禁止通过 `Cast<>` 深入访问 C++ 内部私有实现类去"绕过"扩展点，一旦出现此类需求，应在 C++ 侧新增一个明确的扩展点。

#### 6.3.5 蓝图内部质量要求（当确需使用蓝图时）

即使是允许使用蓝图的场景，也需遵循与 C++ 卫语句同样的"尽早退出、避免深层嵌套"精神：

- Event Graph 中优先使用 `Branch` 节点做**前置校验并提前退出（Return Node）**，而不是把所有逻辑塞进一个 `Branch` 的 True 分支里形成深层嵌套。
- 单个 Event Graph 或 Function 的节点数、连线复杂度应保持在"一屏可读"范围内，超出应拆分为独立的蓝图 Function 或迁移到 C++。
- 必须使用**注释框（Comment Box）**对逻辑分区，关键节点/复杂表达式需添加节点注释，注释与蓝图逻辑同样遵循"同步修改"原则（改了逻辑必须同步改注释框描述）。
- 优先调用 C++ 暴露的 `UFUNCTION(BlueprintCallable)` 函数完成实际计算，蓝图内部不实现复杂表达式/多层条件运算。
- 禁止在蓝图中出现"逻辑黑洞"：同一段规则如果在多个蓝图中重复出现，必须上提为 C++ 或 `UBlueprintFunctionLibrary` 统一实现。

#### 6.3.6 蓝图命名与目录

- 命名遵循第 2.4 节资产命名规则（`BP_` / `WBP_` / `ABP_` 等前缀）。
- 蓝图存放目录需与其所属系统对应，例如 UI 蓝图统一放在 `Content/_Game/UI/`，禁止散落在 `Content` 根目录或按人员分散存放。
- 继承自某 C++ 基类的蓝图，命名应体现父类语义，如 `AWeaponBase`（C++）→ `BP_Weapon_Sword`（蓝图子类）。

#### 6.3.7 蓝图评审要求

- 新增/修改的蓝图逻辑，在 Code Review / 内容评审时需额外确认：是否落在 6.3.2 允许范围内；是否存在应上提为 C++ 的重复逻辑；Event Graph 是否符合"尽早退出、避免深层嵌套"的可读性要求。

### 6.4 委托与事件

- 跨系统通信优先使用 `DECLARE_DYNAMIC_MULTICAST_DELEGATE` 系列（可蓝图绑定）或普通 `DECLARE_MULTICAST_DELEGATE`（纯 C++ 高频事件，性能更优）。
- 绑定委托的一方负责在自身销毁时（`EndPlay` / 析构）显式 `Unbind`，避免野指针回调。

### 6.5 网络同步（如为联机项目）

- 属性同步统一使用 `UPROPERTY(Replicated)` + `GetLifetimeReplicatedProps` 声明，条件同步使用 `DOREPLIFETIME_CONDITION`。
- 所有 `Server` RPC 函数**必须在函数最前端使用卫语句做权限与合法性校验**（例如 `HasAuthority()`、输入合法性），未通过校验直接 `return`，杜绝作弊向量。

---

## 七、错误处理与日志规范

- 自定义日志分类：在模块头文件中通过 `DECLARE_LOG_CATEGORY_EXTERN` 定义专属 `LogCategory`（如 `LogCombat`、`LogInventory`），禁止全项目滥用 `LogTemp`。
- 日志级别使用规范：
  - `Log`：正常流程关键节点记录。
  - `Warning`：非致命异常、卫语句拦截的边界情况。
  - `Error`：逻辑错误但可恢复。
  - `Fatal`：不可恢复的严重错误（谨慎使用，会导致 Crash）。
- 结合卫语句：每个提前 `return` 的异常分支，视重要程度决定是否附带日志，避免"静默失败"导致排查困难。

---

## 八、版本控制规范

### 8.1 `.gitignore` 必须忽略的目录

```
Binaries/
Intermediate/
Saved/
DerivedDataCache/
.vs/
*.sln
*.suo
*.VC.db
*.opensdf
*.sdf
*.opendb
```

### 8.2 Git LFS

- 所有二进制资产（`.uasset`、`.umap`、音频、贴图源文件等）必须通过 `.gitattributes` 配置 LFS 跟踪。
- `Source/` 下的纯文本代码文件禁止走 LFS。

### 8.3 提交信息规范（建议采用 Conventional Commits）

```
<type>(<scope>): <subject>

类型：feat / fix / refactor / perf / docs / style / test / chore
示例：feat(combat): 新增卫语句重构后的连招判定逻辑
```

- 任何涉及第五章"代码与注释同步修改"的提交，commit message 中若只改了逻辑没提注释同步情况，视为不合规提交，需要补充说明或拆分提交。

---

## 九、性能与优化准则

1. `Tick` 函数默认关闭（`PrimaryActorTick.bCanEverTick = false`），确需逐帧更新的逻辑优先考虑 Timer / 事件驱动 / `TickInterval` 降频。
2. 避免在 `Tick` 或高频调用路径中进行动态内存分配、字符串拼接、`Cast<>` 滥用；`Cast` 结果应在卫语句中立即校验并提前返回。
3. 大量对象查找优先使用缓存引用 / `TMap` 索引，避免每帧 `GetAllActorsOfClass` 等昂贵遍历。
4. Gameplay 层与渲染/动画层解耦，避免 C++ 侧直接操作大量骨骼/顶点数据。

---

## 十、测试规范

- 核心玩法逻辑（伤害计算、状态机、技能系统）编写 **Automation Spec 测试**（`Source/ProjectNameTests/`），覆盖卫语句中的每一个提前返回分支（边界条件测试）。
- 测试用例命名：`[模块名].[被测函数].[场景描述]`，如 `Combat.TryAttack.WhenDead_ShouldNotFire`。
- 每个卫语句分支至少对应一条测试用例，确保重构时逻辑回归可被测试捕获。

---

## 十一、Code Review 检查清单

提交/审查代码前，逐项确认：

- [ ] 是否存在超过 2 层的 `if` 嵌套？是否已用卫语句重构？
- [ ] 所有 `UObject*` 成员是否都加了 `UPROPERTY()`？
- [ ] 修改的函数，其 Doxygen/行内注释是否已同步更新？
- [ ] 是否存在与当前代码逻辑不符的过期注释？
- [ ] 命名是否符合第二章前缀/大小写规范？
- [ ] 新增的跨模块依赖是否符合最小依赖 & 无循环依赖原则？
- [ ] 网络 RPC 是否在最前端做了权限校验（卫语句）？
- [ ] 是否新增/更新了对应的 Automation 测试用例？
- [ ] 若本次改动涉及蓝图：是否落在 6.3.2 允许使用蓝图的场景内？是否存在应上提为 C++ 的核心逻辑？
- [ ] 蓝图 Event Graph 是否遵循"尽早退出、避免深层嵌套"，并配有必要的注释框？
- [ ] `.gitignore` / LFS 配置是否遗漏了新增的二进制资产类型？

---
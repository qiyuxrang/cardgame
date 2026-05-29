# 纸牌匹配消除游戏








https://github.com/user-attachments/assets/7ee0608b-a784-4249-a6ad-13022cdc96bc











基于 cocos2d-x 3.17 的纸牌匹配消除游戏，实现手牌区翻牌替换、桌面牌匹配消除、回退功能。

## 功能演示

| 操作 | 说明 | 动画 |
|------|------|------|
| 桌面牌匹配 | 点击主牌区翻开牌，点数与手牌顶部牌差1即匹配 | 弹跳 → 平移飞入手牌区 |
| 手牌切换 | 点击手牌区任意牌，切换为新的顶部牌 | 平移滑到顶部 |
| 翻牌 | 点击"翻牌"从牌堆抽牌 | 旋转飞入手牌区 |
| 回退 | 点击"回退"撤销上一步 | 反向平移飞回原位 |
| 翻面 | 顶部牌被消除后下方牌翻开 | 翻转动画 |

## 快速开始

### 环境要求
- Windows 10+
- Visual Studio 2022 (含 "使用 C++ 的桌面开发")
- CMake 3.10+
- cocos2d-x 3.17 SDK

### 配置 SDK

本项目不包含 cocos2d-x 引擎。下载后创建目录链接：

```batch
# 下载 cocos2d-x 3.17 到任意位置，然后在项目根目录执行：
mklink /J cocos2d <你的cocos2d-x-3.17路径>
```

或直接修改 `CMakeLists.txt` 第 7 行的 `COCOS2DX_ROOT_PATH` 指向你的 SDK 路径。

### 编译运行
```batch
cd cardgame
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A Win32
cmake --build . --config Debug
.\bin\CardGame\Debug\CardGame.exe
```

## 架构设计

```
┌──────────────────────────────────────────────────┐
│                   View Layer                      │
│  GameScene → TableAreaView (1080×1500)           │
│           → HandAreaView  (1080×580)             │
│           → CardSprite (182×282 卡牌渲染)         │
│  职责: 布局、渲染、触摸事件、动画                    │
├──────────────────────────────────────────────────┤
│                Controller Layer                   │
│  GameController (编排器)                           │
│    ├── MatchRule    (策略模式 — 匹配规则)          │
│    ├── UndoManager  (命令栈 — 回退管理)            │
│    └── Actions      (命令 — TableToHand/HandSwitch)│
│  职责: 游戏逻辑、匹配判断、回退编排                  │
├──────────────────────────────────────────────────┤
│                  Model Layer                      │
│  CardData / CardLocation / GameBoard / IAction     │
│  职责: 纯数据结构，零外部依赖                        │
└──────────────────────────────────────────────────┘
```

**依赖方向：Model ← Controller ← View（严格单向）**

### 设计模式

| 模式 | 应用 | 目的 |
|------|------|------|
| **MVC** | 三层目录分离 | 视图/逻辑/数据独立 |
| **Command** | IAction + UndoManager | 每一步可撤销 |
| **Strategy** | IMatchRule + AdjacentRankRule | 匹配规则可插拔 |
| **Observer** | Controller 回调 → View 刷新 | 解耦通知 |

## 核心流程

```
用户点击桌面牌
    │
    ▼
GameScene 命中检测 (按z-order排序)
    │
    ▼
GameController::OnTableCardClicked(cardId)
    │
    ├── FindCard() 验证卡牌存在
    ├── IsCardTopOfColumn() 验证为顶部牌
    └── MatchRule::CanMatch() 匹配判断 (rank差1)
            │
            ▼
    new TableToHandAction → UndoManager::Execute()
            │
            ├── Execute(): 牌从桌面移除 → 加入手牌区 → 下方牌翻面
            └── 回调 View: CardMovedCB
                    │
                    ▼
            GameScene::OnCardMoved()
                └── PlayMatchEffect() → MoveTo() → OnBoardChanged()
```

## 回退流程

```
用户点击回退
    │
    ▼
GameController::Undo()
    │
    ├── PeekTop() 查看栈顶命令
    ├── 获取 cardId + 原位信息
    └── 回调 View: UndoAnimCB
            │
            ▼
    GameScene::OnUndoAnim()
        ├── 计算目标位置 (桌面或手牌)
        └── MoveToPosition() 反向动画 → 动画结束
                │
                ▼
        UndoManager::Undo() + NotifyBoardRefresh()
            └── 牌状态恢复 + View 刷新
```

## 扩展指南

### 新增卡牌类型（如万能牌）

**步骤：**

1. **`Model/CardTypes.h`** — CardData 新增 `CardType cardType` 字段
2. **`View/CardSprite.cpp`** — BuildFaceUpSprites() 新增渲染分支
3. **`Controller/MatchRule.h`** — CanMatch() 处理新类型匹配逻辑
4. **`Controller/GameController.cpp`** — SetupDefaultBoard() 创建新卡牌

**改动量：4 个文件，~30 行。**

```cpp
// 示例：万能牌与任意牌匹配
enum class CardType { Normal, Wild };

bool CanMatch(const CardData& a, const CardData& b) const override {
    if (a.cardType == CardType::Wild || b.cardType == CardType::Wild)
        return true;
    return abs(a.rank - b.rank) == 1;
}
```

### 新增回退类型（如批量回退）

**步骤：**

1. **`Controller/Actions.h/cpp`** — 实现 IAction 子类（Execute/Undo）
2. **`Controller/GameController`** — 新增方法，创建 Action
3. 无需修改 UndoManager / GameBoard / 任何 View 文件

**改动量：3 个文件，~50 行。**

### 新增匹配规则（如同花色消除）

**步骤：**

1. **`Controller/MatchRule.h`** — 实现 IMatchRule 子类
2. **`Controller/GameController.cpp`** — 一行代码切换规则

**改动量：1 个文件，~10 行。** 所有已有代码不做修改。

```cpp
// 示例：同花色匹配
class SameSuitRule : public IMatchRule {
    bool CanMatch(const CardData& a, const CardData& b) const override {
        return a.suit == b.suit;
    }
};
// 切换: m_controller.SetMatchRule(new SameSuitRule());
```

## 扩展性总览

| 扩展场景 | 涉及层 | 文件数 | 改动量 |
|---------|--------|--------|--------|
| 新卡牌类型 | M+V+C | 4 | ~30行 |
| 新匹配规则 | C | 1 | ~10行 |
| 新回退类型 | C | 3 | ~50行 |
| 新动画效果 | V | 1 | ~20行 |
| 新牌区/新布局 | M+C+V | 4 | ~150行 |

## 文件结构

```
cardgame/
├── CMakeLists.txt
├── README.md
├── docs/
│   └── design.md          ← 详细设计文档
├── Classes/
│   ├── AppDelegate.h/cpp  ← 入口：窗口/分辨率 1080×2080
│   ├── Model/
│   │   ├── CardTypes.h/cpp    ← Suit/Rank/CardData/CardLocation
│   │   ├── GameBoard.h/cpp    ← 牌局状态 (列式存储)
│   │   └── IAction.h          ← 命令模式接口
│   ├── Controller/
│   │   ├── GameController.h/cpp ← 逻辑编排器
│   │   ├── MatchRule.h        ← 匹配策略
│   │   ├── UndoManager.h/cpp  ← 回退管理器
│   │   └── Actions.h/cpp      ← 具体命令
│   └── View/
│       ├── GameScene.h/cpp    ← 主场景
│       ├── CardSprite.h/cpp   ← 卡牌渲染
│       ├── TableAreaView.h/cpp ← 主牌区布局
│       └── HandAreaView.h/cpp ← 手牌区布局
└── Resources/
    └── res/
        ├── card_general.png
        ├── number/   ← 数字图片 (big/small × red/black)
        └── suits/    ← 花色图标 (spade/heart/club/diamond)
```

## 技术参数

| 参数 | 值 |
|------|-----|
| 引擎 | cocos2d-x 3.17 |
| 语言 | C++11 |
| 设计分辨率 | 1080×2080 (FIXED_WIDTH) |
| 主牌区 | 1080×1500 |
| 手牌区 | 1080×580 |
| 卡牌尺寸 | 182×282 px |
| 构建系统 | CMake + MSVC |

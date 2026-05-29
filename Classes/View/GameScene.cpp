#include "GameScene.h"
#include <vector>
#include <algorithm>

USING_NS_CC;

GameScene* GameScene::CreateScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) return false;

    m_sceneAlive = Node::create();
    m_sceneAlive->retain();  // keep alive until scene exits
    addChild(m_sceneAlive, -1);

    // Cartoon backgrounds (claymorphism playful palette)
    auto* tableBg = LayerColor::create(
        Color4B(212, 180, 230, 255), (float)W, (float)TH);
    tableBg->setPosition(Vec2(0, (float)(H - TH)));
    addChild(tableBg, -10);

    auto* handBg = LayerColor::create(
        Color4B(180, 155, 200, 255), (float)W, (float)HH);
    handBg->setPosition(Vec2(0, 0));
    addChild(handBg, -10);

    // Title with shadow for cartoon depth
    auto* titleShadow = Label::createWithSystemFont("纸牌匹配消除", "Arial", 50);
    titleShadow->setColor(Color3B(140, 100, 160));
    titleShadow->setPosition(Vec2((float)W / 2 + 2, (float)(H - 38)));
    addChild(titleShadow, 9);

    auto* title = Label::createWithSystemFont("纸牌匹配消除", "Arial", 50);
    title->setColor(Color3B(255, 240, 255));
    title->setPosition(Vec2((float)W / 2, (float)(H - 40)));
    addChild(title, 10);

    m_tableArea = TableAreaView::Create(Size((float)W, (float)TH));
    if (m_tableArea) {
        m_tableArea->setPosition(Vec2(0, (float)(H - TH)));
        addChild(m_tableArea, 0);
    }

    m_handArea = HandAreaView::Create(Size((float)W, (float)HH));
    if (m_handArea) {
        m_handArea->setPosition(Vec2(0, 0));
        addChild(m_handArea, 0);
    }

    SetupUI();
    SetupController();
    m_controller.SetupDefaultBoard();
    OnBoardChanged();

    // Scene-level touch handler: hit-test all cards
    auto* listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false);
    listener->onTouchBegan = [this](Touch* t, Event*) -> bool {
        if (m_animating) return false;
        Vec2 loc = t->getLocation();

        // Hand cards: check RIGHTMOST first (highest X = on top)
        const auto& handCards = m_handArea->GetAllCards();
        std::vector<std::pair<int, CardSprite*>> sortedHand;
        for (auto& kv : handCards)
            sortedHand.push_back(kv);
        std::sort(sortedHand.begin(), sortedHand.end(),
            [](const auto& a, const auto& b) {
                return a.second->getPositionX() > b.second->getPositionX();
            });
        for (auto& kv : sortedHand) {
            if (kv.second->HitTest(loc)) {
                kv.second->InvokeClick();
                return true;
            }
        }

        // Table cards: check topmost (highest z-order) first
        const auto& tableCards = m_tableArea->GetAllCards();
        std::vector<std::pair<int, CardSprite*>> sortedTable;
        for (auto& kv : tableCards)
            sortedTable.push_back(kv);
        std::sort(sortedTable.begin(), sortedTable.end(),
            [](const auto& a, const auto& b) {
                return a.second->getLocalZOrder() > b.second->getLocalZOrder();
            });
        for (auto& kv : sortedTable) {
            if (kv.second->HitTest(loc)) {
                kv.second->InvokeClick();
                return true;
            }
        }
        return false;
    };
    getEventDispatcher()->addEventListenerWithSceneGraphPriority(
        listener, this);

    return true;
}

void GameScene::onExit() {
    if (m_sceneAlive) {
        m_sceneAlive->release();
        m_sceneAlive = nullptr;
    }
    Scene::onExit();
}

void GameScene::SetupUI() {
    // Draw button - bottom left (gold cartoon pill)
    auto* drawBg = DrawNode::create();
    drawBg->drawSolidRect(Vec2(-80, -30), Vec2(80, 30),
        Color4F(1.0f, 0.75f, 0.2f, 1.0f));
    drawBg->setPosition(Vec2(120.0f, (float)(HH - 60)));
    addChild(drawBg, 99);

    Label* drawLbl = Label::createWithSystemFont("翻牌 >>", "Arial", 34);
    drawLbl->setColor(Color3B(120, 50, 0));
    m_drawItem = MenuItemLabel::create(drawLbl,
        [this](Ref*) {
            if (!m_animating) m_controller.DrawFromReserve();
        });
    m_drawItem->setPosition(Vec2(120.0f, (float)(HH - 60)));

    // Undo button - bottom right (pink cartoon pill)
    auto* undoBg = DrawNode::create();
    undoBg->drawSolidRect(Vec2(-80, -30), Vec2(80, 30),
        Color4F(1.0f, 0.45f, 0.55f, 1.0f));
    undoBg->setPosition(Vec2((float)(W - 120), (float)(HH - 60)));
    addChild(undoBg, 99);

    Label* undoLbl = Label::createWithSystemFont("<< 回退", "Arial", 34);
    undoLbl->setColor(Color3B(100, 20, 30));
    m_undoItem = MenuItemLabel::create(undoLbl,
        [this](Ref*) {
            if (!m_animating) m_controller.Undo();
        });
    m_undoItem->setEnabled(false);
    m_undoItem->setPosition(Vec2((float)(W - 120), (float)(HH - 60)));

    Menu* menu = Menu::create(m_undoItem, m_drawItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    addChild(menu, 100);
}

void GameScene::SetupController() {
    m_controller.SetBoardRefreshCB([this]() {
        if (m_sceneAlive) OnBoardChanged();
    });
    m_controller.SetCardMovedCB(
        [this](int id, float x, float y) {
            if (m_sceneAlive) OnCardMoved(id, x, y);
        });
    m_controller.SetHistoryCB(
        [this](int cnt) {
            if (m_sceneAlive) OnHistoryChanged(cnt);
        });

    m_controller.SetHandSwitchAnimCB(
        [this](int cardId, std::function<void()> done) {
            if (m_sceneAlive) OnHandSwitchAnim(cardId, done);
            else if (done) done();
        });

    m_controller.SetUndoAnimCB(
        [this](int cardId, int destCol, int destDepth, bool toTable,
                std::function<void()> done) {
            if (m_sceneAlive) OnUndoAnim(cardId, destCol, destDepth, toTable, done);
            else if (done) done();
        });

    m_controller.SetDrawAnimCB(
        [this](int newCardId, std::function<void()> done) {
            if (m_sceneAlive) OnDrawAnim(newCardId, done);
            else if (done) done();
        });

    m_tableArea->SetClickCB(
        [this](int cardId) { m_controller.OnTableCardClicked(cardId); });
    m_handArea->SetClickCB(
        [this](int cardId) { m_controller.OnHandCardClicked(cardId); });
}

void GameScene::OnBoardChanged() {
    m_tableArea->RefreshFromBoard(m_controller.GetBoard());
    m_handArea->RefreshFromBoard(m_controller.GetBoard());
    m_handArea->UpdateDrawPile(m_controller.GetBoard().ReserveCount());
    if (m_undoItem) m_undoItem->setEnabled(m_controller.CanUndo() && !m_animating);

    // Update draw count label (reuse instead of recreate)
    int cnt = m_controller.GetBoard().ReserveCount();
    if (cnt > 0) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", cnt);
        if (!m_drawCountLbl) {
            m_drawCountLbl = Label::createWithSystemFont(buf, "Arial", 36);
            m_drawCountLbl->setColor(Color3B(255, 220, 80));
            m_drawCountLbl->setPosition(Vec2(95, 50));
            addChild(m_drawCountLbl, 200);
        } else if (cnt != m_lastDrawCount) {
            m_drawCountLbl->setString(buf);
        }
    } else if (m_drawCountLbl) {
        m_drawCountLbl->removeFromParent();
        m_drawCountLbl = nullptr;
    }
    m_lastDrawCount = cnt;
}

void GameScene::OnCardMoved(int cardId, float /*x*/, float /*y*/) {
    m_animating = true;
    CardSprite* sprite = m_tableArea->GetCardSprite(cardId);
    if (sprite) {
        int newTotal = m_controller.GetBoard().HandCardCount();
        Vec2 target = m_handArea->CalcWorldPos(newTotal - 1, newTotal);
        sprite->PlayMatchEffect([this, sprite, target]() {
            if (!m_sceneAlive) return;
            sprite->MoveToPosition(target, 0.2f, [this]() {
                m_animating = false;
                if (!m_sceneAlive) return;
                OnBoardChanged();
            });
        });
    } else {
        m_animating = false;
        OnBoardChanged();
    }
}

void GameScene::OnHistoryChanged(int count) {
    if (m_undoItem) m_undoItem->setEnabled(count > 0);
}

void GameScene::OnHandSwitchAnim(int cardId, std::function<void()> done) {
    m_animating = true;
    CardSprite* sprite = m_handArea->GetCardSprite(cardId);
    if (sprite) {
        int total = m_controller.GetBoard().HandCardCount();
        Vec2 target = m_handArea->CalcWorldPos(total - 1, total);
        sprite->MoveToPosition(target, 0.2f, [this, done]() {
            m_animating = false;
            if (done) done();
        });
    } else {
        m_animating = false;
        if (done) done();
    }
}

void GameScene::OnDrawAnim(int newCardId, std::function<void()> done) {
    m_animating = true;
    const auto& board = m_controller.GetBoard();
    if (board.HandCardCount() == 0) {
        m_animating = false;
        if (done) done();
        return;
    }
    CardData tmpCard = board.HandTopCard();
    CardSprite* tempSprite = CardSprite::CreateWithCard(tmpCard, true);
    if (tempSprite) {
        tempSprite->retain();  // keep alive across async animation
        Vec2 startPos = m_handArea->GetDrawPileWorldPos();
        tempSprite->setPosition(startPos);
        tempSprite->setScale(1.0f);
        addChild(tempSprite, 200);

        int total = board.HandCardCount();
        Vec2 target = m_handArea->CalcWorldPos(total - 1, total);

        auto* spin = RotateBy::create(0.3f, 360.0f);
        auto* scaleUp = ScaleTo::create(0.2f, 1.0f);
        auto* move = MoveTo::create(0.3f, target);
        auto* flyIn = Spawn::create(spin, scaleUp, move, nullptr);
        auto* seq = Sequence::create(
            flyIn,
            CallFunc::create([this, tempSprite, done]() {
                tempSprite->removeFromParent();
                tempSprite->release();
                m_animating = false;
                if (done) done();
            }),
            nullptr);
        tempSprite->runAction(seq);
    } else {
        m_animating = false;
        if (done) done();
    }
}

void GameScene::OnUndoAnim(int cardId, int destCol, int destDepth, bool toTable,
                           std::function<void()> done) {
    m_animating = true;
    CardSprite* sprite = m_handArea->GetCardSprite(cardId);
    if (!sprite) {
        m_animating = false;
        if (done) done();
        return;
    }

    Vec2 target;
    const auto& board = m_controller.GetBoard();
    bool isDrawUndo = (!toTable && destCol == 0 && destDepth == 0
                       && board.ReserveCount() >= 0);

    if (toTable) {
        // TableToHand undo: fly back to table column
        target = m_tableArea->CalcCardPosition(
            destCol, destDepth,
            board.ColumnCount(),
            board.CardsInColumn(destCol) + 1);
        target = m_tableArea->convertToWorldSpace(target);
        sprite->MoveToPosition(target, 0.2f, [this, done]() {
            m_animating = false;
            if (done) done();
        });

    } else if (isDrawUndo) {
        // Draw undo: shrink + spin back to draw pile
        Vec2 pilePos = m_handArea->GetDrawPileWorldPos();
        auto* spin = RotateBy::create(0.25f, -360.0f);
        auto* move = MoveTo::create(0.25f, pilePos);
        auto* flyBack = Spawn::create(spin, move, nullptr);
        sprite->runAction(Sequence::create(
            flyBack,
            CallFunc::create([this, done]() {
                m_animating = false;
                if (done) done();
            }),
            nullptr));

    } else {
        // HandSwitch undo: slide to original hand index
        float yBase = 282.0f / 2.0f + 60.0f;
        float x = 200.0f + static_cast<float>(destCol) * 80.0f;
        target = m_handArea->convertToWorldSpace(Vec2(x, yBase));
        sprite->MoveToPosition(target, 0.2f, [this, done]() {
            m_animating = false;
            if (done) done();
        });
    }
}

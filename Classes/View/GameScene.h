#pragma once

#include "cocos2d.h"
#include "Controller/GameController.h"
#include "View/TableAreaView.h"
#include "View/HandAreaView.h"

class GameScene : public cocos2d::Scene {
public:
    static GameScene* CreateScene();
    virtual bool init() override;
    virtual void onExit() override;
    CREATE_FUNC(GameScene);

private:
    GameScene() = default;

    void SetupUI();
    void SetupController();
    void OnBoardChanged();
    void OnCardMoved(int cardId, float x, float y);
    void OnHandSwitchAnim(int cardId, std::function<void()> done);
    void OnUndoAnim(int cardId, int destCol, int destDepth, bool toTable,
                    std::function<void()> done);
    void OnDrawAnim(int newCardId, std::function<void()> done);
    void OnHistoryChanged(int count);

    GameController          m_controller;
    TableAreaView*          m_tableArea = nullptr;
    HandAreaView*           m_handArea  = nullptr;
    cocos2d::MenuItemLabel* m_undoItem  = nullptr;
    cocos2d::MenuItemLabel* m_drawItem  = nullptr;
    cocos2d::Node*          m_sceneAlive = nullptr;  // guard for async callbacks
    bool m_animating = false;
    cocos2d::Label* m_drawCountLbl = nullptr;
    int m_lastDrawCount = -1;

    enum { W = 1080, H = 2080, TH = 1500, HH = 580 };
};

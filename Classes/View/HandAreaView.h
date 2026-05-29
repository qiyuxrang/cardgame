#pragma once

#include "cocos2d.h"
#include "Model/GameBoard.h"
#include "View/CardSprite.h"
#include <map>
#include <functional>

class HandAreaView : public cocos2d::Node {
public:
    static HandAreaView* Create(const cocos2d::Size& size);

    void RefreshFromBoard(const GameBoard& board);
    CardSprite* GetCardSprite(int cardId);
    cocos2d::Vec2 GetTopCardWorldPos() const;
    const std::map<int, CardSprite*>& GetAllCards() const { return m_sprites; }

    cocos2d::Vec2 CalcWorldPos(int index, int total) const;
    cocos2d::Vec2 CalcLocalPos(int index) const;  // index in hand, local coords

    // Draw pile
    void UpdateDrawPile(int reserveCount);
    cocos2d::Vec2 GetDrawPileWorldPos() const;

    using ClickCB = std::function<void(int handIndex)>;
    void SetClickCB(ClickCB cb) { m_clickCB = std::move(cb); }
    using DrawPileClickCB = std::function<void()>;
    void SetDrawPileClickCB(DrawPileClickCB cb) { m_drawPileCB = std::move(cb); }

    enum { CARD_W = 182, CARD_H = 282, TOP_UP = 35, SPACING = 15 };

private:
    HandAreaView() = default;
    bool InitWithSize(const cocos2d::Size& size);
    cocos2d::Vec2 CalcPos(int index, int total, bool isTop,
                          float yBase) const;

    cocos2d::Size m_size;
    ClickCB m_clickCB;
    DrawPileClickCB m_drawPileCB;
    std::map<int, CardSprite*> m_sprites;
    std::map<int, cocos2d::Vec2> m_positions;
    cocos2d::Node* m_drawPile = nullptr;
};

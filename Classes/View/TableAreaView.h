#pragma once

#include "cocos2d.h"
#include "Model/GameBoard.h"
#include "View/CardSprite.h"
#include <map>
#include <functional>

class TableAreaView : public cocos2d::Node {
public:
    static TableAreaView* Create(const cocos2d::Size& size);

    void RefreshFromBoard(const GameBoard& board);
    CardSprite* GetCardSprite(int cardId);
    cocos2d::Vec2 GetCardWorldPos(int cardId) const;
    const std::map<int, CardSprite*>& GetAllCards() const { return m_sprites; }

    using ClickCB = std::function<void(int cardId)>;
    void SetClickCB(ClickCB cb) { m_clickCB = std::move(cb); }

    // Public for undo animation: calculate card position by col/depth
    cocos2d::Vec2 CalcCardPosition(int col, int depth, int totalCols,
                                   int cardsInCol) const {
        return CalcPos(col, depth, totalCols, cardsInCol);
    }

    enum { CARD_W = 182, CARD_H = 282, SCALE = 100, STACK_Y = 50, PAD_H = 30, PAD_TOP = 80 };

private:
    TableAreaView() = default;
    bool InitWithSize(const cocos2d::Size& size);
    cocos2d::Vec2 CalcPos(int col, int depth, int totalCols,
                          int cardsInCol) const;

    cocos2d::Size m_size;
    ClickCB m_clickCB;
    std::map<int, CardSprite*> m_sprites;
    std::map<int, cocos2d::Vec2> m_positions;
};

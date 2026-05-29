#include "TableAreaView.h"
#include <set>

USING_NS_CC;

TableAreaView* TableAreaView::Create(const Size& size) {
    TableAreaView* v = new (std::nothrow) TableAreaView();
    if (v && v->InitWithSize(size)) { v->autorelease(); return v; }
    CC_SAFE_DELETE(v);
    return nullptr;
}

bool TableAreaView::InitWithSize(const Size& size) {
    if (!Node::init()) return false;
    m_size = size;
    setContentSize(size);
    return true;
}

void TableAreaView::RefreshFromBoard(const GameBoard& board) {
    int totalCols = board.ColumnCount();
    std::set<int> validIds;

    for (int col = 0; col < totalCols; ++col) {
        int n = board.CardsInColumn(col);
        for (int d = 0; d < n; ++d) {
            const CardData& card = board.CardAt(col, d);
            validIds.insert(card.cardId);
            Vec2 pos = CalcPos(col, d, totalCols, n);

            std::map<int, CardSprite*>::iterator it =
                m_sprites.find(card.cardId);
            if (it != m_sprites.end()) {
                bool wasDown = !it->second->IsFaceUp();
                it->second->UpdateCard(card);
                it->second->SetInteractive(card.faceUp);
                Vec2 old = m_positions[card.cardId];
                if (old != pos)
                    it->second->MoveToPosition(pos, 0.15f);
                // Play flip animation when revealed
                if (wasDown && card.faceUp) {
                    it->second->PlayFlipAnim();
                }
            } else {
                CardSprite* sprite = CardSprite::CreateWithCard(card, true);
                if (sprite) {
                    sprite->setPosition(pos);
                    sprite->setScale((float)SCALE / 100.0f);
                    sprite->SetClickCallback([this](int id) {
                        if (m_clickCB) m_clickCB(id);
                    });
                    addChild(sprite, d + 10);
                    sprite->SetInteractive(card.faceUp);
                    m_sprites[card.cardId] = sprite;
                }
            }
            m_positions[card.cardId] = pos;
        }
    }

    // Remove cards no longer in the board
    std::map<int, CardSprite*>::iterator it = m_sprites.begin();
    while (it != m_sprites.end()) {
        if (validIds.find(it->first) == validIds.end()) {
            it->second->removeFromParent();
            m_positions.erase(it->first);
            it = m_sprites.erase(it);
        } else {
            ++it;
        }
    }
}

Vec2 TableAreaView::CalcPos(int col, int depth, int totalCols,
                            int cardsInCol) const {
    float gap = 20.0f;
    float spacing = CARD_W + gap;
    float totalW = (totalCols - 1) * spacing + CARD_W;
    float startX = (m_size.width - totalW) / 2.0f + CARD_W / 2.0f;
    float x = startX + col * spacing;

    // Arch: middle columns' top cards rise higher
    int midCol = totalCols / 2;
    float archRise = (float)(midCol - abs(col - midCol)) * 40.0f;

    float y = m_size.height - PAD_TOP - CARD_H - depth * STACK_Y + archRise;

    return Vec2(x, y);
}

CardSprite* TableAreaView::GetCardSprite(int cardId) {
    std::map<int, CardSprite*>::iterator it = m_sprites.find(cardId);
    return (it != m_sprites.end()) ? it->second : nullptr;
}

Vec2 TableAreaView::GetCardWorldPos(int cardId) const {
    std::map<int, Vec2>::const_iterator it = m_positions.find(cardId);
    return (it != m_positions.end()) ?
        convertToWorldSpace(it->second) : Vec2::ZERO;
}

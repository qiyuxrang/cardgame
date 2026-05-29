#include "HandAreaView.h"
#include <set>

USING_NS_CC;

HandAreaView* HandAreaView::Create(const Size& size) {
    HandAreaView* v = new (std::nothrow) HandAreaView();
    if (v && v->InitWithSize(size)) { v->autorelease(); return v; }
    CC_SAFE_DELETE(v);
    return nullptr;
}

bool HandAreaView::InitWithSize(const Size& size) {
    if (!Node::init()) return false;
    m_size = size;
    setContentSize(size);
    return true;
}

void HandAreaView::RefreshFromBoard(const GameBoard& board) {
    int total = board.HandCardCount();
    std::set<int> validIds;
    float yBase = CARD_H / 2.0f + 60.0f;

    for (int i = 0; i < total; ++i) {
        const CardData& card = board.HandCardAt(i);
        validIds.insert(card.cardId);
        bool isTop = (i == total - 1);
        Vec2 pos = CalcPos(i, total, isTop, yBase);

        std::map<int, CardSprite*>::iterator it = m_sprites.find(card.cardId);
        if (it != m_sprites.end()) {
            it->second->UpdateCard(card);
            it->second->SetInteractive(true);
            it->second->setLocalZOrder(i + 10);  // update z-order for new position
            if (m_positions[card.cardId] != pos)
                it->second->MoveToPosition(pos, 0.15f);
        } else {
            CardSprite* sprite = CardSprite::CreateWithCard(card, true);
            if (sprite) {
                sprite->setPosition(pos);
                int cid = card.cardId;
                sprite->SetClickCallback([cid, this](int /*id*/) {
                    if (m_clickCB) m_clickCB(cid);
                });
                addChild(sprite, i + 10);  // higher index = higher z-order
                sprite->SetInteractive(true);
                m_sprites[card.cardId] = sprite;
            }
        }
        m_positions[card.cardId] = pos;
    }

    // Cleanup
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

Vec2 HandAreaView::CalcPos(int index, int total, bool isTop,
                           float yBase) const {
    // Smooth transition: leave room for draw pile on left
    float leftMargin = 200.0f;
    float rightMargin = 30.0f;
    float maxW = m_size.width - leftMargin - rightMargin;
    float idealSpacing = CARD_W + SPACING;  // 197px, cards fully visible
    float minReveal = 55.0f;               // tightest: 55px visible per card

    float spacing = idealSpacing;
    if (total > 1) {
        float needSpacing = (maxW - CARD_W) / (float)(total - 1);
        if (needSpacing < idealSpacing)
            spacing = needSpacing > minReveal ? needSpacing : minReveal;
    }
    // Top card (last index) slightly raised ~1cm above others
    float y = isTop ? yBase + 38.0f : yBase;
    float totalW = CARD_W + (total - 1) * spacing;
    float startX = leftMargin + (maxW - totalW) / 2.0f + CARD_W / 2.0f;
    return Vec2(startX + index * spacing, y);
}

CardSprite* HandAreaView::GetCardSprite(int cardId) {
    std::map<int, CardSprite*>::iterator it = m_sprites.find(cardId);
    return (it != m_sprites.end()) ? it->second : nullptr;
}

Vec2 HandAreaView::CalcWorldPos(int index, int total) const {
    float yBase = CARD_H / 2.0f + 60.0f;
    bool isTop = (index == total - 1);
    Vec2 local = CalcPos(index, total, isTop, yBase);
    return convertToWorldSpace(local);
}

void HandAreaView::UpdateDrawPile(int reserveCount) {
    if (!m_drawPile) {
        m_drawPile = Node::create();
        addChild(m_drawPile, 5);
    }
    m_drawPile->removeAllChildren();

    if (reserveCount <= 0) {
        m_drawPile->setVisible(false);
        return;
    }
    m_drawPile->setVisible(true);

    // Draw pile at left edge
    float pileX = 95.0f;  // center of card at x=95
    float pileY = CARD_H / 2.0f + 60.0f;
    m_drawPile->setPosition(Vec2(pileX, pileY));

    // Horizontal overlap — same style, same size as hand cards
    int showCount = reserveCount > 3 ? 3 : reserveCount;
    float reveal = 12.0f;
    for (int i = 0; i < showCount; ++i) {
        Sprite* s = Sprite::create(CardSprite::ResourcePath("card_general.png"));
        if (s) {
            s->setScale(1.0f);
            s->setColor(Color3B(60, 80, 140));
            // Stacked left-to-right with minimal reveal
            s->setPosition(Vec2(i * reveal, 0));
            m_drawPile->addChild(s, i);
        }
    }

    // Count is displayed by GameScene
}

Vec2 HandAreaView::GetDrawPileWorldPos() const {
    if (m_drawPile)
        return m_drawPile->convertToWorldSpace(Vec2::ZERO);
    return Vec2(95.0f, CARD_H / 2.0f + 60.0f);
}

Vec2 HandAreaView::GetTopCardWorldPos() const {
    if (m_positions.empty()) return Vec2::ZERO;
    std::map<int, Vec2>::const_iterator it = m_positions.end();
    --it;
    return convertToWorldSpace(it->second);
}

#include "CardSprite.h"

USING_NS_CC;

CardSprite* CardSprite::CreateWithCard(const CardData& card, bool useBig) {
    CardSprite* s = new (std::nothrow) CardSprite();
    if (s && s->InitWithCard(card, useBig)) { s->autorelease(); return s; }
    CC_SAFE_DELETE(s);
    CCLOG("CardSprite: CreateWithCard failed for id=%d", card.cardId);
    return nullptr;
}

bool CardSprite::InitWithCard(const CardData& card, bool useBig) {
    if (!Node::init()) return false;
    m_cardId = card.cardId;
    m_faceUp = card.faceUp;
    m_useBig = useBig;
    m_suit   = card.suit;
    m_rank   = card.rank;
    m_cardSize = Size(W, H);
    setContentSize(m_cardSize);
    setAnchorPoint(Vec2(0.5f, 0.5f));

    if (m_faceUp) BuildFaceUpSprites();
    else          BuildFaceDownSprite();

    return true;
}

Sprite* CardSprite::CreateBackground() const {
    Sprite* bg = Sprite::create(ResourcePath("card_general.png"));
    if (bg) {
        bg->setPosition(Vec2(W / 2, H / 2));
        bg->setScale(W / 182.0f, H / 282.0f);
    } else {
        CCLOG("CardSprite: card_general.png not found");
    }
    return bg;
}

void CardSprite::BuildFaceUpSprites() {
    m_bg = CreateBackground();
    if (m_bg) addChild(m_bg, 0);

    // Use CardData::RankName() (single source of truth) instead of duplicating
    std::string numFile = ResourcePath("number/"
        + std::string(m_useBig ? "big" : "small") + "_"
        + std::string(m_suit == Suit::Heart || m_suit == Suit::Diamond
            ? "red" : "black") + "_"
        + std::string(CardData::RankName(m_rank))
        + ".png");

    m_number = Sprite::create(numFile);
    if (m_number) {
        m_number->setAnchorPoint(Vec2(0, 1));
        m_number->setPosition(Vec2(NUM_X, H - NUM_Y));
        m_number->setScale(0.6f);
        addChild(m_number, 1);
    } else {
        CCLOG("CardSprite: number sprite not found: %s", numFile.c_str());
    }

    static const char* suitFiles[] =
        {"spade.png", "heart.png", "club.png", "diamond.png"};
    m_suitIcon = Sprite::create(
        ResourcePath("suits/" + std::string(suitFiles[(int)m_suit])));
    if (m_suitIcon) {
        m_suitIcon->setAnchorPoint(Vec2(0, 1));
        float sy = H - NUM_Y
            - (m_number ? m_number->getContentSize().height * 0.6f : 0) - 2;
        m_suitIcon->setPosition(Vec2(NUM_X + 5.0f, sy));
        m_suitIcon->setScale(0.55f);
        addChild(m_suitIcon, 1);
    } else {
        CCLOG("CardSprite: suit icon not found for suit=%d", (int)m_suit);
    }
}

void CardSprite::BuildFaceDownSprite() {
    m_bg = CreateBackground();
    if (m_bg) {
        m_bg->setColor(Color3B(60, 80, 140));
        addChild(m_bg, 0);
    }
}

void CardSprite::UpdateCard(const CardData& card) {
    stopAllActions();
    setScale(1.0f);
    setRotation(0.0f);
    removeAllChildren();
    m_bg = m_number = m_suitIcon = nullptr;
    m_cardId = card.cardId;
    m_faceUp = card.faceUp;
    m_suit   = card.suit;
    m_rank   = card.rank;
    if (m_faceUp) BuildFaceUpSprites();
    else          BuildFaceDownSprite();
}

void CardSprite::SetInteractive(bool enabled) {
    m_interactive = enabled;
}

bool CardSprite::HitTest(const Vec2& worldPos) const {
    if (!m_faceUp || !m_interactive) return false;
    Vec2 localPos = convertToNodeSpace(worldPos);
    return Rect(0, 0, m_cardSize.width, m_cardSize.height)
        .containsPoint(localPos);
}

void CardSprite::MoveToPosition(const Vec2& target, float dur,
                                std::function<void()> onComplete) {
    stopAllActions();
    setScale(1.0f);
    MoveTo* mv = MoveTo::create(dur, target);
    if (onComplete) {
        runAction(Sequence::create(mv,
            CallFunc::create(onComplete), nullptr));
    } else {
        runAction(mv);
    }
}

void CardSprite::PlayFlipAnim(std::function<void()> onComplete) {
    Sequence* seq = Sequence::create(
        ScaleTo::create(0.07f, 0.0f, 1.0f),
        CallFunc::create([this]() {
            if (!m_faceUp) {
                m_faceUp = true;
                removeAllChildren();
                m_bg = m_number = m_suitIcon = nullptr;
                BuildFaceUpSprites();
            }
        }),
        ScaleTo::create(0.07f, 1.0f, 1.0f),
        CallFunc::create(onComplete ? onComplete : [](){}),
        nullptr);
    runAction(seq);
}

void CardSprite::PlayMatchEffect(std::function<void()> onComplete) {
    Sequence* seq = Sequence::create(
        ScaleTo::create(0.06f, 1.2f),
        EaseBackOut::create(ScaleTo::create(0.1f, 0.9f)),
        ScaleTo::create(0.15f, 1.0f),
        CallFunc::create(onComplete ? onComplete : [](){}),
        nullptr);
    runAction(seq);
}

#pragma once

#include "cocos2d.h"
#include "Model/CardTypes.h"
#include <functional>

class CardSprite : public cocos2d::Node {
public:
    static CardSprite* CreateWithCard(const CardData& card, bool useBig = true);

    void UpdateCard(const CardData& card);
    void SetInteractive(bool enabled);
    void MoveToPosition(const cocos2d::Vec2& target, float duration,
                        std::function<void()> onComplete = nullptr);
    void PlayMatchEffect(std::function<void()> onComplete = nullptr);
    void PlayFlipAnim(std::function<void()> onComplete = nullptr);
    bool WasFaceDown() const { return m_wasFaceDown; }
    void MarkFlipped() { m_wasFaceDown = false; }

    int  GetCardId() const { return m_cardId; }
    bool IsFaceUp()   const { return m_faceUp; }
    const cocos2d::Size& CardSize() const { return m_cardSize; }

    // Hit-test in world coordinates
    bool HitTest(const cocos2d::Vec2& worldPos) const;

    using ClickCallback = std::function<void(int cardId)>;
    void SetClickCallback(ClickCallback cb) { m_clickCB = std::move(cb); }
    void InvokeClick() { if (m_clickCB) m_clickCB(m_cardId); }

    static std::string ResourcePath(const std::string& filename) {
        return "res/" + filename;
    }

private:
    CardSprite() = default;
    bool InitWithCard(const CardData& card, bool useBig);
    cocos2d::Sprite* CreateBackground() const;
    void BuildFaceUpSprites();
    void BuildFaceDownSprite();

    cocos2d::Sprite* m_bg = nullptr;
    cocos2d::Sprite* m_number = nullptr;
    cocos2d::Sprite* m_suitIcon = nullptr;

    int  m_cardId = -1;
    bool m_faceUp = false;
    bool m_interactive = false;
    bool m_wasFaceDown = false;
    bool m_useBig = true;
    Suit m_suit   = Suit::Spade;
    Rank m_rank   = 1;
    cocos2d::Size m_cardSize;
    ClickCallback m_clickCB;

    static constexpr float W = 182.0f;
    static constexpr float H = 282.0f;
    static constexpr float NUM_X = 10.0f;
    static constexpr float NUM_Y = 5.0f;
};

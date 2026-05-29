#pragma once

#include "CardTypes.h"
#include <vector>
#include <deque>
#include <cassert>

class GameBoard {
public:
    // --- Table area ---
    int  ColumnCount() const { return (int)tableColumns.size(); }
    bool IsValidColumn(int col) const { return col >= 0 && col < ColumnCount(); }

    int  CardsInColumn(int col) const {
        assert(IsValidColumn(col));
        return (int)tableColumns[col].size();
    }
    bool IsColumnEmpty(int col) const {
        assert(IsValidColumn(col));
        return tableColumns[col].empty();
    }
    bool IsCardTopOfColumn(int col, int cardId) const;

    const CardData& CardAt(int col, int depth) const {
        assert(IsValidColumn(col));
        assert(depth >= 0 && depth < (int)tableColumns[col].size());
        return tableColumns[col][depth];
    }

    CardData PopTopCard(int col);
    void     PushCard(int col, const CardData& card);
    void     SetCardFaceUp(int col, int depth, bool faceUp);
    void     AddColumn(const std::vector<CardData>& cards);

    // --- Hand area (back=top card) ---
    int  HandCardCount() const { return static_cast<int>(handCards.size()); }
    bool IsHandEmpty() const { return handCards.empty(); }
    bool IsValidHandIndex(int i) const { return i >= 0 && i < HandCardCount(); }

    const CardData& HandTopCard() const {
        assert(!IsHandEmpty());
        return handCards.back();
    }
    const CardData& HandCardAt(int i) const {
        assert(IsValidHandIndex(i));
        return handCards[i];
    }

    void     SetTopCardIndex(int index);
    void     PushHandCard(const CardData& card);
    void     InsertHandCard(int index, const CardData& card);
    CardData PopHandCard(int index);

    // --- Reserve deck ---
    int      ReserveCount() const { return reserveCount; }
    void     SetReserveCount(int n) { reserveCount = n; }
    CardData DrawFromReserve();

    // --- Query ---
    bool FindCard(int cardId, CardLocation& outLoc) const;

private:
    // Non-const access for internal mutation only
    CardData& CardAt(int col, int depth) {
        assert(IsValidColumn(col));
        assert(depth >= 0 && depth < (int)tableColumns[col].size());
        return tableColumns[col][depth];
    }
    CardData& HandTopCard() {
        assert(!IsHandEmpty());
        return handCards.back();
    }

    std::vector<std::deque<CardData>> tableColumns;
    std::vector<CardData>             handCards;
    int reserveCount = 0;
};

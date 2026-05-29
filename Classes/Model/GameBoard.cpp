#include "GameBoard.h"
#include <random>

namespace {
    std::mt19937& Rng() {
        static std::mt19937 rng(std::random_device{}());
        return rng;
    }
}

bool GameBoard::IsCardTopOfColumn(int col, int cardId) const {
    if (!IsValidColumn(col) || IsColumnEmpty(col)) return false;
    return tableColumns[col].back().cardId == cardId;
}

CardData GameBoard::PopTopCard(int col) {
    assert(IsValidColumn(col));
    assert(!IsColumnEmpty(col));
    CardData card = tableColumns[col].back();
    tableColumns[col].pop_back();
    return card;
}

void GameBoard::PushCard(int col, const CardData& card) {
    assert(IsValidColumn(col));
    tableColumns[col].push_back(card);
}

void GameBoard::SetCardFaceUp(int col, int depth, bool faceUp) {
    assert(IsValidColumn(col));
    assert(depth >= 0 && depth < (int)tableColumns[col].size());
    tableColumns[col][depth].faceUp = faceUp;
}

void GameBoard::AddColumn(const std::vector<CardData>& cards) {
    tableColumns.emplace_back(cards.begin(), cards.end());
}

void GameBoard::SetTopCardIndex(int index) {
    if (!IsValidHandIndex(index)) return;
    CardData selected = handCards[index];
    handCards.erase(handCards.begin() + index);
    handCards.push_back(selected);
}

void GameBoard::PushHandCard(const CardData& card) {
    handCards.push_back(card);
}

void GameBoard::InsertHandCard(int index, const CardData& card) {
    if (index < 0 || index > (int)handCards.size()) return;
    handCards.insert(handCards.begin() + index, card);
}

CardData GameBoard::PopHandCard(int index) {
    assert(IsValidHandIndex(index));
    CardData card = handCards[index];
    handCards.erase(handCards.begin() + index);
    return card;
}

CardData GameBoard::DrawFromReserve() {
    assert(reserveCount > 0);
    CardData card;
    card.cardId = CardData::GenerateId();
    card.suit   = static_cast<Suit>(std::uniform_int_distribution<int>(0, 3)(Rng()));
    card.rank   = std::uniform_int_distribution<int>(1, 13)(Rng());
    card.faceUp = true;
    reserveCount--;
    return card;
}

bool GameBoard::FindCard(int cardId, CardLocation& outLoc) const {
    for (int col = 0; col < ColumnCount(); ++col) {
        for (int d = 0; d < CardsInColumn(col); ++d) {
            if (CardAt(col, d).cardId == cardId) {
                outLoc.area = CardArea::Table;
                outLoc.columnIndex = col;
                outLoc.stackDepth = d;
                return true;
            }
        }
    }
    for (int i = 0; i < (int)handCards.size(); ++i) {
        if (handCards[i].cardId == cardId) {
            outLoc.area = CardArea::Hand;
            outLoc.columnIndex = i;
            outLoc.stackDepth = 0;
            return true;
        }
    }
    return false;
}

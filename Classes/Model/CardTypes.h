#pragma once

#include <string>

enum class Suit { Spade = 0, Heart = 1, Club = 2, Diamond = 3 };
using Rank = int;  // A=1..13

enum class CardArea { Table, Hand, Reserve };

struct CardData {
    int  cardId;
    Suit suit;
    Rank rank;
    bool faceUp;

    static int GenerateId() { static int s = 1000; return s++; }
    bool IsRed() const { return suit == Suit::Heart || suit == Suit::Diamond; }
    static const char* RankName(Rank rank);
    const char* RankName() const { return RankName(rank); }
    const char* SuitName() const;
    const char* ColorName() const;
};

struct CardLocation {
    CardArea area;
    int columnIndex;
    int stackDepth;
};

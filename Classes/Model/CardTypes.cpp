#include "CardTypes.h"
#include <cassert>

const char* CardData::SuitName() const {
    switch (suit) {
        case Suit::Spade:   return "spade";
        case Suit::Heart:   return "heart";
        case Suit::Club:    return "club";
        case Suit::Diamond: return "diamond";
    }
    assert(false);
    return "?";
}

const char* CardData::ColorName() const {
    return IsRed() ? "red" : "black";
}

const char* CardData::RankName(Rank rank) {
    static const char* names[] = {
        "", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"
    };
    assert(rank >= 1 && rank <= 13);
    return names[rank];
}

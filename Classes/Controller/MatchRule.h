#pragma once

#include "Model/CardTypes.h"

// Match rule strategy interface
class IMatchRule {
public:
    virtual ~IMatchRule() = default;
    virtual bool CanMatch(const CardData& a, const CardData& b) const = 0;
    virtual const char* Name() const = 0;
};

// Adjacent rank match (diff=1, any suit)
class AdjacentRankRule : public IMatchRule {
public:
    bool CanMatch(const CardData& a, const CardData& b) const override {
        int diff = a.rank - b.rank;
        return diff == 1 || diff == -1;
    }
    const char* Name() const override { return "AdjacentRank"; }
};

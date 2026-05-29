#pragma once

#include "Model/IAction.h"
#include "Model/GameBoard.h"

// Table card matched -> move to hand area top
class TableToHandAction : public IAction {
public:
    TableToHandAction(GameBoard* board, int cardId);
    bool Execute() override;
    void Undo() override;
    const char* Description() const override { return "TableToHand"; }
    int  GetMovedCardId() const override { return m_cardId; }
    int  GetSourceCol()   const override { return m_srcCol; }
    int  GetSourceDepth() const override { return m_srcDepth; }
    bool IsTableSource()  const override { return true; }

private:
    GameBoard* m_board;
    int m_cardId, m_srcCol, m_srcDepth;
    CardData m_movedCard;
};

// Draw from reserve pile to hand
class DrawReserveAction : public IAction {
public:
    DrawReserveAction(GameBoard* board, int cardId);
    bool Execute() override;
    void Undo() override;
    const char* Description() const override { return "DrawReserve"; }
    int  GetMovedCardId() const override { return m_cardId; }
    int  GetSourceCol()   const override { return 0; }
    int  GetSourceDepth() const override { return 0; }
    bool IsTableSource()  const override { return false; }

private:
    GameBoard* m_board;
    int m_cardId;
    int m_handIndex = -1;  // stored at construction (card was just pushed to hand end)
};

// Hand area internal top-card switch
class HandSwitchAction : public IAction {
public:
    HandSwitchAction(GameBoard* board, int index);
    bool Execute() override;
    void Undo() override;
    const char* Description() const override { return "HandSwitch"; }
    int  GetMovedCardId() const override { return m_cardId; }
    int  GetSourceCol()   const override { return m_fromIndex; }
    int  GetSourceDepth() const override { return 0; }
    bool IsTableSource()  const override { return false; }

private:
    GameBoard* m_board;
    int m_cardId, m_fromIndex, m_oldTopIndex;
};

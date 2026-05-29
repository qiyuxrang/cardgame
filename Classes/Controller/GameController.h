#pragma once

#include "Model/GameBoard.h"
#include "Controller/MatchRule.h"
#include "Controller/Actions.h"
#include "Controller/UndoManager.h"
#include <functional>
#include <memory>

class GameController {
public:
    using CardMovedCB    = std::function<void(int cardId, float toX, float toY)>;
    using BoardRefreshCB = std::function<void()>;
    using HistoryCB      = std::function<void(int count)>;
    // Animation callbacks: call done() after animation completes
    using HandSwitchAnimCB = std::function<void(int cardId, std::function<void()> done)>;
    using UndoAnimCB = std::function<void(int cardId, int destCol, int destDepth,
                                          bool toTable, std::function<void()> done)>;
    using DrawAnimCB = std::function<void(int newCardId, std::function<void()> done)>;

    GameController();
    ~GameController();

    void SetupDefaultBoard();

    bool OnTableCardClicked(int cardId);
    bool OnHandCardClicked(int cardId);
    bool Undo();
    bool DrawFromReserve();

    const GameBoard& GetBoard() const  { return m_board; }
    bool CanUndo() const               { return m_undoManager.CanUndo(); }
    void SetMatchRule(IMatchRule* rule);

    void SetCardMovedCB(CardMovedCB cb)         { m_cardMovedCB = std::move(cb); }
    void SetBoardRefreshCB(BoardRefreshCB cb)   { m_boardRefreshCB = std::move(cb); }
    void SetHistoryCB(HistoryCB cb)             { m_historyCB = std::move(cb); }
    void SetHandSwitchAnimCB(HandSwitchAnimCB cb) { m_handSwitchAnimCB = std::move(cb); }
    void SetUndoAnimCB(UndoAnimCB cb)           { m_undoAnimCB = std::move(cb); }
    void SetDrawAnimCB(DrawAnimCB cb)           { m_drawAnimCB = std::move(cb); }
    void NotifyBoardRefresh()                   { if (m_boardRefreshCB) m_boardRefreshCB(); }

private:
    GameBoard                   m_board;
    UndoManager                 m_undoManager;
    std::unique_ptr<IMatchRule> m_matchRule;
    CardMovedCB    m_cardMovedCB;
    BoardRefreshCB m_boardRefreshCB;
    HistoryCB      m_historyCB;
    HandSwitchAnimCB m_handSwitchAnimCB;
    UndoAnimCB     m_undoAnimCB;
    DrawAnimCB     m_drawAnimCB;
    const CardData& HandTop() const { return m_board.HandTopCard(); }
};

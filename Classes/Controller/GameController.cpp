#include "GameController.h"

GameController::GameController() {
    m_matchRule.reset(new AdjacentRankRule());
    m_undoManager.SetCallback([this](int count) {
        if (m_historyCB) m_historyCB(count);
    });
}

GameController::~GameController() = default;

void GameController::SetupDefaultBoard() {
    // Demo layout for: click D3 → click HA → click S2 → undo×3
    // Hand top = C4 (rank 4), so D3 (rank 3, diff=1) and S2 (rank 2, diff=1) match

    auto makeCard = [](Suit s, Rank r, bool up) -> CardData {
        CardData c;
        c.cardId = CardData::GenerateId();
        c.suit = s; c.rank = r; c.faceUp = up;
        return c;
    };

    // Table (5 columns arch: 2,3,4,3,2). Last card per column = faceUp
    // Step 1 target: D3 (rank 3) matches C4 (rank 4)
    m_board.AddColumn({makeCard(Suit::Club, 10, false),
                       makeCard(Suit::Diamond, 3, true)});
    // Step 3 target: S2 (rank 2) will match HA (rank 1) after step 2
    m_board.AddColumn({makeCard(Suit::Heart, 6, false),
                       makeCard(Suit::Spade, 5, false),
                       makeCard(Suit::Spade, 2, true)});
    // Non-matching fill
    m_board.AddColumn({makeCard(Suit::Diamond, 8, false),
                       makeCard(Suit::Club, 1, false),
                       makeCard(Suit::Heart, 11, false),
                       makeCard(Suit::Spade, 7, true)});
    // Non-matching fill
    m_board.AddColumn({makeCard(Suit::Spade, 13, false),
                       makeCard(Suit::Diamond, 12, false),
                       makeCard(Suit::Club, 4, true)});
    // Non-matching fill
    m_board.AddColumn({makeCard(Suit::Heart, 9, false),
                       makeCard(Suit::Diamond, 6, true)});

    // Hand: HA at index 0, C4 at top (index 4, rank 4)
    m_board.PushHandCard(makeCard(Suit::Heart, 1, true));    // HA (rank 1)
    m_board.PushHandCard(makeCard(Suit::Spade, 10, true));   // S10
    m_board.PushHandCard(makeCard(Suit::Diamond, 5, true));  // D5
    m_board.PushHandCard(makeCard(Suit::Heart, 13, true));   // HK
    m_board.PushHandCard(makeCard(Suit::Club, 4, true));     // C4 ← top (rank 4)

    m_board.SetReserveCount(12);
}

bool GameController::OnTableCardClicked(int cardId) {
    CardLocation loc;
    if (!m_board.FindCard(cardId, loc)) return false;
    if (loc.area != CardArea::Table) return false;
    if (!m_board.IsCardTopOfColumn(loc.columnIndex, cardId)) return false;

    const GameBoard& board = m_board;
    const CardData& clicked = board.CardAt(loc.columnIndex, loc.stackDepth);
    if (!m_matchRule->CanMatch(clicked, HandTop())) return false;

    auto action = std::make_unique<TableToHandAction>(&m_board, cardId);
    if (!m_undoManager.Execute(std::move(action))) return false;

    // Animation callback handles board refresh after move completes
    if (m_cardMovedCB) m_cardMovedCB(cardId, 0, 0);
    else NotifyBoardRefresh();
    return true;
}

bool GameController::OnHandCardClicked(int cardId) {
    int idx = -1;
    for (int i = 0; i < m_board.HandCardCount(); ++i) {
        if (m_board.HandCardAt(i).cardId == cardId) { idx = i; break; }
    }
    if (idx < 0) return false;
    // Already the top card
    if (idx == m_board.HandCardCount() - 1) return false;

    // Animate first, then execute action
    if (m_handSwitchAnimCB) {
        m_handSwitchAnimCB(cardId, [this, idx]() {
            m_undoManager.Execute(
                std::make_unique<HandSwitchAction>(&m_board, idx));
            NotifyBoardRefresh();
        });
    } else {
        auto action = std::make_unique<HandSwitchAction>(&m_board, idx);
        if (!m_undoManager.Execute(std::move(action))) return false;
        NotifyBoardRefresh();
    }
    return true;
}

bool GameController::Undo() {
    if (!m_undoManager.CanUndo()) return false;
    IAction* top = m_undoManager.PeekTop();

    if (m_undoAnimCB && top) {
        int cardId = top->GetMovedCardId();
        int srcCol = top->GetSourceCol();
        int srcDepth = top->GetSourceDepth();
        bool toTable = top->IsTableSource();
        m_undoAnimCB(cardId, srcCol, srcDepth, toTable, [this]() {
            m_undoManager.Undo();
            NotifyBoardRefresh();
        });
    } else {
        m_undoManager.Undo();
        NotifyBoardRefresh();
    }
    return true;
}

bool GameController::DrawFromReserve() {
    if (m_board.ReserveCount() <= 0) return false;
    CardData newCard = m_board.DrawFromReserve();
    int newCardId = newCard.cardId;
    m_board.PushHandCard(newCard);

    // Record for undo
    m_undoManager.Execute(
        std::make_unique<DrawReserveAction>(&m_board, newCardId));

    if (m_drawAnimCB) {
        m_drawAnimCB(newCardId, [this]() { NotifyBoardRefresh(); });
    } else {
        NotifyBoardRefresh();
    }
    return true;
}

void GameController::SetMatchRule(IMatchRule* rule) {
    m_matchRule.reset(rule);
}

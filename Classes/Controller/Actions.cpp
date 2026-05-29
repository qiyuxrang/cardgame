#include "Actions.h"

TableToHandAction::TableToHandAction(GameBoard* board, int cardId)
    : m_board(board), m_cardId(cardId), m_srcCol(-1), m_srcDepth(-1) {}

bool TableToHandAction::Execute() {
    CardLocation loc;
    if (!m_board->FindCard(m_cardId, loc)) return false;
    if (loc.area != CardArea::Table) return false;

    m_srcCol = loc.columnIndex;
    m_srcDepth = loc.stackDepth;
    m_movedCard = m_board->PopTopCard(m_srcCol);

    if (m_srcDepth > 0 && !m_board->IsColumnEmpty(m_srcCol)) {
        m_board->SetCardFaceUp(m_srcCol,
            m_board->CardsInColumn(m_srcCol) - 1, true);
    }
    m_board->PushHandCard(m_movedCard);
    return true;
}

void TableToHandAction::Undo() {
    CardData card = m_board->PopHandCard(m_board->HandCardCount() - 1);
    if (m_srcDepth > 0 && !m_board->IsColumnEmpty(m_srcCol)) {
        m_board->SetCardFaceUp(m_srcCol,
            m_board->CardsInColumn(m_srcCol) - 1, false);
    }
    m_board->PushCard(m_srcCol, card);
    m_board->SetCardFaceUp(m_srcCol,
        m_board->CardsInColumn(m_srcCol) - 1, true);
}

HandSwitchAction::HandSwitchAction(GameBoard* board, int index)
    : m_board(board), m_cardId(-1), m_fromIndex(index), m_oldTopIndex(-1) {}

bool HandSwitchAction::Execute() {
    if (m_fromIndex < 0 || m_fromIndex >= m_board->HandCardCount()) return false;
    m_cardId = m_board->HandCardAt(m_fromIndex).cardId;
    m_oldTopIndex = m_board->HandCardCount() - 1;
    if (m_fromIndex == m_oldTopIndex) return false;
    m_board->SetTopCardIndex(m_fromIndex);
    return true;
}

// ===== DrawReserveAction =====

DrawReserveAction::DrawReserveAction(GameBoard* board, int cardId)
    : m_board(board), m_cardId(cardId)
    , m_handIndex(board->HandCardCount() - 1) {}

bool DrawReserveAction::Execute() {
    // Card is already added to hand by DrawFromReserve before action creation
    return true;
}

void DrawReserveAction::Undo() {
    // Card was pushed to end of hand; verify index is still correct
    int idx = -1;
    if (m_handIndex >= 0 && m_handIndex < m_board->HandCardCount()
        && m_board->HandCardAt(m_handIndex).cardId == m_cardId) {
        idx = m_handIndex;
    } else {
        // Fallback: linear scan (hand indices may have shifted)
        for (int i = 0; i < m_board->HandCardCount(); ++i) {
            if (m_board->HandCardAt(i).cardId == m_cardId) {
                idx = i;
                break;
            }
        }
    }
    if (idx < 0) return;
    m_board->PopHandCard(idx);
    m_board->SetReserveCount(m_board->ReserveCount() + 1);
}

// ===== HandSwitchAction =====

void HandSwitchAction::Undo() {
    // Card was moved to top (last index); check there first
    int count = m_board->HandCardCount();
    int curIdx = -1;
    if (count > 0 && m_board->HandCardAt(count - 1).cardId == m_cardId) {
        curIdx = count - 1;
    } else {
        for (int i = 0; i < count; ++i) {
            if (m_board->HandCardAt(i).cardId == m_cardId) {
                curIdx = i;
                break;
            }
        }
    }
    if (curIdx < 0) return;

    CardData card = m_board->PopHandCard(curIdx);
    // If curIdx > fromIndex, the insert index is still valid since we removed the card
    m_board->InsertHandCard(m_fromIndex, card);
}

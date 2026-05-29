#include "UndoManager.h"

bool UndoManager::Execute(std::unique_ptr<IAction> action) {
    if (!action || !action->Execute()) return false;
    m_history.push_back(std::move(action));
    if (m_limit > 0) {
        while ((int)m_history.size() > m_limit) {
            m_history.erase(m_history.begin());
        }
    }
    Notify();
    return true;
}

bool UndoManager::Undo() {
    if (m_history.empty()) return false;
    auto action = std::move(m_history.back());
    m_history.pop_back();
    action->Undo();
    Notify();
    return true;
}

void UndoManager::ClearHistory() {
    m_history.clear();
    Notify();
}

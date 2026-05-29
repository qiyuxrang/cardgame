#pragma once

#include "Model/IAction.h"
#include <vector>
#include <memory>
#include <functional>

class UndoManager {
public:
    using HistoryCallback = std::function<void(int count)>;

    ~UndoManager() { ClearHistory(); }

    bool Execute(std::unique_ptr<IAction> action);
    bool Undo();
    bool CanUndo() const { return !m_history.empty(); }
    int  HistoryCount() const { return (int)m_history.size(); }
    IAction* PeekTop() const { return m_history.empty() ? nullptr : m_history.back().get(); }
    void ClearHistory();
    void SetLimit(int limit) { m_limit = limit; }
    void SetCallback(HistoryCallback cb) { m_callback = std::move(cb); }

private:
    std::vector<std::unique_ptr<IAction>> m_history;
    int m_limit = 0;
    HistoryCallback m_callback;
    void Notify() { if (m_callback) m_callback((int)m_history.size()); }
};

#pragma once

// IAction - Command pattern interface for undoable operations
class IAction {
public:
    virtual ~IAction() = default;
    virtual bool Execute() = 0;
    virtual void Undo() = 0;
    virtual const char* Description() const = 0;

    // For undo animation: the moved card and where it came from
    virtual int  GetMovedCardId() const = 0;
    virtual int  GetSourceCol() const = 0;
    virtual int  GetSourceDepth() const = 0;
    // Distinguish table-return vs hand-return undo destinations
    virtual bool IsTableSource() const = 0;
};

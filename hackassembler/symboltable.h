#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <QHash>
#include <QString>

class SymbolTable
{
public:
    SymbolTable();

    void clear();

    uint addEntry(const QString& symbol);
    void addEntry(const QString& symbol, uint address);

    bool contains(const QString& symbol) const;

    uint getAddress(const QString& symbol, bool& found) const;
    uint getAddressWithAddEntry(const QString& symbol);

private:
    // { Symbol: RAM Address }
    QHash<QString, uint> m_symbolTable;
    uint m_nextMemoryPos;
};

#endif // SYMBOLTABLE_H

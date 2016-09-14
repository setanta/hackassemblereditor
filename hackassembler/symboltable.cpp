#include "symboltable.h"

SymbolTable::SymbolTable()
{
    clear();
}

void SymbolTable::clear()
{
    m_symbolTable = {
        { "SP",     0x0000 },
        { "LCL",    0x0001 },
        { "ARG",    0x0002 },
        { "THIS",   0x0003 },
        { "THAT",   0x0004 },
        { "SCREEN", 0x4000 },
        { "KBD",    0x6000 }
    };

    for (uint i = 0; i < 16; i++)
        m_symbolTable["R" + QByteArray::number(i)] = i;

    m_nextMemoryPos = 16;
}

uint SymbolTable::addEntry(const QString& symbol)
{
    uint address = m_nextMemoryPos;
    addEntry(symbol, address);
    m_nextMemoryPos++;
    return address;
}

void SymbolTable::addEntry(const QString& symbol, uint address)
{
    m_symbolTable[symbol] = address;
}

bool SymbolTable::contains(const QString& symbol) const
{
    return m_symbolTable.contains(symbol);
}

uint SymbolTable::getAddress(const QString& symbol, bool& found) const
{
    found = contains(symbol);
    if (found)
        return m_symbolTable[symbol];
    return 0;
}

uint SymbolTable::getAddressWithAddEntry(const QString& symbol)
{
    bool found;
    uint address = getAddress(symbol, found);
    if (found)
        return address;
    return addEntry(symbol);
}

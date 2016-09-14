#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

#include "parser.h"
#include "symboltable.h"

class Assembler
{
public:
    struct Error {
        QString message;
        int line;
    };
    typedef QList<Error> ErrorList;

    void setSourceCode(const QString& asmSource);
    const QStringList& asmSrcCode() const { return m_parser.asmSource(); }
    const QStringList& binaryCode() const { return m_binaryCode; }

    bool hasErrors() const { return !m_errors.isEmpty(); }
    const ErrorList& errors() const { return m_errors; }

    int sourceLineForBinaryLine(int binaryLineNumber);
    int binaryLineForSourceLine(int sourceLineNumber);

    void parse();
    void translate();

private:

    Parser m_parser;
    SymbolTable m_symbolTable;

    QStringList m_asmSrcCode;
    QStringList m_binaryCode;
    QHash<int, int> m_srcToBinLines;
    QHash<int, int> m_binToSrcLines;

    ErrorList m_errors;
};

#endif // ASSEMBLER_H

#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QStringList>

class Parser
{
public:
    enum CommandType
    {
        NO_COMMAND,
        A_COMMAND,       // Addressing instruction for "@Xxx" (A-instruction)
        C_COMMAND,       // Compute instruction for "dest=comp;jump" (C-instruction)
        L_COMMAND        // Pseucocommand for "(Xxx)" (L-instruction)
    };

    void setAsmSource(const QStringList& asmSource);
    const QStringList& asmSource() const { return m_asmSource; }
    void reset();

    int currentLine() { return m_currentLine; }
    bool hasMoreLines() const;
    void advance();

    CommandType commandType() const { return m_currentCommandType; }
    const QString& symbol() const { return m_symbol; }
    const QString& dest() const { return m_dest; }
    const QString& comp() const { return m_comp; }
    const QString& jump() const { return m_jump; }
    const QString& error() const { return m_error; }

    inline bool hasError() const { return !m_error.isEmpty(); }

private:
    void clearParseData();

    QString removeComment(const QString& sourceLine) const;
    void processACommand(const QString& sourceLine);
    void processLCommand(const QString& sourceLine);
    void processCCommand(QString sourceLine);

    bool isValidSymbol(const QString& symbol) const;
    bool isValidConstant(const QString& constant) const;
    bool isValidCommand(const QString& command) const;
    bool isValidDestination(const QString& dest) const;
    bool isValidJump(const QString& jump) const;

    QStringList m_asmSource;
    int m_currentLine;

    CommandType m_currentCommandType;
    QString m_symbol;
    QString m_dest;
    QString m_comp;
    QString m_jump;
    QString m_error;
};

#endif // PARSER_H

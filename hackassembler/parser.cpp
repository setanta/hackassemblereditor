#include "parser.h"

void Parser::setAsmSource(const QStringList &asmSource)
{
    m_asmSource = asmSource;
    reset();
}

void Parser::reset()
{
    m_currentLine = -1;
    clearParseData();
}

void Parser::clearParseData()
{
    m_currentCommandType = Parser::NO_COMMAND;
    m_symbol.clear();
    m_dest.clear();
    m_comp.clear();
    m_jump.clear();
    m_error.clear();
}

bool Parser::hasMoreLines() const
{
    return m_currentLine < m_asmSource.length() - 1;
}

QString Parser::removeComment(const QString& sourceLine) const
{
    return sourceLine.mid(0, sourceLine.indexOf("//")).trimmed();
}

void Parser::advance()
{
    QString sourceLine;
    while (sourceLine.isEmpty() && hasMoreLines()) {
        m_currentLine++;
        sourceLine = removeComment(m_asmSource[m_currentLine]);
    }

    clearParseData();

    if (sourceLine.isEmpty()) return;

    if (sourceLine.startsWith('@'))
        processACommand(sourceLine);
    else if (sourceLine.startsWith('(') && sourceLine.endsWith(')'))
        processLCommand(sourceLine);
    else
        processCCommand(sourceLine);
}

void Parser::processACommand(const QString& sourceLine)
{
    m_currentCommandType = Parser::A_COMMAND;
    QString address = sourceLine.mid(1);
    if (isValidSymbol(address) || isValidConstant(address))
        m_symbol = address;
    else
        m_error = QString("Invalid symbol or constant: '%1'").arg(address);
}

void Parser::processLCommand(const QString& sourceLine)
{
    m_currentCommandType = Parser::L_COMMAND;
    QString label = sourceLine.mid(1, sourceLine.length() - 2);
    if (isValidSymbol(label))
        m_symbol = label;
    else
        m_error = QString("Invalid label: '%1'").arg(label);
}

void Parser::processCCommand(QString sourceLine)
{
    // DEST=COMP;JUMP
    m_currentCommandType = Parser::C_COMMAND;

    // DEST=comp;jump
    QStringList cmdParts = sourceLine.split('=');

    if (cmdParts.length() == 2) {
        if (!isValidDestination(cmdParts.first())) {
            m_error = QString("Invalid destination: '%1'").arg(cmdParts.first());
            return;
        }
        m_dest = cmdParts.first();
        sourceLine = cmdParts.last();
    }

    // dest=comp;JUMP
    cmdParts = sourceLine.split(';');
    if (cmdParts.length() == 2) {
        if (!isValidJump(cmdParts.last())) {
            m_dest.clear();
            m_error = QString("Invalid jump: '%1'").arg(cmdParts.last());
            return;
        }
        m_jump = cmdParts.last();
        sourceLine = cmdParts.first();
    }

    // dest=COMP;jump
    if (!isValidCommand(sourceLine)) {
        m_dest.clear();
        m_jump.clear();
        m_error = QString("Invalid computation: '%1'").arg(sourceLine);
        return;
    }
    m_comp = sourceLine;
}

/**
  * A user-defined symbol can be any sequence of letters, digits, underscore (_),
  * dot (.), dollar sign ($), and colon (:) that does not begin with a digit.
  */
bool Parser::isValidSymbol(const QString& symbol) const
{
    static const QRegExp rx("^[a-zA-Z_.$:][\\w.$:]*$");
    return rx.exactMatch(symbol);
}

/**
  * Constants must be non-negative and are written in decimal notation.
  */
bool Parser::isValidConstant(const QString& constant) const
{
    static const QRegExp rx("^\\d+$");
    return rx.exactMatch(constant);
}

/**
  * Valid Commands: 0, 1, -1, D, A, M, !D, !A, !M, -D, -A, -M, D+1, A+1, M+1,
  *                 D-1, A-1, M-1, D+A, D+M, D-A, D-MA-D, M-D, D&A, D&M, D|A, D|M
  */
bool Parser::isValidCommand(const QString& command) const
{
    static const QRegExp rx("^(0|[-]?1|[-!]?[DAM]|[DAM][+-]1|D[+\\-\\&\\|][AM]|[AM]-D)$");
    return rx.exactMatch(command);
}

bool Parser::isValidDestination(const QString& dest) const
{
    static const QStringList destValues { "M", "D", "MD", "A", "AM", "AD", "AMD" };
    return destValues.contains(dest);
}

bool Parser::isValidJump(const QString& jump) const
{
    static const QStringList jumpCommands { "JGT", "JEQ", "JGE", "JLT", "JNE", "JLE", "JMP" };
    return jumpCommands.contains(jump);
}

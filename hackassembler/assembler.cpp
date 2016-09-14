#include "assembler.h"
#include "code.h"

void Assembler::setSourceCode(const QString& asmSource)
{
    m_parser.setAsmSource(asmSource.split(QRegExp("\n|\r\n|\r")));
    m_symbolTable.clear();
    m_binaryCode.clear();
    m_srcToBinLines.clear();
    m_binToSrcLines.clear();
    m_errors.clear();
}

int Assembler::sourceLineForBinaryLine(int binaryLineNumber)
{
    return m_binToSrcLines.value(binaryLineNumber, -1);
}

int Assembler::binaryLineForSourceLine(int sourceLineNumber)
{
    return m_srcToBinLines.value(sourceLineNumber, -1);
}

void Assembler::parse()
{
    m_symbolTable.clear();
    m_srcToBinLines.clear();
    m_binToSrcLines.clear();
    m_errors.clear();

    uint memoryAddress = 0;

    while (m_parser.hasMoreLines()) {
        m_parser.advance();

        switch (m_parser.commandType()) {
        case Parser::A_COMMAND:
        case Parser::C_COMMAND:
            memoryAddress++;
            break;

        case Parser::L_COMMAND:
            m_symbolTable.addEntry(m_parser.symbol(), memoryAddress);
            break;

        default:
            break;
        }

        if (m_parser.hasError())
            m_errors.append({ m_parser.error(), m_parser.currentLine() });
    }
    m_parser.reset();
}

void Assembler::translate()
{
    m_binaryCode.clear();
    m_srcToBinLines.clear();
    m_binToSrcLines.clear();

    while (m_parser.hasMoreLines()) {
        m_parser.advance();

        uint address;
        switch (m_parser.commandType()) {
        case Parser::C_COMMAND:
            m_srcToBinLines[m_parser.currentLine()] = m_binaryCode.length();
            m_binToSrcLines[m_binaryCode.length()] = m_parser.currentLine();
            m_binaryCode.append("111" + Code::comp(m_parser.comp()) +
                                        Code::dest(m_parser.dest()) +
                                        Code::jump(m_parser.jump()));
            break;

        case Parser::A_COMMAND:
            bool isNumeric;
            address = m_parser.symbol().toUInt(&isNumeric);
            if (!isNumeric)
                address = m_symbolTable.getAddressWithAddEntry(m_parser.symbol());
            m_srcToBinLines[m_parser.currentLine()] = m_binaryCode.length();
            m_binToSrcLines[m_binaryCode.length()] = m_parser.currentLine();
            m_binaryCode.append(QString::number(address, 2).rightJustified(16, '0'));
            break;

        default:
            break;
        }
    }
}

#include "hacksyntaxhighlighter.h"

HackSyntaxHighlighter::HackSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Comment
    m_commentFormat.setFontItalic(true);
    m_commentFormat.setForeground(Qt::lightGray);
    rule.pattern = QRegExp("(?:.*)//.*$");
    rule.format = m_commentFormat;
    m_highlightingRules.append(rule);

    // Label
    m_labelInstructionFormat.setFontItalic(true);
    m_labelInstructionFormat.setForeground(Qt::darkCyan);
    rule.pattern = QRegExp("(?:^\\s*)\\([A-Za-z0-9_]+\\)(?:\\s*(//.*)?$)");
    rule.format = m_labelInstructionFormat;
    m_highlightingRules.append(rule);

    // A-Instruction
    m_AInstructionFormat.setForeground(Qt::darkBlue);
    rule.pattern = QRegExp("^(?:\\s*)@[A-Za-z0-9_]+(?:\\s*(//.*)?)");
    rule.format = m_AInstructionFormat;
    m_highlightingRules.append(rule);

    // C-Instruction
    m_CInstructionFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("(?:^\\s*)((M|D|MD|A|AM|AD|AMD)=)?(0|[-]?1|[-!]?[DAM]|[DAM][+-]1|D[+\\-\\&\\|][AM]|[AM]-D)(;J(GT|EQ|GE|LT|NE|LE|MP))?(?:\\s*(//.*)?$)");
    rule.format = m_CInstructionFormat;
    m_highlightingRules.append(rule);
}

void HackSyntaxHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, m_highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
}

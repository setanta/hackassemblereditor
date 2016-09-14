#ifndef HACKSYNTAXHIGHLIGHTER_H
#define HACKSYNTAXHIGHLIGHTER_H

#include <QRegExp>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

class HackSyntaxHighlighter : public QSyntaxHighlighter
{
public:
    HackSyntaxHighlighter(QTextDocument *parent = 0);

    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> m_highlightingRules;

    QTextCharFormat m_commentFormat;
    QTextCharFormat m_labelInstructionFormat;
    QTextCharFormat m_AInstructionFormat;
    QTextCharFormat m_CInstructionFormat;
};

#endif // HACKSYNTAXHIGHLIGHTER_H

#ifndef CODE_H
#define CODE_H

#include <QString>

namespace Code
{
    QString dest(QString mnemonic);
    QString jump(QString mnemonic);
    QString comp(QString mnemonic);
}

namespace DisAsm
{
    QString dest(const QString &code);
    QString jump(const QString &code);
    QString comp(const QString &code);
}

#endif // CODE_H

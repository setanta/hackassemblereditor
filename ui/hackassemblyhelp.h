#ifndef HACKASSEMBLYHELP_H
#define HACKASSEMBLYHELP_H

#include <QDialog>

namespace Ui {
class HackAssemblyHelp;
}

class HackAssemblyHelp : public QDialog
{
    Q_OBJECT

public:
    explicit HackAssemblyHelp(QWidget *parent = 0);
    ~HackAssemblyHelp();

private:
    Ui::HackAssemblyHelp *ui;
};

#endif // HACKASSEMBLYHELP_H

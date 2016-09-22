#include "hackassemblyhelp.h"
#include "ui_hackassemblyhelp.h"

HackAssemblyHelp::HackAssemblyHelp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HackAssemblyHelp)
{
    ui->setupUi(this);
    ui->textBrowser->setSource(QUrl("qrc:/docs/hacklang.html"));
}

HackAssemblyHelp::~HackAssemblyHelp()
{
    delete ui;
}

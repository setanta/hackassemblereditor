#include "ui/hackassemblereditor.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Nand2Tetris");
    QCoreApplication::setApplicationName("HackAssemblerEditor");

    HackAssemblerEditor w;
    w.show();

    return a.exec();
}

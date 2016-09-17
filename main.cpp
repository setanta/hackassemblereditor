#include "ui/hackassemblereditor.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QGuiApplication::setOrganizationName("github.com/setanta");
    QGuiApplication::setApplicationName("HackAssemblyEditor");

    HackAssemblerEditor w;
    w.show();

    return a.exec();
}

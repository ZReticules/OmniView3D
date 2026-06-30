#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("AutoWorkbench");
    QCoreApplication::setApplicationName("OmniView3D");
    QCoreApplication::setApplicationVersion("1.0.0");
    MainWindow w;
    w.show();
    return a.exec();
}

#include "imageviewer.h"

#include <QApplication>
#include <QWindow>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Aazrak");
    QCoreApplication::setOrganizationDomain("Aazrak.com");
    QCoreApplication::setApplicationName("ImageEditor");
    a.setWindowIcon(QIcon("images/pixmaps/logo.ico"));
    ImageViewer w;
    w.show();
    return a.exec();
}

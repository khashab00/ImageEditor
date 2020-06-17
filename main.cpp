#include "imageviewer.h"

#include <QApplication>
#include <QWindow>
#include <QTextCodec>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
       // look up e.g. :/languages/ImageEditor_de.qm
    if (translator.load(QLocale(), QLatin1String("ImageEditor"), QLatin1String("_"), QLatin1String(":/languages")))
           a.installTranslator(&translator);
    QCoreApplication::setOrganizationName("Aazrak");
    QCoreApplication::setOrganizationDomain("Aazrak.com");
    QCoreApplication::setApplicationName("ImageEditor");
    a.setWindowIcon(QIcon("images/pixmaps/logo.ico"));
    ImageViewer w;
    w.show();
    return a.exec();
}

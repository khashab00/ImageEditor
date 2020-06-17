#include "imageviewer.h"

#include <QApplication>
#include <QWindow>
#include <QTextCodec>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    QString locale = QLocale::system().name();
    locale.truncate(locale.lastIndexOf('_'));
    QLocale::setDefault(locale);
    QString path = QApplication::applicationDirPath();
    QString filename = QString("/ImageEditor_%1.qm").arg(locale);
    a.removeTranslator(&translator);
    if (translator.load(path+filename))
         a.installTranslator(&translator);

    QCoreApplication::setOrganizationName("Aazrak");
    QCoreApplication::setOrganizationDomain("Aazrak.com");
    QCoreApplication::setApplicationName("ImageEditor");
    a.setWindowIcon(QIcon("images/pixmaps/logo.ico"));
    ImageViewer w;
    w.show();
    return a.exec();
}

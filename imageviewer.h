#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include<QtCore>
#include<QtGui>
#include<QtWidgets>
#include<QtPrintSupport>
#include <QMainWindow>
#include <QLabel>
#include <QScrollArea>
#include <QGuiApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QImageReader>
#include <QImageWriter>
#include<QDropEvent>
#include<QDragEnterEvent>
#include <QWheelEvent>
#include "Settings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ImageViewer; }
QT_END_NAMESPACE

class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    ImageViewer(QWidget *parent = nullptr);
    bool loadFile(const QString &);

private slots:
    void on_action_Open_triggered();

    void on_action_Print_triggered();

    void on_action_Zoom_in_triggered();

    void on_action_Zoom_out_triggered();

    void on_action_Zoom_100_triggered();

    void on_action_Fit_to_Window_triggered();

    void on_action_Save_triggered();

    void on_action_Save_as_triggered();

    void on_action_Copy_triggered();

    void on_action_Paste_triggered();

    void on_action_About_triggered();

    void on_actionClose_all_triggered();

    void on_actionClose_triggered();

    void on_actionQuit_triggered();


    void on_action_Exit_triggered();

private:
    void updateActions();
    void setImage(const QImage &newImage);
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    bool saveFile(const QString &fileName);

    // for drag and drop implementation
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    /*//////////////////////////*/

    // for mouse zooming
    void wheelEvent(QWheelEvent *event);
    /* -------------- */

    void closeEvent(QCloseEvent *event);
    void saveGeometryState();
    void saveContent();
    bool handleCloseTabs();
    bool handleCloseChildWindow(QMdiSubWindow *subWindow);
    bool saveImage(const QString &fileName, int quality);

    void createKeyboardShortcuts();

    Ui::ImageViewer *ui;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    QImage image;
    QString FileName;
    double scaleFactor = 1;
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
    QPrinter printer;
#endif

};
#endif // IMAGEVIEWER_H

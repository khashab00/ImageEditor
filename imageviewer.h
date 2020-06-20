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
#include "dialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ImageViewer; }
QT_END_NAMESPACE

class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    ImageViewer(QWidget *parent = nullptr);
    bool loadFile(const QString &);

public slots:
    void applyYUV(float Y, float U,float V);
    void applyRGB(float r, float g,float b);
    void rejectChanges();
    void acceptChanges();

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

    void on_action_Show_Dialog_triggered();

    void on_action_About_Qt_triggered();

    void on_action_New_triggered();

    void on_action_Dark_Mood_triggered();

    void updateRecentFilesMenu();

    void on_actionGerman_triggered();

    void on_actionEnglish_triggered();

    void setModified(bool modified);

private:
    void updateActions();
    void setImage(const QImage &newImage);
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    bool saveFile(const QString &fileName);

    // for drag and drop implementation
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    /*-----------------*/

    // for mouse zooming
    void wheelEvent(QWheelEvent *event);
    /* -------------- */

    //Draging by mouse
    void addSettingsWidgets();
    QPoint offset;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    /*---------------*/

    void showError(const QString &message);
    bool fileExists(QString path);
    QString prepareFile(const QString& fileName);
    bool fileTypeSupported(QList<QByteArray> formats, QString ext);
    void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode);

    void closeEvent(QCloseEvent *event);
    void saveGeometryState(QCloseEvent *event);
    void setWindowSize();
    void saveContent();
    bool handleCloseTabs();
    bool handleCloseChildWindow(QMdiSubWindow *subWindow);
    bool saveImage(const QString &fileName, int quality);


    void loadLanguage(const QString& rLanguage);
    void createLanguageMenu(void);
    void changeEvent(QEvent* event);


    void createKeyboardShortcuts();

    void changeFotoTest();


    Ui::ImageViewer *ui;

    QLabel *imageLabel;
    QScrollArea *scrollArea;
    QImage image;
    QImage imagePreview;

    int zoomLevel = 100;
    double scaleFactor = 1.0;

    Dialog *dlg = new Dialog();

    QString FileName;
    bool isModified;

    // language stuff
    QTranslator m_translator;
    QTranslator m_translatorQt;
    QString m_currLang;
    QString m_langPath;
    bool isDarkmood=false;


#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
    QPrinter printer;
#endif
};
#endif // IMAGEVIEWER_H

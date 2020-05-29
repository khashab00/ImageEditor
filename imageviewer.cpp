#include "imageviewer.h"
#include "ui_imageviewer.h"
#include "Settings.h"
namespace
{
    const QString UNTITLED_TAB_NAME = QObject::tr("Untitled");
}
ImageViewer::ImageViewer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImageViewer)
    ,scrollArea(new QScrollArea)
    , scaleFactor(1)
{
    ui->setupUi(this);

    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    scrollArea->setAcceptDrops(false);

    setCentralWidget(scrollArea);

    setAcceptDrops(true);
    setWindowTitle(tr("Image Editor"));

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
    createKeyboardShortcuts();
}

bool ImageViewer::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull())
    {
       QMessageBox::
         information(this, QGuiApplication::applicationDisplayName(),
                     tr("Cannot load %1: %2")
                     .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
           return false;
    }
    setImage(newImage);

    setWindowFilePath(fileName);

    QFileInfo windowTitle(fileName);

    setWindowTitle(tr("%1").arg(windowTitle.fileName()));

    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4").arg(QDir::
                              toNativeSeparators(fileName)).
                                arg(image.width()).
                                arg(image.height()).
                                arg(image.depth());
     statusBar()->showMessage(message);
     return true;
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations =
                QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("Images (*.png *.xpm *.jpeg *.bmp)");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

void ImageViewer::on_action_Open_triggered()
{
   QFileDialog dialog(this, tr("Open File"));
   initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);
   while (dialog.exec() ==
          QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void ImageViewer::updateActions()
{
    ui->action_Save_as->setEnabled(!image.isNull());
    ui->action_Copy->setEnabled(!image.isNull());
    ui->action_Zoom_in->setEnabled(!ui->action_Fit_to_Window->isChecked());
    ui->action_Zoom_out->setEnabled(!ui->action_Fit_to_Window->isChecked());
    ui->action_Zoom_100->setEnabled(!ui->action_Fit_to_Window->isChecked());
}

bool ImageViewer::saveFile(const QString &fileName)
{
    QImageWriter writer(fileName);

    if (!writer.write(image)) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
        }
     const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
     statusBar()->showMessage(message);
     return true;
}

void ImageViewer::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void ImageViewer::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
        if (urls.isEmpty())
            return;

        QString fileName = urls.first().toLocalFile();
        if (fileName.isEmpty())
            return;

        loadFile(fileName);

}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
       int numDegrees = event->angleDelta() .y();

       if (numDegrees > 0) {
           on_action_Zoom_in_triggered();
       } else {
           on_action_Zoom_out_triggered();
       }

       event->accept();
}

void ImageViewer::setImage(const QImage &newImage)
{
    this->image = newImage;
    imageLabel->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;

    scrollArea->setVisible(true);
    ui->action_Print->setEnabled(true);
    ui->action_Fit_to_Window->setEnabled(true);
    updateActions();

    if (!ui->action_Fit_to_Window->isChecked())
        imageLabel->adjustSize();
}

void ImageViewer::scaleImage(double factor)
{
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    ui->action_Zoom_in->setEnabled(scaleFactor < 3.0);
    ui->action_Zoom_out->setEnabled(scaleFactor > 0.333);
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                         + ((factor - 1) * scrollBar->pageStep()/2)));
}

void ImageViewer::on_action_Print_triggered()
{
    Q_ASSERT(imageLabel->pixmap());
  #if QT_CONFIG(printdialog)
        QPrintDialog dialog(&printer, this);
        if (dialog.exec()) {
            QPainter painter(&printer);
            QRect rect = painter.viewport();
            QSize size = imageLabel->pixmap()->size();
            size.scale(rect.size(), Qt::KeepAspectRatio);
            painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
            painter.setWindow(imageLabel->pixmap()->rect());
            painter.drawPixmap(0, 0, *imageLabel->pixmap());
        }
  #endif
}

void ImageViewer::on_action_Zoom_in_triggered()
{
    scaleImage(1.25);
    QSize size = image.size();
    QString sizeString = QString("(%1,%2)").arg(size.width()).arg(size.height());
    statusBar()->showMessage(sizeString);
}

void ImageViewer::on_action_Zoom_out_triggered()
{
    scaleImage(0.8);
    QSize size = image.size();
    QString sizeString = QString("(%1,%2)").arg(size.width()).arg(size.height());
    statusBar()->showMessage(sizeString);
}

void ImageViewer::on_action_Zoom_100_triggered()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void ImageViewer::on_action_Fit_to_Window_triggered()
{
    bool fitToWindow = ui->action_Fit_to_Window->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        on_action_Zoom_100_triggered();
    updateActions();
}

void ImageViewer::on_action_Save_triggered()
{
    //TODO
}

void ImageViewer::on_action_Save_as_triggered()
{
    QFileDialog dialog(this, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave);
    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
}

void ImageViewer::on_action_Copy_triggered()
{
    #ifndef QT_NO_CLIPBOARD
        QGuiApplication::clipboard()->setImage(image);
    #endif // !QT_NO_CLIPBOARD
}

#ifndef QT_NO_CLIPBOARD
static QImage clipboardImage()
{
    if (const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData()) {
        if (mimeData->hasImage()) {
            const QImage image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull())
                return image;
        }
    }
    return QImage();
}
#endif // !QT_NO_CLIPBOARD

void ImageViewer::on_action_Paste_triggered()
{
#ifndef QT_NO_CLIPBOARD
    const QImage newImage = clipboardImage();
    if (newImage.isNull()) {
        statusBar()->showMessage(tr("No image in clipboard"));
    } else {
        setImage(newImage);
        setWindowFilePath(QString());
        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
            .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
        statusBar()->showMessage(message);
    }
#endif // !QT_NO_CLIPBOARD
}


void ImageViewer::createKeyboardShortcuts()
{
    //File Menu
    ui->action_Exit->setShortcut(QString("f4"));
   /* ui->actionNew->setShortcut(QString("Ctrl+N"));
    ui->actionOpen->setShortcut(QString("Ctrl+O"));
    ui->actionSave->setShortcut(QString("Ctrl+S"));
    ui->actionSave_As->setShortcut(QString("Ctrl+Shift+S"));
    ui->actionPrint->setShortcut(QString("Ctrl+P"));
    ui->actionClose->setShortcut(QString("Ctrl+Shift+W"));
    ui->actionQuit->setShortcut(QString("Ctrl+Q"));
    //Edit Menu
    ui->actionCut->setShortcut(QString("Ctrl+X")); //not implemented
    ui->actionCopy->setShortcut(QString("Ctrl+C"));
    ui->actionPaste->setShortcut(QString("Ctrl+V"));
    ui->actionPaste_as_new_image->setShortcut(QString("Ctrl+Shift+V"));
    ui->actionUndo->setShortcut(QString("Ctrl+Z"));
    ui->actionRedo->setShortcut(QString("Ctrl+Y"));
    ui->actionImage_properties->setShortcut(QString("Ctrl+J"));
    //Image Menu
    ui->actionDuplicate->setShortcut(QString("Ctrl+U"));
    ui->actionImage_Size->setShortcut(QString("Ctrl+H"));
    ui->actionCrop->setShortcut(QString("Ctrl+Shift+H"));
    // Selection Menu
    ui->actionSelect_all->setShortcut(QString("Ctrl+A"));
    //View Menu
    ui->actionToolpalette->setShortcut(QString("Ctrl+L"));
    ui->actionZoom_in->setShortcut(QKeySequence::ZoomIn);
    ui->actionZoom_out->setShortcut(QKeySequence::ZoomOut);
    ui->actionOriginal_size->setShortcut(QString("Ctrl+0"));
    ui->actionFull_screen->setShortcut(QString("Ctrl+F"));
    */
}


void ImageViewer::on_action_About_triggered()
{
    QMessageBox::about(this, tr("About Image Viewer"),
                tr("<p>The <b>Image Viewer</b> example shows how to combine QLabel "
                   "and QScrollArea to display an image. QLabel is typically used "
                   "for displaying a text, but it can also display an image. "
                   "QScrollArea provides a scrolling view around another widget. "
                   "If the child widget exceeds the size of the frame, QScrollArea "
                   "automatically provides scroll bars. </p><p>The example "
                   "demonstrates how QLabel's ability to scale its contents "
                   "(QLabel::scaledContents), and QScrollArea's ability to "
                   "automatically resize its contents "
                   "(QScrollArea::widgetResizable), can be used to implement "
                   "zooming and scaling features. </p><p>In addition the example "
                   "shows how to use QPainter to print an image.</p>"));
}

bool ImageViewer::handleCloseTabs()
{
    QList<QMdiSubWindow *> windows;

    for (int i = 0; i < windows.size(); ++i)
    {
       QMdiSubWindow *subWindow = windows.at(i);
        if(subWindow->isWindowModified())
        {
            if (handleCloseChildWindow(subWindow))
               return true;
        }
        else
        {
           subWindow->close();
           qCritical("Close event");
           qCritical("Close event");
        }
    }

    return false;
}

bool ImageViewer::saveImage(const QString &fileName, int quality)
{
    if (fileName.isEmpty())
        return false;
    return image.isNull() ? image.save(fileName,nullptr,quality) : false;
}


void ImageViewer::saveContent()
{
    QString currentFileName = this->FileName;

    if(currentFileName.contains(UNTITLED_TAB_NAME + " [*]"))
    {
        on_action_Save_as_triggered();
    }
    else
    {
        saveImage(currentFileName.mid(0,currentFileName.length() - 4),-1);
        //ui->mdiArea->currentSubWindow()->setWindowModified(false);
    }
}


bool ImageViewer::handleCloseChildWindow(QMdiSubWindow *subWindow)
{
    if (!subWindow)
        return false;

   // ui->mdiArea->setActiveSubWindow(subWindow);

    if (subWindow->isWindowModified())
    {
        int buttonCode = QMessageBox::question(this, tr("Unsaved Changes"), tr("Save changes before leaving?"),
                                               QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);

        if (buttonCode == QMessageBox::Cancel)
        {
            return true;
        }
        else if (buttonCode == QMessageBox::Yes)
        {
            saveContent();
        }
    }

    subWindow->setWindowModified(false);
    subWindow->close();

  //  clearStatusArea();

    return false;
}


void ImageViewer::saveGeometryState()
{
    // Save maximized window state if user maximizes the window manually.
    if (this->isMaximized())
    {
        SETTINGS->setMaximizeWindow(true);
    }
    else if (!SETTINGS->isMaximizeWindow()) // Save custom window geometry.
    {
        SETTINGS->setCustomWindowGeometry(this->geometry());
    }
}

void ImageViewer::closeEvent(QCloseEvent *event)
{
    if (!handleCloseTabs())
    {
        saveGeometryState();
    }
    else
    {
        event->ignore();
    }
}


void ImageViewer::on_actionQuit_triggered()
{
    if (!handleCloseTabs())
    {
        saveGeometryState();
        qApp->quit();
    }
}

void ImageViewer::on_actionClose_triggered()
{
   // handleCloseChildWindow(ui->centralwidget->currentSubWindow());
}

void ImageViewer::on_actionClose_all_triggered()
{
    handleCloseTabs();
}

void ImageViewer::on_action_Exit_triggered()
{
    on_actionQuit_triggered();
}

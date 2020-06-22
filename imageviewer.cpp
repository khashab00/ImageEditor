#include "imageviewer.h"
#include "ui_imageviewer.h"
#include "Settings.h"
#include <QSettings>
#include <QQuickStyle>
#include <QQmlApplicationEngine>
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

    // Setup some other defaults on startup
      setWindowSize();


    // Add Settings Widgets to the Dock
    addSettingsWidgets();


    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    scrollArea->setAcceptDrops(false);

      dlg = new Dialog(this);
      connect(dlg,&Dialog::rejected, this,&ImageViewer::rejectChanges);
      connect(dlg,&Dialog::accepted, this,&ImageViewer::acceptChanges);
      connect(dlg,&Dialog::rgbChanged, this,   &ImageViewer::applyRGB);
      connect(dlg,&Dialog::yuvChanged, this,   &ImageViewer::applyYUV);

      setCentralWidget(scrollArea);

    setAcceptDrops(true);
    setWindowTitle(tr("Image Editor"));

    updateActions();

    setCursor(Qt::ArrowCursor);

   // resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

   setCursor(Qt::ArrowCursor);

   // Update recents
   updateRecentFilesMenu();
   createLanguageMenu();

}

////////////////////
/// \brief ImageViewer::addSettingsWidgets
///////////////////
void ImageViewer::addSettingsWidgets()
{
    const QString message = tr("open file location");
    statusBar()->showMessage(message);


    SETTINGS->settings = new QSettings("Aazrak", "ImageEditor");
    restoreGeometry(SETTINGS->settings->value("ImageViewer/geometry").toByteArray());
    restoreState(SETTINGS->settings->value("ImageViewer/windowState").toByteArray());
    if(!SETTINGS->isMaximizeWindow())
    {
           move(SETTINGS->settings->value( "ImageViewer/pos", pos() ).toPoint());
           resize(SETTINGS->settings->value( "ImageViewer/size", size() ).toSize());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// \brief ImageViewer::loadFile
/// \param fileName
/// \return
///
/////////////////////////////////////////////
bool ImageViewer::loadFile(const QString &fileName)
{
    if(!fileExists(fileName))
    {
        showError(tr("Image does not exist at this file path"));
        return false;
    }
    else
    {
        QString updatedFileName = prepareFile(fileName);

        if (!updatedFileName.isEmpty())
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

            FileName = windowTitle.fileName();

            setWindowTitle(tr("%1").arg(FileName));

            const QString message = tr("Opened \"%1\", %2x%3, Depth: %4").arg(QDir::
                                      toNativeSeparators(fileName)).
                                        arg(image.width()).
                                        arg(image.height()).
                                        arg(image.depth());
             statusBar()->showMessage(message);
             SETTINGS->addRecentFile(updatedFileName);
             updateRecentFilesMenu();
             return true;
        }
    }
    return false;
}

bool ImageViewer::maybeSave()
{
   if(this->isModified)
   {
       auto ret = QMessageBox::warning(
                this,
                "Editor",
                "The file was modified, do u want to save it?",
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
                );
                if(ret == QMessageBox::Save){
                    on_action_Save_triggered();
                    isModified = false;
                }else if(ret == QMessageBox::Discard){
                    isModified = false;
                    setWindowTitle(tr("%1").arg(FileName));
                    return true;
                }else{//QMessageBox::Cancel
                    return false;
                }
        }
        return true;
}


//////////////
/// \brief initializeImageFileDialog
/// \param dialog
/// \param acceptMode
//////////////////
void ImageViewer::initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations =
                QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    // get supported image file types
    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = QImageReader::supportedMimeTypes();
    foreach(const QByteArray& mimeTypeName, supportedMimeTypes) {
        mimeTypeFilters.append(mimeTypeName);
    }
    mimeTypeFilters.sort(Qt::CaseInsensitive);

    // compose filter for all supported types
    QMimeDatabase mimeDB;
    QStringList allSupportedFormats;
    for(const QString& mimeTypeFilter: mimeTypeFilters) {
        QMimeType mimeType = mimeDB.mimeTypeForName(mimeTypeFilter);
        if(mimeType.isValid()) {
            allSupportedFormats.append(mimeType.globPatterns());
        }
    }
    QString allSupportedFormatsFilter = QString("All supported formats (%1)").arg(allSupportedFormats.join(' '));
    if(acceptMode == QFileDialog::AcceptOpen)
            dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setMimeTypeFilters(mimeTypeFilters);
    QStringList nameFilters = dialog.nameFilters();
    nameFilters.append(allSupportedFormatsFilter);
    dialog.setNameFilters(nameFilters);
    if(acceptMode == QFileDialog::AcceptOpen)
        dialog.selectNameFilter(allSupportedFormatsFilter);
    if (acceptMode == QFileDialog::AcceptSave)
       {
            dialog.setAcceptMode(acceptMode);
            QMimeType type = QMimeDatabase().mimeTypeForFile(FileName, QMimeDatabase::MatchContent);
            QFileInfo fi(FileName);
            QString ext = fi.suffix();  // ext = "jpg"
            dialog.setDefaultSuffix(mimeTypeFilters[3]);
            QStringList result = dialog.nameFilters().filter("*."+ext.toLower());
            if (result.length()> 0)
                dialog.selectNameFilter(result[0]);
            else
                dialog.selectNameFilter(allSupportedFormatsFilter);
       }
}

/////////////////
/// \brief clip
/// \param x
/// \return
///
static int clip(float x)
{
    if (x<0)
        return 0;
    if(x > 255 )
        return 255;
    return static_cast<int>(x);
}

////////////////
/// \brief ImageViewer::applyYUV
/// \param Y
/// \param U
/// \param V
///
void ImageViewer::applyYUV(float Y, float U,float V)
{
    imagePreview = image;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QRgb rgb = image.pixel(x,y);
            int R = qRed(rgb);
            int G = qGreen(rgb);
            int B = qBlue(rgb);

            float currY =  0.299    * R +  0.587    * G +  0.114      * B;
            float currU = 0.493 * (B-currY); //-0.168736 * R + -0.331264 * G +  0.5        * B;
            float currV =0.877 * (R-currY); //  0.5      * R + -0.418688 * G + -0.081312   * B;

            float newY = Y/100 * currY;
            float newU = U/100 * currU;
            float newV = V/100 * currV;

              int newR = clip( newY + (1/0.877) * newV ); //newY                      + 1.402 * newV);
              int newG = clip( newY - 0.394 * newU - 0.581 * newV); //newY + -0.344 *newU * -0.714      * newV);
              int newB = clip( newY + (1/0.493)* newU);//newY + 1.772 * newU                     );

              imagePreview.setPixel(x,y,qRgb(newR,newG,newB));

        }
    }
    imageLabel->setPixmap(QPixmap::fromImage(imagePreview));
}
//////////////////////////
/// \brief ImageViewer::applyRGB
/// \param r
/// \param g
/// \param b
///
void ImageViewer::applyRGB(float r, float g,float b)
{
    imagePreview = image;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QRgb rgb = image.pixel(x,y);
            int newR = (r/100) * qRed(rgb);
            int newG = (g/100) * qGreen(rgb);
            int newB = (b/100) * qBlue(rgb);
            imagePreview.setPixel(x,y,qRgb(newR,newG,newB));
        }
    }
    imageLabel->setPixmap(QPixmap::fromImage(imagePreview));
    //setModified(true);
}
//////////////
/// \brief ImageViewer::rejectChanges
///
void ImageViewer::rejectChanges()
{
    imagePreview = image;
    imageLabel->setPixmap(QPixmap::fromImage(image));
}
////////////////
/// \brief ImageViewer::acceptChanges
///
void ImageViewer::acceptChanges()
{
    image = imagePreview  ;
    imageLabel->setPixmap(QPixmap::fromImage(image));
    isModified = true;
    setWindowTitle(tr("%1*").arg(windowTitle()));
}

///////////////////////////////////////////////////////////////////////

void ImageViewer::on_action_Open_triggered()
{
   if(maybeSave())
   {
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);
    while (dialog.exec() ==
           QDialog::Accepted && !loadFile(dialog.selectedFiles().first()))
    {
        FileName = dialog.selectedFiles()[0];
        if(dialog.selectedFiles().length()>0)
        {
            for(int i=0;i<dialog.selectedFiles().length();i++)
            {
                loadFile(dialog.selectedFiles()[i]);
            }

            if(SETTINGS->getPreviouslyOpened() == true)
            {
                QDir d = QFileInfo(dialog.selectedFiles()[0]).absoluteDir();
                SETTINGS->setOpenFolder(d.absolutePath());
            }
        }

    }
   }
   else
   {
       const QString message = tr("Please save the file before You open a new one ;)");
       statusBar()->showMessage(message);
   }
}

bool ImageViewer::fileTypeSupported(QList<QByteArray> formats, QString ext)
{
    bool status = false;
    for(int i=0;i<formats.length();i++)
    {
        if(formats[i] == ext)
        {
            status = true;
        }
    }
    return status;
}

QString ImageViewer::prepareFile(const QString& fileName)
{
    QString newFileName = fileName;
    QFileInfo info(fileName);
    QImageReader reader(fileName);
    reader.setDecideFormatFromContent(true); // Autodetect file type without depending on extension

    if(info.completeSuffix().toLower() != "jpg" && info.completeSuffix() != reader.format() && fileTypeSupported(reader.supportedImageFormats(),reader.format()))
    {
        int ret = QMessageBox::warning(this,
                    tr("Incorrect file extension detected"),
                    tr("Do you want to update this extension?"),
                    QMessageBox::Save,QMessageBox::Cancel);

        if(ret == QMessageBox::Save)
        {
          newFileName = info.path()+QDir::separator()+info.baseName()+"."+reader.format();
          QDir dir (info.baseName());
          dir.rename(fileName,newFileName);
        }
        else if(ret == QMessageBox::Cancel)
        {
            newFileName = "";
        }
    }
    else if(!fileTypeSupported(reader.supportedImageFormats(),reader.format()))
    {
        newFileName = "";
        showError(tr("Please open a valid image file"));
    }
    FileName = newFileName;
    return newFileName;
}

//////////////////
/// \brief ImageViewer::updateRecentFilesMenu
///////////////////
void ImageViewer::updateRecentFilesMenu()
{
    ui->menuRecent_Files->clear();

    QList<QVariant> recentFiles = SETTINGS->getRecentFiles();
    QList<QVariant>::iterator i;

    for(i = recentFiles.begin(); i != recentFiles.end(); i++)
    {
        const QString& fileName = (*i).toString();
        if(fileExists(fileName))
        {
            QAction* action = ui->menuRecent_Files->addAction(fileName);
            connect(action, &QAction::triggered, [this, fileName] () {
                loadFile(fileName);
            });
        }
    }
}

//////////////
/// \brief ImageViewer::fileExists
/// \param path
/// \return
////////////////////
bool ImageViewer::fileExists(QString path) {
    QFileInfo check_file(path);

    return (check_file.exists() && check_file.isFile());
}


///////////
/// \brief ImageViewer::showError
/// \param message
/////////////////
void ImageViewer::showError(const QString &message)
{
    QMessageBox::critical(this, tr("Error"), message);
}

////////////////
/// \brief ImageViewer::on_action_Open_triggered
///////////////////////////7
/*void ImageViewer::on_action_Open_triggered()
{
   QFileDialog dialog(this, tr("Open File"));
   initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);
   while (dialog.exec() ==
          QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}*/

/////////////////////////
/// \brief ImageViewer::updateActions
///////////////////////

void ImageViewer::updateActions()
{
    ui->action_Save_as->setEnabled(!image.isNull());
    ui->action_Save->setEnabled(!image.isNull());
    ui->action_Copy->setEnabled(!image.isNull());
    ui->action_Cut->setEnabled(!image.isNull());
    ui->action_New->setEnabled(!image.isNull());
    ui->action_Zoom_in->setEnabled(!ui->action_Fit_to_Window->isChecked() && !image.isNull() );
    ui->action_Zoom_out->setEnabled(!ui->action_Fit_to_Window->isChecked() && !image.isNull());
    ui->action_Zoom_100->setEnabled(!ui->action_Fit_to_Window->isChecked() && !image.isNull());
}


/////////////////
/// \brief ImageViewer::saveFile
/// \param fileName
/// \return
/////////////////////////////////////7

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
     // Set Save path to previously used location
     if(SETTINGS->getPreviouslyOpenedSave() == true)
     {
         QDir d = QFileInfo(fileName).absoluteDir();
         SETTINGS->setSaveFolder(d.absolutePath());
     }
     else
     {
         SETTINGS->addRecentFile(fileName);
         updateRecentFilesMenu();
     }
     return true;
}

///////////////////////////////////////////////////////////////////////

void ImageViewer::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

//////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    //   changeFotoTest();

       int numDegrees = event->angleDelta() .y();

       if (numDegrees > 0 && scaleFactor < 4) {
          on_action_Zoom_in_triggered();
       } else if (numDegrees < 0 && scaleFactor > 0.25) {
          on_action_Zoom_out_triggered();
       }
       event->accept();

}
//////////////////////////////////////////////////////////////////////
void ImageViewer::mousePressEvent(QMouseEvent *event)
{
     offset = event->pos();
}
//////////////////////////////////////////////////////////////////////
void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
       {
           setCursor(Qt::ClosedHandCursor);
           imageLabel->move(mapToParent(event->pos() - offset));
    }
}
///////////////////////////////////////////////////////////////////////
void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if(!event->buttons() & Qt::LeftButton)
    {
        setCursor(Qt::ArrowCursor);
    }
}

////////////////////////////////////////////////////////////////////////

void ImageViewer::setImage(const QImage &newImage)
{
    image = newImage;
    imageLabel->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;
    scrollArea->setVisible(true);
    ui->action_Print->setEnabled(true);
    ui->action_Fit_to_Window->setEnabled(true);
    updateActions();
    if (!ui->action_Fit_to_Window->isChecked())
        imageLabel->adjustSize();
}

//////////////////////////////////////////////////////////////////////

void ImageViewer::scaleImage(double factor)
{
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor += factor;
    qDebug()<<scaleFactor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());
    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);


    ui->action_Zoom_in->setEnabled(scaleFactor < 4.0);
    ui->action_Zoom_out->setEnabled(scaleFactor > 0.25);

}


void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                         + ((factor - 1) * scrollBar->pageStep()/2)));
}

//////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////

void ImageViewer::on_action_Zoom_in_triggered()
{
    if( !image.isNull())
    {
        scaleImage(0.25);
        zoomLevel +=25;
        QString sizeString = QString("(%1 %2 %3)").arg(tr("Zoom Level: ")).arg(zoomLevel).arg("%");
        statusBar()->showMessage(sizeString);
    }
    else
    {
        ui->statusbar->showMessage(tr("Load an Image"));
    }
}

void ImageViewer::on_action_Zoom_out_triggered()
{
    if( !image.isNull())
    {
        scaleImage(-0.25);
        zoomLevel -=25;
        QString sizeString = QString("(%1 %2 %3)").arg(tr("Zoom Level: ")).arg(zoomLevel).arg("%");
        statusBar()->showMessage(sizeString);
    }
    else
    {
        ui->statusbar->showMessage(tr("Load an Image"));
    }
}

void ImageViewer::on_action_Zoom_100_triggered()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
    zoomLevel =100;
    QString sizeString = QString("(%1 %2 %3)").arg(tr("Zoom Level: ")).arg(zoomLevel).arg("%");
    statusBar()->showMessage(sizeString);
}
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////

void ImageViewer::on_action_Fit_to_Window_triggered()
{
    bool fitToWindow = ui->action_Fit_to_Window->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        on_action_Zoom_100_triggered();
    updateActions();
}

/////////////////////////////////////////////////////////////////////

void ImageViewer::on_action_Save_triggered()
{
    if (!image.isNull())
    {
        if(isModified)
        {
            saveContent();
            setWindowTitle(tr("%1").arg(FileName));
            isModified = false;
        }
        else
        {
            const QString message = tr("There is no change to save :(");
            statusBar()->showMessage(message);
        }

    }
}
/////////////////////////////////////////////////////////////////////

void ImageViewer::on_action_Save_as_triggered()
{
    if(isModified)
    {
        QFileDialog dialog(this, tr("Save File As"));
        initializeImageFileDialog(dialog, QFileDialog::AcceptSave);
        while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
        setWindowTitle(tr("%1").arg(FileName));
        isModified = false;
    }
    else
    {
        const QString message = tr("There is no change to save :(");
        statusBar()->showMessage(message);
    }

}

////////////////////////////////////////////////////////////////////

void ImageViewer::on_action_Copy_triggered()
{
    #ifndef QT_NO_CLIPBOARD
        QGuiApplication::clipboard()->setImage(image);
    #endif // !QT_NO_CLIPBOARD
}

///////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////

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


void ImageViewer::on_action_About_triggered()
{
    QMessageBox::about(this, tr("About Image Editor"),
                tr("<p>The <b>Image Editor</b> project is part of the course "
                   "software development in HTW</p>"
                   "<p>The project demonstrates how QLabel's ability to scale its contents "
                   "(QLabel::scaledContents), and QScrollArea's ability to "
                   "automatically resize its contents "
                   "(QScrollArea::widgetResizable), can be used to implement "
                   "zooming and scaling features. </p>"
                   "<p>In addition the project "
                   "shows how to use QPainter to print an image and QDialog to show a dialog.</p>"));
}


/////////////////////////////////////////////////////////////////

void ImageViewer::on_action_Show_Dialog_triggered()
{
     //dlg->exec(); nicht modal Dialog
    dlg->show(); // modal Dialog
}

////////////////////////////////////////////////////////////////

void ImageViewer::on_action_About_Qt_triggered()
{
    QMessageBox::about(this, tr("About Qt"),
                tr("<p>The <b>Qt Creator 5.13.1</b> Built on Oct 4 2019 01:16:32 "
                   "From revision ea829fa6d5 "
                   "Copyright 2008-2019 The Qt Company Ltd. All rights reserved."
                   " Copyright 2008-2019 The Qt Company Ltd. All rights reserved. "
                   "The program is provided AS IS with NO WARRANTY OF ANY KIND, "
                   "INCLUDING THE WARRANTY OF DESIGN,"
                   "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.</p>"));
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

void ImageViewer::changeFotoTest()
{
   QSize sizeImage = image.size();
   double width = sizeImage.width();
   double height = sizeImage.height();
    int r,g,b, alpha = 0;
    // const QRgb red = 0;
    //const QRgb red = 255;
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
             QColor color = image.pixel(x,y);
             color.getRgb(&r, &g, &b, &alpha);
             r -= 10;
             if(r <= 0)
                 r=0;
            color.setRgb(r,g,b,alpha);
            image.setPixelColor(x,y,color);
            /*color.getRgb(&r,&g,&b,&alpha);
             * if (r == 0 && g == 0 && b == 0) {
                cout << "Gelukt";
                background.setPixel(x,y,Qt::blue);
            }*/
        }
    }
    setImage(image);
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

void ImageViewer::setWindowSize()
{
    bool maximize = SETTINGS->isMaximizeWindow();

    if (maximize)
    {
        this->setWindowState(Qt::WindowMaximized);
    }
    else
    {
        QRect geometry = SETTINGS->customWindowGeometry();
        if (geometry.isValid())
            this->setGeometry(geometry);
    }
}
//////////////////////////////////////////////////////////////////////
void ImageViewer::saveGeometryState(QCloseEvent *event)
{
    // Save maximized window state if user maximizes the window manually.
    if (this->isMaximized())
    {
        SETTINGS->setMaximizeWindow(true);
        SETTINGS->settings = new QSettings ("Aazrak", "ImageEditor");
        SETTINGS->settings->setValue("geometry", saveGeometry());
        SETTINGS->settings->setValue("windowState", saveState());
        QMainWindow::saveState();
        QWidget::closeEvent(event);
    }
    else if (!this->isMaximized()) // Save custom window geometry.
    {
        SETTINGS->setCustomWindowGeometry(this->geometry());
        SETTINGS->settings = new QSettings ("Aazrak", "ImageEditor");
        SETTINGS->settings->setValue("geometry", saveGeometry());
        SETTINGS->settings->setValue("windowState", saveState());
        SETTINGS->settings->setValue("pos", pos() );
        SETTINGS->settings->setValue("size", size() );


        QMainWindow::saveState();
        QWidget::closeEvent(event);
        SETTINGS->setMaximizeWindow(false);
    }
}
//////////////////////////////////////////////////////////////////////
void ImageViewer::closeEvent(QCloseEvent *event)
{
    if(maybeSave() && !handleCloseTabs())
    {
       saveGeometryState(event);
       event->accept();
    }
    else
    {
        event->ignore();
    }

}

//////////////////////////////////////////////////////////////////////
void ImageViewer::on_actionQuit_triggered()
{
    if (!handleCloseTabs())
    {
        saveGeometryState(new QCloseEvent());
        // add dealog to save changes
        qApp->quit();
    }
}
//////////////////////////////////////////////////////////////////////
void ImageViewer::on_actionClose_triggered()
{
   //    handleCloseChildWindow(ui->centralwidget->currentSubWindow());
}
//////////////////////////////////////////////////////////////////////
void ImageViewer::on_actionClose_all_triggered()
{
    handleCloseTabs();
}
//////////////////////////////////////////////////////////////////////
void ImageViewer::on_action_Exit_triggered()
{
    if(maybeSave())
    {
        on_actionQuit_triggered();
    }
    else
    {
        const QString message = tr("Please save the file before You close it :(");
        statusBar()->showMessage(message);
    }

}




void ImageViewer::on_action_New_triggered()
{
    if(maybeSave())
    {
    imageLabel->clear();
    imageLabel->setBackgroundRole(QPalette::Dark);
    image = QImage();
    updateActions();
    statusBar()->showMessage(tr("Open new file"));
    }
    else
    {
        const QString message = tr("Please save the file before You open a new document :)");
        statusBar()->showMessage(message);
    }


}

void ImageViewer::setModified(bool modified)
{
    this->isModified = modified;
}

void ImageViewer::on_action_Dark_Mood_triggered()
{
    if(isDarkmood == false)
    {
        QFile f(":qdarkstyle/style.qss");

        if (!f.exists())   {
            printf("Unable to set stylesheet, file not found\n");
        }
        else   {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            qApp->setStyleSheet(ts.readAll());
        }
       isDarkmood = true;
       ui->menu_View->actions().last()->setText(tr("Normal Mood"));
       ui->menu_View->actions().last()->setIcon(QIcon(":/images/images/Solid_gray.png"));
       return;
    }
    else
    {
        isDarkmood = false;
        ui->menu_View->actions().last()->setText(tr("Dark Mood"));
        ui->menu_View->actions().last()->setIcon(QIcon(":/images/images/pixmaps/logo.svg"));
        QQuickStyle::setStyle("Default");
        QQuickStyle::setFallbackStyle("Material");
        qApp->setStyleSheet("Material");
        QQmlApplicationEngine engine;
        engine.load(QUrl("qrc:/main.qml"));
        return;

    }
}

void ImageViewer::loadLanguage(const QString& rLanguage)
{
    QLocale locale = QLocale(rLanguage);
    QLocale::setDefault(locale);
    QString languageName = QLocale::languageToString(locale.language());
    QString path = QApplication::applicationDirPath();
    QString filename = QString("/ImageEditor_%1.qm").arg(rLanguage);
    qApp->removeTranslator(&m_translator);
    if (m_translator.load(path+filename))
         qApp->installTranslator(&m_translator);

//    filename = QString("/qt_%1.qm").arg(rLanguage);
//    qApp->removeTranslator(&m_translatorQt);
//    if(m_translatorQt.load(path+filename))
//        qApp->installTranslator(&m_translatorQt);
}

void ImageViewer::on_actionGerman_triggered()
{
        loadLanguage("de");
        ui->actionEnglish->setChecked(false);
        ui->actionGerman->setChecked(true);
}

void ImageViewer::on_actionEnglish_triggered()
{
        loadLanguage("en");
        ui->actionEnglish->setChecked(true);
        ui->actionGerman->setChecked(false);
}

void ImageViewer::createLanguageMenu(void)
{
    // format systems language
     QString defaultLocale = QLocale::system().name(); // e.g. "de_DE"
     defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"

     m_langPath = QApplication::applicationDirPath();
     m_langPath.append("/languages");
     QDir dir(m_langPath);
     QStringList fileNames = dir.entryList(QStringList("ImageEditor_*.qm"));
     for (int i = 0; i < fileNames.size(); ++i) {
      // get locale extracted by filename
      QString locale;
      locale = fileNames[i]; // "ImageEditor_de.qm"
      locale.truncate(locale.lastIndexOf('.')); // "ImageEditor_de"
      locale.remove(0, locale.indexOf('_') + 1); // "de"

     QString lang = QLocale::languageToString(QLocale(locale).language());
     QIcon ico(QString("%1/%2.png").arg(m_langPath).arg(locale));

      QAction *action = new QAction(ico, lang, this);
      action->setCheckable(true);
      action->setData(locale);
     // set default translators and language checked
      if (defaultLocale == locale)
      {
          action->setChecked(true);
      }
    }
}

void ImageViewer::changeEvent(QEvent* event)
{
 if(0 != event) {
     try {
  switch(event->type()) {
   // this event is send if a translator is loaded
   case QEvent::LanguageChange:
    ui->retranslateUi(this);
    break;

   // this event is send, if the system, language changes
   case QEvent::LocaleChange:
   {
    QString locale = QLocale::system().name();
    locale.truncate(locale.lastIndexOf('_'));
    loadLanguage(locale);
   }
   break;
  default:
      break;
  }
     } catch (QException e) {
         QErrorMessage* q= new QErrorMessage(this);
         q->showMessage(e.what());
     }
 }
 QMainWindow::changeEvent(event);
}

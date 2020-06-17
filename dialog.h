#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
int r,g,b;
int getR(){return r;}
    void sliderActivation(bool isRGBActive);

    bool getIsRGBActive() const;
    void setIsRGBActive(bool value);

signals:
    void rgbChanged(float , float ,float );
    void yuvChanged(float Y, float U,float V);

private slots:
    void on_RGBradioButoon_clicked();

    void on_YUVradioButton_clicked();

    void on_pushButton_clicked();

private:
    Ui::Dialog *ui;
    bool isRGBActive;

};

#endif // DIALOG_H

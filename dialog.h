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

    void on_R_actionTriggered(int action);

    void on_G_actionTriggered(int action);

    void on_B_actionTriggered(int action);

    void on_Y_actionTriggered(int action);

    void on_U_actionTriggered(int action);

    void on_V_actionTriggered(int action);

private:
    Ui::Dialog *ui;
    bool isRGBActive;
    bool isPreview=false;

};

#endif // DIALOG_H

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

    void sliderActivation(bool isRGBActive);

    bool getIsRGBActive() const;
    void setIsRGBActive(bool value);

Q_SIGNALS:
    void rgbChanged(float r, float g,float b);
    void yuvChanged(float Y, float U,float V);

private slots:
    void on_RGBradioButoon_clicked();

    void on_YUVradioButton_clicked();

    void on_pushButton_clicked();

    void on_R_actionTriggered(int action);

private:
    Ui::Dialog *ui;
    bool isRGBActive;
};

#endif // DIALOG_H

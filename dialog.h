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

private slots:
    void on_RGBradioButoon_clicked();

    void on_YUVradioButton_clicked();

private:
    Ui::Dialog *ui;
    bool isRGBActive;
};

#endif // DIALOG_H

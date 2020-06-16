#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::sliderActivation(bool isRGBActive)
{
    if(isRGBActive)
    {
        ui->R->setEnabled(true);
        ui->G->setEnabled(true);
        ui->B->setEnabled(true);
        ui->Y->setEnabled(false);
        ui->U->setEnabled(false);
        ui->V->setEnabled(false);
    }
    else
    {
        ui->R->setEnabled(false);
        ui->G->setEnabled(false);
        ui->B->setEnabled(false);
        ui->Y->setEnabled(true);
        ui->U->setEnabled(true);
        ui->V->setEnabled(true);
    }
}

bool Dialog::getIsRGBActive() const
{
    return isRGBActive;
}

void Dialog::setIsRGBActive(bool value)
{
    isRGBActive = value;
}

void Dialog::on_RGBradioButoon_clicked()
{
    sliderActivation(true);
}

void Dialog::on_YUVradioButton_clicked()
{
    sliderActivation(false);
}

void Dialog::on_pushButton_clicked()
{

}


#include "startupscreen.h"
#include "ui_startupscreen.h"
#include <QDesktopWidget>

startupScreen::startupScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::startupScreen)
{
    ui->setupUi(this);

    QDesktopWidget desktop;
    int desktopHeight=desktop.geometry().height();
    int desktopWidth=desktop.geometry().width();
    ui->webView->resize(desktopWidth,desktopHeight);


}

startupScreen::~startupScreen()
{
    delete ui;
}

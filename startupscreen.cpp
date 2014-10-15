#include "startupscreen.h"
#include "ui_startupscreen.h"

startupScreen::startupScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::startupScreen)
{
    ui->setupUi(this);
}

startupScreen::~startupScreen()
{
    delete ui;
}

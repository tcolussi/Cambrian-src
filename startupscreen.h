#ifndef STARTUPSCREEN_H
#define STARTUPSCREEN_H

#include <QWidget>

namespace Ui {
class startupScreen;
}

class startupScreen : public QWidget
{
    Q_OBJECT

public:
    explicit startupScreen(QWidget *parent = 0);
    ~startupScreen();

private:
    Ui::startupScreen *ui;
};

#endif // STARTUPSCREEN_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "passwordcallback.hpp"
#include <../opentxs/OTAPI.hpp>
#include <../opentxs/OTAPI_Exec.hpp>
#include <../opentxs/OTAsymmetricKey.hpp>
#include <../opentxs/OTRecordList.hpp>
#include <../opentxs/OTCaller.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void appStarting();
private slots:
    void slot_RoleWindow();



    void on_comboBox_activated(const QString &arg1);

private:
    Ui::MainWindow *ui;
    bool SetupPasswordCallbackM(OTCaller & passwordCaller, OTCallback & passwordCallback);
    bool SetupAddressBookCallbackM(OTLookupCaller & theCaller, OTNameLookup & theCallback);
};


#endif // MAINWINDOW_H



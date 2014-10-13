#include "mainwindow.h"
#include "ui_mainwindow.h"
#ifndef __STABLE_HPP__
#include "stable.hpp"
#endif



#include "passwordcallback.hpp"

#include <../opentxs/OTAPI.hpp>

#include <../opentxs/OTAPI_Exec.hpp>
#include <../opentxs/OTAsymmetricKey.hpp>
#include <../opentxs/OTRecordList.hpp>
#include <../opentxs/OTCaller.hpp>
#include <core/OTX.hpp>
#include <QVBoxLayout>
#include <QDebug>
#include <QSystemTrayIcon>
#include <QDialog>
#include <QLabel>
#include <QToolButton>
#include <QDesktopWidget>
#include <QComboBox>
#include "RoleComboBox.h"

using namespace std;

bool MainWindow::SetupPasswordCallbackM(OTCaller & passwordCaller, OTCallback & passwordCallback)
{
    passwordCaller.setCallback(&passwordCallback);

    bool bSuccess = OT_API_Set_PasswordCallback(passwordCaller);

    if (!bSuccess)
    {
        qDebug() << QString("Error setting password callback!");
        return false;
    }

    return true;
}


bool MainWindow::SetupAddressBookCallbackM(OTLookupCaller & theCaller, OTNameLookup & theCallback)
{
    theCaller.setCallback(&theCallback);

    bool bSuccess = OT_API_Set_AddrBookCallback(theCaller);

    if (!bSuccess)
    {
        qDebug() << QString("Error setting address book callback!");
        return false;
    }

    return true;
}

void MainWindow::on_comboBox_activated(QString const& arg1)
{
    cout << "ComboActivated";

}

MainWindow::MainWindow(QWidget *parent)


{

}


void MainWindow::appStarting()
{
    // ----------------------------------------
    //Moneychanger Details
    QString mc_app_name = "moneychanger-qt";
    QString mc_version = "v0.0.x";

    //Compiled details
    QString mc_window_title = mc_app_name+" | "+mc_version;
    // ----------------------------------------
    // Set Password Callback.
    //
    static OTCaller           passwordCaller;




     static MTPasswordCallback passwordCallbackM;

    if (!SetupPasswordCallbackM(passwordCaller, passwordCallbackM))
    {
        qDebug() << "Failure setting password callback in MTApplicationMC";
        abort();
 }
    // ----------------------------------------
    // Set Address Book Callback.
    //
    static OTLookupCaller theCaller;



    // ----------------------------------------
    // Load OTAPI Wallet
    //
    OTAPI_Wrap::It()->LoadWallet();
    // ----------------------------------------

   // FAKE MENU .....

    QMenuBar * MenuBar = new QMenuBar;

    QMenu *Menu1 = new QMenu("Peers");
    Menu1->addAction("&New..(Fake)");
    Menu1->addAction("Other.. (fake)");
    Menu1->addAction("Connect.. (fake)");

    QMenu *Menu2 = new QMenu("Aplications");
    Menu2->addAction("&Ballot");
    Menu2->addAction("Pomo");
    Menu2->addAction("Other");

    QAction *RoleMgmt = new QAction(tr("&Role Management"), this);

         RoleMgmt->setStatusTip(tr("Create a new Role"));
         connect(RoleMgmt, SIGNAL(triggered()), this, SLOT(slot_RoleWindow()));

    QMenu *Menu3 = new QMenu("Advanced");
    Menu3->addAction(RoleMgmt);
    Menu3->addAction("Bitcoins");
    Menu3->addAction("Messages");



    MenuBar->addMenu(Menu1);
    MenuBar->addMenu(Menu2);
    MenuBar->addMenu(Menu3);


    setMenuBar(MenuBar);
    MenuBar->hide();
    //resize(400, 400);


    //
    // Buttons
    QToolButton * buttonIdentities = new QToolButton;
    QPixmap pixmapIdentities(":identity");
    QIcon identitiesButtonIcon(pixmapIdentities);
    buttonIdentities->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    buttonIdentities->setIcon(identitiesButtonIcon);
    buttonIdentities->setIconSize(pixmapIdentities.rect().size());
    buttonIdentities->setText("Manage Roles");
     buttonIdentities->setStyleSheet("QWidget{ min-width: 10em;background-color: green; border-style: outset; border-width: 2px;border-radius: 10px; border-color: yellow;font: bold 14px;padding: 6px;}");


    buttonIdentities->setAutoFillBackground(true);
    buttonIdentities->setDown(false);



     // Buttons
    QToolButton * QuitButton = new QToolButton;
    QPixmap pixmapQuit(":Quit");
    QIcon QuitButtonIcon(pixmapQuit);
    QuitButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    QuitButton->setIcon(QuitButtonIcon);
   QuitButton->setIconSize(pixmapIdentities.rect().size());
   QuitButton->setText("Quit");
   QuitButton->setStyleSheet("QWidget{ min-width: 10em;background-color: green; border-style: outset; border-width: 2px;border-radius: 10px; border-color: yellow;font: bold 14px;padding: 6px;}");


     // Buttons Layout
     QWidget     * pUserBarWidget        = new QWidget;
     QGridLayout * pUserBarWidget_layout = new QGridLayout;

    // pUserBarWidget_layout->setSpacing(10);
     //pUserBarWidget_layout->setContentsMargins(1,1,1 ,1); // left top right bottom


     pUserBarWidget->setLayout(pUserBarWidget_layout);
     pUserBarWidget->setStyleSheet("QWidget{background-color:white;selection-background-color:#a0aac4;}");

     QHBoxLayout * pButtonLayout = new QHBoxLayout;
     pUserBarWidget_layout->addLayout(pButtonLayout, 1, 1, 1,1, Qt::AlignLeft);


     //Add button to Layout

     pButtonLayout->addWidget(buttonIdentities);


     pButtonLayout->addWidget(QuitButton);

     //attach event to button
     connect(buttonIdentities, SIGNAL(clicked()),  OTX::It(this,false), SLOT(mc_defaultnym_slot()));
     connect(QuitButton,SIGNAL(clicked()),this,SLOT(close()));

      // Combo box  with current roles simulation

         QComboBox *combo = new RoleComboBox(pUserBarWidget);
         QLabel *label = new QLabel("Switch Roles:");

         int nymCount = OTAPI_Wrap::It()->GetNymCount();

         if (nymCount > 0)
         {
         std::string nymid = OTAPI_Wrap::GetNym_ID(0);

         for (int i=1; i <= nymCount; i= i + 1)
         {

         combo->addItem(QString::fromUtf8(OTAPI_Wrap::GetNym_Name(nymid).c_str()));
         nymid = OTAPI_Wrap::GetNym_ID(i);
         }
          combo->addItem("<Create New Role..>");
         }
         pButtonLayout->addWidget(label);
         pButtonLayout->addWidget(combo);




     pUserBarWidget->setParent(this);

    //get screen coordinates
    QDesktopWidget desktop;

    int desktopWidth=desktop.geometry().width();

    //pUserBarWidget->show();
   pUserBarWidget->resize(desktopWidth,100);

   //this->setFixedHeight(100);
  // this->setFixedWidth(desktopWidth) ;

   //pUserBarWidget->showMaximized();

   this->showMaximized();

   // this->show();



}

void MainWindow::slot_RoleWindow()
{

     OTX::It()->mc_defaultnym_slot();
}
MainWindow::~MainWindow()
{
    // Destroy all the signals
    OTX::It(this, true);
}



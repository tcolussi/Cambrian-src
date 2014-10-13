#include <../src/opentxs/OTAPI.hpp>
#include <../src/opentxs/OTAPI_Exec.hpp>
#include <../src/opentxs/OTLog.hpp>
#include <../src/opentxs/OTPaths.hpp>

#include "MainWindow.h"
#include <QApplication>
#include <QTimer>
#include <iostream>



class __OTclient_RAII
{
public:
    __OTclient_RAII()
    {
        // SSL gets initialized in here, before any keys are loaded.
        OTAPI_Wrap::AppInit();
    }
    ~__OTclient_RAII()
    {

        OTAPI_Wrap::AppCleanup();
    }
};


// ----------------------------------------



int smain(int argc, char *argv[])
{

    //
    __OTclient_RAII the_client_cleanup;  // <===== SECOND constructor is called here.
    // ----------------------------------------
    if (NULL == OTAPI_Wrap::It())
    {
        OTLog::vError(0, "Error, exiting: OTAPI_Wrap::AppInit() call must have failed.\n");
        return -1;
    }
    // ----------------------------------------
    //Init qApp
   QApplication a(argc, argv);
    MainWindow w;
    QTimer::singleShot(0, &w, SLOT(appStarting()));


    return a.exec();
    OTAPI_Wrap::It()->LoadWallet();
    std::string nymid=OTAPI_Wrap::It()->GetNym_ID(1);
    std::string nymname=OTAPI_Wrap::It()->GetNym_Name(nymid);
   std::cout <<"Nymid: "+nymid+"  Role Name: "+ nymname;
    return 0;

    // ----------------------------------------------------------------



}

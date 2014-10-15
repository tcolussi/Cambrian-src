#ifndef OTX_WRAP_H
/*
This Class Wraps all the Gui and Libs of OT
It is supposed to be declared once in the main window

*/

#define OTX_WRAP_H
#ifndef PRECOMPILEDHEADERS_H
    #include "PreCompiledHeaders.h"
#endif
#ifdef COMPILE_WITH_OPEN_TRANSACTIONS
#include <Qwidget>
#include <QPointer>
#include <core/passwordcallback.hpp>
#include "OTX.hpp"
#include <core/handlers/contacthandler.hpp>
#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OTAsymmetricKey.hpp>
#include <opentxs/OTRecordList.hpp>
#include <opentxs/OTCaller.hpp>

#include <QDebug>

class OTX_WRAP
{



public:

    enum Constants{
     RCS_NO_ACTION_REQUIRED = -1,
     RCS_ACTION_CANCEL = 0,
     RCS_ACTION_NYM_CREATED = 1,
     RCS_ACTION_NYM_SWITCH = 2,
     RCS_ACTION_NYM_DELETED = 3
   };

    QWidget *pParentWidget;

    OTX_WRAP(QWidget *parent);



    std::string getNym_name(std::string nymId);
    std::string getNymID(int32_t nymIndex);
    std::string getNymPublicKey(std::string nymid);
    void openRoleCreationScreen();

    //Signin and Encryption
    QString signText(QString s_nymId, QString qstrText);
    QString encryptText(QString e_nymId, QString plainText);
    QString signAndEncrypt(QString signerNymId, QString recipientNymId, QString plainText);

    // Verify Signature and Decrypt functions
     bool verifySignature(QString s_nymId, QString signedPlainText, QString &messagePayload);
     QString decryptText(QString nymId,QString encryptedText);//decrypt using current NYM
     bool decryptAndVerify(QString signerNymId,QString recipientNymId,QString signedEncryptedText,QString &decryptedText);


 ~OTX_WRAP();
 private:
 // It will be invoked only once per run in the constructor of this class.
    bool SetupAddressBookCallback(OTLookupCaller & theCaller, OTNameLookup & theCallback);
    bool SetupPasswordCallback(OTCaller & passwordCaller, OTCallback & passwordCallback);
    void LoadWallewithPassprhase();


};

#endif // OTX_WRAP_H
#endif // USING OT

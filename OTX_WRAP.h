#ifdef COMPILE_WITH_OPEN_TRANSACTIONS

#ifndef OTX_WRAP_H
#define OTX_WRAP_H

/*
This Class Wraps all the Gui and Libs of OT
It is supposed to be declared once in the main window

*/


#ifndef PRECOMPILEDHEADERS_H
#include "PreCompiledHeaders.h"
#endif


#include <Qwidget>
#include <QPointer>
#include <QDebug>
#include <iostream>
#include <string>
#include <string.h>
#include <QMainWindow>
#include <QComboBox>
#include <QDebug>
#include <QMessageBox>
#include <QClipboard>
#include <qdebug>

#if defined(_WINDOWS_)
#include <winsock2.h>
#endif

#include <core/passwordcallback.hpp>

#include <opentxs/OTLog.hpp>
#include <opentxs/OTCrypto.hpp>
#include <opentxs/OTAsymmetricKey.hpp>
#include <opentxs/OTSymmetricKey.hpp>
#include <opentxs/OTRecordList.hpp>
#include <opentxs/OTCaller.hpp>
#include <opentxs/OT_ME.hpp>
#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OpenTransactions.hpp>
#include <opentxs/OTASCIIArmor.hpp>
#include <opentxs/OTEnvelope.hpp>
#include <opentxs/OTPseudonym.hpp>
#include <opentxs/OTPasswordData.hpp>
#include <opentxs/OTSignedFile.hpp>
#include <opentxs/OTContract.hpp>
#include <core/handlers/contacthandler.hpp>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <zlib.h>

#include "passwordcallback.hpp"
#include "filedownloader.h"
#include "OTX.hpp"


class OTX_WRAP : public QObject
{
public:
	std::string decompress_string(const std::string& str);
	std::string compress_string(const std::string& str,
								int compressionlevel);

	QWidget *pParentWidget;

	enum Constants{
		RCS_NO_ACTION_REQUIRED = -1,
		RCS_ACTION_CANCEL = 0,
		RCS_ACTION_NYM_CREATED = 1,
		RCS_ACTION_NYM_SWITCH = 2,
		RCS_ACTION_NYM_DELETED = 3
	};

	OTX_WRAP(QWidget *parent);
	~OTX_WRAP();

	std::string getNym_name(std::string nymId);
	std::string getNymID(int32_t nymIndex);
	std::string getNymPublicKey(std::string nymid);
	void openRoleCreationScreen();

	//Signin and Encryption

	void handleErrors(void);
	// OpenSSl calls with unsigned chars
	int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
				unsigned char *iv, unsigned char *ciphertext);
	int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
				unsigned char *iv, unsigned char *plaintext);
	int symmetricDecrypt(unsigned char ciphertext[1024], unsigned char (&plainText)[1024]);
	int symmetricEncrypt(unsigned char * plainText, unsigned char (&encrypted)[1024]);

	// symmetric encryption adapted to use std string
	std::string symmetricDecStr(std::string encText);
	std::string symmetricEncStr(std::string plainText);
	QString signText(QString s_nymId, QString qstrText);
	QString encryptText(QString e_nymId, QString plainText);
	QString signAndEncrypt(QString signerNymId, QString recipientNymId, QString plainText);

	// Verify Signature and Decrypt functions
	bool verifySignature(QString s_nymId, QString signedPlainText, QString &messagePayload);
	QString decryptText(QString nymId,QString encryptedText);//decrypt using current NYM
	bool decryptAndVerify(QString signerNymId,QString recipientNymId,QString signedEncryptedText,QString &decryptedText);

	// Contract handle
	void openContractOTServerScreen();
	bool addOTServerContracts();
	std::string createNym(std::string name,int keysize);
	int publishNymAllServers(std::string nymId);

public slots:
	void SL_DownloadedURL();

private:
	// It will be invoked only once per run in the constructor of this class.
	FileDownloader * m_pImgCtrl;
	bool SetupAddressBookCallback(OTLookupCaller & theCaller, OTNameLookup & theCallback);
	bool SetupPasswordCallback(OTCaller & passwordCaller, OTCallback & passwordCallback);
	void LoadWallewithPassprhase();
	void addOTServerContract(QString Url);
};

#include <OTX_WRAP.h>
extern  OTX_WRAP * pOTX;


#endif // OTX_WRAP_H
#endif // COMPILE_WITH_OPEN_TRANSACTIONS

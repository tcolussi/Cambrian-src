
#ifndef PRECOMPILEDHEADERS_H
    #include "PreCompiledHeaders.h"
#endif

#ifdef COMPILE_WITH_OPEN_TRANSACTIONS
#include <iostream>

#include <QMainWindow>
#include <QComboBox>
#include "passwordcallback.hpp"

#include <../opentxs/OTAsymmetricKey.hpp>
#include <../opentxs/OTRecordList.hpp>
#include <../opentxs/OTCaller.hpp>
#include <QDebug>
#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OT_ME.hpp>
#include <opentxs/OpenTransactions.hpp>
#include <OTCrypto.hpp>
#include <opentxs/OTASCIIArmor.hpp>
#include <opentxs/OTEnvelope.hpp>
#include <opentxs/OTPseudonym.hpp>
#include <opentxs/OTPasswordData.hpp>
#include <opentxs/OTSignedFile.hpp>
#include <opentxs/OTContract.hpp>
#include <core/handlers/contacthandler.hpp>
#include <QMessageBox>
#include <QClipboard>
#include <qdebug>
#include <string>

static OTCaller           passwordCaller;
static MTPasswordCallback passwordCallback;

OTX_WRAP::OTX_WRAP(QWidget *parent)
{
    pParentWidget= parent;




    this->LoadWallewithPassprhase();

}

bool OTX_WRAP::SetupPasswordCallback(OTCaller & passwordCaller, OTCallback & passwordCallback)
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


bool OTX_WRAP::SetupAddressBookCallback(OTLookupCaller & theCaller, OTNameLookup & theCallback)
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

QString OTX_WRAP::signText(QString s_nymId,QString qstrText)
{

    std::string  str_nym    (s_nymId.toStdString());
    OTString     strNym     (str_nym.c_str());
    OTIdentifier nym_id     (strNym);
    std::string  str_text   (qstrText.toStdString());
    OTString     strText    (str_text.c_str());


    QString signedText;

    OTPasswordData thePWData("Signer passphrase");

    OTPseudonym * pNym = OTAPI_Wrap::OTAPI()->GetOrLoadPrivateNym(nym_id,
                                                           false, //bChecking=false
                                                           "DlgEncrypt::on_pushButtonEncrypt_clicked",
                                                           &thePWData);



    if (NULL == pNym) //signer not loaded
    {
        QString qstrErrorMsg = QString("%1: %2").arg("Failed loading the signer; unable to continue. NymID").arg(s_nymId);
        QMessageBox::warning(pParentWidget, "Failed Loading Signer", qstrErrorMsg);
        return "Failed to load signer";
    }
    else //signer loaded
    {

        OTString     strSignedOutput;
        OTSignedFile theSignedFile;

        theSignedFile.SetFilePayload(strText);
        theSignedFile.SignContract(*pNym, &thePWData);
        theSignedFile.SaveContract();
        theSignedFile.SaveContractRaw(strSignedOutput);



        if (!strSignedOutput.Exists()) // The text´s signature fail
        {
            QMessageBox::warning(pParentWidget, "Signing Failed",
                                 "Failed trying to sign, using the selected identity.");
            return "Signing Failed";
        }
        else if (!theSignedFile.VerifySignature(*pNym)) //sign verification
        {
            QMessageBox::warning(pParentWidget, "Test Verification Failed",
                                 "Failed trying to test verify, immediately after signing. Trying authentication key...");

            if (!theSignedFile.VerifySigAuthent(*pNym))
            {
                QMessageBox::warning(pParentWidget, "Authent Test Also Failed",
                                     "Failed trying to verify signature with authentication key as well.");
                return "Failed veryfiing Signature";
            }
            else
                QMessageBox::information(pParentWidget, "SUCCESS USING AUTHENTICATION KEY", "Tried authent key instead of signing key, and it worked!");
        }
        else
        {
             signedText=strSignedOutput.Get();
         }
    } // else (we have pNym.)

return signedText;
}
QString OTX_WRAP::encryptText(QString e_nymId, QString plainText)
{
QString encryptedText="Failed to encrypt : \n "+ plainText;// if encryption fails notify and return plain text
OTString strSignerNymID(e_nymId.toStdString().c_str());
OTIdentifier signer_nym_id(strSignerNymID);
setOfNyms setRecipients;

if (!signer_nym_id.IsEmpty())
{
    OTPasswordData thePWData("Load Credentials");

    OTPseudonym * pNym = OTAPI_Wrap::OTAPI()->GetOrLoadNym(signer_nym_id,
                                                           false, //bChecking=false
                                                           "DlgEncrypt::on_pushButtonEncrypt_clicked",
                                                           &thePWData);
    if (NULL == pNym)
    {
        QString qstrErrorMsg = QString("%1: %2").
                arg("Failed trying to load the signer ").arg(e_nymId);
        QMessageBox::warning(pParentWidget, "Failed Loading Signer", qstrErrorMsg);
    }
    else
    {
        setRecipients.insert(setRecipients.begin(), pNym);

        OTEnvelope theEnvelope;
        OTString   strInput(plainText.toStdString().c_str());

        if (!theEnvelope.Seal(setRecipients, strInput))
        {
            QMessageBox::warning(pParentWidget, "Encryption Failed",
                                 "Failed trying to encrypt message.");
            return encryptedText;//return plain text with failure message
        }
        else
        {
            // Success encrypting!
            //
            OTString     strOutput;
            OTASCIIArmor ascCiphertext(theEnvelope);

            if (ascCiphertext.WriteArmoredString(strOutput, "ENVELOPE")) // -----BEGIN OT ARMORED ENVELOPE-----
            {
                std::string str_output(strOutput.Get());
                encryptedText = QString::fromStdString(str_output);
            }
        }

    }


}

return encryptedText;
}

QString OTX_WRAP::signAndEncrypt(QString signerNymId,QString recipientNymId,QString plainText)
{
  QString signedAndEncrypted=encryptText(recipientNymId,signText(signerNymId,plainText));
  return signedAndEncrypted;
}

// Decryption Functions
bool OTX_WRAP::verifySignature(QString s_nymId, QString signedPlainText,QString &messagePayload)
{
 std::string str_input=signedPlainText.toStdString();
 OTString strInput=signedPlainText.toStdString();
 bool bSuccessVerifying = false;

 if (strInput.Contains("-----BEGIN SIGNED FILE-----"))
 {
     OTSignedFile theSignedFile;

    if (theSignedFile.LoadContractFromString(strInput))
     {


        OTString     strNym     (s_nymId.toStdString().c_str());
        OTIdentifier nym_id     (strNym);


        if (s_nymId.length() > 0)
        {
            OTPasswordData thePWData("load credentials");

            OTPseudonym * pNym = OTAPI_Wrap::OTAPI()->GetOrLoadNym(nym_id,
                                                                   false, //bChecking=false
                                                                   "DlgEncrypt::on_pushButtonDecrypt_clicked",
                                                                   &thePWData);
            if (NULL != pNym)
            {
                if (theSignedFile.VerifySignature(*pNym, &thePWData))
                {
                    bSuccessVerifying = true;

                    OTString strContents = theSignedFile.GetFilePayload();

                    if (strContents.Exists())
                    {
                        strInput  = strContents;
                        str_input = strInput.Get();
                         // armored payload


                        if(strInput.DecodeIfArmored(false)  ){

                           std::string str_decoded(strInput.Get());
                           QString messagePayloadDecoded(str_decoded.c_str());
                           messagePayload=messagePayloadDecoded;
                        } else // not armored
                        {

                         strInput  = strContents;
                         str_input = strInput.Get();
                         messagePayload=QString::fromStdString(str_input);
                        }


                    }
                } // signature verified
            } // pNym exists
        } // if str_signer_nym exists


     } // signed file: load contract from string.
 } // "BEGIN SIGNED FILE"
 return bSuccessVerifying;
}

QString OTX_WRAP::decryptText(QString nymId,QString encryptedText)
{
QString decryptedText="<Failed>";
std::string str_input(encryptedText.toStdString());
OTString  strInput (str_input.c_str());
if (strInput.Exists())
{
   if (strInput.Contains("-----BEGIN OT ARMORED ENVELOPE-----"))
    {
        OTEnvelope theEnvelope;

        if (theEnvelope.SetFromBookendedString(strInput))
        {
            OTString strOutput;
            // -------------------------
            // First we'll try the default nym, if one is available.
            //
            QString qstrTempID = nymId;

            if (!qstrTempID.isEmpty())
            {
                std::string  str_nym    (qstrTempID.toStdString());
                OTString     strNym     (str_nym.c_str());
                OTIdentifier nym_id     (strNym);

                if (!nym_id.IsEmpty())
                {
                    OTPasswordData thePWData("Recipient passphrase");

                    OTPseudonym * pNym = OTAPI_Wrap::OTAPI()->GetOrLoadPrivateNym(nym_id,
                                                                           false, //bChecking=false
                                                                           "DlgEncrypt::on_pushButtonDecrypt_clicked",
                                                                           &thePWData);
                    if (NULL != pNym)
                    {
                        if (theEnvelope.Open(*pNym, strOutput) && strOutput.Exists())
                        {
                            strInput  = strOutput;
                            str_input = strInput.Get();
                            decryptedText  = QString::fromStdString(str_input);
                        }
                    }
                }
            }
            // ------------

            // -----------------------
        } // if (theEnvelope.SetFromBookendedString(strInput))

    } // if (strInput.Contains("-----BEGIN OT ARMORED ENVELOPE-----"))
}
return decryptedText;
}

bool OTX_WRAP::decryptAndVerify(QString signerNymId, QString recipientNymId,
                                  QString signedEncryptedText, QString &decryptedText)
{

  QString signedDecrypted=decryptText(recipientNymId,signedEncryptedText);
  QString messagePayload;
  if (!signedDecrypted.contains("<Failed>")){
     if (verifySignature(signerNymId,signedDecrypted,messagePayload))
           decryptedText=messagePayload;
     else
        decryptedText="Signature Invalid for decrypted message: "+messagePayload;

  }
  else
  {decryptedText="Failed to Decrypt the Text with NymId "+recipientNymId;
   return false;}

    return true;
}



void OTX_WRAP::LoadWallewithPassprhase()
{
    QString mc_app_name = "SoproOTX";
    QString mc_version = "v0.0.1";

    //Compiled details


    if (!SetupPasswordCallback(passwordCaller, passwordCallback))
    {
        qDebug() << "Failure setting password callback in MTApplicationMC";
        abort();
    }
    // ----------------------------------------
    // Set Address Book Callback.
    //
    static OTLookupCaller theCaller;
    static MTNameLookupQT theCallback;

    if (!SetupAddressBookCallback(theCaller, theCallback))
    {
        qDebug() << "Failure setting address book callback in MTApplicationMC";
        abort();
    }
    // ----------------------------------------
    // Load OTAPI Wallet
    //
    OTAPI_Wrap::It()->LoadWallet();
    // ----------------------------------------




}

std::string OTX_WRAP::getNymID(int32_t nymIndex)
{
return OTAPI_Wrap::GetNym_ID(nymIndex);
}

std::string OTX_WRAP::getNym_name(std::string nymId)
{

return OTAPI_Wrap::GetNym_Name(nymId);

}

std::string OTX_WRAP::getNymPublicKey(std::string nymid)
{


return OTAPI_Wrap::GetNym_SourceForID(nymid);
}

void OTX_WRAP::openRoleCreationScreen()
{
    // Get the current profile in Cambrian

    TProfile * pProfile = new TProfile(&g_oConfiguration);	// At the moment, there is only one configuration object, so use the global variable
   // Get the count of current profiles in Cambrian
    int cProfiles;
    std::string nymId ="";// empty nymId means that the default nym must be selected in RoleCredentialScreen

    //Count all profiles in cambrian
    cProfiles=g_oConfiguration.m_arraypaProfiles.GetSize();


    if (cProfiles > 0 ) // There is at least 1 Role created (select current)
     {
        // get the current nym selected
        nymId = pProfile->m_strNymID;

     }
    int pRCS_Operation=0;
    std::string pRCS_nymId_Current=nymId;
    std::string pRCS_nymId_Operation=nymId;
    //OTX::It(pParentWidget)->mc_encrypt_show_dialog(false,true);
    OTX::It(pParentWidget)->mc_show_nym(QString::fromStdString(nymId),pRCS_Operation,pRCS_nymId_Current,pRCS_nymId_Operation);


    // Here, RCS_options holds the values of the nym selected or created (see the meaning of the array in OTX.hpp)

    // After any operation in CREDENTIAL SCREEN
    // Perform the requested operations in RoleCredentials Screen in Cambrian XML

   if (pRCS_Operation == RCS_ACTION_NYM_CREATED)
    {
      // Create a new register in Tprofile
      g_oConfiguration.m_arraypaProfiles.Add(PA_CHILD pProfile);

      std::string RoleName = pOTX->getNym_name(pRCS_nymId_Operation);
      std::string PK=pOTX->getNymPublicKey(pRCS_nymId_Operation);
      pProfile->m_strNameProfile.InitFromStringQ(QString::fromStdString(RoleName));
      pProfile->m_strNymID.InitFromStringQ(QString::fromStdString(pRCS_nymId_Operation));
      pProfile->m_strKeyPublic.InitFromStringQ(QString::fromStdString(PK));
      //Save the new role in xml (force)
      pProfile->m_pConfigurationParent->XmlConfigurationSaveToFile();
      NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(pProfile);
   }
   else if (pRCS_Operation ==RCS_ACTION_NYM_SWITCH) {}
   else if (pRCS_Operation ==RCS_ACTION_NYM_DELETED)  // Delete the selected nym and cambrian role acording to nynm
   {
       // Get All the profiles and select the profile who matches with the deleted nymid
       TProfile ** prgpProfiles = g_oConfiguration.m_arraypaProfiles.PrgpGetProfiles(OUT &cProfiles);
       TProfile * deleteProfile;
       for (int iProfile = 0; iProfile < cProfiles; iProfile++)
           {
            deleteProfile = prgpProfiles[iProfile];
            QString str_NymID =deleteProfile->m_strNymID.ToQString();
           if (pRCS_nymId_Operation.compare(str_NymID.toStdString())==0)
               break; // finish the loop when found  nymID
           }



      if (deleteProfile != NULL)
       {



          if(deleteProfile->m_arraypaAccountsXmpp.GetSize()|deleteProfile->m_arraypaApplications.GetSize())
                    {

                     std::string nymid_d=OTAPI_Wrap::CreateNym(1024,"","");
                     OTAPI_Wrap::SetNym_Name(nymid_d,nymid_d,deleteProfile->m_strNameProfile.ToQString().toStdString());
                     deleteProfile->m_strNymID.InitFromStringQ(QString::fromStdString(nymid_d));
                     EMessageBoxInformation("Before deleting your " d_sza_profile " '$S', you must manually delete all its accounts.", &deleteProfile->m_strNameProfile);
                    } else
                    {
                           deleteProfile->m_pConfigurationParent->m_arraypaProfiles.DeleteTreeItem(PA_DELETING deleteProfile);

                    }
           //Save Changes
           Assert(deleteProfile !=NULL);
           //deleteProfile->m_pConfigurationParent->XmlConfigurationSaveToFile();
           NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(NULL);	// After deleting a profile, display all the remaining profiles

       }

   }


}

OTX_WRAP::~OTX_WRAP()
{   //std::cout << "Destroying otx wrap";
    OTX::It(pParentWidget,true);
}
#endif



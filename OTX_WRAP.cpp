
#ifndef PRECOMPILEDHEADERS_H
    #include "PreCompiledHeaders.h"
#endif

#ifdef COMPILE_WITH_OPEN_TRANSACTIONS
#include <iostream>
#include <OTCrypto.hpp>
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

 std::string EncryptMessage(std::string str_nym,std::string publicKey, QString qstrText)
{
   // Sign the Plain Text

/*    OTString     strNym     (str_nym.c_str());
    OTIdentifier nym_id     (strNym);
    std::string  str_text   (qstrText.toStdString());
    OTString     strText    (str_text.c_str());
    OTPassword   passphrase;

    return "ENCRIPTION FAILED!!--PlainText=>";*/

}
//Decrypt message using the current nymid (loaded from Tprofile)
std::string DecryptMessage(std::string nymId,std::string encryptedText)
{

    return OTAPI_Wrap::Decrypt(nymId,encryptedText);

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



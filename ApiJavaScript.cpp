//	ApiJavaScript.cpp

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TApplicationBallotmaster.h"
#include "qdebug.h"
#ifdef COMPILE_WITH_OPEN_TRANSACTIONS
#include <opentxs/OpenTransactions.hpp>
#include <opentxs/OTPseudonym.hpp>

#endif
OJapiApps::OJapiApps(OJapiCambrian * poCambrian)
	{
    m_poCambrian = poCambrian;
	}

POJapiAppBallotmaster
OJapiApps::ballotmaster()
	{
	//MessageLog_AppendTextFormatCo(d_coGreen, "apps()");
	return m_poCambrian->polls();
    }

POJapiAppChat
OJapiApps::chat()
	{
	return m_poCambrian->chatApp();
	}

POJapiApps
OJapiCambrian::apps()
	{
	return &m_oApps;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
OJapiMe::OJapiMe(OJapiCambrian * poCambrian)
	{
	m_poCambrian = poCambrian;
	}

OJapiList
OJapiMe::groups()
	{
	CListVariants oList(m_poCambrian);
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		oList.AddGroupsMatchingType(IN pAccount->m_arraypaGroups, eGroupType_kzOpen);
		oList.AddGroupsMatchingType(IN pAccount->m_arraypaGroups, eGroupType_keAudience);
		/*
		TGroup ** ppGroupStop;
		TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
		while (ppGroup != ppGroupStop)
			{
			TGroup * pGroup = *ppGroup++;
			Assert(pGroup != NULL);
			Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
			//oList.append(pGroup->m_strNameDisplayTyped.ToQString());
			//oList.append(QVariant::fromValue(new OJapiGroup(pGroup)));
			oList.append(QVariant::fromValue(pGroup->POJapiGet()));
			} // while
		*/
		} // while
	//MessageLog_AppendTextFormatCo(d_coRed, "OJapiMe::groups() (length = $i)\n", oList.length());
	return oList;
	}

//	Return all channels objects
OJapiList
OJapiMe::channels()
	{
	CListVariants oList(m_poCambrian);
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		TGroup ** ppGroupStop;
		TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
		while (ppGroup != ppGroupStop)
			{
			TGroup * pGroup = *ppGroup++;
			Assert(pGroup != NULL);
			Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
			if (pGroup->Group_FuIsChannelUsed())
				oList.append(QVariant::fromValue(pGroup->POJapiGet(m_poCambrian)));
			} // while
		} // while
	return oList;
	}

//	Return a list of all channel names availables
OJapiList
OJapiCambrian::channelsAvailable()
	{
	//MessageLog_AppendTextFormatCo(d_coRed, "channelsAvailable()\n");
	QVariantList oList;
	CChannelName ** ppChannelNameStop;
    CChannelName ** ppChannelName = m_pProfile->m_arraypaChannelNamesAvailables.PrgpGetChannelsStop(OUT &ppChannelNameStop);
    while (ppChannelName != ppChannelNameStop)
		{
		CChannelName * pChannelName = *ppChannelName++;
		//MessageLog_AppendTextFormatCo(d_coRed, "Channel $S\n", &pChannelName->m_strName);
		oList.append(pChannelName->m_strName.ToQString());
		}
	return oList;
	}

/*
OJapiList
OJapiMe::peerLists()
	{
	CListVariants oList(m_poCambrian);
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		oList.AddGroupsMatchingType(IN pAccount->m_arraypaGroups, eGroupType_Audience);
		} // while
	MessageLog_AppendTextFormatCo(d_coBlack, "Groups List length end = $i\n", oList.length());
	return oList;
	}
*/
OJapiList
OJapiMe::peers()
	{
	CListVariants oList(m_poCambrian);
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);

    while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		oList.AddContacts(IN pAccount->m_arraypaContacts);
		}
	//MessageLog_AppendTextFormatCo(d_coRed, "OJapiMe::peers() (length = $i)\n", oList.length());
	return oList;
	}

/*
POJapiGroup
OJapiMe::newPeerList()
	{
	TAccountXmpp *pAccount = (TAccountXmpp*) m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PvGetElementFirst_YZ();
	if ( pAccount == NULL)
		return NULL;
	TGroup *paGroup = pAccount->Group_PaAllocateTemp(eGroupType_Audience);
	m_poCambrian->m_arraypaTemp.Add(PA_CHILD paGroup);
	return paGroup->POJapiGet(m_poCambrian);
	}

POJapiGroup
OJapiMe::newGroup()
{
TAccountXmpp *pAccount = (TAccountXmpp*) m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PvGetElementFirst_YZ();
	if ( pAccount == NULL)
		return NULL;
	TGroup *paGroup = pAccount->Group_PaAllocateTemp(eGroupType_Open);
	m_poCambrian->m_arraypaTemp.Add(PA_CHILD paGroup);
	return paGroup->POJapiGet(m_poCambrian);
}
*/

POJapiGroup
OJapiMe::newGroup(const QString &type)
	{
	TAccountXmpp *pAccount = (TAccountXmpp*) m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PvGetElementFirst_YZ();
	if ( pAccount == NULL)
		return NULL;

    EGroupType eGroupType = eGroupType_kzOpen;
	if ( type.compare("Open", Qt::CaseInsensitive) == 0)
		eGroupType = eGroupType_kzOpen;
	else if ( type.compare("Broadcast", Qt::CaseInsensitive) == 0 )
		eGroupType = eGroupType_keAudience;
	else
		return NULL;

	TGroup *paGroup = pAccount->Group_PaAllocateTemp(eGroupType);
	m_poCambrian->m_arraypaTemp.Add(PA_CHILD paGroup);
	return paGroup->POJapiGet(m_poCambrian);
	}

/*
POJapiGroup
OJapiMe::getPeerList(const QString & sId)
	{
	return getGroup(sId);
	}
*/

POJapiGroup
OJapiMe::getGroup(const QString & sId)
	{
	SHashSha1 hashId ;
	CStr strId = sId;
	HashSha1_FInitFromStringBase85_ZZR_ML(OUT &hashId, strId);

	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;

		TGroup ** ppGroupStop;
		TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
		while (ppGroup != ppGroupStop)
			{
			TGroup * pGroup = *ppGroup++;
			Assert(pGroup != NULL);
			Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));

			if ( HashSha1_FCompareEqual( &pGroup->m_hashGroupIdentifier, &hashId  ))
				return pGroup->POJapiGet(m_poCambrian);
			} // while

		} // while
    return NULL;
}

POJapiContact
OJapiMe::newPeer(const QString &sUsername)
    {
    TContact * pContactSelect = NULL;
    TContact * pContactDuplicate = NULL;
    CStr strContactsDuplicate;
    TAccountXmpp * pAccount =  Configuration_PGetAccountSelectedOrFirstAccount();
    CStr strUsername(sUsername);
    PSZUC pszUsername = strUsername.PszuGetDataNZ();
    if (pszUsername != NULL)
        {
        if (pAccount->m_strJID.FCompareStringsJIDs(pszUsername))
            return NULL;	// Skip the contact, since its JID is the same as its parent account JID

        // We have an invitation, so create the contact
        TContact * pContactInvitation = pAccount->Contact_PFindByJID(pszUsername, eFindContact_zDefault);
        if (pContactInvitation == NULL)
            {
            pContactSelect = pAccount->TreeItemAccount_PContactAllocateNewToNavigationTree_NZ(IN pszUsername);
            //pContactInvitation->TreeItemW_SelectWithinNavigationTree();
            }
        else
            {
            if (pContactInvitation->TreeItemFlags_FuIsInvisible())
                {
                pContactInvitation->TreeItemContact_DisplayWithinNavigationTreeAndClearInvisibleFlag();
                pContactSelect = pContactInvitation;
                }
            else
                {
                pContactDuplicate = pContactInvitation;
                strContactsDuplicate.AppendSeparatorAndTextU("\n", pszUsername);
                }
            }
        if (pContactSelect == NULL)
            {
            // No peer were added, so select one of the duplicate contact.  This dialog "Add Peers" can be used to search for a contact.
            pContactSelect = pContactDuplicate;
            }
        POJapiContact poContact = pContactSelect->POJapiGet();
        //pAccount->TreeItemW_Expand();
        return poContact;
        }
    else
        {
        //pAccount->TreeItemW_Expand();
         return NULL;
        }
    }

///////////////////////////////////////////////////////////////////////////////////////////////////
POJapiGroup
TGroup::POJapiGet(OJapiCambrian * poCambrian) CONST_MCC
	{
	if (m_paoJapiGroup == NULL)
		m_paoJapiGroup = new OJapiGroup(this, poCambrian);
	return m_paoJapiGroup;
	}

OJapiGroup::OJapiGroup(TGroup * pGroup, OJapiCambrian * poCambrian)
	{
	m_pGroup = pGroup;
	m_poCambrian = poCambrian;
	}

QString
OJapiGroup::id()
	{
	#if 0
	return HashSha1_ToQStringBase85(IN &m_pGroup->m_hashGroupIdentifier);
	#else
	return m_pGroup->Group_SGetIdentifier();
	#endif
	}

QString
OJapiGroup::name()
	{
	return m_pGroup->m_strNameDisplayTyped;
	}

void
OJapiGroup::name(const QString &sName)
	{
	MessageLog_AppendTextFormatCo(d_coBlue, "OJapiGroup.name($Q)\n", &sName);
	m_pGroup->m_strNameDisplayTyped = sName;
	}

int
OJapiGroup::count()
	{
	return m_pGroup->m_arraypaMembers.GetSize();
	}

OJapiList
OJapiGroup::members()
	{
	CListVariants oList(m_poCambrian);
	TGroupMember ** ppGroupMemberStop;
	TGroupMember ** ppGroupMember = m_pGroup->m_arraypaMembers.PrgpGetMembersStop(OUT &ppGroupMemberStop);
	while ( ppGroupMember != ppGroupMemberStop )
		{
		TGroupMember * pGroupMember = *ppGroupMember++;
		oList.AddContact(pGroupMember->m_pContact);
		}
	return oList;
	}

QString
OJapiGroup::type()
	{
    if (m_pGroup->Group_FuIsChannel())
		return "channel";
	switch (m_pGroup->EGetGroupType())
		{
	case eGroupType_kzOpen:
		return "open";
	case eGroupType_keAudience:
		return "broadcast";
	default:
		return c_sEmpty;
		}
	}

QString
OJapiGroup::channelName() const
	{
	return m_pGroup->m_strNameChannel_YZ;
	}
void
OJapiGroup::channelName(const QString & sNameChannel)
	{
    m_pGroup->GroupChannel_SetName(CStr(sNameChannel));
	}
QString
OJapiGroup::purpose() const
	{
	return m_pGroup->m_strPurpose;
	}

void
OJapiGroup::purpose(const QString & sPurpose)
	{
	m_pGroup->m_strPurpose = sPurpose;
	}

void
OJapiGroup::addPeer(QObject *pContactAdd)
	{
	OJapiContact * pContact = qobject_cast<OJapiContact *>(pContactAdd); // Make sure we received an object of proper type
	MessageLog_AppendTextFormatCo(d_coBlue, "addPeer($S)\n", &pContact->m_pContact->m_strJidBare);
	m_pGroup->Member_PFindOrAddContact_NZ(pContact->m_pContact);
	}

void
OJapiGroup::removePeer(QObject *pContactRemove)
	{
	OJapiContact * pContact = qobject_cast<OJapiContact *>(pContactRemove); // Make sure we received an object of proper type
	TGroupMember **ppGroupMemberStop;
	TGroupMember **ppGroupMember = m_pGroup->m_arraypaMembers.PrgpGetMembersStop(&ppGroupMemberStop);
	while ( ppGroupMember != ppGroupMemberStop )
		{
		TGroupMember *pGroupMember = *ppGroupMember++;
		if ( pGroupMember->m_pContact == pContact->m_pContact)
			{
			//MessageLog_AppendTextFormatCo(d_coBlue, "removing Peer ");
			m_pGroup->Member_Remove_UI(pGroupMember);
			}
		}
	}

void
OJapiGroup::save()
	{
	Assert(m_pGroup != NULL);
	m_pGroup->TreeItemFlags_SetFlagSerializeToDisk_Yes();
	m_pGroup->m_pAccount->m_arraypaGroups.ElementTransferFrom(m_pGroup, INOUT &m_poCambrian->m_arraypaTemp);	// Transfer the group from 'temp' to the account

	// for normal groups
	if ( m_pGroup->EGetGroupType() == eGroupType_kzOpen && m_pGroup->m_paTreeItemW_YZ == NULL )
		m_pGroup->TreeItemGroup_DisplayWithinNavigationTree();

	/*
	if (m_poCambrian->m_arraypaTemp.RemoveElementFastF(m_pGroup))
		m_pGroup->m_pAccount->m_arraypaGroups.Add(m_pGroup);
	*/
	}

void
OJapiGroup::destroy()
	{
	Assert(m_pGroup != NULL);
	//if (m_pGroup->m_eGroupType != eGroupType_Audience)
	//	return;	// Don't allow a JavaScript to delete a regular group; only a 'peerlist'

	#if 0 // The method Group_MarkForDeletion() includes the equivalent of TreeItemGroup_RemoveFromNavigationTree()
	m_pGroup->TreeItemGroup_RemoveFromNavigationTree();
	#endif
	m_pGroup->Group_MarkForDeletion();
	m_poCambrian->m_arraypaTemp.ElementTransferFrom(m_pGroup, INOUT &m_pGroup->m_pAccount->m_arraypaGroups);	// Transfer the group back to the 'temp' array

	/*
	if ( !m_pGroup || m_pGroup->m_eGroupType != eGroupType_Audience)
		return;
	if ( m_poCambrian->m_arraypaTemp.DeleteRuntimeObjectF(m_pGroup))
		return;
	m_pGroup->m_pAccount->m_arraypaGroups.DeleteRuntimeObject(m_pGroup);
	m_pGroup = NULL;

	*/
	}



///////////////////////////////////////////////////////////////////////////////////////////////////
POJapiContact
TContact::POJapiGet()
	{
	if (m_paoJapiContact == NULL)
		m_paoJapiContact = new OJapiContact(this);
	return m_paoJapiContact;
	}

OJapiContact::OJapiContact(TContact * pContact)
	{
	m_pContact = pContact;
	}

QString
OJapiContact::id()
	{
	return m_pContact->m_strJidBare;
	}

QString
OJapiContact::name()
	{
	return m_pContact->m_strNameDisplayTyped;
	}

void OJapiContact::openChat()
	{
	NavigationTree_SelectTreeItem(m_pContact);
	}



///////////////////////////////////////////////////////////////////////////////////////////////////

CListVariants::CListVariants(OJapiCambrian *poCambrian)
	{
	m_poCambrian = poCambrian;
	}

void
CListVariants::AddContact(TContact *pContact)
	{
	this->append(QVariant::fromValue(pContact->POJapiGet()));
	}

void
CListVariants::AddContacts(const CArrayPtrContacts &arraypaContacts)
	{
	TContact ** ppContactStop;
	TContact ** ppContact = arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);

	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		Assert(pContact != NULL);
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		AddContact(pContact);
		} // while
	}

void
CListVariants::AddGroupMatchingType(TGroup * pGroup, EGroupType eGroupType)
	{
	Assert(pGroup != NULL);
	if (pGroup->EGetGroupType() == eGroupType && !pGroup->TreeItemFlags_FuIsInvisible())
		{
		append(QVariant::fromValue(pGroup->POJapiGet(m_poCambrian)));
		}
	}

void
CListVariants::AddGroupsMatchingType(const CArrayPtrGroups & arraypGroups, EGroupType eGroupType)
	{
	TGroup **ppGroupsStop;
	TGroup **ppGroup = arraypGroups.PrgpGetGroupsStop(OUT &ppGroupsStop);
	while(ppGroup != ppGroupsStop)
		{
		TGroup * pGroup = *ppGroup++;
		AddGroupMatchingType(pGroup, eGroupType);
		}
	}

void
CListVariants::AddGroup(TGroup * pGroup)
	{
	AddGroupMatchingType(pGroup, eGroupType_kzOpen);
	}

void
CListVariants::AddAudience(TGroup * pGroup)
	{
	AddGroupMatchingType(pGroup, eGroupType_keAudience);
	}

//////////////////////////////////////////////////////////////////////////////////


const QString
OJapiBrowserTab::title()
	{
	return m_pTab->m_strNameDisplayTyped;
	}

OJapiBrowserTab::OJapiBrowserTab(TBrowserTab * pTab, OJapiBrowsersList * pBrowsersListParent)
	{
	Assert(pTab != NULL);
	Assert(pBrowsersListParent != NULL);
	m_pTab = pTab;
	m_pBrowsersListParent = pBrowsersListParent;
	}

void
OJapiBrowserTab::back()
	{
	Assert(m_pTab != NULL);
	m_pTab->NavigateBack();
	}

void
OJapiBrowserTab::forward()
	{
	Assert(m_pTab != NULL);
	m_pTab->NavigateForward();
	}

void
OJapiBrowserTab::reload()
	{
	Assert(m_pTab != NULL);
	m_pTab->NavigateReload();
}

void
OJapiBrowserTab::close()
	{
	// TODO: close
	}

void
OJapiBrowserTab::openApp(const QString & sAppName)
	{
	CStr strAppName(sAppName);
	m_pTab->OpenApp(strAppName);
	}

void
OJapiBrowserTab::openUrl(const QString & url)
	{
	CStr sUrl(url);
	m_pTab->SetUrl(sUrl);
	}

POJapiBrowserTab
TBrowserTab::POJapiGet(OJapiBrowsersList *pBrowsersList)
	{
	if ( m_paoJapiBrowser == NULL)
		m_paoJapiBrowser = new OJapiBrowserTab(this, pBrowsersList);

	return m_paoJapiBrowser;
	}


OJapiBrowsersList::OJapiBrowsersList(OJapiProfile *poProfile)
	{
	Assert(poProfile != NULL);
	m_poJapiProfileParent_NZ = poProfile;
	}

POJapiBrowserTab
OJapiBrowsersList::PGetCurrentTab_YZ()
	{
	TBrowserTabs * pBrowserTabs = PGetBrowser_YZ();
	if (pBrowserTabs != NULL)
		{
		TBrowserTab * pTab = pBrowserTabs->PBrowserTabGetCurrentSelected_YZ();
		if (pTab != NULL)
			return pTab->POJapiGet(this);
		}
	return NULL;
	}

TBrowserTabs*
OJapiBrowsersList::PGetBrowser_YZ()
	{
	return m_poJapiProfileParent_NZ->m_pProfile->m_arraypaBrowsersTabbed.PGetBrowserTabsFirst_YZ();
	}

QVariantList
OJapiBrowsersList::listBrowsers()
	{
	QVariantList list;
	TBrowserTabs * pBrowserTabs = PGetBrowser_YZ();
	if (pBrowserTabs != NULL)
		{
		TBrowserTab ** ppBrowserTabStop;
		TBrowserTab ** ppBrowserTab = pBrowserTabs->m_arraypaTabs.PrgpGetBrowserTabStop(OUT &ppBrowserTabStop);
		while (ppBrowserTab != ppBrowserTabStop)
			{
			TBrowserTab *pBrowser = *ppBrowserTab++;
			list.append(QVariant::fromValue(pBrowser->POJapiGet(this) ));
			}
		}
	return list;
	}

POJapiBrowserTab
OJapiBrowsersList::newBrowser()
	{
	TBrowserTabs * pBrowserTabs = PGetBrowser_YZ();
	if (pBrowserTabs == NULL)
		pBrowserTabs = m_poJapiProfileParent_NZ->m_pProfile->BrowserTabs_PCreateAndDisplayBrowserTabs();	// Create the browser tabs
	// add a new tab
	TBrowserTab * pTab = pBrowserTabs->PBrowserTabAdd();
	pBrowserTabs->TreeItemW_SelectWithinNavigationTree();
	return pTab->POJapiGet(this);
	}


POJapiProfile
TProfile::POJapiGet()
	{
	if (m_paoJapiProfile == NULL)
		m_paoJapiProfile = new OJapiProfile(this);
	return m_paoJapiProfile;
	}

OJapiCambrian *
TProfile::POJapiGetCambrian()
	{
	return new OJapiCambrian(this, NULL);
	}

OJapiProfile::OJapiProfile(TProfile *pProfile) : m_oBrowsersList(this) , m_oJurisdiction(this)
	{
	m_pProfile = pProfile;
	}

QString
OJapiProfile::id()
	{
    return m_pProfile->m_strNymID;
	}

QString
OJapiProfile::name()
	{
	return m_pProfile->m_strNameProfile;
	}

POJapiBrowsersList
OJapiProfile::browsers()
	{
	return &m_oBrowsersList;
	}

POJapiJurisdiction
OJapiProfile::jurisdiction()
	{
	return &m_oJurisdiction;
	}






OJapiProfilesList::OJapiProfilesList(OCapiRootGUI *pRootGui)
	{
	m_pRootGui = pRootGui;
	}

TProfile*
OJapiProfilesList::PGetCurrentProfile()
	{
	return g_oConfiguration.m_pProfileSelected;
	}

POJapiProfile
OJapiProfilesList::currentProfile()
	{
	return PGetCurrentProfile()->POJapiGet();
	}

void
OJapiProfilesList::setCurrentProfile(POJapiProfile poJapiProfile)
	{
	OJapiProfile *pProfile = qobject_cast<OJapiProfile*>(poJapiProfile);
	//MessageLog_AppendTextFormatCo(d_coRed, "setCurrentProfile $p\n", pProfile);

	if ( pProfile != NULL )
		{
		NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(pProfile->m_pProfile);
		roleChanged();
		#ifdef COMPILE_WITH_SPLASH_SCREEN
        g_pwMainWindow->hideRolePage();
		#endif
		}
	}

QVariantList
OJapiProfilesList::list()
	{
	QVariantList list;
	//MessageLog_AppendTextFormatCo(d_coRed, "OJapiProfilesList::list() \n");

	TProfile ** ppProfilesStop;
	TProfile ** ppProfiles = g_oConfiguration.m_arraypaProfiles.PrgpGetProfilesStop(OUT &ppProfilesStop);
	while (ppProfiles != ppProfilesStop)
		{
		TProfile * pProfile = *ppProfiles++;
		list.append(QVariant::fromValue(pProfile->POJapiGet()));
		//list.append( QVariant::fromValue(pProfile->m_strNameProfile.ToQString()) );
		}
	return list;
	}

bool
OJapiProfile::destroy()
    {

    bool successDeletion=true;

    // Get the current profile (it is need to work properly in mac)
    int cProfiles=g_oConfiguration.m_arraypaProfiles.GetSize();
    TProfile ** prgpProfiles = g_oConfiguration.m_arraypaProfiles.PrgpGetProfiles(OUT &cProfiles);
	TProfile * pProfile = NULL;
    for (int iProfile = 0; iProfile < cProfiles; iProfile++)
        {
         pProfile = prgpProfiles[iProfile];

        if (m_pProfile==pProfile)
            break;
        }

if (pProfile != NULL)
 {

#ifdef COMPILE_WITH_OPEN_TRANSACTIONS
//first verify if is possible to delete in OT
std::string nymId= pProfile->m_strNymID.ToQString().toStdString();

if (OTAPI_Wrap::It()->Wallet_CanRemoveNym(nymId))
{
    if(OTAPI_Wrap::It()->Wallet_RemoveNym(nymId))
    {successDeletion=true;}
    else
    {successDeletion=false;}
}
 else
{
   successDeletion=false;
}

if (successDeletion)
 {
        //now is possible to delete in Sopro db
#endif

  int xmppCount= pProfile->m_arraypaAccountsXmpp.GetSize();
   if (xmppCount | pProfile->m_arraypaApplications.GetSize())
    {     // Remove all accounts
      if (xmppCount > 0)
        {
            qDebug() << "Deleting Accounts";
            TAccountXmpp ** ppAccountStop;
            TAccountXmpp ** ppAccount = pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
            while (ppAccount != ppAccountStop)
               {
                TAccountXmpp * pAccount = *ppAccount++;
                pAccount->TreeItemAccount_DeleteFromNavigationTree_NoAsk(PA_DELETING);
                }

        }
        qDebug() << "delete profile";
        pProfile->m_pConfigurationParent->m_arraypaProfiles.DeleteTreeItem(PA_DELETING pProfile);
         #ifndef Q_OS_MAC
        m_pProfile->m_pConfigurationParent->XmlConfigurationSaveToFile();
        #endif
        successDeletion=true;
     }
    else
    {
    qDebug() << "Delete profile without roles";
    pProfile->m_pConfigurationParent->m_arraypaProfiles.DeleteTreeItem(PA_DELETING pProfile);
      #ifndef Q_OS_MAC
    m_pProfile->m_pConfigurationParent->XmlConfigurationSaveToFile();
     #endif
    NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(NULL);
    successDeletion=true;
    }// if role has contacts
    #ifdef COMPILE_WITH_OPEN_TRANSACTIONS
    }//if delete in OT was successfull
    #endif

}// if profile was selected

return successDeletion;
}

POJapiProfile
OJapiProfilesList::create(const QString & name)
{
QVariantList profile;
TProfile * pProfile = new TProfile(&g_oConfiguration);
    //Create the new Role

#ifdef COMPILE_WITH_OPEN_TRANSACTIONS

if (OTAPI_Wrap::It()->GetServerCount() >0)
{
    //create the role inside OT
    std::string nymId=OTAPI_Wrap::It()->CreateNym(1024, "","");
// Load the the nym in OTServer to be visible to other nyms
    OTString     strNym     (nymId.c_str());
    OTIdentifier pub_nym_id     (strNym);
    //Publish the nym

   // OTAPI_Wrap::OTAPI()->LoadPublicNym(pub_nym_id);

 if (OTAPI_Wrap::It()->SetNym_Name(nymId,nymId,name.toStdString()))
 { // Everything is ok with OT, now create the Role in TProfile
#endif
     // Create the new profile in Sopro db

     g_oConfiguration.m_arraypaProfiles.Add(PA_CHILD pProfile);
     //set the Role name
     pProfile->m_strNameProfile.InitFromStringQ(name);

#ifdef COMPILE_WITH_OPEN_TRANSACTIONS
    // if compiling with ot, load the nym and pk
     pProfile->m_strNymID.InitFromStringQ(QString::fromStdString(nymId));
     pProfile->m_strKeyPublic.InitFromStringQ(QString::fromStdString(OTAPI_Wrap::GetNym_SourceForID(nymId)));
     //Save the new role in xml (force)
     pProfile->m_pConfigurationParent->XmlConfigurationSaveToFile();
     NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(pProfile);
 }
 else
  {
  //delete nymid created
  OTAPI_Wrap::It()->Wallet_RemoveNym(nymId);
    }

}// if there is a OT server contract
else
{
 QMessageBox msg;
 msg.warning(g_pwMainWindow,"Action Required","Please asociate valid OT server.","OK");

 pOTX->openContractOTServerScreen();

}
#endif

return pProfile->POJapiGet();

}


OCapiRootGUI::OCapiRootGUI() : m_oProfiles(this), m_oPeerMessagesList(this)
	{
	}

POJapiProfilesList
OCapiRootGUI::roles()
	{
	return &m_oProfiles;
}

// this enum must be in sync with g_rgApplicationHtmlInfo[]
enum EApplicationHtmlinfo
	{
	eApplicationHtmlInfoBallotmaster = 9,
	};

//	???: Is the order in this list important?
SApplicationHtmlInfo g_rgApplicationHtmlInfo[] =
{
	{"Navshell Peers"	 , "navshell-contacts/index.html"		, PaAllocateJapiGeneric, NULL },
	{"Navshell Sidebar"  , "navshell-stack/index.html"			, PaAllocateJapiGeneric, NULL },
	{"Navshell Header"   , "navshell-header/index.html"			, PaAllocateJapiGeneric, NULL },
	{"Office Kingpin"    , "html5-office-kingpin/index.html"	, PaAllocateJapiGeneric, NULL },
	{"Pomodoro"          , "html5-pomodoro/index.html"			, PaAllocateJapiGeneric, NULL },
	{"JAPI Tests"        , "japi/test/test.html"				, PaAllocateJapiGeneric, NULL },
	{"Scratch"           , "html5-scratch/index.html"			, PaAllocateJapiGeneric, NULL },
	{"HTML5 xik"         , "html5-xik/index.html"						, PaAllocateJapiGeneric, NULL },
    {d_szNameApplicationHtml_GroupManager		, "html5-group-manager/index.html#/groups"				, PaAllocateJapiGeneric, NULL },
	{d_szNameApplicationHtml_Ballotmaster		, "html5-pollmaster/index.html"					, PaAllocateJapiGeneric, NULL },
	{d_szNameApplicationHtml_Channels			, "html5-channels/index.html"					, PaAllocateJapiGeneric, NULL },
    {d_szNameApplicationHtml_Corporations		, "html5-group-manager/index.html#/corps"				, PaAllocateJapiGeneric, NULL },
	{d_szNameApplicationHtml_PeerManager		, "html5-peers-manager/index.html"				, PaAllocateJapiGeneric, NULL },
	{"Home"		         , "html5-static/home/index.html"				, PaAllocateJapiGeneric, NULL },
	{"Default NewTab"	 , "html5-static/default-new-tab/index.html"	, PaAllocateJapiGeneric, NULL },
	{"Underconstruction" , "html5-static/underconstruction/index.html"	, PaAllocateJapiGeneric, NULL },
	{"Configuration"	 , "html5-config/index.html"					, PaAllocateJapiGeneric, NULL },
};

SApplicationHtmlInfo * PGetApplicationHtmlInfoBallotmaster() { return &g_rgApplicationHtmlInfo[9]; }

// This function was moved to make the code compile
const SApplicationHtmlInfo *
PGetApplicationHtmlInfo(PSZAC pszNameApplication)
	{
	//MessageLog_AppendTextFormatCo(d_coRed, "sizeof=$i\n", sizeof(c_rgApplicationHtmlInfo)/sizeof(SApplicationHtmlInfo) );
	const SApplicationHtmlInfo * pInfo = g_rgApplicationHtmlInfo;
	while (pInfo != g_rgApplicationHtmlInfo + LENGTH(g_rgApplicationHtmlInfo))
		{
		if (FCompareStringsNoCase((PSZUC) pInfo->pszName, (PSZUC)pszNameApplication))
			return pInfo;
		pInfo++;
		}
	return NULL;
	}


//	List all the applications available
QVariantList
OCapiRootGUI::apps()
	{
	QVariantList list;
	for (SApplicationHtmlInfo * pInfo = &g_rgApplicationHtmlInfo[0]; pInfo != g_rgApplicationHtmlInfo + LENGTH(g_rgApplicationHtmlInfo); pInfo++)
		{
		if (pInfo->paoJapi == NULL)
			pInfo->paoJapi = pInfo->pfnPaAllocateJapi(pInfo);
		list.append(QVariant::fromValue(pInfo->paoJapi));
		}
	return list;
	}

POJapiNotificationsList
OCapiRootGUI::notifications()
	{
	return &m_oNotificationsList;
	}

POJapiPeerRequestsList
OCapiRootGUI::peerRequests()
	{
	return &m_oPeerRequestsList;
	}

POJapiPeerMessagesList
OCapiRootGUI::peerMessages()
	{
	return &m_oPeerMessagesList;
	}

#ifdef TEST_QT_IMAGE_PROVIDER
OCapiImageProvider::OCapiImageProvider()  : QQuickImageProvider(QQuickImageProvider::Pixmap)
	{
	}

QPixmap OCapiImageProvider::requestPixmap(const QString &id, QSize * /*size*/, const QSize & /*requestedSize*/)
	{
	/*
	 * image://sopro/[:id:]
	 *
	 * expected id values
	 *  - roles/current
	 *	- roles/[:roleName:]
	 *	- appInfo/[:appName:]
	 *
	 * TODO: return actual images
	 */

	QStringList srgParts = id.split("/", QString::SkipEmptyParts);
	QString sFirst = srgParts.first();
	QPixmap profilepic;

	if ( sFirst.compare("roles") == 0 )
		{
		profilepic.load(":/ico/SocietyPro");
		}
	else if ( sFirst.compare("appInfo") == 0 )
		{
		profilepic.load(":/ico/SocietyPro");
		}

	return profilepic;
	}
#endif


///////////////////////////////////////////////////////////////////////////////////

OJapiGroupList::OJapiGroupList(OJapiCambrian *poCambrian)
	{
	m_poCambrian = poCambrian;
	}


POJapiGroup
OJapiGroupList::build(const QString &type)
	{
    return m_poCambrian->m_oMe.newGroup(type);
	}

OJapiList
OJapiGroupList::list()
	{
	return m_poCambrian->m_oMe.groups();
	}


POJapiGroup
OJapiGroupList::get(const QString & sId)
	{
	return m_poCambrian->m_oMe.getGroup(sId);
	}


////////////////////////////////////////// Utilities /////////////////////////////////////

QString
OJapiUtil::base64encode(const QString & sText)
	{
	CStr strBase64;
	strBase64.InitFromTextEncodedInBase64(sText);
	return strBase64;
	}

QString
OJapiUtil::base64decode(const QString &sBase64)
	{
	return QByteArray::fromBase64(sBase64.toUtf8());
}

///////////////////////////////////////////////////////
QString
OJapiPollAttatchment::name()
	{
	return m_pBallotAttatchment->m_strName;
	}

QString
OJapiPollAttatchment::mimeType()
	{
	return m_pBallotAttatchment->m_strMimeType;
	}

QString
OJapiPollAttatchment::content()
	{
	MessageLog_AppendTextFormatCo(d_coRed, "PollAttatchment::contetn()\n");
	CStr strBase64;
	strBase64.BinAppendStringBase64FromBinaryData(&m_pBallotAttatchment->m_binContent);
	return strBase64;
	}

OJapiPollAttatchment::OJapiPollAttatchment(CEventBallotAttatchment *pBallotAttatchment)
	{
	Assert(pBallotAttatchment != NULL);
	m_pBallotAttatchment = pBallotAttatchment;
	}

void
OJapiPollAttatchment::destroy()
	{
	MessageLog_AppendTextFormatCo(d_coBlack, "OJapiPollAttatchment::destroy\n");
	CArrayPtrPollAttatchments * parraypaAtattchments = &m_pBallotAttatchment->m_pPollParent->m_arraypaAtattchments;
	CEventBallotAttatchment **ppBallotAttatchmentStop;
	CEventBallotAttatchment **ppBallotAttatchment = parraypaAtattchments->PrgpGetAttatchmentsStop(&ppBallotAttatchmentStop);
	while ( ppBallotAttatchment != ppBallotAttatchmentStop)
		{
		CEventBallotAttatchment * pBallotAttatchment = *ppBallotAttatchment++;
		if (pBallotAttatchment == m_pBallotAttatchment)
			{
			parraypaAtattchments->RemoveElementI(pBallotAttatchment);
			delete pBallotAttatchment;
			return;
			}
		}
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to destroy attatchment $S\n", &m_pBallotAttatchment->m_strName);
	}





OJapiAppInfo:: OJapiAppInfo(const SApplicationHtmlInfo *pApplicationInfo)
	{
	Assert ( pApplicationInfo != NULL );
	m_pApplicationInfo = pApplicationInfo;
	}

QString OJapiAppInfo::name()
	{
	Assert ( m_pApplicationInfo != NULL );
	return QString(m_pApplicationInfo->pszName);
	}

QString OJapiAppInfo::tooltip()
	{
	return "SoPro Application";
	}

QString OJapiAppInfo::launchUrl()
	{
	Assert ( m_pApplicationInfo != NULL );
	return QString(m_pApplicationInfo->pszLocation);
	}

QString OJapiAppInfo::iconUrl()
	{
	Assert ( m_pApplicationInfo != NULL );
	return "image://application/" + QString(m_pApplicationInfo->pszName);
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	POJapiGet_NZ(), virtual
//
//	Return a pointer to an object offering an interface with JavaScript.
//	By default, return a generic OJapiEvent to return basic information about the event.
POJapiEvent
IEvent::POJapiGet_NZ() CONST_MCC
	{
	if (m_paoJapiEvent == NULL)
		m_paoJapiEvent = new OJapiEvent(this);
	return m_paoJapiEvent;
	}

/*
POJapi
IEvent::POJapiGet()
	{
	if ( m_paoJapiEvent == NULL)
		m_paoJapiEvent = new OJapiNotification();

	return m_paoJapiEvent;
	}
*/

OJapiNotification::OJapiNotification(IEvent * /*pEvent*/)
	{
	}

QString
OJapiNotification::title()
	{
	return "Incoming File Transfer";
	}

QString
OJapiNotification::text()
	{
	return "corp2014.pdf 4.3 Mb";
	}

QDateTime
OJapiNotification::date()
	{
	return QDateTime::currentDateTime();
	}

QString
OJapiNotification::cardLink()
	{
	return "cardLink";
	}

QString
OJapiNotification::actionLabel()
	{
	return "ACCEPT";
	}

QString
OJapiNotification::actionLink()
	{
	return "actionLink";
	}

void
OJapiNotification::clear()
	{
	// TODO: remove this notification from the list
	}

QVariantList
OJapiNotificationsList::recent(int /*nMax*/)
	{
	QVariantList list;
	list.append(QVariant::fromValue(new OJapiNotification()));/*??? memory leak */
	list.append(QVariant::fromValue(new OJapiNotification()));
	list.append(QVariant::fromValue(new OJapiNotification()));

	return list;
	}

void
OJapiNotificationsList::clearRecent()
	{
	// TODO: remove all notifications from the list
	}





OJapiPeerRequestsList::OJapiPeerRequestsList()
	{
	}

OJapiPeerRequestsList::~OJapiPeerRequestsList()
	{
	}

QVariantList
OJapiPeerRequestsList::list(int /*nMax*/)
	{
	QVariantList list;
	list.append(QVariant::fromValue(new OJapiPeerRequest()));
	list.append(QVariant::fromValue(new OJapiPeerRequest()));
	list.append(QVariant::fromValue(new OJapiPeerRequest()));
	list.append(QVariant::fromValue(new OJapiPeerRequest()));
	list.append(QVariant::fromValue(new OJapiPeerRequest()));
	return list;
	}



QString
OJapiPeerRequest::id()
	{
	return "plato@xmpp.cambrian.org";
	}

QString
OJapiPeerRequest::name()
	{
	return "Plato";
}


TProfile * OJapiPeerMessagesList::PGetProfileSelected_YZ()
	{
	return NavigationTree_PGetSelectedTreeItemMatchingInterfaceTProfile();
	}

OJapiPeerMessagesList::OJapiPeerMessagesList(OCapiRootGUI *pParentCapiRoot)
	{
	m_pParentCapiRoot = pParentCapiRoot;
	}

int
OJapiPeerMessagesList::recentCount()
	{
	TProfile *pProfile = PGetProfileSelected_YZ();
	if (pProfile == NULL)
		return 0;

	return pProfile->m_arraypEventsRecentMessagesReceived.GetSize();
	}


QVariantList
OJapiPeerMessagesList::recent(int nMax)
	{
	QVariantList list;
	// Search the array from the end, as the event to search is likely to be a recent one
	Assert(nMax >= 0);

	TProfile *pProfile = PGetProfileSelected_YZ();
	if (pProfile != NULL)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = pProfile->m_arraypEventsRecentMessagesReceived.PrgpGetEventsStopLast(OUT &ppEventStop);
		//IEvent ** ppEvent = g_arraypEventsReceived.PrgpGetEventsStop(OUT &ppEventStop);
		while (ppEvent != ppEventStop)
			{
			IEvent * pEvent = *--ppEventStop;
			if ( pEvent->EGetEventClass() == CEventMessageTextReceived::c_eEventClass )
				{
				list.append(QVariant::fromValue(new OJapiPeerMessage(this, (IEventMessageText*) pEvent)));
				}
			}
		}
	return list;
	}

void
OJapiPeerMessagesList::clearAll()
	{
	TProfile *pProfile = PGetProfileSelected_YZ();
	if (pProfile != NULL)
	{
	pProfile->m_arraypEventsRecentMessagesReceived.Flush();
	}
	//g_arraypEventsReceived.RemoveAllElements();
	/*
	IEvent ** ppEventStop;
	IEvent ** ppEvent = g_arraypEventsReceived.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *--ppEventStop;
		if ( pEvent->EGetEventClass() == CEventMessageTextReceived::c_eEventClass )
			{
			pEvent->m_uFlagsEvent |= IEvent::FE_kfArchived;
			}
		}
	*/
	}

OJapiPeerMessage::OJapiPeerMessage(OJapiPeerMessagesList *pParent, IEventMessageText *pEventMessage)
	{
	m_pEventMessage = pEventMessage;
	m_pParent = pParent;
	}

QString
OJapiPeerMessage::message()
	{
	return m_pEventMessage->m_strMessageText;
	}

QDateTime OJapiPeerMessage::date()
	{
	return Timestamp_ToQDateTime(m_pEventMessage->m_tsEventID);
	}

POJapiContact
OJapiPeerMessage::peer()
	{
	TContact *pContact = PGetContact_YZ();
	if ( pContact != NULL)
		return pContact->POJapiGet();
	return NULL;
	}

void OJapiPeerMessage::clear()
	{
	TProfile *pProfile = m_pParent->PGetProfileSelected_YZ();
	if ( pProfile != NULL )
		{
		pProfile->m_arraypEventsRecentMessagesReceived.RemoveEvent(m_pEventMessage);// TODO: handle memory leak //
		}
	}



TProfile * OJapiJurisdiction::PGetProfile_NZ()
	{
	return m_pProfileParent->m_pProfile;
	}

OJapiJurisdiction::OJapiJurisdiction(OJapiProfile * pProfileParent)
	{
	m_pProfileParent = pProfileParent;
	}

void OJapiJurisdiction::setCurrent(const QString & sJurisdiction)
	{
	PGetProfile_NZ()->m_strJurisdiction = sJurisdiction;
	}

QString OJapiJurisdiction::current()
	{
	return PGetProfile_NZ()->m_strJurisdiction;
	}


OJapiAppChat::OJapiAppChat(OJapiCambrian * poCambrian)
	{
	m_poCambrian = poCambrian;
	}

bool OJapiAppChat::open(const QString &strJabberId)
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		TContact ** ppContactStop;
		TContact ** ppContact = pAccount->m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);

		while (ppContact != ppContactStop)
			{
			TContact * pContact = *ppContact++;
			Assert(pContact != NULL);
			Assert(pContact->EGetRuntimeClass() == RTI(TContact));

			if ( strJabberId.compare(pContact->m_strJidBare) == 0)
				{
				NavigationTree_SelectTreeItem(pContact);
				return true;
				}
			} // while
		}// while
	return false;
	}


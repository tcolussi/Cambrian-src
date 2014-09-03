//	ApiJavaScript.cpp

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "ApiJavaScript.h"
#include "TApplicationBallotmaster.h"


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
		oList.AddGroupsMatchingType(IN pAccount->m_arraypaGroups, eGroupType_Open);
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
	MessageLog_AppendTextFormatCo(d_coBlack, "Groups List length end = $i\n", oList.length());
	return oList;
	}

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
		/*
		TContact ** ppContactStop;
		TContact ** ppContact = pAccount->m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);

		while (ppContact != ppContactStop)
			{
			TContact * pContact = *ppContact++;
			Assert(pContact != NULL);
			Assert(pContact->EGetRuntimeClass() == RTI(TContact));
			//oList.append(QVariant::fromValue(new OJapiContact(pContact)));
			oList.append(QVariant::fromValue(pContact->POJapiGet()));
			} // while
		} // while
		*/
	return oList;
}

POJapiGroup
OJapiMe::newPeerList()
	{
	TAccountXmpp *pAccount = (TAccountXmpp*) m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PvGetElementFirst_YZ();
	if ( pAccount == NULL)
		return NULL;
	TGroup *paGroup = pAccount->Group_PaAllocateAudience();
	m_poCambrian->m_arraypaTemp.Add(paGroup);
	return paGroup->POJapiGet(m_poCambrian);
	}

POJapiGroup
OJapiMe::getPeerList(const QString & sId)
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

POJapiGroup
OJapiMe::getGroup(const QString & sId)
	{
	return getPeerList(sId);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
POJapiGroup
TGroup::POJapiGet(OJapiCambrian * poCambrian)
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
	/*
	CStr str;
	str.Format("$p", this);
	return str;
	*/
	return HashSha1_ToQStringBase85(IN &m_pGroup->m_hashGroupIdentifier);
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
	TGroupMember **ppGroupMemberStop;
	TGroupMember **ppGroupMember = m_pGroup->m_arraypaMembers.PrgpGetMembersStop(&ppGroupMemberStop);
	while ( ppGroupMember != ppGroupMemberStop )
	{
		TGroupMember *pGroupMember = *ppGroupMember++;
		oList.AddContact(pGroupMember->m_pContact);
	}

	return oList;
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
	m_pGroup->TreeItemFlags_SerializeToDisk_Yes();
	m_pGroup->m_pAccount->m_arraypaGroups.ElementTransferFrom(m_pGroup, INOUT &m_poCambrian->m_arraypaTemp);	// Transfer the group from 'temp' to the account
	/*
	if (m_poCambrian->m_arraypaTemp.RemoveElementFastF(m_pGroup))
		m_pGroup->m_pAccount->m_arraypaGroups.Add(m_pGroup);
	*/
	}

void
OJapiGroup::destroy()
	{
	Assert(m_pGroup != NULL);
	if (m_pGroup->m_eGroupType != eGroupType_Audience)
		return;	// Don't allow a JavaScript to delete a regular group; only a 'peerlist'
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
CListVariants::AddGroupMatchingType(TGroup *pGroup, EGroupType eGroupType)
	{
	Assert(pGroup != NULL);
	if ( pGroup->m_eGroupType == eGroupType && !pGroup->TreeItemFlags_FuIsInvisible() )
	{
		append(QVariant::fromValue(pGroup->POJapiGet(m_poCambrian)));
	}
}

void
CListVariants::AddGroupsMatchingType(const CArrayPtrGroups &arraypGroups, EGroupType eGroupType)
	{
	TGroup **ppGroupsStop;
	TGroup **ppGroup = arraypGroups.PrgpGetGroupsStop(&ppGroupsStop);
		while( ppGroup != ppGroupsStop)
		{
		TGroup *pGroup = *ppGroup++;
		AddGroupMatchingType(pGroup, eGroupType);
		}
	}

void
CListVariants::AddGroup(TGroup *pGroup)
	{
	AddGroupMatchingType(pGroup, eGroupType_Open);
	}

void
CListVariants::AddAudience(TGroup *pGroup)
	{
	AddGroupMatchingType(pGroup, eGroupType_Audience);
	}

//////////////////////////////////////////////////////////////////////////////////


const QString
OJapiBrowserTab::title()
	{
	return m_pTab->m_strNameDisplayTyped.ToQString();
	}

OJapiBrowserTab::OJapiBrowserTab(TBrowserTab *pTab, OJapiBrowsersList *pBrowsersListParent)
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
OJapiBrowserTab::openApp(const QString & appName)
	{
	CStr sAppName(appName);
	const SApplicationHtmlInfo *pInfo = ApplicationGetInfo(sAppName.PszaGetUtf8NZ());
	if ( pInfo != NULL )
		{
		/*???*/
		CStr url = "file:///" + m_pBrowsersListParent->m_poJapiProfileParent->m_pProfile->m_pConfigurationParent->SGetPathOfFileName(pInfo->pszLocation);
		m_pTab->SetUrl(url);
		}
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
	m_poJapiProfileParent	= poProfile;
	}

POJapiBrowserTab
OJapiBrowsersList::PGetCurrentTab_YZ()
	{
	TBrowserTabs *pBrowserTabs = PGetBrowser_YZ();
	if ( pBrowserTabs != NULL)
		{
		TBrowserTab *pTab = pBrowserTabs->PGetCurrentBrowserTab_YZ();
		if ( pTab != NULL )
			return pTab->POJapiGet(this);
		}

	return NULL;
	}

TBrowserTabs*
OJapiBrowsersList::PGetBrowser_YZ()
	{
	return (TBrowserTabs*) m_poJapiProfileParent->m_pProfile->m_arraypaBrowsersTabbed.PvGetElementFirst_YZ();
	}

QVariantList
OJapiBrowsersList::listBrowsers()
	{
	QVariantList list;
	TBrowserTabs *pBrowserTabs = PGetBrowser_YZ();
	if ( pBrowserTabs )
		{
		TBrowserTab **ppBrowserTabStop;
		TBrowserTab **ppBrowserTab = pBrowserTabs->m_arraypaTabs.PrgpGetBrowserTabStop(&ppBrowserTabStop);
		while( ppBrowserTab != ppBrowserTabStop )
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
	TBrowserTabs *pBrowserTabs = PGetBrowser_YZ();
	TProfile *pProfile = m_poJapiProfileParent->m_pProfile;

	if ( !pBrowserTabs )
		{
		// create browser
		CStr sTreeItemName("Web Browser");
		pBrowserTabs = new TBrowserTabs(pProfile);
		pBrowserTabs->SetIconAndName(eMenuAction_DisplaySecureWebBrowsing, sTreeItemName);
		pProfile->m_arraypaBrowsersTabbed.Add(PA_CHILD pBrowserTabs);
		pBrowserTabs->TreeItemBrowser_DisplayWithinNavigationTree();
		}

	// add a new tab
	TBrowserTab *pTab = pBrowserTabs->AddTab();
	pBrowserTabs->TreeItemW_SelectWithinNavigationTree();

	return pTab->POJapiGet(this);
	}





POJapiProfile
TProfile::POJapiGet()
	{
	if ( m_paoJapiProfile == NULL)
		m_paoJapiProfile = new OJapiProfile(this);

	return m_paoJapiProfile;
	}

OJapiProfile::OJapiProfile(TProfile *pProfile) : m_oBrowsersList(this)
	{
	m_pProfile = pProfile;
	}

QString
OJapiProfile::id()
	{
	return m_pProfile->m_binKeyPublic.ToQString();
	}

QString
OJapiProfile::name()
	{
	return m_pProfile->m_strNameProfile.ToQString();
	}

POJapiBrowsersList
OJapiProfile::browsers()
	{
	return &m_oBrowsersList;
	}

void
OJapiProfile::destroy()
	{
	// TODO
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
	/*??? Need to check for the proper type (OJapiProfile) */
	OJapiProfile *pProfile = (OJapiProfile*) poJapiProfile;
	MessageLog_AppendTextFormatCo(d_coRed, "setCurrentProfile $p\n", pProfile);

	if ( pProfile != NULL )
		NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(pProfile->m_pProfile);
	}

QVariantList
OJapiProfilesList::list()
	{
	QVariantList list;

	TProfile **ppProfilesStop;
	TProfile **ppProfiles = g_oConfiguration.m_arraypaProfiles.PrgpGetProfilesStop(&ppProfilesStop);
	while(ppProfiles != ppProfilesStop)
		{
		TProfile *pProfile = *ppProfiles++;
		list.append( QVariant::fromValue(pProfile->POJapiGet()) );
		//list.append( QVariant::fromValue(pProfile->m_strNameProfile.ToQString()) );
		}

	return list;
	}

POJapiProfile
OJapiProfilesList::create(const QString &name)
	{
	return NULL; // TODO
	}


OCapiRootGUI::OCapiRootGUI() : m_oProfiles(this)
	{
	}

POJapiProfilesList
OCapiRootGUI::roles()
	{
	return &m_oProfiles;
	}


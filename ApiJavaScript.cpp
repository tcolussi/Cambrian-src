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
	if ( pGroup->m_eGroupType == eGroupType)
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


OCapiTabs::OCapiTabs(TProfile *pProfile)
	{
	m_pProfile = pProfile;
	//MessageLog_AppendTextFormatCo(d_coRed, "pProfile = $p - OCapiTabs::ctor\n", m_pProfile);
	}

void OCapiTabs::back()
	{
	TBrowserTab *pTab = GetCurrentTab();
	if ( pTab != NULL)
		pTab->NavigateBack();
	}


void OCapiTabs::forward()
	{
	TBrowserTab *pTab = GetCurrentTab();
	if ( pTab != NULL)
		pTab->NavigateForward();
	}

void OCapiTabs::reload()
	{
	QObject *pObj = QObject::sender();
	if ( pObj ){
	QString qObjName = pObj->objectName();
	MessageLog_AppendTextFormatCo(d_coRed, "Object is $Q\n", &qObjName );
	}
	else
	MessageLog_AppendTextFormatCo(d_coRed, "Object is {NULL}\n");


	TBrowserTab *pTab = GetCurrentTab();
	if ( pTab != NULL)
		pTab->NavigateReload();
	}

TBrowserTabs*
OCapiTabs::GetBrowserOpen()
	{
	//MessageLog_AppendTextFormatCo(d_coRed, "pProfile = $p - OCapiTabs::GetBrowserOpen\n", m_pProfile);
	// find a browser with tabs already opened
	TBrowserTabs ** ppBrowserStop;
	TBrowserTabs ** ppBrowser = m_pProfile->m_arraypaBrowsersTabbed.PrgpGetBrowsersStop(OUT &ppBrowserStop);
	while (ppBrowser != ppBrowserStop)
		{
		TBrowserTabs * pBrowser = *ppBrowser++;
		return pBrowser;
		}
	return NULL;
	}

TBrowserTab*
OCapiTabs::GetCurrentTab()
	{
	TBrowserTabs *pBrowser = GetBrowserOpen();
	if ( pBrowser != NULL)
		{
		TBrowserTab *pTab = pBrowser->GetCurrentBrowserTab();
		return pTab;
		}
	return NULL;
	}

/////////////////////////////////////////////////////


OCapiCambrian::OCapiCambrian(TProfile *pProfile) : m_Tabs(pProfile)
	{
	m_pProfile = pProfile;
	//MessageLog_AppendTextFormatCo(d_coRed, "pProfile = $p - OCapiCambrian::ctor\n", m_pProfile);
	}

POCapiTabs OCapiCambrian::tabs()
	{
	return &m_Tabs;
	}

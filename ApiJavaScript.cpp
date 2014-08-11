//	ApiJavaScript.cpp

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "ApiJavaScript.h"


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
	CListVariants listVariants;
	QVariantList oList;
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		listVariants.AddGroupsMatchingType(IN pAccount->m_arraypaGroups, eGroupType_Open);
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
	return oList;
	}

OJapiList
OJapiMe::peers()
	{
	QVariantList oList;
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
			//oList.append(QVariant::fromValue(new OJapiContact(pContact)));
			oList.append(QVariant::fromValue(pContact->POJapiGet()));
			} // while
		} // while
	return oList;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
POJapiGroup
TGroup::POJapiGet()
	{
	if (m_paoJapiGroup == NULL)
		m_paoJapiGroup = new OJapiGroup(this);
	return m_paoJapiGroup;
	}

OJapiGroup::OJapiGroup(TGroup * pGroup)
	{
	m_pGroup = pGroup;
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

int
OJapiGroup::count()
	{
	return m_pGroup->m_arraypaMembers.GetSize();
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




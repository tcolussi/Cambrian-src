//	Identities.cpp
//	Classes and methods to handle identities and identifiers.

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "Identities.h"

void
TProfile::InitHashTablesOfIdentifiers(IOUT CHashTableIdentifiersOfContacts * pHashTableContacts, IOUT CHashTableIdentifiersOfGroups * pHashTableGroups) const
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		TContact ** ppContactStop;
		TContact ** ppContact = pAccount->m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
		while (ppContact != ppContactStop)
			{
			TContact * pContact = *ppContact++;
			pHashTableContacts->AddIdentifiersOfContact(pContact);
			}
		TGroup ** ppGroupStop;
		TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
		while (ppGroup != ppGroupStop)
			{
			TGroup * pGroup = *ppGroup++;
			Assert(pGroup != NULL);
			Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
			pHashTableGroups->AddIdentifierOfGroup(pGroup);
			}
		} // while
	}

CHashTableIdentifiersOfContacts::CHashTableIdentifiersOfContacts() : CHashTable((PFn_PszGetHashKey)S_PszGetHashKeyContactIdentifier, eHashFunctionStringNoCase)
	{
	SetHashElementSize(sizeof(CHashElementIdentifierOfContact));
	}

void
CHashTableIdentifiersOfContacts::AddIdentifiersOfContact(TContact * pContact)
	{
	PSZUC pszContactIdentifier = pContact->m_strJidBare;	// At the moment, a contact has only one identifier (its JID)
	BOOL fElementNewlyAllocated = FALSE;
	CHashElementIdentifierOfContact * pHashElement = (CHashElementIdentifierOfContact *)PFindHashElementAllocate(pszContactIdentifier, INOUT &fElementNewlyAllocated);
	Assert(pHashElement != NULL);
	Assert(IS_ALIGNED_32(pHashElement));
	pHashElement->m_pContact = pContact;
	Assert(PFindContactByIdentifier(pszContactIdentifier) == pContact);
	}

TContact *
CHashTableIdentifiersOfContacts::PFindContactByIdentifier(PSZUC pszContactIdentifier) const
	{
	CHashElementIdentifierOfContact * pHashElement = (CHashElementIdentifierOfContact *)PFindHashElement(pszContactIdentifier);
	if (pHashElement != NULL)
		{
		Assert(pHashElement->m_pContact->EGetRuntimeClass() == RTI(TContact));
		return pHashElement->m_pContact;
		}
	return NULL;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
CHashTableIdentifiersOfGroups::CHashTableIdentifiersOfGroups() : CHashTable((PFn_PszGetHashKey)S_PGetHashKeyGroupIdentifier, eHashFunctionHashSha1)
	{
	SetHashElementSize(sizeof(CHashElementIdentifierOfGroup));
	}

void
CHashTableIdentifiersOfGroups::AddIdentifierOfGroup(TGroup * pGroup)
	{
	BOOL fElementNewlyAllocated = FALSE;
	CHashElementIdentifierOfGroup * pHashElement = (CHashElementIdentifierOfGroup *)PFindHashElementAllocate((PSZUC)&pGroup->m_hashGroupIdentifier, INOUT &fElementNewlyAllocated);
	Assert(pHashElement != NULL);
	Assert(IS_ALIGNED_32(pHashElement));
	pHashElement->m_pGroup = pGroup;
	}
/*
TGroup *
CHashTableIdentifiersOfGroups::PFindGroupByIdentifier(PSZUC pszGroupIdentifier) const
	{
	// Convert the group identifier from a base85-encoded string to a SHA-1
	Assert(0 && "NYI");
	return NULL;
	}
*/

TGroup *
CHashTableIdentifiersOfGroups::PFindGroupByIdentifier(const SHashSha1 * phashGroupIdentifier) const
	{
	CHashElementIdentifierOfGroup * pHashElement = (CHashElementIdentifierOfGroup *)PFindHashElement((PSZUC)phashGroupIdentifier);
	if (pHashElement != NULL)
		{
		Assert(pHashElement->m_pGroup->EGetRuntimeClass() == RTI(TGroup));
		return pHashElement->m_pGroup;
		}
	return NULL;
	}

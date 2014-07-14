//	Identities.h
#ifndef IDENTITIES_H
#define IDENTITIES_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class CHashElementIdentifier : public CHashElement
{
public:
	PVPARAM m_pvParamIdentifier;		// User-defined parameter to associate witht the identifier hash element
};

class CHashElementIdentifierOfContact : public CHashElementIdentifier
{
public:
	TContact * m_pContact;
};

//	Hash table to quickly find a TContact given an identifier.
//	Since contacts may have multiple identifiers, and there may be thousands of contacts, it make sense to have a hash table.
class CHashTableIdentifiersOfContacts : public CHashTable
{
public:
	CHashTableIdentifiersOfContacts();
	void AddIdentifiersOfContact(TContact * pContact, PVPARAM pvParamIdentifier = NULL);
	TContact * PFindContactByIdentifier(PSZUC pszContactIdentifier) const;

public:
	// At the moment, return the JID as the contact identity
	static PSZUC S_PszGetHashKeyContactIdentifier(const CHashElementIdentifierOfContact * pHashElement) { return pHashElement->m_pContact->m_strJidBare; }		// This routine must have a compatible interface as PFn_PszGetHashKey()
};


///////////////////////////////////////////////////////////////////////////////////////////////////
class CHashElementIdentifierOfGroup : public CHashElementIdentifier
{
public:
	TGroup * m_pGroup;
};


class CHashTableIdentifiersOfGroups : public CHashTable
{
public:
	CHashTableIdentifiersOfGroups();
	void AddIdentifierOfGroup(TGroup * pGroup);
	TGroup * PFindGroupByIdentifier(PSZUC pszGroupIdentifier) const;
	TGroup * PFindGroupByIdentifier(const SHashSha1 * phashGroupIdentifier) const;
public:
	static const SHashSha1 * S_PGetHashKeyGroupIdentifier(const CHashElementIdentifierOfGroup * pHashElement) { return &pHashElement->m_pGroup->m_hashGroupIdentifier; }		// This routine must have a compatible interface as PFn_PszGetHashKey()
};

#endif // IDENTITIES_H

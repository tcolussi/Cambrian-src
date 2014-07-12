//	TRecommendations.h
//
//	Class to display a layout of the recommendations made by a user (contact).
//
#ifndef TRECOMMENDATIONS_H
#define TRECOMMENDATIONS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "CHashTable.h"

class CHashElementContactIdentifier : public CHashElement
{
public:
	TContact * m_pContact;
};

//	Hash table to quickly find a TContact given an identifier.
//	Since contacts may have multiple identifiers, and there may be thousands of contacts, it make sense to have a hash table.
class CHashTableContactIdentities : public CHashTable
{
public:
	CHashTableContactIdentities();
	TContact * PFindContactByIdentifier(PSZUC pszContactIdentifier) const;
	void AddAllIdentifiersOfContact(TContact * pContact);
public:
	// At the moment, return the JID as the contact identity
	static PSZUC S_PszGetHashKeyContactIdentity(const CHashElementContactIdentifier * pHashElement) { return pHashElement->m_pContact->m_strJidBare; }		// This routine must have a compatible interface as PFn_PszGetHashKey()
};


class CRecommendation
{
public:
	union
		{
		ITreeItem * pTreeItem;
		TContact * pContact;
		TGroup * pGroup;
		} mu_existing;			// If already in the contact list
	CStr m_strIdentifier;
	CArrayPtrContacts m_arraypRecommendedBy;	// Other contacts who also made the same recommendation
};

//	Each recommendation has a corresponding Tree Item
class CTreeWidgetItemRecommendation : public CTreeWidgetItem
{
public:
	CRecommendation * m_pRecommendation_YZ;		// If the pointer is NULL, it means there no recommendation because the 'widget' is a category

public:
	CTreeWidgetItemRecommendation() { m_pRecommendation_YZ = NULL; }
};

class CArrayPtrRecommendations : public CArray
{
public:
	inline CRecommendation ** PrgpGetRecommendationsStop(OUT CRecommendation *** pppRecommendationStop) const { return (CRecommendation **)PrgpvGetElementsStop(OUT (void ***)pppRecommendationStop); }
};

class CRecommendations
{
public:
	TProfile * m_pProfile;	// Profile to find duplicate recommendations
	CArrayPtrRecommendations m_arraypRecommendationsContacts;	// Recommended contacts to display in the layout
	CArrayPtrRecommendations m_arraypRecommendationsGroups;

private:
	CHashTableContactIdentities m_oHashTableContacts;

public:
	CRecommendations(TProfile * pProfile);
	void RecommendationsAddNew(const TContact * pContact);
	void RecommendationsAddDuplicates(const TContact * pContact);
	CRecommendation * _PAllocateRecommendation(PSZUC pszIdentifier);
};


class TRecommendations : public ITreeItem
{
public:
	TContact * m_pContact;
public:
	TRecommendations(TContact * pContact);
	virtual void TreeItem_GotFocus();		// From ITreeItem

	RTI_IMPLEMENTATION(TRecommendations)
};


class WLayoutRecommendations : public WLayout
{
protected:
	CRecommendations * m_paRecommendations;	// What are the recommendations to display in the layout

	QTreeWidget * m_pwTreeViewApplications;
	QTreeWidgetItem * m_poTreeItemRecommendedContacts;
	QTreeWidgetItem * m_poTreeItemRecommendedGroups;

public:
	WLayoutRecommendations(TContact * pContact);
	virtual ~WLayoutRecommendations() { delete m_paRecommendations; }
	CTreeWidgetItemRecommendation * _PTreeWidgetItemAdd(EMenuAction eMenuIcon, PSZAC pszName, PSZAC pszDescription, QTreeWidgetItem * pParent);
	CTreeWidgetItemRecommendation * _PTreeWidgetItemAddRecommendation(EMenuAction eMenuIcon, CRecommendation * pRecommendation, QTreeWidgetItem * pParent);

protected slots:
	void SL_TreeItemClicked(QTreeWidgetItem * pItemClicked, int iColumn);
public:
	Q_OBJECT
};

#endif // TRECOMMENDATIONS_H

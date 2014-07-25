//	TRecommendations.h
//
//	Class to display a layout of the recommendations made by a user (contact).
//
#ifndef TRECOMMENDATIONS_H
#define TRECOMMENDATIONS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "Identities.h"


class IRecommendation : public CTreeWidgetItem
{
public:
	EMenuAction m_eMenuIconRecommendationType;	// Use the menu action/icon to classify the recommendation type (each type of recommendation will have a different icon)
	ITreeItem * m_paTreeItemNew;				// Newly allocated recommendation
	ITreeItem * m_pTreeItemExisting;			// Existing recommendation in the Navigation Tree
	CArrayPtrContacts m_arraypContactsAlsoRecommending;	// Other contacts also recommending this recommendation
public:
	IRecommendation(EMenuAction eMenuIconRecommendationType);
	virtual ~IRecommendation();
};

class CArrayPtrRecommendations : public CArray
{
public:
	inline IRecommendation ** PrgpGetRecommendationsStop(OUT IRecommendation *** pppRecommendationStop) const { return (IRecommendation **)PrgpvGetElementsStop(OUT (void ***)pppRecommendationStop); }
};

class CArrayPtrRecommendationsWithHashTables : public CArrayPtrRecommendations
{
public:
	TProfile * m_pProfile;
	TAccountXmpp * m_pAccount;
	TContact * m_pContact_YZ;
	CHashTableIdentifiersOfContacts m_oHashTableContactsProfile;
	CHashTableIdentifiersOfGroups m_oHashTableGroupsProfile;
	CHashTableIdentifiersOfContacts m_oHashTableContactsRecommended;
protected:
	CXmlTree m_oXmlTreeCache;	// For performance, cache the XML tree so it is not re-created for each contact having a recommendation

public:
	CArrayPtrRecommendationsWithHashTables(TProfile * pProfile, TContact * pContact = NULL);
	void AddRecommendationsAllocateNew(const CBin & binXmlRecommendations);
	void AddRecommendationsOfOtherContacts();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class CRecommendationCategory : public IRecommendation
{
public:
	inline CRecommendationCategory(EMenuAction eMenuIconRecommendationType) : IRecommendation(eMenuIconRecommendationType) { }
};

class CRecommendationContact : public IRecommendation
{
public:
	CRecommendationContact(PA_PARENT CArrayPtrRecommendationsWithHashTables * parraypaParent, const CXmlNode * pXmlNodeContact);
public:
	static const EMenuAction c_eMenuIcon = eMenuAction_Contact;
};

class CRecommendationGroup : public IRecommendation
{
public:
	CRecommendationGroup(PA_PARENT CArrayPtrRecommendationsWithHashTables * parraypaParent, const CXmlNode * pXmlNodeGroup);
public:
	static const EMenuAction c_eMenuIcon = eMenuAction_Group;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
class TRecommendations : public ITreeItem
{
public:
	TContact * m_pContact;
public:
	TRecommendations(TContact * pContact);
	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piObjectSecondary) const;			// From IRuntimeObject
	virtual void TreeItem_GotFocus();		// From ITreeItem

	RTI_IMPLEMENTATION(TRecommendations)
};

class TMyRecommendations : public ITreeItem
{
public:
	TProfile * m_pProfile;
public:
	TMyRecommendations(TProfile * pProfile);
	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piObjectSecondary) const;			// From IRuntimeObject
	virtual void TreeItem_GotFocus();		// From ITreeItem

	RTI_IMPLEMENTATION(TMyRecommendations)
};


class WLayoutRecommendations : public WLayout
{
protected:
	TProfile * m_pProfile;
	TContact * m_pContact_YZ;
	PSZUC m_pszNameDisplayContact;
	WTreeWidget * m_pwTreeRecommendations;

public:
	WLayoutRecommendations(TProfile * pProfile, TContact * pContact = NULL);
	void PopulateTreeWidget();
	CRecommendationCategory * _PAllocateRecommendationCategory(EMenuAction eMenuIconRecommendationType, PSZAC pszFmtTemplate0);

protected slots:
	void SL_TreeItemClicked(QTreeWidgetItem * pItemClicked, int iColumn);
	void SL_SearchTextChanged(const QString & sText);
public:
	Q_OBJECT
};

#endif // TRECOMMENDATIONS_H

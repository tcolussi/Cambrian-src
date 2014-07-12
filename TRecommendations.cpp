#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TRecommendations.h"

CHashTable * PInitHashTableWithAllIdentifiers();
TAccountXmpp * PFindContactByIdentifier(PSZUC pszContactIdentifier);
TGroup * PFindGroupByIdentifier(PSZUC pszContactIdentifier);

CHashTableContactIdentities::CHashTableContactIdentities() : CHashTable((PFn_PszGetHashKey)S_PszGetHashKeyContactIdentity, eHashFunctionStringNoCase)
	{
	SetHashElementSize(sizeof(CHashElementContactIdentifier));
	}

TContact *
CHashTableContactIdentities::PFindContactByIdentifier(PSZUC pszContactIdentifier) const
	{
	CHashElementContactIdentifier * pHashElement = (CHashElementContactIdentifier *)PFindHashElement(pszContactIdentifier);
	if (pHashElement != NULL)
		{
		Assert(pHashElement->m_pContact->EGetRuntimeClass() == RTI(TContact));
		return pHashElement->m_pContact;
		}
	return NULL;
	}

void
CHashTableContactIdentities::AddAllIdentifiersOfContact(TContact * pContact)
	{
	PSZUC pszContactIdentifier = pContact->m_strJidBare;
	BOOL fElementNewlyAllocated = FALSE;
	CHashElementContactIdentifier * pHashElement = (CHashElementContactIdentifier *)PFindHashElementAllocate(pszContactIdentifier, INOUT &fElementNewlyAllocated);
	Assert(pHashElement != NULL);
	Assert(IS_ALIGNED_32(pHashElement));
	pHashElement->m_pContact = pContact;
	Assert(PFindContactByIdentifier(pszContactIdentifier) == pContact);
	}

#if 0
	/*
	A suggestion is an idea or plan by someone that is  put forward for certain considerations. While
	A recommendation  is an opinion which is given by someone who did an analysis of something presented to his/her clients and is considering whether to buy it out or not.
	Usually done by more knowlegeable people in that field being tackled. Anyone can make a suggestion.  But to make recommendations someone has to be more expert on that field.
	*/

	// Sample recommendations from a contact
	<R>
		<C i='contact identifier' n='name of the contact' d='description of the contact, or a comment why the contact is recommded' />	// Recommend a contact
		<G i='group identifier' n='' d='' />	// Recommend a group
	</R>

#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Serialize all recommendations related to a profile

#define d_chAPIe_Recommendations_					'R'
#define d_szAPIe_Recommendations_					"R"
#define d_chAPIe_Recommendations_TContacts			'C'
#define d_szAPIe_Recommendations_TContacts			"C"
#define d_chAPIe_Recommendations_TGroups			'G'
#define d_szAPIe_Recommendations_TGroups			"G"

#define d_chAPIa_RecommendationContact_strIdentifier	'i'					// At the moment use the JID as the identifier
#define d_szAPIe_RecommendationContact_p_str			"C i='^i' n='^S'"
#define d_chAPIa_RecommendationGroup_shaIdentifier		'i'
#define d_szAPIe_RecommendationGroup_h_str				"G i='{h|}' n='^S'"
#define d_chAPIa_Recommendation_strName					'n'
#define d_chAPIa_Recommendation_strDescription			'd'	// NYI

void
TProfile::XcpApiProfile_RecommendationsSerialize(INOUT CBinXcpStanza * pbinXcpStanzaReply) const
	{
	Assert(pbinXcpStanzaReply != NULL);
	pbinXcpStanzaReply->BinAppendText("<" d_szAPIe_Recommendations_ "><" d_szAPIe_Recommendations_TContacts ">");
	CArrayPtrContacts arraypaContactsRecommended;
	GetRecommendations_Contacts(OUT &arraypaContactsRecommended);
	TContact ** ppContactStop;
	TContact ** ppContact = arraypaContactsRecommended.PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szAPIe_RecommendationContact_p_str "/>", pContact, &pContact->m_strNameDisplayTyped);	// Need to fix this for a real name of the contact (not what was typed for the Navigation Tree)
		}
	pbinXcpStanzaReply->BinAppendText("</" d_szAPIe_Recommendations_TContacts "><" d_szAPIe_Recommendations_TGroups ">");
	CArrayPtrGroups arraypaGroupsRecommended;
	GetRecommendations_Groups(OUT &arraypaGroupsRecommended);
	TGroup ** ppGroupStop;
	TGroup ** ppGroup = arraypaGroupsRecommended.PrgpGetGroupsStop(OUT &ppGroupStop);
	while (ppGroup != ppGroupStop)
		{
		TGroup * pGroup = *ppGroup++;
		Assert(pGroup != NULL);
		Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
		pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szAPIe_RecommendationGroup_h_str "/>", &pGroup->m_hashGroupIdentifier, &pGroup->m_strNameDisplayTyped);
		}
	pbinXcpStanzaReply->BinAppendText( "</" d_szAPIe_Recommendations_TGroups "></" d_szAPIe_Recommendations_ ">");
	} // XcpApiProfile_RecommendationsSerialize()

void
TContact::Contact_RecommendationsUpdateFromXml(const CXmlNode * pXmlNodeApiParameters)
	{
	Assert(pXmlNodeApiParameters != NULL);
	m_binXmlRecommendations.Empty();
	if (!pXmlNodeApiParameters->FIsEmptyElement())
		m_binXmlRecommendations.BinAppendXmlNodeNoWhiteSpaces(pXmlNodeApiParameters);
	//MessageLog_AppendTextFormatCo(d_coRed, "Contact_RecommendationsUpdateFromXml(): $B\n", &m_binXmlRecommendations);
	Contact_RecommendationsDisplayWithinNavigationTree();
	TreeItemW_Expand();
	}

void
TContact::Contact_RecommendationsDisplayWithinNavigationTree()
	{
	if (m_paTreeItemW_YZ == NULL)
		return;	// If the parent contact is not present, then do not show its recommendations
	TRecommendations * pRecommendation = (TRecommendations *)m_paTreeItemW_YZ->PFindChildItemMatchingRuntimeClass(RTI(TRecommendations));	// Search if there is already a node recommendations
	if (pRecommendation == NULL)
		{
		pRecommendation = new TRecommendations(this);
		pRecommendation->TreeItemW_DisplayWithinNavigationTreeExpand(this,  "Recommendations", eMenuAction_TreeItemRecommended);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CRecommendations::CRecommendations(TProfile * pProfile)
	{
	m_pProfile = pProfile;
	// Build the hash table
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		TContact ** ppContactStop;
		TContact ** ppContact = pAccount->m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
		while (ppContact != ppContactStop)
			{
			TContact * pContact = *ppContact++;
			m_oHashTableContacts.AddAllIdentifiersOfContact(pContact);
			}

		TGroup ** ppGroupStop;
		TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
		while (ppGroup != ppGroupStop)
			{
			TGroup * pGroup = *ppGroup++;
			Assert(pGroup != NULL);
			Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
			// TBD
			}
		}
	}

void
CRecommendations::RecommendationsAddNew(const TContact * pContact)
	{
	CXmlTree oXmlTree;
	(void)oXmlTree.EParseFileDataToXmlNodesCopy_ML(IN pContact->m_binXmlRecommendations);
	CXmlNode * pXmlNodeContacts = oXmlTree.PFindElement(d_chAPIe_Recommendations_TContacts);
	if (pXmlNodeContacts != NULL)
		{
		CXmlNode * pXmlNodeContact = pXmlNodeContacts->m_pElementsList;
		while (pXmlNodeContact != NULL)
			{
			PSZUC pszIdentifierJID = pXmlNodeContact->PszuFindAttributeValue(d_chAPIa_RecommendationContact_strIdentifier);
			CRecommendation * pRecommendation = new CRecommendation;
			m_arraypRecommendationsContacts.Add(pRecommendation);
			pRecommendation->m_strIdentifier = pszIdentifierJID;
			pRecommendation->mu_existing.pContact = m_oHashTableContacts.PFindContactByIdentifier(pszIdentifierJID);
			pXmlNodeContact = pXmlNodeContact->m_pNextSibling;
			} // while
		} // if
	CXmlNode * pXmlNodeGroups = oXmlTree.PFindElement(d_chAPIe_Recommendations_TGroups);
	if (pXmlNodeGroups != NULL)
		{
		CXmlNode * pXmlNodeGroup = pXmlNodeGroups->m_pElementsList;
		while (pXmlNodeGroup != NULL)
			{
			PSZUC pszIdentifierGroup = pXmlNodeGroups->PszuFindAttributeValue(d_chAPIa_RecommendationGroup_shaIdentifier);
			CRecommendation * pRecommendation = new CRecommendation;
			m_arraypRecommendationsGroups.Add(pRecommendation);
			pRecommendation->m_strIdentifier = pXmlNodeGroup->PszuFindAttributeValue(d_chAPIa_Recommendation_strName);
			pRecommendation->mu_existing.pGroup = NULL;
			pXmlNodeGroup = pXmlNodeGroup->m_pNextSibling;
			} // while
		} // if
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WLayoutRecommendations::WLayoutRecommendations(TContact * pContact)
	{
	Assert(pContact != NULL);
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	m_paRecommendations = new CRecommendations(pContact->m_pAccount->m_pProfileParent);
	m_paRecommendations->RecommendationsAddNew(pContact);

	m_poTreeItemRecommendedContacts = NULL;
	m_poTreeItemRecommendedGroups = NULL;

	OLayoutVerticalAlignTop * m_poLayoutApplications = Splitter_PoAddGroupBoxAndLayoutVertical_VE("Recommendations made by '$s'", pContact->TreeItem_PszGetNameDisplay());

	WEdit * m_pwEditSearchApplications = new WEdit;
	//pwEdit->setMaximumHeight(20);
	m_pwEditSearchApplications->Edit_SetWatermark("Search Recommendations");
	m_poLayoutApplications->addWidget(m_pwEditSearchApplications);
	m_pwEditSearchApplications->setFocus();
	//connect(m_pwEditSearchApplications, SIGNAL(textEdited(QString)), this, SLOT(SL_SearchTextChanged(QString)));

	m_pwTreeViewApplications = new QTreeWidget(this);
	m_poLayoutApplications->addWidget(m_pwTreeViewApplications, 100);
	m_pwTreeViewApplications->setColumnCount(2);
	QHeaderView * pwHeader = m_pwTreeViewApplications->header();
	pwHeader->hide();
	pwHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	pwHeader->setSectionResizeMode(1, QHeaderView::Stretch);

	// Populate the table
	CRecommendation ** ppRecommendationStop;
	CRecommendation ** ppRecommendation = m_paRecommendations->m_arraypRecommendationsContacts.PrgpGetRecommendationsStop(OUT &ppRecommendationStop);
	while (ppRecommendation != ppRecommendationStop)
		{
		CRecommendation * pRecommendation = *ppRecommendation++;
		if (m_poTreeItemRecommendedContacts == NULL)
			m_poTreeItemRecommendedContacts = _PTreeWidgetItemAdd(eMenuAction_Contact, "Contacts", "Recommended contacts", NULL);
		TContact * pContactExisting = pRecommendation->mu_existing.pContact;
		_PTreeWidgetItemAddRecommendation((pContactExisting != NULL) ? pContactExisting->Contact_EGetMenuActionPresence() : eMenuAction_Contact, pRecommendation, m_poTreeItemRecommendedContacts);
		}

	ppRecommendation = m_paRecommendations->m_arraypRecommendationsGroups.PrgpGetRecommendationsStop(OUT &ppRecommendationStop);
	while (ppRecommendation != ppRecommendationStop)
		{
		CRecommendation * pRecommendation = *ppRecommendation++;
		if (m_poTreeItemRecommendedGroups == NULL)
			m_poTreeItemRecommendedGroups = _PTreeWidgetItemAdd(eMenuAction_Group, "Groups", "Recommended groups", NULL);
		_PTreeWidgetItemAddRecommendation(eMenuAction_Group, pRecommendation, m_poTreeItemRecommendedGroups);
		}
	m_pwTreeViewApplications->expandAll();
	connect(m_pwTreeViewApplications, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(SL_TreeItemClicked(QTreeWidgetItem*,int)));
	}

void
WLayoutRecommendations::SL_TreeItemClicked(QTreeWidgetItem * pItemClicked, int iColumn)
	{
	MessageLog_AppendTextFormatCo(d_coBlack, "SL_TreeItemClicked($p, $i) \n", pItemClicked, iColumn);
	CRecommendation * pRecommendation = ((CTreeWidgetItemRecommendation *)pItemClicked)->m_pRecommendation_YZ;
	Assert(pRecommendation != NULL);
	if (pRecommendation->mu_existing.pContact == NULL)
		{

		}
	}


CTreeWidgetItemRecommendation *
WLayoutRecommendations::_PTreeWidgetItemAdd(EMenuAction eMenuIcon, PSZAC pszName, PSZAC pszDescription, QTreeWidgetItem * pParent)
	{
	CTreeWidgetItemRecommendation * poTreeItem = new CTreeWidgetItemRecommendation;
	poTreeItem->setIcon(0, PGetMenuAction(eMenuIcon)->icon());
	poTreeItem->setText(0, pszName);
	poTreeItem->setText(1, pszDescription);
	if (pParent != NULL)
		pParent->addChild(poTreeItem);
	else
		m_pwTreeViewApplications->addTopLevelItem(poTreeItem);
	return poTreeItem;
	}

CTreeWidgetItemRecommendation *
WLayoutRecommendations::_PTreeWidgetItemAddRecommendation(EMenuAction eMenuIcon, CRecommendation * pRecommendation, QTreeWidgetItem * pParent)
	{
	CTreeWidgetItemRecommendation * poTreeItem = _PTreeWidgetItemAdd(eMenuIcon, pRecommendation->m_strIdentifier, NULL, pParent);
	poTreeItem->m_pRecommendation_YZ = pRecommendation;
	const Qt::CheckState eCheckState = (pRecommendation->mu_existing.pTreeItem == NULL) ? Qt::Unchecked : Qt::Checked;
	poTreeItem->setCheckState(0, eCheckState);
	if (eCheckState != Qt::Unchecked)
		poTreeItem->setFlags(poTreeItem->flags() & ~ Qt::ItemIsUserCheckable);
	return poTreeItem;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
TRecommendations::TRecommendations(TContact * pContact)
	{
	m_pContact = pContact;
	}

//	TRecommendations::ITreeItem::TreeItem_GotFocus()
void
TRecommendations::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayoutAutoDelete(new WLayoutRecommendations(m_pContact));
	}

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TRecommendations.h"

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
#define d_chAPIe_Recommendations_TContacts				'C'
#define d_szAPIe_Recommendations_TContacts				"C"
#define d_szAPIe_Recommendation_TContact_p_str			"C i='^i' n='^S'"
#define d_chAPIa_Recommendation_TContact_strIdentifier	'i'					// At the moment use the JID as the identifier
#define d_chAPIa_Recommendation_TContact_strName		'n'

#define d_chAPIe_Recommendations_TGroups				'G'
#define d_szAPIe_Recommendations_TGroups				"G"
#define d_szAPIe_Recommendation_TGroup_h_str_i			"G i='{h|}' n='^S' c='$i'"
#define d_chAPIa_Recommendation_TGroup_shaIdentifier	'i'
#define d_chAPIa_Recommendation_TGroup_strName			'n'
#define d_chAPIa_Recommendation_TGroup_cMembers			'c'		// Number of members in the group

#define d_chAPIa_Recommendation_TGroup_strDescription	'd'	// NYI

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
		pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szAPIe_Recommendation_TContact_p_str "/>", pContact, &pContact->m_strNameDisplayTyped);	// Need to fix this for a real name of the contact (not what was typed for the Navigation Tree)
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
		pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szAPIe_Recommendation_TGroup_h_str_i "/>", &pGroup->m_hashGroupIdentifier, &pGroup->m_strNameDisplayTyped, pGroup->m_arraypaMembers.GetSize());
		}
	pbinXcpStanzaReply->BinAppendText( "</" d_szAPIe_Recommendations_TGroups "></" d_szAPIe_Recommendations_ ">");
	} // XcpApiProfile_RecommendationsSerialize()


CRecommendationContact::CRecommendationContact(PA_PARENT CArrayPtrRecommendationsWithHashTables * parraypaParent, const CXmlNode * pXmlNodeContact) : IRecommendation(c_eMenuIcon)
	{
	EMenuAction eMenuIcon = c_eMenuIcon;
	PSZUC pszName = pXmlNodeContact->PszuFindAttributeValue(d_chAPIa_Recommendation_TContact_strName);
	PSZUC pszContactIdentifier = pXmlNodeContact->PszuFindAttributeValue(d_chAPIa_Recommendation_TContact_strIdentifier);
	TContact * pContact;
	m_pTreeItemExisting = pContact = parraypaParent->m_oHashTableContactsProfile.PFindContactByIdentifier(IN pszContactIdentifier);
	if (m_pTreeItemExisting == NULL)
		{
		// The contact is not present, therefore allocate one so it may be added to the profile
		m_paTreeItemNew = pContact = new TContact(parraypaParent->m_pAccount);
		pContact->m_strJidBare = pszContactIdentifier;
		pContact->TreeItem_SetNameDisplaySuggested(pszName);
		}
	else
		{
		eMenuIcon = ((TContact *)m_pTreeItemExisting)->Contact_EGetMenuActionPresence();
		}
	InitIconAndText(eMenuIcon, pszName, NULL, pszContactIdentifier);
	parraypaParent->Add(PA_CHILD this);
	parraypaParent->m_oHashTableContactsRecommended.AddIdentifiersOfContact(pContact);
	}

CRecommendationGroup::CRecommendationGroup(PA_PARENT CArrayPtrRecommendationsWithHashTables * parraypaParent, const CXmlNode * pXmlNodeGroup) : IRecommendation(c_eMenuIcon)
	{
	PSZUC pszName = pXmlNodeGroup->PszuFindAttributeValue(d_chAPIa_Recommendation_TGroup_strName);
	SHashSha1 shaGroupIdentifier;
	if (!HashSha1_FInitFromStringBase85_ZZR_ML(OUT &shaGroupIdentifier, IN pXmlNodeGroup->PszuFindAttributeValue(d_chAPIa_Recommendation_TGroup_shaIdentifier)))
		return;	// Ignore any recommendation with an invalid SHA-1
	m_pTreeItemExisting = parraypaParent->m_oHashTableGroupsProfile.PFindGroupByIdentifier(IN &shaGroupIdentifier);
	if (m_pTreeItemExisting == NULL)
		{
		// The group is not present, therefore allocate one so it may be added to the profile
		TGroup * pGroup;
		m_paTreeItemNew = pGroup = new TGroup(parraypaParent->m_pAccount);
		pGroup->m_hashGroupIdentifier = shaGroupIdentifier;
		pGroup->m_pContactWhoRecommended = parraypaParent->m_pContact;
		pGroup->TreeItem_SetNameDisplaySuggested(pszName);
		}
	InitIconAndText(c_eMenuIcon, pszName, pXmlNodeGroup->PszuFindAttributeValue(d_chAPIa_Recommendation_TGroup_cMembers));
	parraypaParent->Add(PA_CHILD this);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
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
		pRecommendation->TreeItemW_DisplayWithinNavigationTreeExpand(this, "Recommendations", eMenuAction_TreeItemRecommended);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
IRecommendation::IRecommendation(EMenuAction eMenuIconRecommendationType)
	{
	m_eMenuIconRecommendationType = eMenuIconRecommendationType;
	m_paTreeItemNew = NULL;
	m_pTreeItemExisting = NULL;
	}

IRecommendation::~IRecommendation()
	{
	delete m_paTreeItemNew;
	}

CArrayPtrRecommendationsWithHashTables::CArrayPtrRecommendationsWithHashTables(TContact * pContact)
	{
	m_pContact = pContact;
	m_pAccount = pContact->m_pAccount;
	m_pProfile = m_pAccount->m_pProfileParent;
	m_pProfile->InitHashTablesOfIdentifiers(IOUT &m_oHashTableContactsProfile, IOUT &m_oHashTableGroupsProfile);
	}

//	The recommendations are dynamically allocated by the method and therefore must be deleted by the caller
void
CArrayPtrRecommendationsWithHashTables::AddRecommendationsAllocateNew(const CBin & binXmlRecommendations)
	{
	//MessageLog_AppendTextFormatCo(d_coRed, "AddRecommendationsNew(): $B\n", &binXmlRecommendations);
	(void)m_oXmlTreeCache.EParseFileDataToXmlNodesCopy_ML(IN binXmlRecommendations);
	CXmlNode * pXmlNodeContacts = m_oXmlTreeCache.PFindElement(d_chAPIe_Recommendations_TContacts);
	if (pXmlNodeContacts != NULL)
		{
		const CXmlNode * pXmlNodeContact = pXmlNodeContacts->m_pElementsList;
		while (pXmlNodeContact != NULL)
			{
			(void)new CRecommendationContact(PA_PARENT this, IN pXmlNodeContact);
			pXmlNodeContact = pXmlNodeContact->m_pNextSibling;
			} // while
		} // if
	CXmlNode * pXmlNodeGroups = m_oXmlTreeCache.PFindElement(d_chAPIe_Recommendations_TGroups);
	if (pXmlNodeGroups != NULL)
		{
		const CXmlNode * pXmlNodeGroup = pXmlNodeGroups->m_pElementsList;
		while (pXmlNodeGroup != NULL)
			{
			(void)new CRecommendationGroup(PA_PARENT this, IN pXmlNodeGroup);
			pXmlNodeGroup = pXmlNodeGroup->m_pNextSibling;
			} // while
		} // if
	}

void
CArrayPtrRecommendationsWithHashTables::AddRecommendationsOfOtherContacts()
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		TContact ** ppContactStop;
		TContact ** ppContact = pAccount->m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
		while (ppContact != ppContactStop)
			{
			TContact * pContact = *ppContact++;
			(void)m_oXmlTreeCache.EParseFileDataToXmlNodesCopy_ML(IN pContact->m_binXmlRecommendations);
			CXmlNode * pXmlNodeContacts = m_oXmlTreeCache.PFindElement(d_chAPIe_Recommendations_TContacts);
			if (pXmlNodeContacts != NULL)
				{
				const CXmlNode * pXmlNodeContact = pXmlNodeContacts->m_pElementsList;
				while (pXmlNodeContact != NULL)
					{
					// Search if this contact is not already recommended

					pXmlNodeContact = pXmlNodeContact->m_pNextSibling;
					} // while
				} // if
			}
		} // while
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WLayoutRecommendations::WLayoutRecommendations(TContact * pContact)
	{
	Assert(pContact != NULL);
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	m_pContact = pContact;

	OLayoutVerticalAlignTop * m_poLayoutApplications = Splitter_PoAddGroupBoxAndLayoutVertical_VE("Recommendations made by '$s'", pContact->TreeItem_PszGetNameDisplay());

	WEdit * pwEditSearchRecommendations = new WEdit;
	//pwEdit->setMaximumHeight(20);
	pwEditSearchRecommendations->Edit_SetWatermark("Search Recommendations");
	m_poLayoutApplications->addWidget(pwEditSearchRecommendations);
	pwEditSearchRecommendations->setFocus();
	connect(pwEditSearchRecommendations, SIGNAL(textEdited(QString)), this, SLOT(SL_SearchTextChanged(QString)));


	m_pwTreeRecommendations = new WTreeWidget;
	m_poLayoutApplications->addWidget(m_pwTreeRecommendations, 100);
	m_pwTreeRecommendations->setColumnCount(2);
	m_pwTreeRecommendations->setHeaderLabels((QStringList) "Name" << "#" << "Description" << "Also Recommended By (TBD)");
	QHeaderView * pwHeader = m_pwTreeRecommendations->header();
	pwHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	pwHeader->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//pwHeader->setSectionResizeMode(1, QHeaderView::Stretch);
	//pwHeader->setSectionResizeMode(QHeaderView::Interactive);

	PopulateTreeWidget();
	}

void
WLayoutRecommendations::PopulateTreeWidget()
	{
	CRecommendationCategory * pRecommendationCategoryContactsNew = _PAllocateRecommendationCategory(CRecommendationContact::c_eMenuIcon, "Contacts recommended by '$s'");
	CRecommendationCategory * pRecommendationCategoryGroupsNew = _PAllocateRecommendationCategory(CRecommendationGroup::c_eMenuIcon, "Groups recommended by '$s'");
	CRecommendationCategory * pRecommendationCategoryContactsExisting = _PAllocateRecommendationCategory(CRecommendationContact::c_eMenuIcon, "Contacts recommended by '$s' already in your contact list");
	CRecommendationCategory * pRecommendationCategoryGroupsExisting = _PAllocateRecommendationCategory(CRecommendationGroup::c_eMenuIcon, "Groups recommended by '$s' you already subscribed");
	CRecommendationCategory * pCategoryParent;

	CArrayPtrRecommendationsWithHashTables arraypaRecommendations(m_pContact);
	arraypaRecommendations.AddRecommendationsAllocateNew(IN m_pContact->m_binXmlRecommendations);
	IRecommendation ** ppRecommendationStop;
	IRecommendation ** ppRecommendation = arraypaRecommendations.PrgpGetRecommendationsStop(OUT &ppRecommendationStop);
	while (ppRecommendation != ppRecommendationStop)
		{
		IRecommendation * paRecommendation = *ppRecommendation++;
		ITreeItem * pTreeItemExisting = paRecommendation->m_pTreeItemExisting;
		switch (paRecommendation->m_eMenuIconRecommendationType)
			{
		case CRecommendationContact::c_eMenuIcon:
			pCategoryParent = (pTreeItemExisting == NULL) ? pRecommendationCategoryContactsNew : pRecommendationCategoryContactsExisting;
			break;
		case CRecommendationGroup::c_eMenuIcon:
			pCategoryParent = (pTreeItemExisting == NULL) ? pRecommendationCategoryGroupsNew : pRecommendationCategoryGroupsExisting;
			break;
		default:
			Assert(FALSE);
			return;
			} // switch
		const Qt::CheckState eCheckState = (paRecommendation->m_pTreeItemExisting == NULL) ? Qt::Unchecked : Qt::Checked;
		paRecommendation->setCheckState(0, eCheckState);
		if (eCheckState != Qt::Unchecked)
			paRecommendation->ItemFlagsRemove(Qt::ItemIsUserCheckable);
		pCategoryParent->addChild(PA_CHILD paRecommendation);
		} // while
	m_pwTreeRecommendations->DeleteAllRootNodesWhichAreEmpty();
	connect(m_pwTreeRecommendations, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(SL_TreeItemClicked(QTreeWidgetItem*,int)));
	}

void
WLayoutRecommendations::SL_TreeItemClicked(QTreeWidgetItem * pItemClicked, int iColumn)
	{
	MessageLog_AppendTextFormatCo(d_coBlack, "SL_TreeItemClicked($p, $i) \n", pItemClicked, iColumn);
	TAccountXmpp * pAccount = m_pContact->m_pAccount;
	IRecommendation * pRecommendation = (IRecommendation *)pItemClicked;
	ITreeItem * paTreeItemNew = pRecommendation->m_paTreeItemNew;
	if (paTreeItemNew != NULL)
		{
		Assert(pRecommendation->m_pTreeItemExisting == NULL);	// Those two pointers are mutually exclusive
		// Display the recommendation within the Navigation Tree
		pAccount->TreeItemAccount_RecommendationAdd(PA_CHILD paTreeItemNew);
		pRecommendation->m_pTreeItemExisting = paTreeItemNew;
		pRecommendation->m_paTreeItemNew = NULL;
		}
	else
		{
		Assert(pRecommendation->m_pTreeItemExisting != NULL);
		Assert(pRecommendation->m_paTreeItemNew == NULL && "Memory leak!");
		pAccount->TreeItemAccount_RecommendationRemove(INOUT PA_CAST_FROM_P pRecommendation->m_pTreeItemExisting);
		pRecommendation->m_paTreeItemNew = pRecommendation->m_pTreeItemExisting;
		pRecommendation->m_pTreeItemExisting = NULL;
		}
	}

CRecommendationCategory *
WLayoutRecommendations::_PAllocateRecommendationCategory(EMenuAction eMenuIconRecommendationType, PSZAC pszFmtTemplate0)
	{
	CRecommendationCategory * paCategory = new CRecommendationCategory(eMenuIconRecommendationType);
	paCategory->InitIconAndText(eMenuIconRecommendationType, g_strScratchBufferStatusBar.Format(pszFmtTemplate0, m_pContact->TreeItem_PszGetNameDisplay()));
	paCategory->setTextColor(0, d_coGrayDark);
	m_pwTreeRecommendations->addTopLevelItem(PA_CHILD paCategory);
	paCategory->setFirstColumnSpanned(true);
	paCategory->setExpanded(true);
	return paCategory;
	}

void
WLayoutRecommendations::SL_SearchTextChanged(const QString & sText)
	{
	m_pwTreeRecommendations->ShowAllTreeItemsContainingText(sText);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
TAccountXmpp::TreeItemAccount_RecommendationAdd(PA_CHILD ITreeItem * paTreeItemRecommendationAdd)
	{
	Assert(paTreeItemRecommendationAdd != NULL);
	switch (paTreeItemRecommendationAdd->EGetRuntimeClass())
		{
	case RTI(TContact):
		Contact_AddToNavigationTree(PA_CHILD (TContact *)paTreeItemRecommendationAdd);
		break;
	case RTI(TGroup):
		Group_AddToNavigationTree(PA_CHILD (TGroup *)paTreeItemRecommendationAdd);
		break;
	default:
		Assert(FALSE && "Unknown object");
		} // switch
	}

void
TAccountXmpp::Contact_AddToNavigationTree(PA_CHILD TContact * paContact)
	{
	m_arraypaContacts.Add(PA_CHILD paContact);
	paContact->TreeItemContact_DisplayWithinNavigationTree();
	Contact_RosterSubscribe(IN paContact);
	}

void
TAccountXmpp::Group_AddToNavigationTree(PA_CHILD TGroup * paGroup)
	{
	m_arraypaGroups.Add(PA_CHILD paGroup);
	paGroup->TreeItemGroup_DisplayWithinNavigationTree();
	if (paGroup->m_arraypaMembers.FIsEmpty() && paGroup->m_pContactWhoRecommended != NULL)
		paGroup->XcpApiGroup_Profile_GetFromContact(paGroup->m_pContactWhoRecommended);
	}

//	This method does NOT delete the Tree Item; it just remove it from the account the the Navigation Tree.
void
TAccountXmpp::TreeItemAccount_RecommendationRemove(ITreeItem * pTreeItemRecommendationRemove)
	{
	switch (pTreeItemRecommendationRemove->EGetRuntimeClass())
		{
	case RTI(TContact):
		m_arraypaContacts.RemoveElementAssertI(pTreeItemRecommendationRemove);
		break;
	case RTI(TGroup):
		m_arraypaGroups.RemoveElementAssertI(pTreeItemRecommendationRemove);
		((TGroup *)pTreeItemRecommendationRemove)->TreeItemGroup_RemoveFromNavigationTree();
		return;
	default:
		Assert(FALSE && "Unknown object");
		} // switch
	pTreeItemRecommendationRemove->TreeItemW_RemoveFromNavigationTree();
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

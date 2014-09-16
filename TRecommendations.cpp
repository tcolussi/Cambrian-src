//	TRecommendations.cpp
//
//	A suggestion is an idea or plan by someone that is put forward for certain considerations.
//	A recommendation is an opinion which is given by someone who did an analysis of something presented to his/her clients and is considering whether to buy it out or not.
//	Usually done by more knowlegeable people in that field being tackled. Anyone can make a suggestion.  But to make recommendations someone has to be more expert on that field.


#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TRecommendations.h"

enum
	{
	iColumnLayoutRecommendation_Icon = 0,
	iColumnLayoutRecommendation_Name = 0,
	iColumnLayoutRecommendation_Count,
	iColumnLayoutRecommendation_Description,
	iColumnLayoutRecommendation_AlsoRecommendedBy,
	};

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
		pbinXcpStanzaReply->BinAppendText_VE("<" d_szAPIe_Recommendation_TContact_p_str "/>", pContact, &pContact->m_strNameDisplayTyped);	// Need to fix this for a real name of the contact (not what was typed for the Navigation Tree)
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
		pbinXcpStanzaReply->BinAppendText_VE("<" d_szAPIe_Recommendation_TGroup_h_str_i "/>", &pGroup->m_hashGroupIdentifier, &pGroup->m_strNameDisplayTyped, pGroup->m_arraypaMembers.GetSize());
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
	parraypaParent->m_oHashTableContactsRecommended.AddIdentifiersOfContact(pContact, this);
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
		pGroup->m_pContactWhoRecommended_YZ = parraypaParent->m_pContact_YZ;
		pGroup->TreeItem_SetNameDisplaySuggested(pszName);
		}
	InitIconAndText(c_eMenuIcon, pszName, pXmlNodeGroup->PszuFindAttributeValue(d_chAPIa_Recommendation_TGroup_cMembers));
	parraypaParent->Add(PA_CHILD this);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
TContact::Contact_RecommendationsUpdateFromXml(const CXmlNode * pXmlNodeApiParameters)
	{
	m_uFlagsContact &= ~FC_kfContactRecommendationsNeverReceived;
	m_binXmlRecommendations.Empty();
	if (pXmlNodeApiParameters != NULL && !pXmlNodeApiParameters->FIsEmptyElement())
		m_binXmlRecommendations.BinAppendXmlNodeNoWhiteSpaces(pXmlNodeApiParameters);
	//MessageLog_AppendTextFormatCo(d_coRed, "Contact_RecommendationsUpdateFromXml(): $B\n", &m_binXmlRecommendations);
	Contact_RecommendationsDisplayWithinNavigationTree();
	TreeItemW_Expand();
	}

void
TContact::Contact_RecommendationsDisplayWithinNavigationTree(BOOL fSetFocus)
	{
	if (m_paTreeItemW_YZ == NULL)
		return;	// If the parent contact is not present, then do not show its recommendations
	TRecommendations * pRecommendation = (TRecommendations *)m_paTreeItemW_YZ->PFindChildItemMatchingRuntimeClass(RTI(TRecommendations));	// Search if there is already a node recommendations
	if (pRecommendation == NULL)
		{
		pRecommendation = new TRecommendations(this);
		pRecommendation->TreeItemW_DisplayWithinNavigationTreeExpand(this, "Recommendations", eMenuAction_TreeItemRecommended);
		}
	if (fSetFocus)
		pRecommendation->TreeItemLayout_SetFocus();
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

CArrayPtrRecommendationsWithHashTables::CArrayPtrRecommendationsWithHashTables(TProfile * pProfile, TContact * pContact)
	{
	Assert(pProfile != NULL);
	m_pContact_YZ = pContact;
	if (pContact != NULL)
		{
		m_pAccount = pContact->m_pAccount;
		}
	else
		{
		// If no contact is specified, use the first account of the profile.  We need an account to temporary add contacts to display them in the GUI
		m_pAccount = (TAccountXmpp *)pProfile->m_arraypaAccountsXmpp.PvGetElementFirst_YZ();
		//Assert(m_pAccount != NULL && "Do not display the node TMyRecommendations if there is no account present!");	// This Assert() appears when deleting the last XMPP account
		}
	m_pProfile = pProfile;
	pProfile->InitHashTablesOfIdentifiers(IOUT &m_oHashTableContactsProfile, IOUT &m_oHashTableGroupsProfile);
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
			Assert(pContact != NULL);
			Assert(pContact->EGetRuntimeClass() == RTI(TContact));
			if (pContact == m_pContact_YZ)
				continue;	// Skip our own contact
			(void)m_oXmlTreeCache.EParseFileDataToXmlNodesCopy_ML(IN pContact->m_binXmlRecommendations);
			CXmlNode * pXmlNodeContacts = m_oXmlTreeCache.PFindElement(d_chAPIe_Recommendations_TContacts);
			if (pXmlNodeContacts != NULL)
				{

				const CXmlNode * pXmlNodeContact = pXmlNodeContacts->m_pElementsList;
				while (pXmlNodeContact != NULL)
					{
					// Search if this contact is already recommended
					CHashElementIdentifierOfContact * pHashElement = (CHashElementIdentifierOfContact *)m_oHashTableContactsRecommended.PFindHashElement(IN pXmlNodeContact->PszuFindAttributeValue(d_chAPIa_Recommendation_TContact_strIdentifier));
					if (pHashElement != NULL)
						{
						CRecommendationContact * pRecommendation = (CRecommendationContact *)pHashElement->m_pvParamIdentifier;
						if (pRecommendation != NULL)
							pRecommendation->m_arraypContactsAlsoRecommending.Add(pContact);
						//MessageLog_AppendTextFormatCo(d_coRed, "^j is recommending 0x$p\n", pContact, pRecommendation);
						}
					pXmlNodeContact = pXmlNodeContact->m_pNextSibling;
					} // while
				} // if
			}
		} // while
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WLayoutRecommendations::WLayoutRecommendations(TProfile * pProfile, TContact * pContact)
	{
	Assert(pProfile != NULL);
	Assert(pProfile->EGetRuntimeClass() == RTI(TProfile));
	Endorse(pContact == NULL);	// Display all my recommendations
	m_pProfile = pProfile;
	m_pContact_YZ = pContact;
	m_pszNameDisplayContact = (pContact != NULL) ? pContact->TreeItem_PszGetNameDisplay() : (PSZUC)"me";

	OLayoutVerticalAlignTop * m_poLayoutApplications = Splitter_PoAddGroupBoxAndLayoutVertical_VE((pContact == NULL) ? "My personal recommendations for my peers" : "Recommendations made by '$s'", m_pszNameDisplayContact);
	if (pContact == NULL)
		m_poLayoutApplications->Layout_PwAddRowLabel("To recommend a peer or a group to your friends, right-click on the peer or group and select the menu item 'Recommend'");

	WEdit * pwEditSearchRecommendations = new WEdit;
	//pwEdit->setMaximumHeight(20);
	pwEditSearchRecommendations->Edit_SetWatermark("Search Recommendations");
	m_poLayoutApplications->addWidget(pwEditSearchRecommendations);
	pwEditSearchRecommendations->setFocus();
	connect(pwEditSearchRecommendations, SIGNAL(textEdited(QString)), this, SLOT(SL_SearchTextChanged(QString)));


	m_pwTreeRecommendations = new WTreeWidget;
	m_poLayoutApplications->addWidget(m_pwTreeRecommendations, 100);
	m_pwTreeRecommendations->setColumnCount(2);
	m_pwTreeRecommendations->setHeaderLabels((QStringList) "Name" << "#" << "Description" << "Also Recommended By");
	QHeaderView * pwHeader = m_pwTreeRecommendations->header();
	pwHeader->setSectionResizeMode(iColumnLayoutRecommendation_Name, QHeaderView::ResizeToContents);
	pwHeader->setSectionResizeMode(iColumnLayoutRecommendation_Count, QHeaderView::ResizeToContents);
	//pwHeader->setSectionResizeMode(1, QHeaderView::Stretch);
	//pwHeader->setSectionResizeMode(QHeaderView::Interactive);

	PopulateTreeWidget();
	}

void
WLayoutRecommendations::PopulateTreeWidget()
	{
	CRecommendationCategory * pRecommendationCategoryContactsNew = _PAllocateRecommendationCategory(CRecommendationContact::c_eMenuIcon, "Peers recommended by '$s'");
	CRecommendationCategory * pRecommendationCategoryGroupsNew = _PAllocateRecommendationCategory(CRecommendationGroup::c_eMenuIcon, "Groups recommended by '$s'");
	CRecommendationCategory * pRecommendationCategoryContactsExisting = _PAllocateRecommendationCategory(CRecommendationContact::c_eMenuIcon, (m_pContact_YZ != NULL) ? "Peers recommended by '$s' already in your peer list" : "My recommended peers");
	CRecommendationCategory * pRecommendationCategoryGroupsExisting = _PAllocateRecommendationCategory(CRecommendationGroup::c_eMenuIcon, (m_pContact_YZ != NULL) ? "Groups recommended by '$s' you already subscribed" : "My recommended groups");
	CRecommendationCategory * pCategoryParent;

	CArrayPtrRecommendationsWithHashTables arraypaRecommendations(m_pProfile, m_pContact_YZ);
	if (m_pContact_YZ != NULL)
		{
		arraypaRecommendations.AddRecommendationsAllocateNew(IN m_pContact_YZ->m_binXmlRecommendations);
		}
	else
		{
		CBinXcpStanza binXcpStanza;
		m_pProfile->XcpApiProfile_RecommendationsSerialize(IOUT &binXcpStanza);
		arraypaRecommendations.AddRecommendationsAllocateNew(binXcpStanza);
		}
	arraypaRecommendations.AddRecommendationsOfOtherContacts();
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
		paRecommendation->setCheckState(iColumnLayoutRecommendation_Name, eCheckState);
		if (eCheckState != Qt::Unchecked && m_pContact_YZ != NULL)
			paRecommendation->ItemFlagsRemove(Qt::ItemIsUserCheckable);
		paRecommendation->m_arraypContactsAlsoRecommending.SortByNameDisplay();
		(void)paRecommendation->m_arraypContactsAlsoRecommending.PszFormatDisplayNames(OUT_IGNORED &g_strScratchBufferStatusBar);
		paRecommendation->setText(iColumnLayoutRecommendation_AlsoRecommendedBy, g_strScratchBufferStatusBar);
		pCategoryParent->addChild(PA_CHILD paRecommendation);
		} // while
	m_pwTreeRecommendations->DeleteAllRootNodesWhichAreEmpty();
	connect(m_pwTreeRecommendations, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(SL_TreeItemClicked(QTreeWidgetItem*,int)));
	}

void
WLayoutRecommendations::SL_TreeItemClicked(QTreeWidgetItem * pItemClicked, int iColumn)
	{
	MessageLog_AppendTextFormatCo(d_coBlack, "SL_TreeItemClicked($p, $i) \n", pItemClicked, iColumn);
	IRecommendation * pRecommendation = (IRecommendation *)pItemClicked;
	ITreeItem * paTreeItemNew = pRecommendation->m_paTreeItemNew;
	if (m_pContact_YZ != NULL)
		{
		TAccountXmpp * pAccount = m_pContact_YZ->m_pAccount;
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
	else
		{
		// Toggle the recommendation flag
		Assert(pRecommendation->m_pTreeItemExisting != NULL);
		pRecommendation->m_pTreeItemExisting->m_uFlagsTreeItem ^= ITreeItem::FTI_kfRecommended;
		}
	}

CRecommendationCategory *
WLayoutRecommendations::_PAllocateRecommendationCategory(EMenuAction eMenuIconRecommendationType, PSZAC pszFmtTemplate0)
	{
	CRecommendationCategory * paCategory = new CRecommendationCategory(eMenuIconRecommendationType);
	paCategory->InitIconAndText(eMenuIconRecommendationType, g_strScratchBufferStatusBar.Format(pszFmtTemplate0, m_pszNameDisplayContact));
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
	if (paGroup->m_arraypaMembers.FIsEmpty() && paGroup->m_pContactWhoRecommended_YZ != NULL)
		paGroup->XcpApiGroup_Profile_GetFromContact(paGroup->m_pContactWhoRecommended_YZ);
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

TRecommendations::~TRecommendations()
	{
	MessageLog_AppendTextFormatCo(eSeverityNoise, "TRecommendations::~TRecommendations(0x$p) - ^j\n", this, m_pContact);
	}

//	TRecommendations::IRuntimeObject::PGetRuntimeInterface()
//
//	Enable the TRecommendations object to respond to the interfaces of its parent contact.
POBJECT
TRecommendations::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Report(piParent == NULL);
	return ITreeItem::PGetRuntimeInterface(rti, m_pContact);
	}

void
TRecommendations::TreeItem_RemoveAllReferencesToObjectsAboutBeingDeleted()
	{

	}

//	TRecommendations::ITreeItem::TreeItem_GotFocus()
void
TRecommendations::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayoutAutoDelete(new WLayoutRecommendations(m_pContact->m_pAccount->m_pProfileParent, m_pContact));
	}

TMyRecommendations::TMyRecommendations(TProfile * pProfile)
	{
	m_pProfile = pProfile;
	TreeItemW_DisplayWithinNavigationTreeExpand((pProfile->m_paTreeItemW_YZ != NULL) ? pProfile : NULL, "My Recommendations", eMenuAction_TreeItemRecommended);
	}

//	TMyRecommendations::IRuntimeObject::PGetRuntimeInterface()
//
//	Enable the TMyRecommendations object to respond to the interfaces of its parent profile.
POBJECT
TMyRecommendations::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Report(piParent == NULL);
	return ITreeItem::PGetRuntimeInterface(rti, m_pProfile);
	}

//	TMyRecommendations::ITreeItem::TreeItem_GotFocus()
void
TMyRecommendations::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayoutAutoDelete(new WLayoutRecommendations(m_pProfile));
	}


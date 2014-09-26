#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#define d_chAttributeGroupMember_Contact		'c'

TGroupMember::TGroupMember(TGroup * pGroup, TContact * pContact) : IContactAlias(pContact)
	{
	Assert(pGroup != NULL);
	Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
	m_pGroup = pGroup;
	m_uRolesAffiliationsPrivileges = 0;
	m_cMessagesUnread = 0;
	m_tsOtherLastSynchronized = d_ts_zNA;
	}

void
TGroupMember::TreeItemGroupMember_DisplayWithinNavigationTree()
	{
	TreeItemW_DisplayWithinNavigationTree(m_pGroup, eMenuAction_Contact);
	}

//	TGroupMember::IXmlExchange::XmlExchange()
void
TGroupMember::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	XmlExchangeContactAlias(pXmlExchanger, d_chAttributeGroupMember_Contact);
	pXmlExchanger->XmlExchangeInt("MessagesUnread", INOUT_F_UNCH_S &m_cMessagesUnread);
	pXmlExchanger->XmlExchangeTimestamp("tsSync", INOUT_F_UNCH_S &m_tsOtherLastSynchronized);
	}

//	TGroupMember::ITreeItem::TreeItem_MenuAppendActions()
void
TGroupMember::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionAdd(eMenuAction_ContactRemove);
	}

//	TGroupMember::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TGroupMember::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	if (eMenuAction == eMenuAction_ContactRemove)
		{
		// Remove the member from the group
		m_pGroup->Member_Remove_UI(PA_DELETING this);
		return ezMenuActionNone;
		}
	return ITreeItem::TreeItem_EDoMenuAction(eMenuAction);
	} // TreeItem_EDoMenuAction()


class WLayoutGroupMember : public WLayout
{
public:
	TGroupMember * m_pMember;
public:
	WLayoutGroupMember(TGroupMember * pMember);
};

WLayoutGroupMember::WLayoutGroupMember(TGroupMember * pMember)
	{
	m_pMember = pMember;
	WLabel * pLabel = new WLabelSelectableWrap;
	pLabel->Label_SetTextFormat_VE_Gsb("Placeholder to display all messages written by $s for the group $s", pMember->TreeItem_PszGetNameDisplay(), pMember->m_pGroup->TreeItem_PszGetNameDisplay());
	pLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	pLabel->setFrameShape(QFrame::Box);
	addWidget(pLabel);
	}

//	TGroupMember::ITreeItem::TreeItem_GotFocus()
void
TGroupMember::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayoutAutoDelete(IN new WLayoutGroupMember(this));
	}

void
TGroupMember::TreeItem_IconUpdate()
	{
	if (m_cMessagesUnread <= 0)
		TreeItemW_SetTextColorAndIcon(d_coTreeItem_Default, m_pContact->Contact_EGetMenuActionPresence());
	else
		TreeItemW_SetTextColorAndIcon(d_coTreeItem_UnreadMessages, eMenuAction_MessageNew);
	}

//	TGroupMember::IContactAlias::ContactAlias_IconChanged()
void
TGroupMember::ContactAlias_IconChanged(EMenuAction eMenuIconDisplay, EMenuAction eMenuIconPresence)
	{
	QRGB coText = d_coTreeItem_Default;
	if (m_cMessagesUnread > 0)
		{
		coText = d_coTreeItem_UnreadMessages;
		eMenuIconDisplay = eMenuAction_MessageNew;
		}
	else if (eMenuIconDisplay == eMenuAction_MessageNew)
		eMenuIconDisplay = eMenuIconPresence;
	TreeItemW_SetTextColorAndIcon(coText, eMenuIconDisplay);
	}

//	The allocation of a group member requires two parameters: the parent group and the contact.
IXmlExchange *
TGroupMember::S_PaAllocateGroupMember(PVOID poGroupParent, const CXmlNode * pXmlNodeElement)
	{
	Assert(poGroupParent != NULL);
	Assert(pXmlNodeElement != NULL);
	TGroup * pGroup = (TGroup *)poGroupParent;
	Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
	// Find the contact
	TContact * pContact = (TContact *)pGroup->m_pAccount->m_arraypaContacts.PFindObjectByIdFromXmlNodeValue(pXmlNodeElement->PFindAttribute(d_chAttributeGroupMember_Contact));
	if (pContact != NULL)
		return new TGroupMember(pGroup, pContact);
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to allocate TGroupMember from the data of the following element:\n^N", pXmlNodeElement);
	return NULL;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
IXmlExchange *
TGroup::S_PaAllocateGroup(POBJECT pAccountParent)
	{
	Assert(pAccountParent != NULL);
	return new TGroup((TAccountXmpp *)pAccountParent);
	}

TGroup::TGroup(TAccountXmpp * pAccount) : ITreeItemChatLogEvents(pAccount)
	{
	HashSha1_InitEmpty(OUT &m_hashGroupIdentifier);
	m_pContactWhoRecommended_YZ = NULL;
	m_paoJapiGroup = NULL;
	m_eGroupType = eGroupType_Open;
	}

TGroup::~TGroup()
	{
	m_arraypaMembers.DeleteAllTreeItems();
	}

void
TGroup::Group_RemoveAllReferencesToContactsAboutBeingDeleted()
	{
	if (m_pContactWhoRecommended_YZ != NULL && m_pContactWhoRecommended_YZ->TreeItemFlags_FuIsDeleted())
		m_pContactWhoRecommended_YZ = NULL;	// The user decided to remove the contact form his list, however to keep the group referred by the contact
	m_arraypaMembers.DeleteAllAliasesRelatedToContactsAboutBeingDeleted();
	Vault_RemoveAllReferencesToObjectsAboutBeingDeleted();
	}

void
TGroup::GroupInitNewIdentifier()
	{
	HashSha1_InitRandom(OUT &m_hashGroupIdentifier);
	}

TGroupMember *
TGroup::Member_PFindOrAddContact_NZ(TContact * pContact)
	{
	Assert(pContact != NULL);
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	TGroupMember * pMember = (TGroupMember *)m_arraypaMembers.PFindAliasMatchingContact(pContact);
	if (pMember == NULL)
		{
		// Add a new member to the group
		pMember = new TGroupMember(this, pContact);
		m_arraypaMembers.Add(pMember);
		if (m_paTreeItemW_YZ != NULL)
			{
			pMember->TreeItemGroupMember_DisplayWithinNavigationTree();	// Display the group member to the Navigation Tree only if its group is visible
			TreeItemW_SetTextToDisplayNameIfGenerated();
			}
		}
	Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
	return pMember;
	}

void
TGroup::Member_Add_UI(TContact * pContact)
	{
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	if (m_arraypaMembers.PFindAliasMatchingContact(pContact) == NULL)
		{
		// Add a new member to the group
		TGroupMember * pMember = new TGroupMember(this, pContact);
		m_arraypaMembers.Add(pMember);
		pMember->TreeItemGroupMember_DisplayWithinNavigationTree();
		TreeItemW_SetTextToDisplayNameIfGenerated();
		//pMember->TreeItemWidget_EnsureVisible();	// The contact was added to the array, therefore display it on the GUI as well
		// Create an event in the Chat Log
		/*
		CEventGroupMemberJoin * pEvent = new CEventGroupMemberJoin(pContact);
		Vault_AddEventToChatLogAndSendToContacts(PA_CHILD pEvent);
		*/
		}
	TreeItemW_SelectWithinNavigationTreeExpanded();
	}

void
TGroup::Member_Remove_UI(PA_DELETING TGroupMember * paMember)
	{
	m_arraypaMembers.DeleteTreeItem(PA_DELETING paMember);
	TreeItemW_SetTextToDisplayNameIfGenerated();
	}


TGroupMember *
TGroup::Member_PFindOrAllocate_NZ(PSZUC pszMemberJID)
	{
	if (pszMemberJID == NULL || pszMemberJID[0] == '\0')
		pszMemberJID = (PSZUC)"error@unknown";
	TGroupMember ** ppMemberStop;
	TGroupMember ** ppMember = m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
	while (ppMember != ppMemberStop)
		{
		TGroupMember * pMember = *ppMember++;
		Assert(pMember != NULL);
		Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
		if (pMember->m_pContact->m_strJidBare.FCompareStringsJIDs(pszMemberJID))
			return pMember;
		}
	// We have not found the member in the contact
	TContact * pContact = m_pAccount->Contact_PFindByJID(pszMemberJID, eFindContact_kfCreateNew);
	Assert(pContact != NULL);
	TGroupMember * pMember = new TGroupMember(this, pContact);
	m_arraypaMembers.Add(pMember);
	pMember->TreeItemGroupMember_DisplayWithinNavigationTree();
	TreeItemW_SetTextToDisplayNameIfGenerated();
	return pMember;
	} // Member_PFindOrAllocate_NZ()


//	Fetch the name of the contact if necessary (the attribute is 'n')
TContact *
TGroup::Member_PFindOrAddContact_YZ(PSZUC pszContactJID, const CXmlNode * pXmlNodeContactName)
	{
	// First of all, find the contact
	TContact * pContact = m_pAccount->Contact_PFindByJIDorAllocate(pszContactJID, IN pXmlNodeContactName->PszuFindAttributeValue('n'));
	if (pContact != NULL)
		(void)Member_PFindOrAddContact_NZ(pContact);
	return pContact;
	}

void
TGroup::Members_GetContacts(IOUT CArrayPtrContacts * parraypContacts) const
	{
	Assert(parraypContacts != NULL);
	TGroupMember ** ppMemberStop;
	TGroupMember ** ppMember = m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
	while (ppMember != ppMemberStop)
		{
		TGroupMember * pMember = *ppMember++;
		Assert(pMember != NULL);
		Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
		Assert(pMember->m_pContact->EGetRuntimeClass() == RTI(TContact));
		parraypContacts->Add(pMember->m_pContact);
		}
	}

//	TGroup::IRuntimeObject::PGetRuntimeInterface()
//
//	Enable the TGroup object to respond to the interface of its parent, the TAccountXmpp.
//
//	This method is identical to TContact::PGetRuntimeInterface()
POBJECT
TGroup::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Report(piParent == NULL);
	return ITreeItemChatLogEvents::PGetRuntimeInterface(rti, m_pAccount);
	} // PGetRuntimeInterface()

#define d_chElementName_Members		'M'
//	TGroup::IXmlExchange::XmlExchange()
void
TGroup::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	ITreeItemChatLogEvents::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeSha1("ID", INOUT_F_UNCH_S &m_hashGroupIdentifier);
	pXmlExchanger->XmlExchangePointer('Y', PPX &m_pContactWhoRecommended_YZ, IN &m_pAccount->m_arraypaContacts);
	pXmlExchanger->XmlExchangeObjects2(d_chElementName_Members, INOUT_F_UNCH_S &m_arraypaMembers, TGroupMember::S_PaAllocateGroupMember, this);
	pXmlExchanger->XmlExchangeInt("Type", INOUT (int *)&m_eGroupType);
	} // XmlExchange()


const EMenuActionByte c_rgzeActionsMenuContactGroup[] =
	{
	eMenuAction_GroupAddContacts,
	eMenuAction_BallotSend,
	eMenuActionSeparator,
	eMenuAction_GroupRename,
	eMenuAction_GroupDelete,
	eMenuActionSeparator,
	eMenuAction_SynchronizeWithGroup,
	eMenuActionSeparator,
	eMenuAction_TreeItemRecommended,
	eMenuAction_GroupProperties,
	ezMenuActionNone
	};

//	TGroup::ITreeItem::TreeItem_MenuAppendActions()
void
TGroup::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionsAdd(c_rgzeActionsMenuContactGroup);
	pMenu->ActionSetCheck(eMenuAction_TreeItemRecommended, m_uFlagsTreeItem & FTI_kfRecommended);
	}


//	TGroup::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TGroup::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_GroupAddContacts:
		DisplayDialogAddContactsToGroupFu();
		return ezMenuActionNone;
	case eMenuAction_GroupDelete:
		m_pAccount->m_pProfileParent->DeleteGroup(PA_DELETING this);
		return ezMenuActionNone;
	case eMenuAction_GroupProperties:
		DisplayDialogProperties();
		return ezMenuActionNone;
	case eMenuAction_BallotSend:
		DisplayDialogBallotSend();
		return ezMenuActionNone;
	default:
		return ITreeItemChatLogEvents::TreeItem_EDoMenuAction(eMenuAction);
		} // switch
	} // TreeItem_EDoMenuAction()

void TGroup::TreeItemW_DisplayWithinNavigationTree(ITreeItem * pParent_YZ, EMenuAction eMenuActionIcon)
	{
	ITreeItem::TreeItemW_DisplayWithinNavigationTree(pParent_YZ, eMenuActionIcon);
	if (m_paTreeItemW_YZ != NULL)
		{
		TGroupMember **ppMemberStop;
		TGroupMember **ppMember = m_arraypaMembers.PrgpGetMembersStop(&ppMemberStop);
		while( ppMember != ppMemberStop)
			{
			TGroupMember *pMember = *ppMember++;
			pMember->TreeItemGroupMember_DisplayWithinNavigationTree();	// Display the group member to the Navigation Tree only if its group is visible
			TreeItemW_SetTextToDisplayNameIfGenerated();
			}
	}
}

void TGroup::TreeItemW_DisplayWithinNavigationTree(ITreeItem *pParent_YZ)
	{
	ITreeItem::TreeItemW_DisplayWithinNavigationTree(pParent_YZ);
	}

/*
class WLayoutGroup : public WLayout
{
public:
	TGroup * m_pGroup;
public:
	WLayoutGroup(TGroup * pGroup);
};

WLayoutGroup::WLayoutGroup(TGroup * pGroup)
	{
	m_pGroup = pGroup;
	WLabel * pLabel = new WLabel;
	pLabel->Label_SetTextFormat_VE_Gsb("Placeholder to display all messages related to group $S", &pGroup->m_strNameDisplay);
	pLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	pLabel->setFrameShape(QFrame::Box);
	addWidget(pLabel);
	}
*/

//	Return TRUE if the group may be displayed within the Navigation Tree
bool
TGroup::TreeItemGroup_FCanDisplayWithinNavigationTree() const
	{
	return (m_eGroupType == eGroupType_Open) && TreeItemFlags_FCanDisplayWithinNavigationTree();
	}

void
TGroup::TreeItemGroup_DisplayWithinNavigationTree(ITreeItem * pTreeItemParent)
	{
	TreeItemW_DisplayWithinNavigationTree((pTreeItemParent == NULL) ? m_pAccount : pTreeItemParent);

	// Display the contacts which are members of the group
	int cMessagesUnreadTotal = 0;
	TGroupMember ** ppMemberStop;
	TGroupMember ** ppMember = m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
	while (ppMember != ppMemberStop)
		{
		TGroupMember * pMember = *ppMember++;
		Assert(pMember != NULL);
		Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
		pMember->TreeItemGroupMember_DisplayWithinNavigationTree();
		const int cMessagesUnread = pMember->m_cMessagesUnread;
		cMessagesUnreadTotal += cMessagesUnread;
		if (cMessagesUnread > 0)
			pMember->TreeItemW_SetTextToDisplayMessagesUnread(cMessagesUnread);
		pMember->TreeItem_IconUpdate();
		}
	if (cMessagesUnreadTotal > m_cMessagesUnread)
		{
		MessageLog_AppendTextFormatSev(eSeverityComment, "Adjusting the number of unread messages of group $s from $I to $I\n", TreeItem_PszGetNameDisplay(), m_cMessagesUnread, cMessagesUnreadTotal);	// This situation may happen if someone manually modifies Config.xml
		m_cMessagesUnread = cMessagesUnreadTotal;
		}
	TreeItemChatLog_UpdateTextToDisplayMessagesUnread();
	TreeItem_IconUpdate();
	TreeItemW_ExpandAccordingToSavedState();
	} // TreeItemGroup_DisplayWithinNavigationTree()

//	Remove the group and its member(s) from the Navigation Tree
void
TGroup::TreeItemGroup_RemoveFromNavigationTree()
	{
	TGroupMember ** ppMemberStop;
	TGroupMember ** ppMember = m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
	while (ppMember != ppMemberStop)
		{
		TGroupMember * pMember = *ppMember++;
		Assert(pMember != NULL);
		Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
		pMember->TreeItemW_RemoveFromNavigationTree();
		}
	TreeItemW_RemoveFromNavigationTree();
	}

//	TGroup::ITreeItem::TreeItem_PszGetNameDisplay()
PSZUC
TGroup::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	PSZUC pszNameDisplay = m_strNameDisplayTyped;
	if (pszNameDisplay[0] != '\0')
		return pszNameDisplay;
	m_uFlagsTreeItem |= FTI_kfTreeItem_NameDisplayedGenerated;
	// Generate a group name from the contacts
	m_strNameDisplayTyped.AppendTextU((PSZUC)"Group: ");
	TGroupMember * pMember = NULL;
	TGroupMember ** ppMemberStop;
	TGroupMember ** ppMember = m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
	while (ppMember != ppMemberStop)
		{
		if (pMember != NULL)
			{
			if (m_strNameDisplayTyped.CchGetLength() < 30)
				m_strNameDisplayTyped.AppendTextU((PSZUC)c_szaCommaSeparator);
			else
				{
				m_strNameDisplayTyped.AppendTextU((PSZUC)" ...");
				break;
				}
			}
		pMember = *ppMember++;
		Assert(pMember != NULL);
		Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
		m_strNameDisplayTyped.AppendTextU(pMember->TreeItem_PszGetNameDisplay());
		}
	if (pMember == NULL)
		m_strNameDisplayTyped.AppendFormat("'{h|}'", &m_hashGroupIdentifier); // The group does not have any member, therefore use the group identifier as its name
	return m_strNameDisplayTyped;
	} // TreeItem_PszGetNameDisplay()


//	TGroup::ITreeItem::TreeItem_IconUpdate()
void
TGroup::TreeItem_IconUpdate()
	{
	if (m_cMessagesUnread <= 0)
		TreeItemW_SetTextColorAndIcon(d_coTreeItem_Default, eMenuAction_Group);
	else
		TreeItemW_SetTextColorAndIcon(d_coTreeItem_UnreadMessages, eMenuAction_MessageNew);
	}

//	TGroup::ITreeItemChatLogEvents::Vault_GetHashFileName()
//
//	The hash of the filename for a group CANNOT be the identifier of the group because
//	two accounts may subscribe to the same group, and therefore there will be a conflict of the filename.
void
TGroup::Vault_GetHashFileName(OUT SHashSha1 * pHashFileNameVault) const
	{
	g_strScratchBufferStatusBar.BinInitFromBinaryData(IN &m_hashGroupIdentifier, sizeof(m_hashGroupIdentifier));
	g_strScratchBufferStatusBar.BinAppendCBinLowercaseAscii(m_pAccount->m_strJID);
	g_strScratchBufferStatusBar.BinAppendBinaryData(m_pAccount->PGetSaltOfConfiguration(), CChatConfiguration::c_cbSalt);
	HashSha1_CalculateFromCBin(OUT pHashFileNameVault, IN g_strScratchBufferStatusBar);
	}

TGroup *
TAccountXmpp::Group_PAllocate()
	{
	TGroup * pGroup = new TGroup(this);
	int iGroup = m_arraypaGroups.Add(PA_CHILD pGroup);
	pGroup->m_strNameDisplayTyped.Format("Group #$i", iGroup + 1);
	pGroup->GroupInitNewIdentifier();
	pGroup->TreeItemW_DisplayWithinNavigationTree(this, eMenuAction_Group);
	return pGroup;
	}

TGroup *TAccountXmpp::Group_PaAllocateTemp(EGroupType eGroupType)
	{
	TGroup * pGroup = new TGroup(this);
	pGroup->m_eGroupType = eGroupType;
	pGroup->GroupInitNewIdentifier();
	return pGroup;
	}

void
TAccountXmpp::Group_AddNewMember_UI(TContact * pContact, int iGroup)
	{
	Assert(pContact != NULL);
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	Assert(pContact->m_pAccount == this);
	// Create a new group if not present
	TGroup * pGroup = (TGroup *)m_arraypaGroups.PvGetElementAtSafe_YZ(iGroup);
	if (pGroup == NULL)
		pGroup = Group_PAllocate();
	// Check if the contact is not already in the group
	pGroup->Member_Add_UI(pContact);
	/*
	if (!pGroup->m_arraypaMembers.FFoundContact(pContact))
		{
		// Add a new member to the group
		TGroupMember * pMember = new TGroupMember(pGroup, pContact);
		pGroup->m_arraypaMembers.Add(pMember);
		pMember->TreeItemGroupMember_DisplayWithinNavigationTree();
		//pMember->TreeItemWidget_EnsureVisible();	// The contact was added to the array, therefore display it on the GUI as well
		pMember->TreeItem_SelectWithinNavigationTree();
		}
	*/
	} // Group_AddNewMember_UI()


//	Find the group matching the identifier, and if not found and eFindGroupCreate then allocate a new group.
//	This method may return NULL despite eFindGroupCreate if the identifier is not valid.
TGroup *
TAccountXmpp::Group_PFindByIdentifier_YZ(PSZUC pszGroupIdentifier, INOUT CBinXcpStanza * pbinXcpApiExtraRequest, EFindGroup eFindGroup)
	{
	Report(pszGroupIdentifier != NULL);	// This is not a bug, however unsual
	Assert(pbinXcpApiExtraRequest != NULL);
	SHashSha1 shaGroupIdentifier;
	if (!HashSha1_FInitFromStringBase85_ZZR_ML(OUT &shaGroupIdentifier, IN pszGroupIdentifier))
		{
		pbinXcpApiExtraRequest->BinXmlAppendXcpAttributesForApiRequestError(eErrorXcpApi_IdentifierInvalid, pszGroupIdentifier);
		return NULL;
		}
	TGroup * pGroup;
	TGroup ** ppGroupStop;
	TGroup ** ppGroup = m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
	while (ppGroup != ppGroupStop)
		{
		pGroup = *ppGroup++;
		Assert(pGroup != NULL);
		Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
		Assert(pGroup->m_pAccount == this);
		Assert(sizeof(pGroup->m_hashGroupIdentifier) == sizeof(shaGroupIdentifier));
		if (memcmp(&pGroup->m_hashGroupIdentifier, &shaGroupIdentifier, sizeof(shaGroupIdentifier)) == 0)
			return pGroup;
		}
	// Unable to find the group, therefore allocate a new one
	if (eFindGroup == eFindGroupCreate)
		{
		pGroup = new TGroup(this);
		m_arraypaGroups.Add(PA_CHILD pGroup);
		pGroup->m_hashGroupIdentifier = shaGroupIdentifier;
		pGroup->TreeItemW_DisplayWithinNavigationTree(this, eMenuAction_Group);
		pbinXcpApiExtraRequest->BinXmlAppendXcpApiRequest_Group_Profile_Get(pszGroupIdentifier);	// If the group does not exist, then query the contact who sent the stanza to get more information about the group
		return pGroup;
		}
	pbinXcpApiExtraRequest->BinXmlAppendXcpAttributesForApiRequestError(eErrorXcpApi_IdentifierNotFound, pszGroupIdentifier);
	return NULL;
	} // Group_PFindByIdentifier_YZ()

TGroup *
TAccountXmpp::Group_PFindByIdentifier_YZ(PSZUC pszGroupIdentifier)
	{
	// Search for the contact matching the identifier
	SHashSha1 shaGroupIdentifier;
	if (!HashSha1_FInitFromStringBase85_ZZR_ML(OUT &shaGroupIdentifier, IN pszGroupIdentifier))
		{
		if (pszGroupIdentifier != NULL && *pszGroupIdentifier != '\0')
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Invalid group identifier '$s'\n", pszGroupIdentifier);
		return NULL;
		}
	TGroup * pGroup;
	TGroup ** ppGroupStop;
	TGroup ** ppGroup = m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
	while (ppGroup != ppGroupStop)
		{
		pGroup = *ppGroup++;
		Assert(pGroup != NULL);
		Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
		Assert(sizeof(pGroup->m_hashGroupIdentifier) == sizeof(shaGroupIdentifier));
		if (HashSha1_FCompareEqual(IN &pGroup->m_hashGroupIdentifier, IN &shaGroupIdentifier))
			return pGroup;
		}
	return NULL;
	}

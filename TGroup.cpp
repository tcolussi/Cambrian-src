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
	TreeItem_DisplayWithinNavigationTree(m_pGroup, eMenuAction_Contact);
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
	if (m_paTreeWidgetItem == NULL)
		return;	// The Tree Item is not yet in the Navigation Tree
	if (m_cMessagesUnread <= 0)
		TreeItem_SetTextColorAndIcon(d_coTreeItem_Default, m_pContact->Contact_EGetMenuActionPresence());
	else
		TreeItem_SetTextColorAndIcon(d_coTreeItem_UnreadMessages, eMenuAction_MessageNew);
	}

//	TGroupMember::IContactAlias::ContactAlias_IconChanged()
void
TGroupMember::ContactAlias_IconChanged(EMenuAction eMenuIconDisplay, EMenuAction eMenuIconPresence)
	{
	if (m_paTreeWidgetItem == NULL)
		return;	// The Tree Item is not yet in the Navigation Tree
	QRGB coText = d_coTreeItem_Default;
	if (m_cMessagesUnread > 0)
		{
		coText = d_coTreeItem_UnreadMessages;
		eMenuIconDisplay = eMenuAction_MessageNew;
		}
	else if (eMenuIconDisplay == eMenuAction_MessageNew)
		eMenuIconDisplay = eMenuIconPresence;
	TreeItem_SetTextColorAndIcon(coText, eMenuIconDisplay);
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
	}

TGroup::~TGroup()
	{
	m_arraypaMembers.DeleteAllTreeItems();
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
		pMember->TreeItemGroupMember_DisplayWithinNavigationTree();
		TreeItem_SetTextToDisplayNameIfGenerated();
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
		TreeItem_SetTextToDisplayNameIfGenerated();
		//pMember->TreeItemWidget_EnsureVisible();	// The contact was added to the array, therefore display it on the GUI as well
		// Create an event in the Chat Log
		/*
		CEventGroupMemberJoin * pEvent = new CEventGroupMemberJoin(pContact);
		Vault_InitEventForVaultAndDisplayToChatLog(PA_CHILD pEvent);
		*/
		}
	TreeItem_SelectWithinNavigationTreeExpanded();
	}

void
TGroup::Member_Remove_UI(PA_DELETING TGroupMember * paMember)
	{
	m_arraypaMembers.DeleteTreeItem(PA_DELETING paMember);
	TreeItem_SetTextToDisplayNameIfGenerated();
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
	TContact * pContact = m_pAccount->Contact_PFindByJID(pszMemberJID, TAccountXmpp::eFindContactCreate);
	Assert(pContact != NULL);
	TGroupMember * pMember = new TGroupMember(this, pContact);
	m_arraypaMembers.Add(pMember);
	pMember->TreeItemGroupMember_DisplayWithinNavigationTree();
	TreeItem_SetTextToDisplayNameIfGenerated();
	return pMember;
	} // Member_PFindOrAllocate_NZ()

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
void *
TGroup::PGetRuntimeInterface(const RTI_ENUM rti) const
	{
	switch (rti)
		{
	case RTI(TProfile):
	case RTI(TAccountXmpp):
	case RTI(TCertificate):
	case RTI(TCertificateServerName):
		return m_pAccount->PGetRuntimeInterface(rti);
	default:
		return ITreeItemChatLogEvents::PGetRuntimeInterface(rti);
		}
	} // PGetRuntimeInterface()

#define d_chElementName_Members		'M'
//	TGroup::IXmlExchange::XmlExchange()
void
TGroup::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	if (pXmlExchanger->m_fSerializing)
		Vault_WriteEventsToDiskIfModified();		// This line is important to be first because saving the events may modify some variables which may be serialized by ITreeItemChatLogEvents::XmlExchange()
	ITreeItemChatLogEvents::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeSha1("ID", INOUT_F_UNCH_S &m_hashGroupIdentifier);
	pXmlExchanger->XmlExchangeObjects2(d_chElementName_Members, INOUT_F_UNCH_S &m_arraypaMembers, TGroupMember::S_PaAllocateGroupMember, this);
	} // XmlExchange()


const EMenuActionByte c_rgzeActionsMenuContactGroup[] =
	{
	eMenuAction_GroupAddContacts,
	eMenuAction_GroupRename,
	eMenuAction_GroupDelete,
	eMenuAction_GroupProperties,
	eMenuAction_BallotSend,
	ezMenuActionNone
	};

//	TGroup::ITreeItem::TreeItem_MenuAppendActions()
void
TGroup::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionsAdd(c_rgzeActionsMenuContactGroup);
	}


//	TGroup::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TGroup::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_GroupAddContacts:
		DisplayDialogAddContactsToGroup();
		return ezMenuActionNone;
	case eMenuAction_GroupDelete:
		m_pAccount->Group_Delete(PA_DELETING this);
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

void
TGroup::TreeItemGroup_DisplayWithinNavigationTree()
	{
	TreeItem_DisplayWithinNavigationTree(m_pAccount);

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
			pMember->TreeItem_SetTextToDisplayMessagesUnread(cMessagesUnread);
		}
	if (cMessagesUnreadTotal > m_cMessagesUnread)
		{
		MessageLog_AppendTextFormatSev(eSeverityComment, "Adjusting the number of unread messages of group $s from $I to $I\n", TreeItem_PszGetNameDisplay(), m_cMessagesUnread, cMessagesUnreadTotal);	// This situation may happen if someone manually modifies Config.xml
		m_cMessagesUnread = cMessagesUnreadTotal;
		}
	TreeItemChatLog_UpdateTextToDisplayMessagesUnread();
	TreeItem_IconUpdate();
	} // TreeItemGroup_DisplayWithinNavigationTree()

//	TGroup::ITreeItem::TreeItem_PszGetNameDisplay()
PSZUC
TGroup::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	PSZUC pszNameDisplay = m_strNameDisplayTyped;
	if (pszNameDisplay[0] != '\0')
		return pszNameDisplay;
	m_uFlagsTreeItem |= FTI_kfTreeItemNameDisplayedGenerated;
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
		TreeItem_SetTextColorAndIcon(d_coTreeItem_Default, eMenuAction_Group);
	else
		TreeItem_SetTextColorAndIcon(d_coTreeItem_UnreadMessages, eMenuAction_MessageNew);
	}

//	TGroup::ITreeItemChatLogEvents::Vault_GetHashFileName()
//
//	The hash of the filename for a group CANNOT be the identifier of the group because
//	two accounts may subscribe to the same group, and therefore there will be a conflict of the filename.
void
TGroup::Vault_GetHashFileName(OUT SHashSha1 * pHashFileNameVault) const
	{
	g_strScratchBufferStatusBar.BinInitFromBinaryData(IN &m_hashGroupIdentifier, sizeof(m_hashGroupIdentifier));
	g_strScratchBufferStatusBar.BinAppendCBinLowercase(m_pAccount->m_strJID);
	g_strScratchBufferStatusBar.BinAppendBinaryData(m_pAccount->PGetSaltOfConfiguration(), CChatConfiguration::c_cbSalt);
	HashSha1_CalculateFromCBin(OUT pHashFileNameVault, IN g_strScratchBufferStatusBar);
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
		{
		pGroup = new TGroup(this);
		iGroup = m_arraypaGroups.Add(PA_CHILD pGroup);
		pGroup->m_strNameDisplayTyped.Format("Group #$i", iGroup + 1);
		pGroup->GroupInitNewIdentifier();
		pGroup->TreeItem_DisplayWithinNavigationTree(this, eMenuAction_Group);
		}
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

void
TAccountXmpp::Group_Delete(PA_DELETING TGroup * paGroup)
	{
	Assert(paGroup != NULL);
	Assert(paGroup->EGetRuntimeClass() == RTI(TGroup));
	Assert(paGroup->m_pAccount == this);
	m_arraypaGroups.DeleteTreeItem(PA_DELETING paGroup);
	} // Group_Delete()

//	Find the group matching the identifier, and if not found and eFindGroupCreate then allocate a new group.
//	This method may return NULL despite eFindGroupCreate if the identifier is not valid.
TGroup *
TAccountXmpp::Group_PFindByIdentifier_YZ(PSZUC pszGroupIdentifier, INOUT CBinXcpStanzaType * pbinXcpApiExtraRequest, EFindGroup eFindGroup)
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
		pGroup->TreeItem_DisplayWithinNavigationTree(this, eMenuAction_Group);
		pbinXcpApiExtraRequest->BinXmlAppendXcpApiRequest_ProfileGet(pszGroupIdentifier);	// If the group does not exist, then query the contact who sent the stanza to get more information about the group
		return pGroup;
		}
	pbinXcpApiExtraRequest->BinXmlAppendXcpAttributesForApiRequestError(eErrorXcpApi_IdentifierNotFound, pszGroupIdentifier);
	return NULL;
	} // Group_PFindByIdentifier_YZ()

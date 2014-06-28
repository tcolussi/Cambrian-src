#ifndef TGROUP_H
#define TGROUP_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Class to represent a contact who is a member of a group
class TGroupMember : public IContactAlias	// (member)
{
	RTI_IMPLEMENTATION(TGroupMember)
public:
	TGroup * m_pGroup;	// Parent group where the member belongs to
	UINT m_uRolesAffiliationsPrivileges;
	int m_cMessagesUnread;					// Number of unread messages from the contact (this number is displayed in parenthesis after the contact name)
	TIMESTAMP m_tsOtherLastSynchronized;			// Timestamp of last synchronization
public:
	TGroupMember(TGroup * pGroup, TContact * pContact);
	void TreeItemGroupMember_DisplayWithinNavigationTree();

	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);					// From IXmlExchange
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);					// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);			// From ITreeItem
	virtual void TreeItem_GotFocus();												// From ITreeItem
	virtual void TreeItem_IconUpdate();
	void ContactAlias_IconChanged(EMenuAction eMenuIconDisplay, EMenuAction eMenuIconPresence);	// From IContactAlias

public:
	static IXmlExchange * S_PaAllocateGroupMember(PVOID poGroupParent, const CXmlNode * pXmlNodeElement);	// This static method must be compatible with interface PFn_PaAllocateXmlObject2_YZ()
}; // TGroupMember

class CArrayPtrGroupMembers : public CArrayPtrContactAliases
{
public:
	inline TGroupMember ** PrgpGetMembersStop(OUT TGroupMember *** pppMemberStop) const { return (TGroupMember **)PrgpvGetElementsStop(OUT (void ***)pppMemberStop); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Class holding information for group chat
class TGroup : public ITreeItemChatLogEvents	// (group)
{
	RTI_IMPLEMENTATION(TGroup)
public:
	SHashSha1 m_hashGroupIdentifier;	// Unique identifier for the group
	CArrayPtrGroupMembers m_arraypaMembers;
public:
	TGroup(TAccountXmpp * pAccount);
	~TGroup();
	inline void DeleteAllAliasesRelatedToContactsAboutBeingDeleted() { m_arraypaMembers.DeleteAllAliasesRelatedToContactsAboutBeingDeleted(); }
	void GroupInitNewIdentifier();
	void DisplayDialogProperties();
	void DisplayDialogAddContacts();
	void XcpApiGroup_ProfileSerialize(INOUT CBinXcpStanzaType * pbinXcpStanzaReply) const;
	void XcpApiGroup_ProfileUnserialize(const CXmlNode * pXmlNodeApiParameters, INOUT CBinXcpStanzaType * pbinXcpApiExtraRequest);

	void Member_Add_UI(TContact * pContact);
	void Member_Remove_UI(PA_DELETING TGroupMember * pMember);
	TGroupMember * Member_PFindOrAddContact_NZ(TContact * pContact);
	TGroupMember * Member_PFindOrAllocate_NZ(PSZUC pszMemberJID);
	void Members_BroadcastChatState(EChatState eChatState) const;
	void Members_GetContacts(IOUT CArrayPtrContacts * parraypContacts) const;

	virtual void * PGetRuntimeInterface(const RTI_ENUM rti) const;					// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);					// From IXmlExchange
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;							// From ITreeItem
	virtual void TreeItem_IconUpdate();												// From ITreeItem
	virtual void TreeItem_IconUpdateOnMessagesRead();								// From ITreeItem
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);					// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);			// From ITreeItem
	virtual void Vault_GetHashFileName(OUT SHashSha1 * pHashFileNameVault) const;	// From ITreeItemChatLogEvents

	void TreeItemGroup_DisplayWithinNavigationTree();
	void TreeItemGroup_NewMessageArrived(PSZUC pszMessage, TContact * pContact);

public:
	static IXmlExchange * S_PaAllocateGroup(POBJECT pAccountParent);	// This static method must be compatible with interface PFn_PaAllocateXmlObject()
	friend class CTaskGroupSendText;
}; // TGroup

class CArrayPtrGroups : public CArrayPtrTreeItems
{
public:
	inline TGroup ** PrgpGetGroupsStop(OUT TGroup *** pppGroupStop) const { return (TGroup **)PrgpvGetElementsStop(OUT (void ***)pppGroupStop); }
};


#endif // TGROUP_H

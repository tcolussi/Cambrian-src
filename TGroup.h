///////////////////////////////////////////////////////////////////////////////////////////////////
//	TGroup.h
//
//	Class enabling communication with multiple contacts.
//	There are many types of groups: open groups, private groups (distribution lists), and channels.
//
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
	void TreeItemGroupMember_SetIconComposingStarted();
	void TreeItemGroupMember_SetIconComposingStopped();

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

enum EGroupType
	{
	eGroupType_kzOpen		= 0,	// Default group type which shows in the Navigation Tree
	eGroupType_keAudience	= 1,	// Private distribution list to send polls (at the moment)
	eGroupType_kmMask		= 0x000F
	};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Class holding information for communicating with multiple contacts.

class TGroup : public ITreeItemChatLogEvents	// (group)
{
	RTI_IMPLEMENTATION(TGroup)
public:
	SHashSha1 m_hashGroupIdentifier;		// Unique identifier for the group
	TContact * m_pContactWhoRecommended_YZ;	// Contact who recommended the group
	enum
		{
		FG_kmGroupTypeMask	= eGroupType_kmMask,	// Bits to store the EGroupType
		FG_kfIsChannel		= 0x0010				// The group is a channel, and therefore m_strNameChannel_YZ is not empty
		};
	UINT m_uFlagsGroup;
	CStr m_strNameChannel_YZ;				// Channel name without the # hashtag (Example: "Bitcoin").  A channel is a topic for group conversation.  If this field is empty, then the group is not a channel.
	CStr m_strPurpose;						// Purpose/description of the channel/group describing what it will be used for.
	CArrayPtrGroupMembers m_arraypaMembers;
protected:
	POJapiGroup m_paoJapiGroup;

public:
	TGroup(TAccountXmpp * pAccount);
	~TGroup();
	inline void Group_MarkForDeletion() { TreeItem_MarkForDeletion(); }
	BOOL Group_FCanBePermenentlyDeleted() const;
	void Group_RemoveAllReferencesToContactsAboutBeingDeleted();
	void Group_InitNewIdentifier();
	inline EGroupType EGetGroupType() const { return (EGroupType)(m_uFlagsGroup & FG_kmGroupTypeMask); }
	inline BOOL Group_FuIsChannel() const { return (m_uFlagsGroup & FG_kfIsChannel); }
	void GroupChannel_SetName(PSZUC pszChannelName);

	void XcpApiGroup_ProfileSerialize(INOUT CBinXcpStanza * pbinXcpStanzaReply) const;
	void XcpApiGroup_ProfileUnserialize(const CXmlNode * pXmlNodeApiParameters, INOUT CBinXcpStanza * pbinXcpApiExtraRequest);
	void XcpApiGroup_Profile_GetFromContact(TContact * pContact);


	void Member_Add_UI(TContact * pContact);
	void Member_Remove_UI(PA_DELETING TGroupMember * pMember);
	TGroupMember * Member_PFindOrAddContact_NZ(TContact * pContact);
	TGroupMember * Member_PFindOrAllocate_NZ(PSZUC pszMemberJID);
	TContact * Member_PFindOrAddContact_YZ(PSZUC pszContactJID, const CXmlNode * pXmlNodeContactName);
	void Members_GetContacts(IOUT CArrayPtrContacts * parraypContacts) const;

	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const;					// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);					// From IXmlExchange
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;							// From ITreeItem
	virtual void TreeItem_IconUpdate();												// From ITreeItem
	virtual void TreeItem_IconUpdateOnMessagesRead();								// From ITreeItem
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);					// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);			// From ITreeItem
	virtual void Vault_GetHashFileName(OUT SHashSha1 * pHashFileNameVault) const;	// From ITreeItemChatLogEvents

	bool TreeItemGroup_FCanDisplayWithinNavigationTree() const;
	void TreeItemGroup_DisplayWithinNavigationTree(ITreeItem * pTreeItemParent = d_zDEFAULT);

	void TreeItemGroup_RemoveFromNavigationTree();

	void DisplayDialogProperties();
	void DisplayDialogAddContacts();

	POJapiGroup POJapiGet(OJapiCambrian * poCambrian);

public:
	static IXmlExchange * S_PaAllocateGroup(POBJECT pAccountParent);	// This static method must be compatible with interface PFn_PaAllocateXmlObject()
}; // TGroup

class CArrayPtrGroups : public CArrayPtrTreeItemChatLogEvents
{
public:
	inline TGroup ** PrgpGetGroupsStop(OUT TGroup *** pppGroupStop) const { return (TGroup **)PrgpvGetElementsStop(OUT (void ***)pppGroupStop); }
	inline TGroup ** PrgpGetGroupsStopMax(OUT TGroup *** pppGroupStop, int cGroupsMax) const { return (TGroup **)PrgpvGetElementsStopMax(OUT (void ***)pppGroupStop, cGroupsMax); }
};


#endif // TGROUP_H

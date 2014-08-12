///////////////////////////////////////////////////////////////////////////////////////////////////
//	TContact.h
//
//	Class holding information regarding a contact.
//	The contact may be a user of another instant messaging, an email account, or something else.
//	Each TAccount may contain multiple contacts
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TCONTACT_H
#define TCONTACT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class TContact : public ITreeItemChatLogEvents
{
public:
	CStr m_strJidBare;							// Bare Jabber Identifier of the contact (this is similar as an email address).  The case is preserve, so the comparison must be case insensitive.  By keeping the Bare JID separate from its resource, we may use a hash table to quickly find a contact from its JID.
	mutable CStr m_strRessource;				// Resource of the JID.  This field is either empty or begins with "/".  This value is not serialized as it is specific to an XMPP session.  This value may be updated each time the method PFindChatContactByJID() is called.
	CStr m_strComment;							// Comment regarding the contact.  This comment is also used to store the original invitation for the handshake
	CBin m_binXmlRecommendations;				// Store the raw XML of the recommendations from the contact
	/*
	enum
		{
		FCS_kfServiceDiscovery					= 0x00000001,
		FCS_kfServiceDiscoveryGroupChat			= 0x00000002,
		FCS_kfServiceDiscoveryFileTransfers		= 0x00000004,
		FCS_kmServiceDiscovery					= 0x0000000F,
		};
	UINT m_uFlagsContactSerialized;
	*/
	int m_cVersionXCP;								// Which version of the Cambrian Protocol is supported by the contact.  This field is a temporary 'hack' to determine if an XMPP stanza should be sent through XCP or regular XMPP.
	TIMESTAMP m_tsOtherLastSynchronized;			// Timestamp of last synchronization
	CListTasks m_listTasksSocket;					// Socket tasks to be dispatched when the contact becomes online

protected:
	enum
		{
		FC_kfContactUnsolicited								= 0x00000001,	// The contact was created from an unsolicited message.  Such contact is displayed with the gray color
		FC_kfContactNeedsInvitation							= 0x00000002,	// The contact was added, however there was never been any communication between the two paties, suggesting an invitation is recommended
		FC_kfContactRecommendationsNeverReceived			= 0x00000004,	// The recommendations have never been received, therefore download them as soon as the contact becomes online
//		FC_kfCommentContainsInvitation						= 0x00000004,
		//FC_kfNoCambrianProtocol								= 0x00000008,	// The contact does NOT understand the Cambrian Protocol, and therefore the application should NOT send stanzas with the element <xcp>.
		FC_kmFlagsSerializeMask								= 0x000000FF,	// Mask of the flags to serialize
		FC_kmPresenceMask									= 0x00000F00,	// Mask of the flags representing the presence
		FC_kzPresenceOffline								= 0x00000000,
		FC_kePresenceOnline									= 0x00000100,
		FC_kePresenceAway									= 0x00000200,
		FC_kePresenceAwayExtended							= 0x00000300,
		FC_kePresenceBusy									= 0x00000400,
		FC_kfRosterItem										= 0x00001000,	// The contact is found in the roster
		FC_kfRosterSubscriptionFrom							= 0x00002000,
		FC_kfRosterSubscriptionTo							= 0x00004000,
		FC_kmRosterSubscriptionBoth							= FC_kfRosterSubscriptionFrom | FC_kfRosterSubscriptionTo,
		FC_kfSubscribeAsk									= 0x00010000,
		FC_kfSubscribe										= 0x00020000,
		FC_kfSubscribed										= 0x00040000,
		//FC_kfXcpNotSupported								= 0x00100000,	// The contact does NOT understand the Cambrian Protocol, and therefore the application should NOT send stanzas with the element <xcp>.
		FC_kfXcpComposingSendTimestampsOfLastKnownEvents	= 0x00200000,
		FC_kfXcpRequestingDataForSynchronization			= 0x00400000
		};
	UINT m_uFlagsContact;		// Various flags for the chat contact

	//UINT m_uFlagsContactUnserialized;

	IContactAlias * m_plistAliases;
	TIMESTAMP m_tsLastSeenOnline;					// Date & time when the contact was last seen online
	TIMESTAMP_MINUTES m_tsmLastStanzaReceived;		// Timestamp where the last network packet was received by the contact. This is useful to determine if the contact became idle.
	POJapiContact m_paoJapiContact;					//

public:
	TContact(TAccountXmpp * pAccount);
private:
	~TContact();
public:
	void MarkForDeletion();
	void Contact_UpdateFlagCannotBeDeleted();
	inline void _ClearFlags() { m_uFlagsContact = 0; }
	inline void SetFlagContactAsUnsolicited() { m_uFlagsContact |= FC_kfContactUnsolicited; }
	inline void SetFlagContactAsInvited() { m_uFlagsContact &= ~FC_kfContactNeedsInvitation; }
	inline void SetFlagXcpComposingSendTimestampsOfLastKnownEvents() { m_uFlagsContact |= FC_kfXcpComposingSendTimestampsOfLastKnownEvents; }
	//inline BOOL Contact_FuIsInsecure() const { return (m_uFlagsContact & FC_kfNoCambrianProtocol); }
	BOOL Contact_FuCommunicateViaXcp() const;

	CChatConfiguration * PGetConfiguration() const;

	void Invitation_InitFromXml(const CStr & strInvitationXml);
	void Invitation_ReplyWithBase64(const CStr & strInvitationBase64);
	BOOL Contact_FIsInvitationRecommended();

	void ChatLogContact_RemoveInvitationMessage();
	void ChatLogContact_AppendExtraTextToChatState(INOUT OCursor & oCursor);
	void ChatLogContact_DisplayStanzaToUI(const CXmlNode * pXmlNodeMessageStanza);
	void ChatLogContact_ChatStateIconUpdate(EChatState eChatState);
	void ChatLogContact_ChatStateIconUpdateComposingStarted();
	void ChatLogContact_ChatStateIconUpdateComposingStopped();

	virtual const QBrush & ChatLog_OGetBrushForNewMessageReceived();
	virtual void Vault_GetHashFileName(OUT SHashSha1 * pHashFileNameVault) const;
	virtual void Vault_GetEventsForChatLog(OUT CArrayPtrEvents * parraypEventsChatLog) CONST_MCC;
	void Vault_AllocateEventMessageReceivedAndDisplayToChatLog(const CXmlNode * pXmlNodeMessageStanza, PSZUC pszuMessageBody, WChatLog * pwChatLog);
	CEventFileReceived * Vault_PAllocateEventFileReceived(const CXmlNode * pXmlNodeStreamInitiation);

	void XmppRosterSubscribe();
	void XmppRosterSubscribed();
	void XmppRosterUnsubscribe();
	void XmppRosterSubscriptionUpdate(PSZUC pszSubscription);
	PSZUC XmppRoster_PszGetSubscription() const;
	void XmppPresenceUpdateIcon(const CXmlNode * pXmlNodeStanzaPresence);
	void Xmpp_WriteXmlChatState(EChatState eChatState) CONST_MCC;
	CSocketXmpp * Xmpp_PGetSocketOnlyIfContactIsUnableToCommunicateViaXcp() const;
	void XmppXcp_ProcessStanza(const CXmlNode * pXmlNodeXmppXcp);
	void Xcp_ProcessStanzasAndUnserializeEvents(const CXmlNode * pXmlNodeXcpEvent);
	void Xcp_ServiceDiscovery();
	void Xcp_Synchronize();

	void XcpApiContact_TaskQueue(PA_TASK ITask * paTask);
	void XcpApiContact_ProfileSerialize(INOUT CBinXcpStanza * pbinXcpStanzaReply) const;
	void XcpApiContact_ProfileUnserialize(const CXmlNode * pXmlNodeApiParameters);

	void Contact_RecommendationsUpdateFromXml(const CXmlNode * pXmlNodeApiParameters);
	void Contact_RecommendationsDisplayWithinNavigationTree(BOOL fSetFocus = FALSE);

	//void BinAppendXmlAttributeOfContactIdentifier(IOUT CBin * pbin, CHS chAttributeName) const;
	void Contact_AddToGroup(int iGroup);
	EMenuAction Contact_EGetMenuActionPresence() const;

	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const;		// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);							// From IXmlExchange
	virtual BOOL TreeItem_FContainsMatchingText(PSZUC pszTextSearchLowercase) CONST_MCC;	// From ITreeItem
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;									// From ITreeItem
	virtual void TreeItem_IconUpdate();														// From ITreeItem
	virtual void TreeItem_IconUpdateOnMessagesRead();										// From ITreeItem
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);							// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);					// From ITreeItem

	void TreeItemContact_DisplayWithinNavigationTree();
	void TreeItemContact_DisplayWithinNavigationTreeAndClearInvisibleFlag();
	void TreeItemContact_DeleteFromNavigationTree_MB(PA_DELETING);
	void TreeItemContact_UpdateIconComposingStarted(ITreeItemChatLogEvents * pContactOrGroup);
	void TreeItemContact_UpdateIconComposingStopped(ITreeItemChatLogEvents * pContactOrGroup);
	void TreeItemContact_UpdateIconOffline();
	void TreeItemContact_UpdateIcon();
	void TreeItemContact_GenerateDisplayNameFromJid();
	void TreeItemContact_UpdateNameDisplayOfAliases();
	BOOL TreeItemContact_FSetDisplayNameUnique(PSZUC pszBegin, PCHUC pchCopyUntil);

	void DisplayDialogProperties();

	POJapiContact POJapiGet();

public:
	static IXmlExchange * S_PaAllocateContact(POBJECT pAccountParent);	// This static method must be compatible with interface PFn_PaAllocateXmlObject()
	static int S_NCompareSortContactsByNameDisplay(TContact * pContactA, TContact * pContactB, LPARAM lParamCompareSort = d_zNA);
	friend class WLayoutChatLog;	// Access m_uFlagsContact
	friend class IContactAlias;
	friend class CArrayPtrContacts;
	friend class IEvent;
	RTI_IMPLEMENTATION(TContact)
}; // TContact

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Interface for an alias (aka pointer) to a contact
class IContactAlias : public ITreeItem	// alias
{
public:
	IContactAlias * m_pNextAlias;	// Next contact alias in the linked list
	TContact * m_pContact;			// Parent contact of the alias

public:
	IContactAlias(TContact * pContact);
	~IContactAlias();
	void XmlExchangeContactAlias(INOUT CXmlExchanger * pXmlExchanger, CHS chAttributeContactAlias);
	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const;				// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);				// From IXmlExchange
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;						// From ITreeItem
	virtual void ContactAlias_IconChanged(EMenuAction eMenuIconDisplay, EMenuAction eMenuIconPresence);
};

class CArrayPtrContactAliases : public CArrayPtrTreeItems
{
public:
	inline IContactAlias ** PrgpGetAliasStop(OUT IContactAlias *** pppAliasStop) const { return (IContactAlias **)PrgpvGetElementsStop(OUT (void ***)pppAliasStop); }
	IContactAlias * PFindAliasMatchingContact(TContact * pContact) const;
	void DeleteAllAliasesRelatedToContactsAboutBeingDeleted();
};


///////////////////////////////////////////////////////////////////////////////////////////////////
class CArrayPtrContacts : public CArrayPtrTreeItems
{
public:
	inline TContact ** PrgpGetContactsStop(OUT TContact *** pppContactStop) const { return (TContact **)PrgpvGetElementsStop(OUT (void ***)pppContactStop); }
	TContact * PFindContactByNameDisplay(PSZUC pszContactNameDisplay, const TContact * pContactExclude) const;
	TContact * PFindChatContactByComment(PSZUC pszComment) const;
	//TContact * PFindChatContactByJID(PSZUC pszContactJID) const;
	//TContact * PFindChatContactByJIDwithResource(PSZUC IN_MOD_TMP pszContactJIDwithResource) const;
	void RemoveAllContactsComposingWhoAreIdle();

	void ForEach_ChatLogResetNickNameAndRepopulateAllEvents();

	void SortByNameDisplay();
	PSZUC PszFormatDisplayNames(OUT CStr * pstrScratchBuffer) const;
}; // CArrayPtrContacts

#define d_szInvitation				"Invitation"
#define d_szInvitationPrefix		"[[Invitation="
#define d_szInvitationPostfix		"]]"

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Tree Item displaying the text "<New Peer...>" in the Navigation Tree.
//	When selected, the Tree Item displays a layout to add contact(s)
class TContactNew : public ITreeItem
{
public:
	TAccountXmpp * m_pAccount;
public:
	TContactNew(TAccountXmpp * pAccount);
	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const;		// From IRuntimeObject
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;				// From ITreeItem
	virtual void TreeItem_GotFocus();									// From ITreeItem

	void TreeItemContactNew_DisplayWithinNavigationTree();

	RTI_IMPLEMENTATION(TContactNew)
};

#endif // TCONTACT_H

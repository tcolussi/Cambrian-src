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
	int m_cVersionXCP;								// Which version of the Cambrian Protocol is supported by the contact.  This field is a temporary 'hack' to determine if an XMPP stanza should be sent through XCP or regular XMPP.
	*/
	TIMESTAMP m_tsOtherLastSynchronized;			// Timestamp of last synchronization
	TIMESTAMP m_tsTaskIdDownloadedLast;				// Timestamp of the last task downloaded.  This Task ID is essential to avoid repeating/executing the same task when there is a retry to resend the task data.
	//CListTasks m_listTasksSocket;					// Socket tasks to be dispatched when the contact becomes online
	CListTasksSendReceive m_listaTasksSendReceive;	// Pending tasks to be completed (sent or received) when the contact becomes online

protected:
	enum
		{
		FC_kfXospSynchronizeWhenPresenceOnline				= 0x00000001,	// As soon as the contact appears online, initiate a synchronization
		FC_kfXospPendingTasks								= 0x00000002,	// The contact has pending tasks to be completed
		FC_kfContactRecommendationsNeverReceived			= 0x00000004,	// The recommendations have never been received, therefore download them as soon as the contact becomes online
		FC_kfContactNeedsInvitation							= 0x00000008,	// The contact was added, however there was never been any communication between the two paties, suggesting an invitation is recommended
		FC_kfNativeXmppOnly									= 0x00000010,	// Send all messages via native XMPP, and do not use XOSP.
		FC_kfContactUnsolicited								= 0x00000020,	// The contact was created from an unsolicited message.  Such contact is displayed with the gray color
		//FC_kfNoCambrianProtocol								= 0x00000008,	// The contact does NOT understand the Cambrian Protocol, and therefore the application should NOT send stanzas with the element <xcp>.
		//FC_kfCommentContainsInvitation						= 0x00000004,	// Old stuff where an invitation was stored in a comment

		FC_kmFlagsSerializeMask								= 0x000000FF,	// Mask of the flags to serialize
		FC_kmPresenceMaskOnline								= 0x00000700,	// Mask of the flags representing the online presence
		FC_kmPresenceMaskOnlineXosp							= 0x00000F00,	// Mask with the XOSP flag
		FC_kzPresenceOffline								= 0x00000000,	// The user is offline, and therefore no data may be sent
		FC_kePresenceChat									= 0x00000100,	// The user is available for chat (this is the green icon)
		FC_kePresenceAway									= 0x00000200,	// The user is away or idle (this is the yellow icon)
		FC_kePresenceAwayExtended							= 0x00000300,
		FC_kePresenceBusy									= 0x00000400,	// The user is busy and should not be disturbed (this is the red icon)
		FC_kfPresenceXosp									= 0x00000800,	// Special bit indicating the contact is online (chatty, away, busy) and its client software is capable to received XOSP messages.
		FC_kfRosterItem										= 0x00001000,	// The contact is found in the roster
		FC_kfRosterSubscriptionFrom							= 0x00002000,
		FC_kfRosterSubscriptionTo							= 0x00004000,
		FC_kmRosterSubscriptionBoth							= FC_kfRosterSubscriptionFrom | FC_kfRosterSubscriptionTo,
		FC_kfSubscribeAsk									= 0x00010000,
		FC_kfSubscribe										= 0x00020000,
		FC_kfSubscribed										= 0x00040000,
		FC_kfXcpComposingSendTimestampsOfLastKnownEvents	= 0x00200000,
		FC_kfXcpRequestingDataForSynchronization			= 0x00400000
		};
	UINT m_uFlagsContact;		// Various flags for the contact (some of those bits are serialized)

	IContactAlias * m_plistAliases;
	TIMESTAMP m_tsLastSeenOnline;					// Date & time when the contact was last seen online
	TIMESTAMP_MINUTES m_tsmLastStanzaReceived;		// Timestamp where the last network packet was received by the contact. This is useful to determine if the contact became idle.
	POJapiContact m_paoJapiContact;					//

public:
	TContact(TAccountXmpp * pAccount);
private:
	~TContact();
public:
	void Contact_MarkForDeletion();
	BOOL Contact_FCanBePermenentlyDeleted() const;
	void Contact_UpdateFlagCannotBeDeleted();
	inline void _ClearFlags() { m_uFlagsContact = 0; }
	inline void SetFlagContactAsUnsolicited() { m_uFlagsContact |= FC_kfContactUnsolicited; }
	inline void SetFlagContactAsInvited() { m_uFlagsContact &= ~FC_kfContactNeedsInvitation; }
	inline void SetFlagXcpComposingSendTimestampsOfLastKnownEvents() { m_uFlagsContact |= FC_kfXcpComposingSendTimestampsOfLastKnownEvents; }
	inline BOOL Contact_FuCommunicateViaXmppOnly() const { return (m_uFlagsContact & FC_kfNativeXmppOnly); }
	inline BOOL Contact_FuCommunicateViaXosp() const { return (m_uFlagsContact & FC_kfPresenceXosp); }
	inline BOOL Contact_FuNeedSynchronizeWhenPresenceOnline() const { return (m_uFlagsContact & FC_kfXospSynchronizeWhenPresenceOnline); }
	inline void Contact_SetFlagSynchronizeWhenPresenceOnline() { m_uFlagsContact |= FC_kfXospSynchronizeWhenPresenceOnline; }
	inline BOOL Contact_FuIsOnline() const { return (m_uFlagsContact & FC_kmPresenceMaskOnline); }

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
	void Vault_XmppAllocateEventMessageReceivedAndDisplayToChatLog(const CXmlNode * pXmlNodeMessageStanza, PSZUC pszuMessageBody, WChatLog * pwChatLog);
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

	void XcpApiContact_ProfileSerialize(INOUT CBinXcpStanza * pbinXcpStanzaReply) const;
	void XcpApiContact_ProfileUnserialize(const CXmlNode * pXmlNodeApiParameters);

	ITreeItemChatLogEvents * PGetContactOrGroupDependingOnIdentifier_YZ(const CXmlNode * pXmlAttributeGroupIdentifier);

	void Contact_RecommendationsUpdateFromXml(const CXmlNode * pXmlNodeApiParameters);
	void Contact_RecommendationsDisplayWithinNavigationTree(BOOL fSetFocus = FALSE);

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
	static NCompareResult S_NCompareSortContactsByNameDisplay(TContact * pContactA, TContact * pContactB, LPARAM lParamCompareSort = d_zNA);
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

///////////////////////////////////////////////////////////////////////////////////////////////////
//	ITreeItemChatLogEvents.h
//
//	Interface to display events in a Chat Log.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ITreeItemChatLogEvents_H
#define ITreeItemChatLogEvents_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

enum EChatState
	{
	eChatState_zComposing				= 0,	// The user is currently typing
	eChatState_Paused					= 1,	// The user stopped typing
	eChatState_PausedNoUpdateChatLog	= 2,	// Same as above except do not update the Chat Log.  This is an optimization to avoid flicker when pausing and adding events
	};

enum EUserCommand
{
	eUserCommand_zMessageTextSent,
	eUserCommand_ComposingStopped,
	eUserCommand_Error
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Tree Item participating in the chat log: this is either an account, or a contact or a group
class ITreeItemChatLog : public ITreeItem
{
public:
	mutable CStr m_strNicknameChatLog;	// Nickname to appear in the Chat Log (this is a very short name to distinguish between the contact and the user)
	QRGBX m_coxColorChatLog;

public:
	void ChatLog_ResetNickname();
	PSZUC ChatLog_PszGetNickname() CONST_MCC;
private:
	BOOL _FSetNickname(PSZUC pszBegin, PCHUC pchCopyUntil) CONST_MODIFIED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Base interface for a contact or a group
class ITreeItemChatLogEvents : public ITreeItemChatLog
{
public:
	TAccountXmpp * m_pAccount;				// Account where the object (Tree Item) belongs to.  Essentially, this pointer is the 'parent' of the contact or group.
	CVaultEvents * m_paVaultEvents;			// Vault containing the events of the Chat Log.  This vault contains a linked list of its chained history, however this implementation is transparently handled by CVaultEvents.
	TIMESTAMP m_tsGuiCreated;				// Date & time when the contact or group was created.  This gives an idea to the user when the contact was first added to the GUI.
	TIMESTAMP m_tsGuiLastActivity;			// Date & time when there was some activity with the contact or group.  This timestamp is typically the largest value of the last event sent or received, however may be something else related to some activity.  This timestamp is used for sorting most recent contacts or groups.
	TIMESTAMP m_tsEventIdLastSentCached;	// [Sync] Cached version of the last event sent (this cache is useful when a user is only receiving message, aka listening, and therefore optimize SocietyPro when searching of events to find the last message sent).
	TIMESTAMP m_tsOtherLastReceived;		// [Sync] Timestamp of the last received message.  This timestamp is also useful to display in the GUI how long it has been since a message was received.
	CStr m_strPathFolderDownload;			// Path to store where the downloaded files should be stored (each contact may have a different folder, so it is easier to categorize the download by projects). This field is in this class because if a class is capable to display a Chat Log, then it is also capable to receive files.
	int m_cMessagesUnread;					// Number of unread messages from the contact (this number is displayed in parenthesis after the contact name)
protected:
	WLayoutChatLog * m_pawLayoutChatLog;	// Pointer to the allocated widget to display the Chat Log

public:
	ITreeItemChatLogEvents(TAccountXmpp * pAccount);
	~ITreeItemChatLogEvents();
	void Vault_RemoveAllReferencesToObjectsAboutBeingDeleted();
	TProfile * PGetProfile() const;
	CChatConfiguration * PGetConfiguration() const;

	CVaultEvents * Vault_PGet_NZ();
	void Vault_AddEventToChatLogAndSendToContacts(PA_CHILD IEvent * paEvent);
	void Vault_SetNotModified();
	void Vault_SetModified();
	IEvent * Vault_PFindEventByID(TIMESTAMP tsEventID) CONST_MCC;
	CEventMessageTextSent * Vault_PFindEventLastMessageTextSentMatchingText(const CStr & strMessageText) const;
	void Vault_WriteEventsToDiskIfModified();
	QString Vault_SGetPath() const;
	void Vault_SendToJID(PSZAC pszJID);

	WLayoutChatLog * ChatLog_PwGetLayout_YZ() const { return m_pawLayoutChatLog; }
	WLayoutChatLog * ChatLog_PwGetLayout_NZ() CONST_MCC;
	PSZUC ChatLog_PszGetPathFolderDownload() const;
	inline int ChatLog_GetCountMessagesUnread() const { return m_cMessagesUnread; }
	void ChatLog_EventEditMessageSent(CEventMessageTextSent * pEventMessageSent);
	void ChatLog_ScrollToLastMessage();
	void ChatLog_FindText();
	void ChatLog_ChatStateIconUpdateComposingStopped(INOUT TContact * pContact);
	void ChatLog_ChatStateIconUpdate(EChatState eChatState, TContact * pContact);
	HChatLog * ChatLog_PwGet_YZ() const;

	void TreeItemChatLog_UpdateTextToDisplayMessagesUnread();
	void TreeItemChatLog_UpdateTextAndIcon();
	void TreeItemChatLog_IconUpdateOnNewMessageArrivedFromContact(PSZUC pszMessage, TContact * pContact, TGroupMember * pMember = NULL);
	void TreeItemChatLog_IconUpdateOnMessagesReadForContact(TContact * pContact);

	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const;			// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);			// From IXmlExchange
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);	// From ITreeItem
	virtual void TreeItem_GotFocus();										// From ITreeItem

	// New virtual methods
	virtual const QBrush & ChatLog_OGetBrushForNewMessageReceived();
	virtual void Vault_GetHashFileName(OUT SHashSha1 * pHashFileNameVault) const = 0;
	virtual void Vault_GetEventsForChatLog(OUT CArrayPtrEvents * parraypEventsChatLog) CONST_MCC;

	EUserCommand Xmpp_EParseUserCommandAndSendEvents(IN_MOD_INV CStr & strCommandLineOrMessage);
	void Xmpp_SendEventMessageText(PSZUC pszMessage);
	void Xmpp_SendEventFileUpload(PSZUC pszFileUpload);
	void Xmpp_SendEventsFileUpload(IN_MOD_INV PSZU pszmFilesUpload);
	void Xmpp_Ping();
	void Xmpp_QueryVersion();
	void Xmpp_WriteXmlChatState(EChatState eChatState) CONST_MCC;
	CSocketXmpp * Xmpp_PGetSocketOnlyIfReady() const;

	void XcpApi_Invoke(PSZUC pszApiName, PSZUC pszXmlApiParameters = d_zNA);
	void XcpApi_Invoke_RecommendationsGet();
	void XcpApi_Invoke_Synchronize();

	void DisplayDialogBallotSend(CEventBallotSent * pEventBallotInit = NULL);
	void DisplayDialogSendFile();
	BOOL DisplayDialogAddContactsToGroupFu();

	void UpdateTimestampLastActivity();
	static NCompareResult S_NCompareSortByTimestampLastActivity_RecentFirst(ITreeItemChatLogEvents * pChatLogA, ITreeItemChatLogEvents * pChatLogB, LPARAM lParamCompareSort = d_zNA);
	static NCompareResult S_NCompareSortByTimestampEventLastReceived_RecentFirst(ITreeItemChatLogEvents * pChatLogA, ITreeItemChatLogEvents * pChatLogB, LPARAM lParamCompareSort = d_zNA);
	friend class CVaultEvents;
}; // ITreeItemChatLogEvents

class CArrayPtrTreeItemChatLogEvents : public CArrayPtrTreeItems
{
public:
	void SortByLastActivity();
	void SortByEventLastReceived();

};

#endif // ITreeItemChatLogEvents_H

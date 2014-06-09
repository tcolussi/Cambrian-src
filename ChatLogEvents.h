///////////////////////////////////////////////////////////////////////////////////////////////////
//	ChatLogEvents.h
//
//	Classes holding events for a Chat Log.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHATLOGEVENTS_H
#define CHATLOGEVENTS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//	class CVaultEvents
//
//	Class holding events for a Chat Log.
//	The motivation for this class is having events stored into multiple files.
//	Since a chat history may spawn many years and the chat log may become quite large,
//	for performance reasons, it is important to display the Chat Log with the most recent events,
//	however with the ability to display the entire chat history.
//
//	This class is named 'Vault' because each vault is storing events in a separate encrypted file.
//
//	IMPLEMENTATION NOTE
//	A vault is somewhat autonomus, and if it contains a chain of history, then the class CVaultEvents should handle it transparently.
//	Most methods accessing the history vaults are using recursion.  This tail recursion may be a bit hard on the stack, however given a user may have no more than 1000 history vaults, it is fine on today's hardware.
//
class CVaultEvents // (vault)
{
public:
	ITreeItemChatLogEvents * m_pParent;				// Parent of the vault which is either a contact or a group
	CArrayPtrEvents m_arraypaEvents;				// Events which belongs to the current vault (file). This array is sorted in chronological order.
private:
	IEvent * m_pEventLastSaved;						// Pointer to the last event saved to disk.  If this pointer is NULL, it means the vault was modified and must be saved to disk.  This pointer is the equivalent of the 'dirty' flag.
	QString m_sPathFileName;						// Full path where the vault was read from disk, and therefore the full path to save to disk (if modified).
	struct
		{
		SHashSha1 m_hashFileName;					// Hash of the vault.  This hash is the filename to load the rest of the history.
		TIMESTAMP tsEventFirst;						// Timestamp of the first event within the vault
		int cEventsVault;							// How many events are in the vault
		int cEventsTotal;							// Total number of events in the entire history
		CVaultEvents * m_paVault;					// Pointer to the vault of the rest of the history.  This is useful to chain a long history of events into multiple files (vaults).
		} m_history;	// Data necessary to load the vault history
public:
	CVaultEvents(PA_PARENT ITreeItemChatLogEvents * pTreeItemParent, const SHashSha1 * pHashFileName);
	~CVaultEvents();
	inline void SetNotModified() { m_pEventLastSaved = m_arraypaEvents.PGetEventLast_YZ(); }
	inline void SetModified() { m_pEventLastSaved = NULL; }
	void Events_UnserializeFromStanza(const CXmlNode * pXmlNodeEventsStanza);
	void ReadEventsFromDisk(const SHashSha1 * pHashFileName);
	void WriteEventsToDiskIfModified();
	void GetEventsForChatLog(OUT CArrayPtrEvents * parraypEventsChatLog) CONST_MCC;
	int UEventsRemaining(IEvent * pEvent) const;
	inline IEvent * PGetEventLast_YZ() const { return m_arraypaEvents.PGetEventLast_YZ(); }
	inline IEvent * PFindEventByID(TIMESTAMP tsEventID) const { return m_arraypaEvents.PFindEventByID(tsEventID); }
	IEvent * PFindEventByTimestampOther(TIMESTAMP tsOther) CONST_MCC;
	IEvent * PFindEventReceivedByTimestampOther(TIMESTAMP tsOther) CONST_MCC;
	IEvent * PFindEventReceivedByTimestampOtherMatchingContactSender(TIMESTAMP tsOther, TContact * pContactGroupSender) CONST_MCC;
	IEvent * PFindEventReceivedLastMatchingContactSender(TContact * pContactGroupSender) CONST_MCC;
	IEvent * PFindEventSentLargerThanTimestamp(TIMESTAMP tsEventID) CONST_MCC;
	IEvent * PFindEventNext(TIMESTAMP tsEventID, OUT int * pcEventsRemaining) CONST_MCC;
	IEvent * PFindEventLargerThanTimestamp(TIMESTAMP tsEventID) CONST_MCC;
	IEvent * PFindEventLargerThanTimestampOther(TIMESTAMP tsOther) CONST_MCC;
}; // CVaultEvents

enum EChatState
	{
	eChatState_zComposing,
	eChatState_fPaused
	};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Tree item participating in the chat log: this is either an account, or a contact or a group
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
	TIMESTAMP m_tsCreated;					// Date & time when the contact or group was created.  This gives an idea to the user when the contact was first added to the GUI.
	TIMESTAMP m_tsEventIdLastSent;
	TIMESTAMP m_tsOtherLastReceived;		// Timestamp of the last received message.  This timestamp is useful to display in the GUI how long it has been since a message was received, but most importantly it is used to synchronize to determine if there are any missing message(s)
	CStr m_strPathFolderDownload;			// Path to store where the downloaded files should be stored (each contact may have a different folder, so it is easier to categorize the download by projects). This field is in this class because if a class is capable to display a Chat Log, then it is also capable to receive files.
protected:
	int m_cMessagesUnread;					// Number of unread messages from the contact (this number is displayed in parenthesis after the contact name)
	WLayoutChatLog * m_pawLayoutChatLog;	// Pointer to the allocated widget to display the Chat Log

public:
	ITreeItemChatLogEvents(TAccountXmpp * pAccount);
	~ITreeItemChatLogEvents();
	CChatConfiguration * PGetConfiguration() const;

	CVaultEvents * Vault_PGet_NZ();
	void Vault_InitEventForVaultAndDisplayToChatLog(PA_CHILD IEvent * paEvent);
	void Vault_SetNotModified();
	void Vault_SetModified();
	IEvent * Vault_PFindEventByID(TIMESTAMP tsEventID) CONST_MCC;
	CEventMessageTextSent * Vault_PGetEventLastMessageSent_YZ() const;
	void Vault_WriteEventsToDiskIfModified();
	QString Vault_SGetPath() const;

	WLayoutChatLog * ChatLog_PwGetLayout_NZ() CONST_MCC;
	PSZUC ChatLog_PszGetPathFolderDownload() const;
	inline int ChatLog_GetCountMessagesUnread() const { return m_cMessagesUnread; }
	void ChatLog_EventEditMessageSent(CEventMessageTextSent * pEventMessageSent);
	void ChatLog_DisplayNewEvent(IEvent * pEvent);
	void ChatLog_ScrollToLastMessage();
	void ChatLog_FindText();
	void ChatLog_ChatStateIconUpdate(EChatState eChatState, TContact * pContact);
	WChatLog * ChatLog_PwGet_YZ() const;

	void TreeItemChatLog_UpdateTextToDisplayMessagesUnread();
	void TreeItemChatLog_UpdateTextAndIcon();
	void TreeItemChatLog_IconUpdateOnNewMessageArrivedFromContact(PSZUC pszMessage, TContact * pContact);
	void TreeItemChatLog_IconUpdateOnMessagesReadForContact(TContact * pContact);

	virtual void * PGetRuntimeInterface(const RTI_ENUM rti) const;			// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);			// From IXmlExchange
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);	// From ITreeItem
	virtual void TreeItem_GotFocus();										// From ITreeItem

	// New virtual methods
	virtual const QBrush & ChatLog_OGetBrushForNewMessageReceived();
	virtual void Vault_GetHashFileName(OUT SHashSha1 * pHashFileNameVault) const = 0;
	virtual void Vault_GetEventsForChatLog(OUT CArrayPtrEvents * parraypEventsChatLog) CONST_MCC;

	void XmppUploadFile(PSZUC pszFileUpload);
	void XmppUploadFiles(IN_MOD_INV PSZU pszmFilesUpload);
	CSocketXmpp * Xmpp_PGetSocketOnlyIfReady() const;
	//void Socket_WriteXmlFormatted(PSZAC pszFmtTemplate, ...) const;
	void XcpStanza_AppendTimestampsOfLastKnownEvents(IOUT CBinXcpStanzaType * pbinXcpStanza) const;

	void DisplayDialogSendFile();
	friend class CVaultEvents;
}; // ITreeItemChatLogEvents

#endif // CHATLOGEVENTS_H

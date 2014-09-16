#ifndef CVAULTEVENTS_H
#define CVAULTEVENTS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
//	class CVaultEvents
//
//	Core data structure to store events.
//	This class does very little, however provide an abstraction layer how events are stored.
//
class CVaultEventsCore	// vault
{
public:
	CArrayPtrEvents m_arraypaEvents;	// Events belonging to a vault.  A vault is typically saved to a file on disk, however may be saved in memory as part of a larger object. This array is sorted in chronological order.

public:
	CVaultEventsCore();
	~CVaultEventsCore();
};

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
class CVaultEvents : public CVaultEventsCore // (vault)
{
public:
	ITreeItemChatLogEvents * m_pParent;				// Parent of the vault which is either a contact or a group
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
	CVaultEvents(PA_PARENT ITreeItemChatLogEvents * pTreeItemParent, const SHashSha1 * pHashFileName = d_zNA);
	~CVaultEvents();
	inline void SetNotModified() { m_pEventLastSaved = m_arraypaEvents.PGetEventLast_YZ(); }
	inline void SetModified() { m_pEventLastSaved = NULL; }
	void EventAddAndDispatchToContacts(PA_CHILD IEvent * paEvent, PA_CHILD CEventUpdaterSent * paEventUpdater = NULL);
	void EventsSerializeForMemory(IOUT CBinXcpStanza * pbinXmlEvents) const;
	void EventsUnserialize(const CXmlNode * pXmlNodeEvents);
	void EventsUnserialize(PSZUC pszXmlNodeEvents);
	void ReadEventsFromDisk(const SHashSha1 * pHashFileName);

	void XmlExchange(PSZAC pszTagNameVault, INOUT CXmlExchanger * pXmlExchanger);

	void WriteEventsToDiskIfModified();
	void GetEventsForChatLog(OUT CArrayPtrEvents * parraypEventsChatLog) CONST_MCC;
	int UEventsRemaining(IEvent * pEvent) const;
	int UCountEventsReceivedByOtherGroupMembersSinceTimestampEventID(TIMESTAMP tsEventID, TContact * pContactExclude) CONST_MCC;
	int UCountEventsReceivedByOtherGroupMembersSinceTimestampOther(TIMESTAMP tsOther) CONST_MCC;
	int UGetEventsSinceTimestamp(TIMESTAMP tsEventID, OUT CArrayPtrEvents * parraypEvents) CONST_MCC;

	inline IEvent * PGetEventLast_YZ() const { return m_arraypaEvents.PGetEventLast_YZ(); }
	inline IEvent * PFindEventByID(TIMESTAMP tsEventID) const { return m_arraypaEvents.PFindEventByID(tsEventID); }
	IEvent * PFindEventReceivedByTimestampOther(TIMESTAMP tsOther, TGroupMember * pMember) CONST_MCC;
	IEvent * PFindEventReceivedByTimestampOther(TIMESTAMP tsOther, TContact * pContactGroupSender) CONST_MCC;
	IEvent * PFindEventNext(TIMESTAMP tsEventID, OUT int * pcEventsRemaining) CONST_MCC;
	IEvent * PFindEventNextReceivedByOtherGroupMembers(TIMESTAMP tsEventID, TContact * pContactExclude, OUT int * pcEventsRemaining) CONST_MCC;
	IEvent * PFindEventReplacedBy(IEvent * pEventReplacing) CONST_MCC;
	IEvent * PFindEventReplacing(IEvent * pEventReplaced) CONST_MCC;
}; // CVaultEvents

#endif // CVAULTEVENTS_H

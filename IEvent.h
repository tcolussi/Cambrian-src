///////////////////////////////////////////////////////////////////////////////////////////////////
//	IEvent.h
//
//	Defines the interface and event objects for the Chat Log.  Events include sending and receiving a text message, a file, or other objects.
//	This file also includes the interfaces for socket tasks related to the events.  For instance, downloading a file requires a socket task to send chunks of data until completion.
//
//	An event is class containing data to display in the Chat Log.
//	A task is a class containing code to process an event.  Once an event completed, its corresponding task is deleted/destroyed.  As a result, most events do not have a running task because they completed.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef IEVENT_H
#define IEVENT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//	#defines for events
#define d_szEventDebug_strContactSource					"_CONTACTSOURCE"	// Add an extra field to the base class IEvent to include the contact who transmitted the event.  This field is useful to debug group chat.
#define d_szEventDebug_strVersion						"_VERSION"			// Add the Cambrian version when the event was created (sent or received).  Again, this is useful to save time by avoiding debugging old events received before a given bugfix.

#define d_chEvent_Attribute_tsEventID					'i'	// Identifier of the event
#define d_szEvent_Attribute_tsEventID_t					" i='$t'"
#define d_chEvent_Attribute_tsOther						'o'	// Other timestamp for the event
#define d_szEvent_Attribute_tsOther_t					" o='$t'"

//	Shorter for the timestamps
#define _tsI											d_szEvent_Attribute_tsEventID_t
#define _tsO											d_szEvent_Attribute_tsOther_t

//	Essentially, there are 4 reserved attributes by the Cambrian Protocol ('i', 'o', 'g' and 'G')
//	The following attributes 'make sense' for the receiver of the XCP data.
#define d_chXCPa_tsEventID				d_chEvent_Attribute_tsEventID
#define d_szXCPa_tsEventID_t			d_szEvent_Attribute_tsEventID_t
#define d_chXCPa_tsOther				d_chEvent_Attribute_tsOther
#define d_szXCPa_tsOther_t				d_szEvent_Attribute_tsOther_t
#define d_chXCPa_pContactGroupSender	'g'		// Pointer of the contact who sent the event/message to the group.  Of course, this attribute is present only when receiving a group message
#define d_chXCPa_IEvent_uFlagsEvent		'G'		// Rare attribute indicating some bits of IEvent::m_uFlagsEvent are serialized.  Since 'g' is already reserved by the XCP protocol, then 'G' is used for flaGs.
//	X for signature, and x for extra data

//	The enumeration EEventClass is used to serialize and unserialize events.
//	This enumeration is somewhat as RTI_ENUM, however since the events are serialized, letters of the alphabet are used to identify different classes of events.
//	This way, this enum may be serialized directly into an XML file.
//	Also, all events sent begin with an uppercase, and all events received begin with a lowercase.  This way, looking at the event class (or the first character in the XML element)
//	gives us useful information about what type of event we are dealing.
enum EEventClass
	{
	eEventClass_kfNeverSerializeToDisk			= _USZUF(0x02),	// This event is never serialized to disk (example: pinging a remote client, or sending raw XML to the socket)
	eEventClass_kfNeverSerializeToXCP			= _USZUF(0x04),	// This event is never serialized (sent) via the XCP protocol (example: downloading a large stanza from a contact)
	eEventClass_mfNeverSerialize				= eEventClass_kfNeverSerializeToDisk | eEventClass_kfNeverSerializeToXCP,

	eEventClass_kfReceivedByRemoteClient		= _USZUF(0x08),	// This event was received by the remote client.  This bit is useful to quickly determine what was sent, and what was received, as the GUI often displays different colors.  Roughly half of the eEventClass_* will have this bit set.

	_g_											= 'g',			// ALL EVENTS received for a group must begin with this letter.  This way, it is possible to quickly determine of an event is for a group

	eEventClass_eMessageTextComposing			= _USZU1('Y'),	// The user is tYping something...  (this is a notification to the remote client, as there is no instance of this 'event' nor it is saved to disk)
		#define d_chXCPa_MessageTextComposing_State				's'	// Attribute to indicate the state of the 'composing' event.  If this attribute is not present, it means the user is just typing.
		#define d_chXCPv_MessageTextComposing_State_Paused		'p'	// The user paused, or stopped typing for 10 seconds.
		#define d_chXCPv_MessageTextComposing_State_Deleting	'd'	// NYI: The user is currently deleting what he typed.  This indicates the user is a bit hesitant about what to reply.
		/*
		#define d_szXCPe_MessageTextComposingStarted			"<Y/>"
		#define d_szXCPe_MessageTextComposingPaused				"<Y s='p'/>"
		*/

	eEventClass_eMessageTextSent					= _USZU1('T'),
	eEventClass_eMessageTextReceived				= _USZU1('t'),
	eEventClass_eMessageTextReceived_class			= eEventClass_eMessageTextReceived | eEventClass_kfReceivedByRemoteClient,
	eEventClass_eMessageXmlRaw						= _USZU1('~'),	// Send raw XML directly to the server (this message is never serialized in any way
	eEventClass_eMessageXmlRaw_class				= eEventClass_eMessageXmlRaw | eEventClass_kfNeverSerializeToDisk,	// The raw XML is never 'serialized' as part of the Cambrian Protocol because it is sent directly to the server unencrypted.

	eEventClassLegacy_eFileSent						= _USZU1('F'),
	eEventClassLegacy_eFileReceived					= _USZU1('f'),	// Basic compatibility with old file format so we can load old events

	eEventClass_eFileSent							= _USZU2('F', 'D'),	// TODO: Replace this by 'F' in about August, 2015
	eEventClass_eFileSentTo							= _USZU3('F', 'T', 'O'),
	eEventClass_eFileReceived						= _USZU2('f', 'd'),
	eEventClass_eFileReceived_class					= eEventClass_eFileReceived | eEventClass_kfReceivedByRemoteClient,

	eEventClass_ePing								= _USZU1('P'),	// There is no 'pong' event as the remote client direcly replies to the ping, without allocating any event.
	eEventClass_ePing_class							= eEventClass_ePing | eEventClass_kfNeverSerializeToDisk,
		#define d_chXCPa_PingTime								't'
		#define d_szXCPa_PingTime_t								" t='$t'"

	eEventClass_eVersion							= _USZU3('V', 'E', 'R'),	// The event version is somewhat similar as 'ping', as the remote client directly replies without allocating any event.
	eEventClass_eVersion_class						= eEventClass_eVersion | eEventClass_kfNeverSerializeToDisk,
		#define d_chXCPa_eVersion_Version						'v'
		#define d_szXCPa_eVersion_Version						" v='" d_szApplicationVersion "'"
		#define d_chXCPa_eVersion_Platform						'p'
		#define d_szXCPa_eVersion_Platform						" p='" d_szOS "'"	// Platform / Operating System
		#define d_chXCPa_eVersion_Client						'c'
		#define d_szXCPa_eVersion_Client						" c='" d_szApplicationName "'"

	eEventClass_eHelp_class							= _USZU3('H', 'L', 'P') | eEventClass_mfNeverSerialize,	// This event is never serialized to disk nor via XCP

	eEventClass_eGroupInfo							= _USZU2(_g_, 'i'),
	eEventClass_eGroupMemberJoins					= _USZU2(_g_, 'j'),
	eEventClass_eGroupMemberInvited_class			= eEventClass_eGroupMemberJoins,


	eEventClass_eUpdaterSent							= _USZU1('U'),
	eEventClass_eUpdaterReceived						= _USZU1('u'),
	eEventClass_eUpdaterReceived_class					= eEventClass_eUpdaterReceived | eEventClass_kfReceivedByRemoteClient,

	eEventClass_eWalletTransactionSent				= _USZU1('W'),
	eEventClass_eWalletTransactionReceived			= _USZU1('w'),
	eEventClass_eWalletTransactionReceived_class	= eEventClass_eWalletTransactionReceived | eEventClass_kfReceivedByRemoteClient,

	eEventClass_eBallotSent							= _USZU3('B', 'A', 'L'),
	eEventClass_eBallotSent_class					= eEventClass_eBallotSent,
	eEventClass_eBallotReceived						= _USZU3('b', 'a', 'l'),
	eEventClass_eBallotReceived_class				= eEventClass_eBallotReceived | eEventClass_kfReceivedByRemoteClient,
    eEventClass_eBallotPoll							= _USZU3('P', 'O', 'L'),

	// Not used
	eEventClass_eServiceDiscovery_Query				= _USZU2('S', 'D'),		// There is no 'event' for service discovery, however the "SD"
	eEventClass_eServiceDiscovery_Response			= _USZU2('s', 'd'),

	// To be deleted in 2015.  These were old event types used in previous file formats
	eEventClassLegacy_chMessageSent					= 'I',	// Instant text message
	eEventClassLegacy_chMessageReceived				= 'i',
	eEventClassLegacy_chMessageOldFormat			= 'm',	// Very very early serialization of messages, before using events.

	eEventClass_eNull = 0	// Unknown event class
	}; // EEventClass

inline EEventClass EEventClassFromPsz(PSZUC pszEventClass) { return (EEventClass)UszuFromPsz(pszEventClass); }
#define FEventClassReceived(chEventClass0)		((chEventClass0) >= 'a')	// All received events begin with a lowercase

//	Enumeration describing serialized XML
enum EXml
{
	eXml_zAttributesOnly	= 0,	// The event contains XML attribute only
	eXml_ElementPresent		= 1,	// The event contains one or more XML element
	eXml_NoSerialize		= 2,	// The event shall not be serialized.  The virtual method XmlSerializeCoreE() is therefore used for processing rather than serialization.
};

//	Enumeration describing how to update the GUI (typically the Chat Log)
enum EGui
{
	eGui_zUpdate			= 0,	// The event should be updated in the Chat Log
	eGui_NoUpdate			= 1,	// The event was unchanged and therefore there is no need to update the Chat Log
};

///////////////////////////////////////////////////////////////////////////////////////////////////
enum EErrorXcpApi
{
	eErrorXcpApi_zSuccess,			// No error
	eErrorXcpApi_ApiUnknown,
	eErrorXcpApi_IdentifierMissing,		// No identifier was supplied while the API is expecting one
	eErrorXcpApi_IdentifierInvalid,		// The identifier is not valid for the API, typically of an incorrect syntax
	eErrorXcpApi_IdentifierNotFound,	// The identifier could not be found.
	eErrorXcpApi_ParameterMissing,		// The API is missing an essential parameter, thus failing
	eErrorXcpApi_ParameterInvalid,		// One of the parameter is invalid
};


#define DEBUG_XCP_TASKS				// Extra code to use tasks to transmit data via the XMPP protocol so we can test this code path.  This is done by lowering the threshold to split XMPP stanzas into tasks.

//	Object to store information necessary to send an XCP stanza.
//	An XCP Stanza is an XML within a XMPP message.
class CBinXcpStanza : public CBin
{
protected:
	enum
		{
		F_kzSendAsXmppMessage	= 0x0000,	// The stanza is cached on the XMPP server if the remote client is unavailable to receive it
		F_kfSendAsXmppIQ		= 0x0001,	// The stanza is sent directly to the remote client, or it is ignored if the remote client is unavailable

		F_kzSerializeToContact	= 0x0000,	// The events are serialized to be sent to a contact
		F_kfSerializeToDisk		= 0x0002,	// The events are serialized to be saved to disk
		F_kfSerializeForCloning	= 0x0004,	// The event is serialized to be cloned (duplicated)

		F_kfTaskAlreadyIncluded	= 0x0008,	// A task was included to the XMPP stanza, and therefore there should be no more
		F_kfContainsSyncData	= 0x0010,	// The blob contains synchronization data, and therefore if a task has to be created, then it should be marked as so, so there are no duplicate tasks to synchronize
		};
	UINT m_uFlags;				// Various flags how to send the XMPP stanza
public:
	TContact * m_pContact;		// Contact to serialize for.  This pointer is NULL if serializing to disk.
	SOffsets m_oOffsets;		// Offsets where the XML element begins as well as its data.  This structure is useful to detect empty XML elements as well as removing them.
	#ifdef DEBUG_XCP_TASKS
	int m_cbStanzaThresholdBeforeSplittingIntoTasks;	// Threshold before splitting an XMPP stanza into tasks
	#endif

public:
	CBinXcpStanza();
	int CbGetAvailablePayloadToSendBinaryData() const;
	inline BOOL FuSerializingEventToDisk() const { return (m_uFlags & F_kfSerializeToDisk); }
	inline BOOL FuSerializingEventToDiskOrCloning() const { return (m_uFlags & (F_kfSerializeToDisk | F_kfSerializeForCloning)); }
	void BinXmlInitStanzaWithGroupSelector(TGroup * pGroup);
	void BinXmlInitStanzaWithXmlRaw(PSZUC pszMessageXml);
	void BinXmlAppendTimestampsToSynchronizeWithContact(TContact * pContact);
	void BinXmlAppendTimestampsToSynchronizeWithGroupMember(TGroupMember * pMember);
	void BinXmlAppendAttributeOfContactIdentifierOfGroupSenderForEvent(const IEvent * pEvent);
	void BinXmlAppendAttributeUIntHexadecimalExcludeForXcp(CHS chAttributeName, UINT uAttributeValueHexadecimal, UINT kmFlagsExcludeForXcp);

	void BinAppendXmlEventSerializeOpen(const IEvent * pEvent, TIMESTAMP tsOther);
	void BinAppendXmlEventSerializeDataAndClose(const IEvent * pEvent);

	void XcpApi_ExecuteApiList(const CXmlNode * pXmlNodeApiList);
	void XcpApi_ExecuteApiName(PSZUC pszApiName, const CXmlNode * pXmlNodeApiData);
	void XcpApi_ExecuteApiResponse(PSZUC pszApiName, const CXmlNode * pXmlNodeApiResponse);
	void XcpApi_CallApiWithResponseToEvent(const IEvent * pEvent, PSZUC pszApiName);
	void XcpApi_SendDataToEvent_VE(const IEvent * pEvent, PSZAC pszFmtTemplate, ...);
	void BinXmlAppendXcpAttributesForApiRequestError(EErrorXcpApi eErrorXcpApi, PSZUC pszxErrorData);

	void BinXmlAppendXcpApiCall_SendEventToContact(TContact * pContact, IEvent * pEvent, CEventUpdaterSent * pEventUpdater = NULL);

	void BinXmlAppendXcpApiRequest(PSZAC pszApiName, PSZUC pszXmlApiParameters);
	void BinXmlAppendXcpApiRequest_Group_Profile_Get(PSZUC pszGroupIdentifier);

	void BinXmlAppendXcpApiMessageSynchronization(const CXmlNode * pXmlNodeSynchronize, BOOL fNewMessage, const CXmlNode * pXmlAttributeGroupIdentifier);

	void BinXmlSerializeEventForDisk(const IEvent * pEvent);
	void BinXmlSerializeEventForXcpCore(const IEvent * pEvent, TIMESTAMP tsOther);

	void XcpSendTaskDataToContact(TContact * pContact, const CTaskSendReceive * pTaskUpload, int ibData = 0);

	void XcpSendStanzaToContactOrGroup(const ITreeItemChatLogEvents * pContactOrGroup) CONST_MCC;
	void XospSendStanzaToContactAndEmpty(TContact * pContact) CONST_MCC;
	void XcpSendStanza() CONST_MCC;

	void XmppWriteStanzaToSocket();
	void XmppWriteStanzaToSocketOnlyIfContactIsUnableToCommunicateViaXcp_VE(PSZAC pszFmtTemplate, ...);
public:
	/*
	https://tools.ietf.org/html/draft-saintandre-rfc3920bis-09

	XMPP is optimized for the exchange of relatively large numbers of relatively small stanzas. A client or server MAY enforce a maximum stanza size.
	The maximum stanza size MUST NOT be smaller than 10000 bytes, from the opening "<" character to the closing ">" character.
	If an entity receives a stanza that exceeds its maximum stanza size, it MUST return a <not-acceptable/> stanza error or a <policy-violation/> stream error.

	http://tools.ietf.org/html/rfc6122
	Each allowable portion of a JID (localpart, domainpart, and resourcepart) MUST NOT be more than 1023 bytes in length,
	resulting in a maximum total size (including the '@' and '/' separators) of 3071 bytes.
	*/
	static const int c_cbStanzaMaxSize		= 10000;	// Hard limit on the stanza size
	static const int c_cbStanzaMaxPayload	= 8000;		// Split any stanza having a payload larger than 8000 bytes, as Cambrian reserved 2000 bytes for the XMPP header. Sure some XMPP clients may have JIDs of 3071 bytes, however those clients won't be able to communicate with the Cambrian Protocol.
	static const int c_cbStanzaMaxBinary	= 6400;		// Since binary data is transmitted in Base85, the encoding of 6400 bytes requires 25% more space (6400 * 1.25 = 8000)
	static const int c_cbStanzaThresholdBeforeSplittingIntoTasks = 8200;	// This value is slightly higher than c_cbStanzaMaxPayload so there may be smaller data sent along with the payload
}; // CBinXcpStanza

class CBinXospStanzaForDisk : public CBinXcpStanza
{
public:
	inline CBinXospStanzaForDisk() { m_uFlags = F_kfSerializeToDisk; }
};

//	Send an XMPP stanza however do not attempt to retry (or cache) its value if it fails.
//	This is good for pings and other API calls which should not be retried or cached.
class CBinXospStanzaNoRetry : public CBinXcpStanza
{
public:
	inline CBinXospStanzaNoRetry() { m_uFlags = F_kfSendAsXmppIQ; }
};

class CBinXcpStanzaTypeSynchronize : public CBinXcpStanza
{
public:
	CBinXcpStanzaTypeSynchronize(ITreeItemChatLogEvents * pContactOrGroup);
protected:
	void BinAppendXospSynchronizeTimestampsAndClose(ITreeItemChatLogEvents * pContactOrGroup, const TIMESTAMP * ptsOtherLastSynchronized);
};

//	Generic class to copy the data from an event to another event.
class CBinXcpStanzaEventCopier : public CBinXcpStanza
{
protected:
	TContact * m_paContact;		// We need a contact to clone the event, so use an empty contact so there is no interference with existing data
public:
	CBinXcpStanzaEventCopier(ITreeItemChatLogEvents * pContactOrGroup);
	~CBinXcpStanzaEventCopier();

	void EventCopy(IN const IEvent * pEventSource, OUT IEvent * pEventDestination);
};

//	Generic class to clone an event
class CBinXcpStanzaEventCloner : public CBinXcpStanzaEventCopier
{
public:
	IEvent * PaEventClone(IEvent * pEventToClone);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Define a custom scheme to handle events in the Chat Log.
//	When the user moves the mouse over an hyperlink or clicks on an action in the Chat Log, the scheme will instruct the application to invoke the virtual methods HyperlinkGetTooltipText() and HyperlinkClicked().
#define d_chSchemeCambrian			'c'					// "c:"	(This letter must be lowercase because Qt makes all schemes lowercase)
#define d_szSchemeCambrian			"c"
#define d_chSchemeCambrianActionSeparator		','		// Seperator between the EventID and the Action

#define d_SzMakeCambrianAction(szCambrianAction)	"c:,"szCambrianAction

#define d_wSchemeCambrian			UINT16_FROM_CHARS(d_chSchemeCambrian, ':')
#define FIsSchemeCambrian(pszUrl)	(*(WORD *)(pszUrl) == d_wSchemeCambrian)

CHS ChGetCambrianActionFromUrl(PSZUC pszUrl);

#define d_chCambrianAction_None					'\0'
#define d_chCambrianAction_DisplayAllHistory	'*'
#define d_szCambrianAction_DisplayAllHistory	"*"

///////////////////////////////////////////////////////////////////////////////////////////////////
#define d_szClassForChatLog_HyperlinkDisabled		"Hd"	// Display an hyperlink in gray, giving the visual effect is is disabled
#define d_szClassForChatLog_ButtonHtml				"Hb"	// Use HTML CSS to create an hyperlink looking like a push button.

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Special values for IEvent::m_tsOther
#define	d_tsOther_ezEventNeverSent		0			// The event has never been sent. As soon as there is a valid socket connection with the server, the method BinXmlSerializeEventForXcp() will attempt to send the event.
#define	d_tsOther_eEventSentOnce		1			// The event was sent to the server, however was not delivered.  The proof of delivery is the timestamp of the confirmation which must be larger than d_tsOther_kmReserved.
#define d_tsOther_kmReserved			0xFFFFFFFF	// Reserving the first 32 bits for the timestamp.  This leaves plenty of room to have various flags while making little difference in the 64-bit value. (btw: 0xFFFFFFFF milliseconds is 1970-02-19@11:02:47, and serialized as "3TSmc9t")
#define d_tsOther_tsEventDeliveryMin   0x100000000	// Minimum value of a timestamp to be considered delivered.  This value is not used in the code, as it is there to document a successful delivery is larger than d_tsOther_kmReserved.


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Interface IEvent
//
//	Interface to attach an event to the Chat Log.
//
//	The motivation for this interface is providing a uniform mechanism for interacting with entry in the Chat Log.
//	In a nutshell, every line (text block) in the Chat Log has a corresponding event, and the same event may be shared by multiple Chat Logs, or other applications such as a wallet or the Ballotmaster.
//	For instance, a Chat Log may display all events related to a contact, while another Chat Log may display only the files downloaded from the same contact.
//
//	Each event has a unique identifier (m_tsEventID) which is useful to detect duplicates, or update the state of an event, such as updating the progress of a download, or correcting a typo in a previously sent message.
//	As a result, the same event may be serialized multiple times, and unserialized multiples times, thus updating the state of the event each time.
//	Likewise, each event has another timestamp to determine when the event completed.  By default this value is initialized to zero, as many events do not have a completion time.
//
//	Every event has a parent owner, however may be in the possession by another object.  A good example is the wallet holding the transaction events for a contact.
//	The name 'event' was chosen because the Chat Log is 'event driven'.  The word 'message' could be a bit confusing between a message between two Cambrian clients, and a message typed by the user.
//
class IEvent	// (event)
{
public:
	CVaultEvents * m_pVaultParent_NZ;		// Pointer of the vault holding the event
	TContact * m_pContactGroupSender_YZ;	// Pointer to the contact who sent the group event.  If this pointer is NULL, it means the event is not part of group conversation, or the event was sent by the user.
	TIMESTAMP m_tsEventID;		// Timestamp to identify the event.  This field is initialized with Timestamp_GetCurrentDateTime()
	TIMESTAMP m_tsOther;		// Other timestamp related to the event.  Typically this timestamp is the time when the event completed, however it may be interpreted as the time when the event started, such as the time the remote contact typed the message.  This timestame is useful to determine how long it took for the event/task to complete.
	enum
		{
		FE_kzDefault				= 0x0000,
		FE_kfReplacing				= 0x0001,	// The event is replacing another [previous] event (for instance, the event contains the text replacing an older event).  To find the old event, use CEventUpdater.
		FE_kfReplaced				= 0x0002,	// The event has been replaced by another event (for instance, the text was edited, and therefore another event contains the updated text).  The flags FE_kfReplacing and FE_kfReplaced are not mutually exclusive, as an event may replacing another event, which in turn was replaced by a more recent event.
		FE_kfArchived				= 0x0004,	// The event is archieved

		FE_kmSerializeMask			= 0x000F,

		FE_kfEventHidden			= 0x0010,	// The event should not be displayed in the Chat Log, typically when an event replaces another event.
		FE_kfEventDeleted			= 0x0020,	// The event was marked as deleted and therefore should not be saved
		FE_kfEventOutOfSync			= 0x0040,	// The event is out of sync, and therefore display a little icon to show to the user
		FE_kfEventDeliveryConfirmed	= 0x0080,	// The event was delivered and its checkmark was displayed.  This flag should be eventually removed, however it is a workaround a Qt bug in the QTextEdit
		FE_kfEventProtocolWarning	= 0x0100,	// Therew was a minor error while transmitting the event.  This bit is set to give a second chance to retry, however to prevent an infinite loop to retry over and over
		FE_kfEventProtocolError		= 0x0200,	// There was a protocol error while sending the event (this means one of the client is out-of-date and is unable to allocate the event because it is unknown)
		FE_kfEventError				= 0x0400,	// There has been an error related to the event.  Perhaps next time SocietyPro restarts the error will go away.  This flag is there to prevent to repeat displaying an error message, or performing an expensive computation, over and over again.
		};
	mutable UINT m_uFlagsEvent;				// Flags related to the event.  Most of those flags are never serialized, however under rare cases some bits are serialized.
	POJapiEvent m_paoJapiEvent;				// Object to allow the event to interact with JavaScript

public:
	IEvent(const TIMESTAMP * ptsEventID = d_ts_pNULL_AssignToNow);
	virtual ~IEvent();
	void Event_InitFromDataOfEvent(const IEvent * pEventSource);

	void EventAddToVault(PA_PARENT CVaultEvents * pVaultParent);
	void EventAddToVault(PA_PARENT TContact * pContactParent);
	TAccountXmpp * PGetAccount() const;
	TProfile * PGetProfile() const;

	virtual EEventClass EGetEventClass() const  = 0;
	virtual EEventClass EGetEventClassForXCP() const;
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual EGui XospDataE(const CXmlNode * pXmlNodeData, INOUT CBinXcpStanza * pbinXospReply);
	void XcpRequesExtraData();

	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
	virtual void HyperlinkGetTooltipText(PSZUC pszActionOfHyperlink, IOUT CStr * pstrTooltipText);
	virtual void HyperlinkClicked(PSZUC pszActionOfHyperlink, INOUT OCursor * poCursorTextBlock);
	virtual PSZUC PszGetTextOfEventForSystemTray(OUT_IGNORE CStr * pstrScratchBuffer) const;
	virtual void DetachFromObjectsAboutBeingDeleted();
	virtual POJapiEvent POJapiGet_NZ() CONST_MCC;

	BOOL Event_FIsEventBelongsToGroup() const;
	BOOL Event_FIsEventTypeSent() const;
	BOOL Event_FIsEventTypeReceived() const;
	EGui Event_ESetCompletedTimestamp();
	void Event_SetCompletedAndUpdateChatLog(QTextEdit * pwEditChatLog);
	void Event_SetCompletedAndUpdateWidgetWithinParentChatLog();
	void Event_UpdateWidgetWithinParentChatLog();
	BOOL Event_FHasCompleted() const;
	BOOL Event_FIsEventRecentThanMinutes(int cMinutes) const;
	inline void Event_SetFlagOutOfSync() { m_uFlagsEvent |= FE_kfEventOutOfSync; }
	inline void Event_SetFlagErrorProtocol() { m_uFlagsEvent |= FE_kfEventProtocolError; }

	QTextBlock ChatLog_GetTextBlockRelatedToDocument(QTextDocument * poDocument) const;
	QTextBlock ChatLog_GetTextBlockRelatedToWidget(QTextEdit * pwEditChatLog) const;
	void ChatLog_UpdateEventWithinWidget(QTextEdit * pwEditChatLog);
	void ChatLog_UpdateEventWithinSelectedChatLogFromNavigationTree();
	const QBrush & ChatLog_OGetBrushForEvent() const;
	PSZUC ChatLog_PszGetNickNameOfContact() const;

	ITreeItemChatLogEvents * PGetContactOrGroup_NZ() const;
	TContact * PGetContactForReply_YZ() const;
	TAccountXmpp * PGetAccount_NZ() const;
	CSocketXmpp * PGetSocket_YZ() const;
	CSocketXmpp * PGetSocketOnlyIfReady() const;
	void Socket_WriteXmlFormatted(PSZAC pszFmtTemplate, ...);
	void Socket_WriteXmlIqSet_VE_Gso(TContact * pContact, PSZAC pszFmtTemplate, ...);
	void Socket_WriteXmlIqError_VE_Gso(PSZAC pszErrorType, PSZUC pszErrorID, PSZAC pszFmtTemplate, ...);
	void Socket_WriteXmlIqReplyAcknowledge();

	const TIMESTAMP * PtsGetTimestampForChronology() const;

protected:
	void _BinHtmlInitWithTime(OUT CBin * pbinTextHtml) const;
	void _BinHtmlAppendHyperlinkToLocalFile(INOUT CBin * pbinTextHtml, PSZUC pszFilename, BOOL fDisabled = FALSE) const;
	void _BinHtmlAppendHyperlinkAction(INOUT CBin * pbinTextHtml, CHS chActionOfHyperlink) const;
	void _BinHtmlAppendHyperlinkAction(INOUT CBin * pbinTextHtml, CHS chActionOfHyperlink, PSZAC pszButtonName) const;
	void _XmlUnserializeAttributeOfContactIdentifier(CHS chAttributeName, OUT TContact ** ppContact, const CXmlNode * pXmlNodeElement) const;

public:
	static EEventClass S_EGetEventClassFromXmlStanzaXCP(IN const CXmlNode * pXmlNodeEventsStanza, INOUT TContact * pContact, INOUT ITreeItemChatLogEvents * pChatLogEvents, INOUT CBinXcpStanza * pbinXmlStanzaReply);
	static IEvent * S_PaAllocateEvent_YZ(EEventClass eEventClass, const TIMESTAMP * ptsEventID);
	static IEvent * S_PaAllocateEvent_YZ(const CXmlNode * pXmlNodeEvent, const TIMESTAMP * ptsEventID);
	static NCompareResult S_NCompareSortEventsByIDs(IEvent * pEventA, IEvent * pEventB, LPARAM lParamCompareSort = d_zNA);
	static NCompareResult S_NCompareSortEventsByChronology(IEvent * pEventA, IEvent * pEventB, LPARAM lParamCompareSort = d_zNA);
}; // IEvent

///////////////////////////////////////////////////////////////////////////////////////////////////
enum EWalletViewFlags	// Various flags to determine what data to display to the user
	{
	eWalletViewFlag_kfDisplayTransactionsSent		= 0x0001,
	eWalletViewFlag_kfDisplayTransactionsReceived	= 0x0002,
	eWalletViewFlag_kmDisplayTransactionsAll		= eWalletViewFlag_kfDisplayTransactionsSent | eWalletViewFlag_kfDisplayTransactionsReceived
	};

class CArrayPtrEvents : public CArray
{
protected:

public:
	inline void EventAdd(IEvent * pEvent) { Add(pEvent); }
	BOOL Event_FoosAddSorted(IEvent * pEventNew);
	inline IEvent ** PrgpGetEventsStop    (OUT IEvent *** pppEventStop)                       const { return (IEvent **)PrgpvGetElementsStop    (OUT (void ***)pppEventStop);                   }
	inline IEvent ** PrgpGetEventsStopLast(OUT IEvent *** pppEventStop, int cElementsLastMax) const { return (IEvent **)PrgpvGetElementsStopLast(OUT (void ***)pppEventStop, cElementsLastMax); }
	inline IEvent * PGetEventLast_YZ() const { return (IEvent *)PvGetElementLast_YZ(); }
	IEvent * PFindEventLastSent() const;
	TIMESTAMP TsEventIdLastEventSent() const;
	TIMESTAMP TsEventOtherLastEventReceived() const;
	TIMESTAMP TsEventOtherLast() const;
	TIMESTAMP TsEventIdLast() const;

	void EventsSerializeForMemory(INOUT CBinXcpStanza * pbinXmlEvents) const;
	void EventsSerializeForDisk(INOUT CBinXcpStanza * pbinXmlEvents) const;
	void EventsUnserializeFromDisk(const CXmlNode * pXmlNodeEvent, ITreeItemChatLogEvents * pParent);
	CEventMessageTextReceived * PFindEventMessageReceivedByTimestamp(TIMESTAMP tsOther) const;
	IEvent * PFindEventReplacedBy(int iEventReplacing) const;
	IEvent * PFindEventByID(TIMESTAMP tsEventID) const;
	void SortEventsByIDs();
	BOOL FEventsSortedByIDs() const;
	void AppendEventsSortedByIDs(IN_MOD_SORT CArrayPtrEvents * parraypEventsUnsorted);
	void SortEventsByChronology();
	void GroupEventsBySender();

	IEvent * PAllocateEvent_YZ(const CXmlNode * pXmlNodeEvent, TIMESTAMP tsEventID, TIMESTAMP tsOther);
	void DeleteAllEvents();
	void DeleteAllEventsReceivedHavingDuplicateTsOther();
	void ForEach_DetachFromObjectsAboutBeingDeleted() const;

	void Wallets_AppendEventsTransactionsFor(ITreeItem * pFilterBy, EWalletViewFlags eWalletViewFlags);
}; // CArrayPtrEvents

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Data to attach to a QTextBlock.
//	Unfortunately Qt does not allow to just attach a pointer to a text block, consequently a QTextBlockUserData must be allocated on the heap.
//	Using setUserData(NULL) does not work either because the method setUserData() will delete the existing pointer before setting the user data to NULL.
//	As a result, a redundant memory allocation of 8 bytes must be done for every text block.  What a waste of CPU time and memory!
class OTextBlockUserDataEvent : public QTextBlockUserData
{
public:
	IEvent * m_pEvent;
public:
	inline OTextBlockUserDataEvent(IEvent * pEvent) { m_pEvent = pEvent; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Event containing a text message
class IEventMessageText : public IEvent
{
public:
	CStr m_strMessageText;				// Message text (may contain plain text or HTML content)
	enum
		{
		FM_kzMessagePlainText	= 0x0000,
		FM_kfMessageHtml		= 0x0001,	// The content of m_strMessage is in HTML
		FM_kfMessageUpdated		= 0x0002,	// The message was updated (edited) later
		// NYI
		FM_kfContentCode,
		FM_kfContentCodeCPP,
		FM_kfDrawBorder
		};
	UINT m_uFlagsMessage;		// Flags related to the message, such as HTML and formatting.
public:
	IEventMessageText(const TIMESTAMP * ptsEventID);
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	void _BinHtmlInitWithTimeAndMessage(OUT CBin * pbinTextHtml) CONST_VIRTUAL;
}; // IEventMessageText

class CEventMessageXmlRawSent : public IEventMessageText	// This class is mostly for debugging by sending raw XML data directly to the socket
{
public:
	CEventMessageXmlRawSent(PSZUC pszXmlStanza);
	virtual EEventClass EGetEventClass() const { return eEventClass_eMessageXmlRaw_class; }
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
};

class CEventMessageTextSent : public IEventMessageText
{
public:
	static const EEventClass c_eEventClass = eEventClass_eMessageTextSent;
public:
	CEventMessageTextSent(const TIMESTAMP * ptsEventID);
	CEventMessageTextSent(PSZUC pszMessageText);
	virtual ~CEventMessageTextSent();
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EEventClass EGetEventClassForXCP() const;
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
	void EventUpdateMessageText(const CStr & strMessageUpdated, INOUT WLayoutChatLog * pwLayoutChatLogUpdate);
	CEventMessageTextSent * PFindEventMostRecent_NZ() CONST_OBJECT;
};

class CEventMessageTextReceived : public IEventMessageText
{
public:
	static const EEventClass c_eEventClass = eEventClass_eMessageTextReceived_class;
public:
	CEventMessageTextReceived(const TIMESTAMP * ptsEventID);
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EEventClass EGetEventClassForXCP() const { return CEventMessageTextSent::c_eEventClass; }
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
	virtual PSZUC PszGetTextOfEventForSystemTray(OUT_IGNORE CStr * pstrScratchBuffer) const;
	void MessageUpdated(PSZUC pszMessageUpdated, INOUT WChatLog * pwChatLog);

};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	The following #define must be negative (however their values have been arbitrary chosen)
#define d_IEventFile_cblFileSize_FileNotFound						(-1)	// The file could not be found (this is for a file offer where the offered file has been moved and/or deleted)
#define d_IEventFile_cblDataTransferred_WriteError					(-1)	// Unable to save the file disk
#define d_IEventFile_cblDataTransferred_CancelledByLocalUser		(-2)
#define d_IEventFile_cblDataTransferred_CancelledByRemoteContact	(-3)

class IEventFile : public IEvent
{
public:
	CStr m_strFileName;
	L64 m_cblFileSize;			// Size of the file (in bytes)
	L64 m_cblDataTransferred;	// Number of bytes transferred.  This value is useful to show progress to the user, and perhaps in the future, a resume transfer operation.  Of course, for a file sent to a group, this value is the largest amount of data transferred to a single recipient.
protected:
	CFile * m_paFile;			// Pointer to the file object to read (or write) the data.  If this pointer is non-NULL, it means the event is transmitting data.

public:
	IEventFile(const TIMESTAMP * ptsEventID);
	virtual ~IEventFile();
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual void HyperlinkGetTooltipText(PSZUC pszActionOfHyperlink, IOUT CStr * pstrTooltipText);
	virtual void HyperlinkClicked(PSZUC pszActionOfHyperlink, INOUT OCursor * poCursorTextBlock);
	void _BinAppendHtmlForEvent(INOUT CBin * pbinTextHtml, PSZAC pszTextHtmlTemplate) const;
	void _FileTransferCancelledByLocalUser(OCursor * poCursorTextBlock);
	CFile * _PFileOpenReadOnly_NZ();
	CFile * _PFileOpenWriteOnly_NZ();
	void _FileClose();
}; // IEventFile

//	The user is offering a file to the contact(s)
class CEventFileSent : public IEventFile
{
public:
	static const EEventClass c_eEventClass = eEventClass_eFileSent;
public:
	CEventFileSent(const TIMESTAMP * ptsEventID) : IEventFile(ptsEventID) { }
	CEventFileSent(PSZUC pszFileToSend);
	virtual ~CEventFileSent();
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EEventClass EGetEventClassForXCP() const;
	virtual EGui XospDataE(const CXmlNode * pXmlNodeData, INOUT CBinXcpStanza * pbinXospReply);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;

	void XmppProcessStanzaFromContact(const CXmlNode * pXmlNodeStanza, TContact * pContact);
	void XmppProcessStanzaVerb(const CXmlNode * pXmlNodeStanza, PSZAC pszaVerbContext, const CXmlNode * pXmlNodeVerb);

	static const int c_cbBufferSizeMaxXmppBase64 = 4096;	// 4 KiB us the default block size to transfer files via XMPP when encoding in Base64
}; // CEventFileSent

//	Very similar as CEventFileSent however capable to send a file directly to a JID
//	At the moment this event is used for debugging by sending information to the Cambrian developers
class CEventFileSentTo : public CEventFileSent
{
public:
	CStr m_strJidTo;		// Which JID to send the file
public:
	CEventFileSentTo(const TIMESTAMP * ptsEventID) : CEventFileSent(ptsEventID) { }
	CEventFileSentTo(PSZUC pszFileToSend, PSZAC pszJidTo);
	virtual EEventClass EGetEventClass() const { return eEventClass_eFileSentTo; }
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
};

//	The user received a file offer from the contact
class CEventFileReceived : public IEventFile
{
public:
	static const EEventClass c_eEventClass = eEventClass_eFileReceived_class;
public:
	CEventFileReceived(const TIMESTAMP * ptsEventID);
	virtual ~CEventFileReceived();
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EEventClass EGetEventClassForXCP() const { return CEventFileSent::c_eEventClass; }
	virtual EGui XospDataE(const CXmlNode * pXmlNodeData, INOUT CBinXcpStanza * pbinXospReply);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
	virtual void HyperlinkClicked(PSZUC pszActionOfHyperlink, INOUT OCursor * poCursorTextBlock);
	virtual PSZUC PszGetTextOfEventForSystemTray(OUT_IGNORE CStr * pstrScratchBuffer) const;
};


/*
//	The file was received (offered) from an XMPP contact.  Since non-Cambrian clients do not have access to the "Extra Data", we need to remember the Stanza and Stream Identifier.
class CEventFileReceivedXmpp : public IEventFile
{
public:
	CStr m_strxStanzaID;	// The Stanza Identifier is important to reply to the
	CStr m_strxSID;			// The Stream Identifier represents the ID of the download.  The class CEventFileSent does not need this field, as it uses m_tsEventID as the Stream Identifier.

public:
	CEventFileReceived(ITreeItemChatLogEvents * pContactSendingFile, const TIMESTAMP * ptsEventID);
	CEventFileReceived(TContact * pContactSendingFile, const CXmlNode * pXmlNodeStreamInitiation);
	virtual ~CEventFileReceived();
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
	virtual void HyperlinkClicked(PSZUC pszActionOfHyperlink, INOUT OCursor * poCursorTextBlock);
	virtual BOOL SystemTray_FDisplayEvent() const;

	friend class CTaskFileDownload;
	friend class CArrayPtrTasksDownloading;
	static const EEventClass c_eEventClass = eEventClass_eFileReceived_class;
}; // CEventFileReceivedXmpp
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
class IEventWalletTransaction : public IEvent
{
public:
	AMOUNT m_amtQuantity;			// Quantity of Satoshis involved in the transaction
	CStr m_strValue;
	CStr m_strComment;				// Transaction description/note

public:
	IEventWalletTransaction(const TIMESTAMP * ptsEventID);
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
	BOOL FuIsTransactionMatchingViewFlags(EWalletViewFlags eWalletViewFlags) const;
};

class CArrayPtrEventsWalletTransactions : public CArrayPtrEvents
{
public:
	inline IEventWalletTransaction ** PrgpGetTransactionsStop(OUT IEventWalletTransaction *** pppTransactionStop) const { return (IEventWalletTransaction **)PrgpvGetElementsStop(OUT (void ***)pppTransactionStop); }
};

class CEventWalletTransactionSent : public IEventWalletTransaction
{
public:
	static const EEventClass c_eEventClass = eEventClass_eWalletTransactionSent;
public:
	CEventWalletTransactionSent(const TIMESTAMP * ptsEventID = d_ts_pNULL_AssignToNow) : IEventWalletTransaction(ptsEventID) { }
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EEventClass EGetEventClassForXCP() const;
};

class CEventWalletTransactionReceived : public IEventWalletTransaction
{
public:
	static const EEventClass c_eEventClass = eEventClass_eWalletTransactionReceived_class;
public:
	CEventWalletTransactionReceived(const TIMESTAMP * ptsEventID) : IEventWalletTransaction(ptsEventID) { }
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EEventClass EGetEventClassForXCP() const { return CEventWalletTransactionSent::c_eEventClass; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Ping the contact and wait for the response
class CEventPing : public IEvent
{
public:
	static const EEventClass c_eEventClass = eEventClass_ePing_class;
public:
	TIMESTAMP m_tsContact;	// Date & time (in UTC) of the device (computer) of the contact
	CStr m_strError;		// Error response from the server (if any)
public:
	CEventPing();
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual EGui XospDataE(const CXmlNode * pXmlNodeData, INOUT CBinXcpStanza * pbinXospReply);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
}; // CEventPing

//	Query the the version of the client and wait for the response
class CEventVersion : public IEvent
{
public:
	static const EEventClass c_eEventClass = eEventClass_eVersion_class;
	CStr m_strClient;
	CStr m_strVersion;
	CStr m_strOperatingSystem;
public:
	CEventVersion();
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual EGui XospDataE(const CXmlNode * pXmlNodeData, INOUT CBinXcpStanza * pbinXospReply);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;

	void XmppProcessStanzaFromContact(const CXmlNode * pXmlNodeStanza);
};

//	Display help in the Chat Log
class CEventHelp : public IEvent
{
public:
	CStr m_strHtmlHelp;

public:
	CEventHelp(PSZUC pszHtmlHelp);
	virtual EEventClass EGetEventClass() const { return eEventClass_eHelp_class; }
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
};


//	Interface to indicate an event was replaced by another event.
//	A typical use is a message edited by the user.
class IEventUpdater : public IEvent
{
public:
	TIMESTAMP m_tsEventIdOld;	// ID of the old message to update
	TIMESTAMP m_tsEventIdNew;	// ID of the new message containing the text data

public:
	IEventUpdater(const TIMESTAMP * ptsEventID);
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
};

//	Event indicating a message was replaced by another one.
//	This event is used when a message is edited
class CEventUpdaterSent : public IEventUpdater
{
public:
	static const EEventClass c_eEventClass = eEventClass_eUpdaterSent;
	CEventUpdaterSent(const TIMESTAMP * ptsEventID) : IEventUpdater(ptsEventID) { }
	CEventUpdaterSent(const IEvent * pEventOld);
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EEventClass EGetEventClassForXCP() const { return eEventClass_eUpdaterReceived_class; }
};

class CEventUpdaterReceived : public IEventUpdater
{
public:
	static const EEventClass c_eEventClass = eEventClass_eUpdaterReceived_class;
	CEventUpdaterReceived(const TIMESTAMP * ptsEventID) : IEventUpdater(ptsEventID) { }
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EEventClass EGetEventClassForXCP() const { return CEventUpdaterSent::c_eEventClass; }
};


class CArrayPtrEventsRecent : private CArrayPtrEvents
{
	TProfile *m_pProfile;
	int m_cEventsMax;
	void EventsUnserialize();
	void EventsUnserialize(const CXmlNode * pXmlNodeEvents);

public:
	CBin m_binXmlEvents;
	CArrayPtrEventsRecent(TProfile *pProfile, int cEventsMax);

	void AddEvent(IEvent *pEvent);
	inline IEvent ** PrgpGetEventsStopLast(OUT IEvent *** pppEventStop) CONST_MCC { EventsUnserialize(); return (IEvent **)PrgpvGetElementsStopLast(OUT (void ***)pppEventStop, m_cEventsMax); }
	int GetSize();
	inline void Flush() { RemoveAllElements(); }
	inline void RemoveEvent(IEvent * pEvent) { RemoveElementAssertI(pEvent); }


};


#ifdef DEBUG_WANT_ASSERT
	void AssertValidEvent(const IEvent * pEvent);
#else
	#define AssertValidEvent(pEvent)
#endif

#endif // IEVENT_H

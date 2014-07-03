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
#define d_szEventDebug_strContactSource					"_CONTACTSOURCE"	// Add an extra field to the base class IEvent to include the contact who transmitted the event.  This field is useful to debug group chat.
#define d_szEventDebug_strVersion						"_VERSION"			// Add the Cambrian version when the event was created (sent or received).  Again, this is useful to save time by avoiding debugging old events received before a given bugfix.

#define d_chEvent_Attribute_tsEventID					'i'	// Identifier of the event
#define d_szEvent_Attribute_tsEventID_t					" i='$t'"
#define d_chEvent_Attribute_tsOther						'o'	// Other timestamp for the event
#define d_szEvent_Attribute_tsOther_t					" o='$t'"

//	Shorter for the timestamps
#define _tsI		d_szEvent_Attribute_tsEventID_t
#define _tsO		d_szEvent_Attribute_tsOther_t

//	XCP is an abbreviation for "eXtensible Cambrian Protocol".
//	In a nutshell, this protocol is a layer taking care of end-to-end encryption between clients, as well as automatically
//	splitting large pieces of data into smaller xmpp-stanzas when the data does not fit in the recommended xmpp-stanza size of 4 KiB.
#define d_chXCP_					'_'
#define d_szXCP_					"_"

//	Essentially, there are 3 reserved attributes by the Cambrian Protocol ('i', 'o' and 'g')
//	The following attributes 'make sense' for the receiver of the XCP data.
#define d_chXCPa_tsEventID				d_chEvent_Attribute_tsEventID
#define d_szXCPa_tsEventID_t			d_szEvent_Attribute_tsEventID_t
#define d_chXCPa_tsOther				d_chEvent_Attribute_tsOther
#define d_szXCPa_tsOther_t				d_szEvent_Attribute_tsOther_t
#define d_chXCPa_pContactGroupSender	'g'		// Pointer of the contact who sent the event/message to the group.  Of course, this attribute is present only when receiving a group message


//	The enumeration EEventClass is used to serialize and unserialize events.
//	This enumeration is somewhat as RTI_ENUM, however since the events are serialized, letters of the alphabet are used to identify different classes of events.
//	This way, this enum may be serialized directly into an XML file.
//	Also, all events sent begin with an uppercase, and all events received begin with a lowercase.  This way, looking at the event class (or the first character in the XML element)
//	gives us useful information about what type of event we are dealing.
enum EEventClass
	{
	eEventClass_kzSerializeDataAsXmlAttributes	= 0,			// This is the default as all the data of the event are serialized as XML attributes
	eEventClass_kfSerializeDataAsXmlElement		= _USZUF(0x01),	// NYI: The event has a lot of data and must be serialized as an XML element containing elements and attributes.  This flag is useful during the serialization to determine if there is a need for a closing XML tag.

	eEventClass_kfNeverSerializeToDisk			= _USZUF(0x02),	// This event is never serialized to disk (example: pinging a remote client, or sending raw XML to the socket)
	eEventClass_kfNeverSerializeToXCP			= _USZUF(0x04),	// This event is never serialized (sent) via the XCP protocol (example: downloading a large stanza from a contact)
	eEventClass_mfNeverSerialize				= eEventClass_kfNeverSerializeToDisk | eEventClass_kfNeverSerializeToXCP,

	eEventClass_kfReceivedByRemoteClient		= _USZUF(0x08),	// This event was received by the remote client.  This bit is useful to quickly determine what was sent, and what was received, as the GUI often displays different colors.  Roughly half of the eEventClass_* will have this bit set.

	eEventClass_eXCP							= d_chXCP_,		// The underscore is reserved for the XCP protocol, however share the same namespace as the other events
	_g_											= 'g',			// ALL EVENTS received for a group must begin with this letter.  This way, it is possible to quickly determine of an event is for a group

	eEventClass_eMessageTextComposing			= _USZU1('Y'),	// The user is tYping something...  (this is a notification to the remote client, as there is no instance of this 'event' nor it is saved to disk)
		#define d_chXCPa_MessageTextComposing_State				's'	// Attribute to indicate the state of the 'composing' event.  If this attribute is not present, it means the user is just typing.
		#define d_chXCPv_MessageTextComposing_State_Paused		'p'	// The user paused, or stopped typing for 10 seconds.
		#define d_chXCPv_MessageTextComposing_State_Deleting	'd'	// NYI: The user is currently deleting what he typed.  This indicates the user is a bit hesitant about what to reply.
		#define d_szXCPe_MessageTextComposingStarted			"<Y/>"
		#define d_szXCPe_MessageTextComposingPaused				"<Y s='p'/>"

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

	eEventClass_eDownloader							= _USZU2('d', 'l'),
	eEventClass_eDownloader_class					= _USZU2('d', 'l') | eEventClass_kfNeverSerializeToXCP | eEventClass_kfReceivedByRemoteClient,	// The downloader is saved to disk and its class never serialized for XCP.
		#define d_szXCPe_CEventDownloader_i_tsO		"dl s='$i'" _tsO		// At the moment, the downloader class will only send 32 bit of data, hence $i
		#define d_chXCPa_CEventDownloader_cblDataToDownload		's'	// Size of the data to download
		#define d_chXCPa_CEventDownloader_bin85DataReceived		'd'	// Data was received so far.  This value is used when saving the downloader to disk to resume the download later.
		#define d_chXCPa_CEventDownloader_tsForwarded			'f'	// Timestamt for a forwarded event
		#define d_szXCPa_CEventDownloader_tsForwarded_t			" f='$t'"

	eEventClass_eWalletTransactionSent				= _USZU1('W'),
	eEventClass_eWalletTransactionReceived			= _USZU1('w'),
	eEventClass_eWalletTransactionReceived_class	= eEventClass_eWalletTransactionReceived | eEventClass_kfReceivedByRemoteClient,

	eEventClass_eBallotSent							= _USZU3('B', 'A', 'L'),
	eEventClass_eBallotSent_class					= eEventClass_eBallotSent | eEventClass_kfSerializeDataAsXmlElement,
	eEventClass_eBallotReceived						= _USZU3('b', 'a', 'l'),
	eEventClass_eBallotReceived_class				= eEventClass_eBallotReceived | eEventClass_kfReceivedByRemoteClient | eEventClass_kfSerializeDataAsXmlElement,

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


///////////////////////////////////////////////////////////////////////////////////////////////////
enum EStanzaType
	{
	eStanzaType_zInformation,		// The stanza is sent directly to the remote client, or it is ignored if the remote client is unavailable
	eStanzaType_eMessage,			// The stanza is cached on the XMPP server if the remote client is unavailable to receive it
	eStanzaType_eBroadcast			// The stanza is broadcasted by the server to every contact on the roster
	};

//	Object to store information necessary to send an XCP stanza.
//	An XCP Stanza is an XML within a XMPP message.
class CBinXcpStanzaType : public CBin	// TODO: Rename to CBinXcpStanza
{
public:
	EStanzaType m_eStanzaType;	// How to send the XCP stanza.
	TContact * m_pContact;		// Contact to send the XCP stanza
	int m_ibXmlApiReply;		// Offset where the XML of the API reply is (this field is useful to append errors to the object)
protected:
	CBinXcpStanzaType(EStanzaType eStanzaType);
public:
	inline BOOL FSerializingEventToDisk() const { return (m_pContact == NULL); }
	void BinXmlInitStanzaWithGroupSelector(TGroup * pGroup);
	void BinXmlInitStanzaWithXmlRaw(PSZUC pszMessageXml);
	void BinXmlAppendTimestampsToSynchronizeWithContact(TContact * pContact);
	void BinXmlAppendTimestampsToSynchronizeWithGroupMember(TGroupMember * pMember);
	void BinXmlAppendAttributeOfContactIdentifierOfGroupSenderForEvent(const IEvent * pEvent);

	BOOL XcpApi_FIsXmlElementOpened() const { return (m_ibXmlApiReply > 0); }
	BOOL XcpApi_FIsXmlElementClosedBecauseOfError() const { return (m_ibXmlApiReply == 0); }
	void BinXmlAppendXcpElementForApiRequest_ElementOpen(PSZUC pszApiName);
	void BinXmlAppendXcpElementForApiRequest_AppendApiParameterData(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters);
	void BinXmlAppendXcpElementForApiRequest_ElementClose();
	void BinXmlAppendXcpAttributesForApiRequestError(EErrorXcpApi eErrorXcpApi, PSZUC pszxErrorData);

	void BinXmlAppendXcpElementForApiReply(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters);

	void BinXmlAppendXcpApiRequest(PSZAC pszApiName, PSZUC pszXmlApiParameters);
	void BinXmlAppendXcpApiRequest_ProfileGet(PSZUC pszGroupIdentifier);

	void BinXmlSerializeEventForDisk(const IEvent * pEvent);
	void BinXmlSerializeEventForXcp(const IEvent * pEvent);
	void BinXmlSerializeEventForXcpCore(const IEvent * pEvent, TIMESTAMP tsOther);
	void XcpSendStanzaToContact(TContact * pContact) CONST_MCC;
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
}; // CBinXcpStanzaType


class CBinXcpStanzaTypeInfo : public CBinXcpStanzaType
{
public:
	CBinXcpStanzaTypeInfo();
	CBinXcpStanzaTypeInfo(IEvent * pEvent);
};

class CBinXcpStanzaTypeMessage : public CBinXcpStanzaType
{
public:
	CBinXcpStanzaTypeMessage() : CBinXcpStanzaType(eStanzaType_eMessage) { }
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
//	The motivation for this interface is providing a mechanism for interacting with every entry in the Chat Log.
//	In a nutshell, every line (text block) in the Chat Log has a corresponding event, and the same event may be shared by multiple Chat Log, or a wallet.
//	For instance, a Chat Log may display all events related to a contact, while another Chat Log may display only the files downloaded from the same contact.
//
//	Each event has a unique identifier (m_tsEventID) which is useful to detect duplicates, or update the state of an event, such as updating the progress of a download, or correcting a typo in a previously sent message.
//	As a result, the same event may be serialized multiple times, and unserialized multiples times, thus updating the state of the event each time.
//	Likewise, each event has another timestamp to determine when the event completed.  By default this value is initialized to zero, as many events do not have a completion time.
//
//	Every event has a parent owner, however may be in the possession by another object.  A good example is the wallet holding the transaction events for a contact.
//	Finally, every event has a pointer to a task.
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
		FE_kfEventOutOfSync			= 0x0001,	// The event is out of sync, and therefore display a little icon to show to the user
		FE_kfEventDeliveryConfirmed	= 0x0002,	// The event was delivered and its checkmark was displayed.  This flag should be eventually removed, however it is a workaround a Qt bug in the QTextEdit
		FE_kfEventProtocolWarning	= 0x0004,	// Therew was a minor error while transmitting the event.  This bit is set to give a second chance to retry, however to prevent an infinite loop to retry over and over
		FE_kfEventProtocolError		= 0x0008	// There was a protocol error while sending the event (this means one of the client is out-of-date and is unable to allocate the event because it is unknown)
		};
	mutable UINT m_uFlagsEvent;				// Flags related to the event (not serialized)
	#ifdef d_szEventDebug_strContactSource
	CStr m_strDebugContactSource;				// Contact JID who transmitted the event
	#endif
	#ifdef d_szEventDebug_strVersion
	CStr m_strDebugVersion;
	#endif

public:
	IEvent(const TIMESTAMP * ptsEventID = NULL);
	virtual ~IEvent();
	void EventAddToVault(PA_PARENT CVaultEvents * pVaultParent);
	void EventAddToVault(PA_PARENT TContact * pContactParent);

	virtual EEventClass EGetEventClass() const  = 0;
	virtual EEventClass EGetEventClassForXCP() const;
	virtual void XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual void XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply);
	virtual void XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply);
	void XcpRequesExtraData();

	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
	virtual void HyperlinkGetTooltipText(PSZUC pszActionOfHyperlink, IOUT CStr * pstrTooltipText);
	virtual void HyperlinkClicked(PSZUC pszActionOfHyperlink, INOUT OCursor * poCursorTextBlock);
	virtual PSZUC PszGetTextOfEventForSystemTray(OUT_IGNORE CStr * pstrScratchBuffer) const;

	BOOL Event_FIsEventBelongsToGroup() const;
	BOOL Event_FIsEventTypeSent() const;
	BOOL Event_FIsEventTypeReceived() const;
	void Event_SetCompletedTimestamp();
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

	void Event_WriteToSocket();
	void Event_WriteToSocketIfReady();

protected:
	void _BinHtmlInitWithTime(OUT CBin * pbinTextHtml) const;
	void _BinHtmlAppendHyperlinkToLocalFile(INOUT CBin * pbinTextHtml, PSZUC pszFilename, BOOL fDisabled = FALSE) const;
	void _BinHtmlAppendHyperlinkAction(INOUT CBin * pbinTextHtml, CHS chActionOfHyperlink) const;
	void _BinHtmlAppendHyperlinkAction(INOUT CBin * pbinTextHtml, CHS chActionOfHyperlink, PSZAC pszButtonName) const;
	void _XmlUnserializeAttributeOfContactIdentifier(CHS chAttributeName, OUT TContact ** ppContact, const CXmlNode * pXmlNodeElement) const;

public:
	static EEventClass S_EGetEventClassFromXmlStanzaXCP(IN const CXmlNode * pXmlNodeEventsStanza, INOUT TContact * pContact, INOUT ITreeItemChatLogEvents * pChatLogEvents, INOUT CBinXcpStanzaType * pbinXmlStanzaReply);
	static IEvent * S_PaAllocateEvent_YZ(EEventClass eEventClass, const TIMESTAMP * ptsEventID);
	static int S_NCompareSortEventsByIDs(IEvent * pEventA, IEvent * pEventB, LPARAM lParamCompareSort = d_zNA);
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
	inline IEvent ** PrgpGetEventsStop(OUT IEvent *** pppEventStop) const { return (IEvent **)PrgpvGetElementsStop(OUT (void ***)pppEventStop); }
	inline IEvent * PGetEventLast_YZ() const { return (IEvent *)PvGetElementLast_YZ(); }
	IEvent * PFindEventLastSent() const;
	IEvent * PFindEventNextForXcp(TIMESTAMP tsEventID, OUT int * pcEventsRemaining) const;
	TIMESTAMP TsEventIdLastEventSent() const;
	TIMESTAMP TsEventOtherLastEventReceived() const;
	TIMESTAMP TsEventOtherLast() const;
	TIMESTAMP TsEventIdLast() const;

	void EventsSerializeForDisk(INOUT CBinXcpStanzaType * pbinXmlEvents) const;
	void EventsUnserializeFromDisk(const CXmlNode * pXmlNodeEvent, ITreeItemChatLogEvents * pParent);
	CEventMessageTextReceived * PFindEventMessageReceivedByTimestamp(TIMESTAMP tsOther) const;
	IEvent * PFindEventByID(TIMESTAMP tsEventID) const;
	void SortEventsByIDs();
	BOOL FEventsSortedByIDs() const;
	void DeleteAllEvents();
	void DeleteAllEventsReceivedHavingDuplicateTsOther();

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
	virtual void XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	void _BinHtmlInitWithTimeAndMessage(OUT CBin * pbinTextHtml) CONST_VIRTUAL;
}; // IEventMessageText

class CEventMessageXmlRawSent : public IEventMessageText	// This class is mostly for debugging by sending raw XML data directly to the socket
{
public:
	CEventMessageXmlRawSent(PSZUC pszXmlStanza);
	virtual EEventClass EGetEventClass() const { return eEventClass_eMessageXmlRaw_class; }
	virtual void XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const;
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
	void MessageResendUpdate(const CStr & strMessageUpdated, INOUT WLayoutChatLog * pwLayoutChatLogUpdate);
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
	virtual void XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const;
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
	virtual void XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply);
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
	virtual void XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply);
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
	virtual void XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const;
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
	virtual void XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const;
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
	CEventWalletTransactionSent(const TIMESTAMP * ptsEventID = NULL) : IEventWalletTransaction(ptsEventID) { }
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
	virtual void XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const;
	virtual void XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, CBinXcpStanzaType * pbinXcpStanzaReply);
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
	virtual void XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const;
	virtual void XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, CBinXcpStanzaType * pbinXcpStanzaReply);
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

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Class to download a large event.
//	This class is a hybrid between an event and the core of the XCP protocol.
//	The CEventDownloader is never allocated by the sender, however it is always unserialized and allocated by the receiver.
//	Until the large event has been fully downloaded, the downloader will have its own class, and later will emulate the behavior of the downloaded event.
class CEventDownloader : public IEvent
{
protected:
	TIMESTAMP m_tsForwarded;				// Forwarded timestamp (this happens when a group event is forwarded by another group member)
	enum { c_cbDataToDownload_Error = -1 }; // Use variable m_cbDataToDownload to store an arror code
	int m_cbDataToDownload;			// How much data needs to be downloaded (this variable is good for a progress bar).  Since the 'event downloader' is not to transfer files, but large text messages, a 32-bit integer is sufficient for storing such a message.
	CBin m_binDataDownloaded;		// Data downloaded (so far)
	IEvent * m_paEvent;				// Allocated event (when the download is complete)
public:
	CEventDownloader(const TIMESTAMP * ptsEventID);
	virtual ~CEventDownloader();
	virtual EEventClass EGetEventClass() const;
	virtual EEventClass EGetEventClassForXCP() const;
	virtual void XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual void XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply);
	virtual void XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, CBinXcpStanzaType * pbinXcpStanzaReply);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
	virtual void HyperlinkGetTooltipText(PSZUC pszActionOfHyperlink, IOUT CStr * pstrTooltipText);
	virtual void HyperlinkClicked(PSZUC pszActionOfHyperlink, INOUT OCursor * poCursorTextBlock);

	void XcpDownloadedDataArrived(const CXmlNode * pXmlNodeData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply, QTextEdit * pwEditChatLog);
	inline BOOL FIsDownloaderMatchingEvent(const IEvent * pEvent) const { return (pEvent == m_paEvent); }

	static const EEventClass c_eEventClass = eEventClass_eDownloader_class;
	friend class CBinXcpStanzaType;
}; // CEventDownloader

class CDataXmlLargeEvent;	// Object holding the data to send to CEventDownloader
class CListaDataXmlLargeEvents : public CList
{
public:
	~CListaDataXmlLargeEvents();
	void DeleteIdleNodes();
};

#ifdef DEBUG_WANT_ASSERT
	void AssertValidEvent(const IEvent * pEvent);
#else
	#define AssertValidEvent(pEvent)
#endif

#endif // IEVENT_H

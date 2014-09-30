#ifndef XCPAPI_H
#define XCPAPI_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

/*
XOSP Architecture
<a n='nameApi' g='shaGroupID' _i='idEvent'> ... API parameters ... </a>
<i i='idEvent'>
<o o='idTask'>

Xop_	Xml element (opcode) for an XOSP operation
Xa_		Xml attribute of an XOSP opcode
Xv_		Xml value for an attribute
*/

//	There are 3 categories of XOSP opcodes: Messages, Tasks, and APIs.
//
//	Messages: Notify the user is typing a message and to notify about new and old messages.
//	Tasks: Any data too big to fit within a XMPP stanza (about 4 KB of effective payload) are queued into tasks.  Also, all APIs are queued into tasks to guarantee delivery and order of execution.  Tasks are the backbone of XOSP.
//	APIs: Any communication between the clients are done via APIs.  Communication is divided into 3 categories:
//		1) P2P are Personal APIs between two SocietyPro clients.  Examples: GetProfileInfo, GetRecommendations.
//		2) P2E are Personal APIs from a SocietyPro client with a response to an event.  Examples: GetVersion, Ping.
//		3) E2E are Personal APIs between two events.  Examples: BallotVoting, FileTransfer.  E2E may be queued as tasks (example BallotVoting), or sent directly (FileTransfer).
//		Note: An event is any object which may appear in the Chat Log, within a one-to-one conversation or a group conversation.
//
#define d_chXop_MessageTyping						't'		// The user is typing (composing text) in the Chat Log.  This opcode is somewhat special because it is not cached.  If the remote contact(s) are offline, then this opcode is not sent.
#define d_chXop_MessageNew							'm'		// There is a new message from the contact.
#define d_chXop_MessagesSynchronize					's'		// Synchronize the messages to make sure all of them have been sent and received.
#define d_chXop_ApiCall								'a'		// Make a remote API call/request, either as a P2P or P2E.  The difference between P2P and P2E depends on the response prefix which is the underscore character: _r, _i or _o
#define d_chXop_ApiResponse							'r'		// Response for P2P where the name of the API is returned within the attribute value of _r
#define d_chXop_ApiResponseToEventID				'i'		// Response for P2E or E2E for an event sent
#define d_chXop_ApiResponseToEventOther				'o'		// Response for P2E or E2E for an event received
#define d_chXop_ApiResponseError					'e'		// There was an error processing the API request.  Typical errors are: invalid API name or the destination event (_i or _o) cannot be found. This error may also include invalid parameters.
#define d_chXop_TaskUploading						'u'		// Upload/send task data to a contact.
#define d_chXop_TaskDownloading						'd'		// Download task data from a contact.
#define d_chXop_TaskExecuted						'x'		// The task successfully executed after completing its download.
#define d_chXop_TaskNotFound						'n'		// The requested task cannot be found, therefore abort the download process.

	#define d_chXa_GroupIdentifier_shaBase85			'g'		// The group identifier may present for XOSP messages (d_chXop_Message*) and APIs (d_chXop_Api*)
	#define d_szXa_GroupIdentifier_shaBase85			" g='{h|}'"	// This is how a group identifier is encoded.  To simplify the code, the string g='{h|}' is present in many #define
	#define d_chXa_GroupChannel_strName					'h'
	#define d_szXa_GroupChannel_strName					" h='^S'"


///////////////////////////////////////////////////////////////////////////////////////////////////
//	The following #define must match the #define d_chXop above
///////////////////////////////////////////////////////////////////////////////////////////////////
#define d_szXop_MessageTyping_xmlStarted				"<t/>"
#define d_szXop_MessageTyping_xmlStartedGroup_h			"<t g='{h|}'/>"
#define d_szXop_MessageTyping_xmPaused					"<t>p</t>"
#define d_szXop_MessageTyping_xmPausedGroup_h			"<t g='{h|}'>p</t>"

/////////////////////////////////////////////////
#define d_szXop_MessagesSynchronize						"s"
#define d_szXop_MessagesSynchronizeGroup_h				"s g='{h|}'"
#define d_szXop_MessagesSynchronizeGroup_A				"s^A"

/////////////////////////////////////////////////
//	Opcodes for XOSP Synchronization (XS)
#define d_chXSop_RequestIDsLargerThanTimestamp			'r'		// Return all event IDs which are larger than a timestamp
#define d_chXSop_EventIDs								'e'
#define d_chXSop_EventIDsMine							'm'
#define d_chXSop_EventIDsOthers							'o'
#define d_chXSop_FetchEvents							'E'
#define d_chXSop_FetchEventsMine						'M'
#define d_chXSop_FetchEventsOthers						'O'
#define d_chXSop_EventsData								'D'		// We are receiving event data from the contact
#define d_chXSop_EventsDataMine							'B'		// We are receiving back our own event (this happens when the Chat Log is lost due to a disk crash or when installing on a new computer)
#define d_chXSop_EventsDataOther						'T'		// We are receiving events written by a third party (this is happens only in group chat when the third party is offline)
#define d_chXSop_EventsConfirmation						'C'		// Include the IDs (timestamps) to confirm any event received
#define d_chXSop_EventsUnknown							'U'		// Include the IDs of events received, however unknown.  This is somwhat a list of errors.  This list is mutually exclusive with d_chXSop_EventsConfirmation.
#define d_chXSop_zNULL									'\0'	// Special value to skip a synchronization opcode

	// Generic attributes for XOSP Synchronization
	#define d_chXSa_tsEventID								'i'		// Attribute to get tsEventID
	#define d_chXSa_tsOther									'o'		// Attribute to get tsOther
	#define d_chXSa_tsm_									't'		// Attribute to store multiple timestamps
	#define d_szXSa_tsm_									" t='"
	#define d_chXSa_pContact								'c'
	#define d_chXSa_strNameContact							'n'
	#define d_szXSa_strNameContact							" n=''"	// Include an empty attribute to indicate we want to fetch the name and JID of the contact
	#define d_chXSa_strJidContact							'j'

#define d_szXSop_RequestIDsLargerThanTimestamp_tsI_tsO	"r"	_tsI _tsO
#define d_szXSop_EventIDs_tsO_							"e"	_tsO	d_szXSa_tsm_
#define d_szXSop_EventIDsMine_							"m"			d_szXSa_tsm_
#define d_szXSop_EventIDsOthers_p_						"o c='^i'"	d_szXSa_tsm_
#define d_szXSop_FetchEvents_tsI_						"E"	_tsI	d_szXSa_tsm_
#define d_szXSop_FetchEventsMine_						"M"			d_szXSa_tsm_
#define d_szXSop_FetchEventsOthers_s_					"O c='$s'"	d_szXSa_tsm_
#define d_szXSop_EventsData								"D"
#define d_szXSop_EventsData_tsO							"D" _tsO
#define d_szXSop_EventsDataMine							"B"
#define d_szXSop_EventsDataOther						"T"
#define d_szXSop_EventsDataOther_s						"T c='$s'"
#define d_szXSop_EventsDataOther_s_S_p					"T c='$s' n='^S' j='^j'"	// Include the contact identifier, name and JID
#define d_szXSop_EventsConfirmation_					"C"			d_szXSa_tsm_

/////////////////////////////////////////////////
#define d_szXop_MessageNew_xmlOpen_pE_tsO				"<m^E><" d_szXSop_EventsData_tsO ">"
#define d_szXop_MessageNew_xmlClose						"</" d_szXSop_EventsData "></m>"

/////////////////////////////////////////////////
#define d_szXop_ApiCall_s							"a n='$s' _r=''"
#define d_szXop_ApiCall_RespondToEventID_s_ts_pE	"a n='$s' _i='$t'^E"
#define d_szXop_ApiCall_RespondToEventOther_s_ts_pE	"a n='$s' _o='$t'^E"
#define d_szXop_ApiDataToEventID					"i"
#define d_szXop_ApiDataToEventID_ts_pE				"i i='$t'^E"
#define d_szXop_ApiDataToEventOther					"o"
#define d_szXop_ApiDataToEventOther_ts_pE			"o o='$t'^E"
#define d_szXop_ApiResponseError_Name_s				"e n='$s'"
	#define d_chXa_ApiName								'n'	// Fetch the name of the API
	#define d_chXa_ApiResponsePrefix					'_'

	#define d_chXv_ApiName_Version						'v'	// Query the version of the client (Example: SocietyPro 1.2.3.4)
	#define d_szXv_ApiName_Version						"v"
	#define d_chXv_ApiName_Ping							'p'
	#define d_szXv_ApiName_Ping							"p"

/////////////////////////////////////////////////
#define d_szXop_TaskDownloading_ts				"d i='$t'"
#define d_szXop_TaskUploading_ts				"u i='$t'"
#define d_szXop_TaskExecuted_ts					"x i='$t'"
#define d_szXop_TaskNotFound_ts					"n i='$t'"

	#define d_chXa_TaskID									'i'			// Identifier of the task
	#define d_chXa_TaskDataSizeTotal						's'			// Total size of the task's data
	#define d_szXa_TaskDataSizeTotal_i						" s='$i'"
	#define d_chXa_TaskDataOffset							'o'
	#define d_szXa_TaskDataOffset_i							" o='$i'"
	#define d_chXa_TaskDataBinary							'd'			// Chunk of data
	#define d_szXa_TaskDataBinary_Bii						" d='{o|}'"

#endif // XCPAPI_H

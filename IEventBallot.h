//	IEventBallot.h
#ifndef IEVENTBALLOT_H
#define IEVENTBALLOT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

typedef UINT UINT_BALLOT_CHOICES;	// (ukm) Use a 32-bit integer to store multiple ballot choices.  Each bit represent a choice, which means a ballot is limited to 32 choices.  In the future, this data type may be extended to 64 bits
#define d_cBallotChoicesMax			(sizeof(UINT_BALLOT_CHOICES) * 8) // Number of bits in UINT_BALLOT_CHOICES (which at the moment is 32)

class _CEventBallotChoice
{
public:
	CStr m_strQuestion;				// Question/choice to appear on the ballot
	CStr m_strGroupAutoAssign;		// Which sub-group to automatically assign when a user selects a choice
	int m_cVotes;					// How many people voted for this question.  This value is a caching of _CEventBallotVote::m_ukmChoices
};

class CArrayPtrBallotChoices : public CArray
{
public:
	inline _CEventBallotChoice ** PrgpGetChoices() const { return (_CEventBallotChoice **)PrgpvGetElements(); }
	inline _CEventBallotChoice ** PrgpGetChoicesStop(OUT _CEventBallotChoice *** pppChoiceStop) const { return (_CEventBallotChoice **)PrgpvGetElementsStop(OUT (void ***)pppChoiceStop); }
	void DeleteAllChoices();

	void BinHtmlAppendVoteChoices(INOUT CBin * pbinHtml, UINT_BALLOT_CHOICES ukmChoices, const CStr & strComment) const;
};

class _CEventBallotVote
{
public:
	TContact * m_pContact;				// Who voted
	UINT_BALLOT_CHOICES m_ukmChoices;	// What was voted
	TIMESTAMP m_tsVote;					// When the vote was casted
	CStr m_strComment;
	OJapiPollResultsComment * m_paoJapiPollResultsComment;
};

class CArrayPtrBallotVotes : public CArray
{
public:
	inline _CEventBallotVote ** PrgpGetVotesStop(OUT _CEventBallotVote *** pppVoteStop) const { return (_CEventBallotVote **)PrgpvGetElementsStop(OUT (void ***)pppVoteStop); }
	void DeleteAllVotes();
};

class CEventBallotAttatchment
{
	OJapiPollAttatchment * m_paoJapiAttatchment;

public:
	CEventBallotPoll * m_pPollParent;
	CBin m_binContent;
	CStr m_strName;
	CStr m_strMimeType;

	CEventBallotAttatchment(CEventBallotPoll * pPollParent);
	CEventBallotAttatchment(CEventBallotPoll * pPollParent, const CBin & binContent, const CStr & strName, const CStr & strMimeType);
	~CEventBallotAttatchment();

	POJapi POJapiGet();
};

class CArrayPtrPollAttatchments : public CArray
{
public:
	inline CEventBallotAttatchment ** PrgpGetAttatchmentsStop(OUT CEventBallotAttatchment *** pppAttatchmentsStop) const { return (CEventBallotAttatchment **)PrgpvGetElementsStop(OUT (void ***)pppAttatchmentsStop); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Common code for the ballot event
//	For simplicity, the votes are stored in the base class, however are never used by CEventBallotReceived
class IEventBallot : public IEvent
{
public:
	enum
		{
		FB_kfAllowMultipleChoices			= 0x0001,	// Voters may select multiple choices
		FB_kfAllowNoComments				= 0x0002,	// Prevent voters to send feedback comments (this flag uses the negation, so by default comments are enabled)
		FB_kfStopAcceptingVotes				= 0x0004,	// Do not accept votes from user (this is the case when a poll is stopped)
		FB_kfVotesTailied					= 0x0008,	// The votes have been calculated.  When a new vote arrives, clear this flag to force a recompilation.
		FB_kfFromBallotmaster				= 0x0010,	// The event was created by the Ballotmaster, and therefore update should also notify the Ballotmaster
		};
	UINT m_uFlagsBallot;							// Various options for the ballot
	CStr m_strTitle;
	CStr m_strDescription;
	CStr m_strButtonSubmit;
	CStr m_strButtonDismiss;

protected:
	CArrayPtrBallotChoices m_arraypaChoices;		// Which choices are available on the ballot
public:
	CArrayPtrBallotVotes m_arraypaVotes;			// Results of those who voted.  This variable is used by CEventBallotSent

public:
	IEventBallot(const TIMESTAMP * ptsEventID);
	virtual ~IEventBallot();
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
	virtual void HyperlinkGetTooltipText(PSZUC pszActionOfHyperlink, IOUT CStr * pstrTooltipText);
	virtual void HyperlinkClicked(PSZUC pszActionOfHyperlink, INOUT OCursor * poCursorTextBlock);
	virtual void DetachFromObjectsAboutBeingDeleted();

	void DeleteChoicesAndVotes();
	void SetChoices(const QVariantList & lsChoices);
	QVariantList LsGetChoices() CONST_MCC;

	_CEventBallotChoice ** PrgpGetChoicesStopWithTally(OUT _CEventBallotChoice *** pppChoiceStop) CONST_MCC;

	_CEventBallotChoice * PAllocateNewChoice();
	_CEventBallotVote * PAllocateNewVote();

	friend class DDialogBallotVote;
	friend class DDialogBallotSend;
};


//	This is the class sending/broadcasting a ballot to multiple recipients.
class CEventBallotSent : public IEventBallot
{
public:
	static const EEventClass c_eEventClass = eEventClass_eBallotSent_class;
public:
    CEventBallotSent(const TIMESTAMP * ptsEventID = d_ts_pNULL_AssignToNow);
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EEventClass EGetEventClassForXCP() const { return eEventClass_eBallotReceived_class; }
	virtual EGui XospDataE(const CXmlNode * pXmlNodeData, INOUT CBinXcpStanza * pbinXospReply);
};

class CEventBallotReceived : public IEventBallot
{
public:
	static const EEventClass c_eEventClass = eEventClass_eBallotReceived_class;
public:
	UINT_BALLOT_CHOICES m_ukmChoices;	// Selected choice(s)
	TIMESTAMP m_tsConfirmationReceipt;	// Timestamp to confirm the choices were received by the ballot sender
	CStr m_strComment;					// Feedback comment to send to the ballot creator

public:
	CEventBallotReceived(const TIMESTAMP * ptsEventID);
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EEventClass EGetEventClassForXCP() const { return CEventBallotSent::c_eEventClass; }
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual PSZUC PszGetTextOfEventForSystemTray(OUT_IGNORE CStr * pstrScratchBuffer) const;

	void DisplayDialogBallotVote(BOOL fPreviewMode = FALSE);
	void UpdateBallotChoices(UINT_BALLOT_CHOICES ukmChoices, WEditTextArea * pwEditComments);
};

// This event is used by the poll master, it is not an event by itself however used as a template to send ballots
//
class CEventBallotPoll : public CEventBallotSent
{
public:
	static const EEventClass c_eEventClass = eEventClass_eBallotPoll;
public:
    TIMESTAMP m_tsStarted;
    TIMESTAMP m_tsStopped;
    int m_cSecondsPollLength;
	CArrayPtrPollAttatchments m_arraypaAtattchments;

	// In order to send a ballot, we need a group or a contact.  Since this class is not stored into a regular vault for a group or a contact, we need the identifier of that vault so we may fetch the results.
	CStr m_strTargetIdentifier;				// Identifier to send the poll.  This is typically the SHA of a group, however may be something else in the future
private:
	CEventBallotSent * m_pEventBallotSent;	// Pointer to the actual ballot sent.  This event will accumulate/tally the votes.  The identifier (m_tsEventID) is the same as m_tsStarted, so this is how the two events may be linked.

public:
	CEventBallotPoll(const TIMESTAMP * ptsEventID = d_ts_pNULL_AssignToNow);
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	CEventBallotAttatchment * PAllocateNewAttatchment();

	bool FStartPoll();
	void StopPoll();
	ITreeItemChatLogEvents * PGetGroupTarget_YZ();
	CEventBallotSent * PGetEventBallotSend_YZ() CONST_MCC;
};


#endif // IEVENTBALLOT_H



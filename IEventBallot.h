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
class IEventBallot : public IEvent
{
public:
	CStr m_strTitle;
	CStr m_strDescription;
	CArrayPtrBallotChoices m_arraypaChoices;	// Which choices are available on the ballot
	CArrayPtrBallotVotes m_arraypaVotes;		// Results of those who votes
	enum
		{
		FB_kfAllowMultipleChoices			= 0x0001,	// Voters may select multiple choices
		FB_kfAllowNoComments				= 0x0002,	// Prevent voters to send feedback comments (this flag uses the negation, so by default comments are enabled)
		FB_kfStopAcceptingVotes				= 0x0004	// Do not accept votes from user (this is the case when a poll is stopped)
		};
	UINT m_uFlagsBallot;							// Various options for the ballot

public:
	IEventBallot(const TIMESTAMP * ptsEventID);
	virtual ~IEventBallot();
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
	virtual void HyperlinkGetTooltipText(PSZUC pszActionOfHyperlink, IOUT CStr * pstrTooltipText);
	virtual void HyperlinkClicked(PSZUC pszActionOfHyperlink, INOUT OCursor * poCursorTextBlock);
	virtual void DetachFromObjectsAboutBeingDeleted();

	void DeleteChoices();
	void SetChoices(const QStringList & lsChoices);
	QStringList LsGetChoices() const;

	_CEventBallotChoice * PAllocateNewChoice();
	_CEventBallotVote * PAllocateNewVote();
};


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
class CEventBallotPoll : public CEventBallotSent
{
public:
    TIMESTAMP m_tsStarted;
    TIMESTAMP m_tsStopped;
    int m_cSecondsPollLength;
	CArrayPtrPollAttatchments m_arraypaAtattchments;

	CEventBallotAttatchment *PAllocateNewAttatchment();
    CEventBallotPoll(const TIMESTAMP * ptsEventID = d_ts_pNULL_AssignToNow);
    virtual EEventClass EGetEventClass() const { return eEventClass_eBallotPoll; }
    virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
    virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);

};


#endif // IEVENTBALLOT_H



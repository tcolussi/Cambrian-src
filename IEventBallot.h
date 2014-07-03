#ifndef IEVENTBALLOT_H
#define IEVENTBALLOT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

typedef UINT UINT_BALLOT_CHOICES;	// (ukm) Use a 32-bit integer to store multiple ballot choices.  Each bit represent a choice, which means a ballot is limited to 32 choices.  In the future, this data type may be extended to 64 bits

class _CEventBallotChoice
{
public:
	CStr m_strQuestion;				// Question/choice to appear on the ballot
	CStr m_strGroupAutoAssign;		// Which sub-group to automatically assign when a user selects a choice
};

class CArrayPtrBallotChoices : public CArray
{
public:
	inline _CEventBallotChoice ** PrgpGetChoicesStop(OUT _CEventBallotChoice *** pppChoiceStop) const { return (_CEventBallotChoice **)PrgpvGetElementsStop(OUT (void ***)pppChoiceStop); }
};

class _CEventBallotVote
{
public:
	TContact * m_pContact;				// Who voted
	UINT_BALLOT_CHOICES m_ukmChoices;	// What was voted
};

class CArrayPtrBallotVotes : public CArray
{
public:
	inline _CEventBallotVote ** PrgpGetVotesStop(OUT _CEventBallotVote *** pppVoteStop) const { return (_CEventBallotVote **)PrgpvGetElementsStop(OUT (void ***)pppVoteStop); }
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

public:
	IEventBallot(const TIMESTAMP * ptsEventID);
	virtual ~IEventBallot();
	virtual void XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual void XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply);
	virtual void XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, CBinXcpStanzaType * pbinXcpStanzaReply);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
	virtual void HyperlinkGetTooltipText(PSZUC pszActionOfHyperlink, IOUT CStr * pstrTooltipText);
	virtual void HyperlinkClicked(PSZUC pszActionOfHyperlink, INOUT OCursor * poCursorTextBlock);

	_CEventBallotChoice * PAllocateNewChoice();
	_CEventBallotVote * PAllocateNewVote();
};


class CEventBallotSent : public IEventBallot
{
public:
	static const EEventClass c_eEventClass = eEventClass_eBallotSent_class;
public:
	// Responses
public:
	CEventBallotSent(const TIMESTAMP * ptsEventID = NULL);
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EEventClass EGetEventClassForXCP() const { return eEventClass_eBallotReceived_class; }
	virtual void XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply);
};

class CEventBallotReceived : public IEventBallot
{
public:
	static const EEventClass c_eEventClass = eEventClass_eBallotReceived_class;
public:
	UINT_BALLOT_CHOICES m_ukmChoices;	// Selected choice(s)
	TIMESTAMP m_tsChoicesReceived;		// Timestamp to confirm the choices were received by the ballot sender

public:
	CEventBallotReceived(const TIMESTAMP * ptsEventID);
	virtual EEventClass EGetEventClass() const { return c_eEventClass; }
	virtual EEventClass EGetEventClassForXCP() const { return CEventBallotSent::c_eEventClass; }
	virtual void XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);

	void DisplayDialogBallotVote();
	void SetChoices(UINT_BALLOT_CHOICES ukmChoices);
};

#endif // IEVENTBALLOT_H

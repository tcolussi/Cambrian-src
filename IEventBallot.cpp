#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "IEventBallot.h"

#define d_chAPIa_Ballot_strTitle		't'
#define d_chAPIa_Ballot_strDescription	'd'
#define d_chAPIa_BallotReceived_uVotedChoice	'v'		// Which choice(s) the user voted
#define d_chAPIa_BallotReceived_tsConfirmation	'c'		// Timestamp where the confirmation
#define d_chAPIe_BallotEntries			'E'
#define d_szAPIe_BallotEntries			"E"
#define d_chAPIe_BallotChoice			'C'
#define d_szAPIe_BallotChoice_s			"C t='^S'"
#define d_chAPIa_BallotChoice_strText	't'

#define d_chAPIe_BallotVotes			'V'
#define d_szAPIe_BallotVotes			"V"
#define d_chAPIe_BallotVote				'o'
#define d_szAPIe_BallotVote				"o"
#define d_szAPIa_BallotVote_pContact	'c'


IEventBallot::IEventBallot(const TIMESTAMP * ptsEventID) : IEvent(ptsEventID)
	{
	}

IEventBallot::~IEventBallot()
	{
	_CEventBallotChoice ** ppChoiceStop;
	_CEventBallotChoice ** ppChoice = m_arraypaChoices.PrgpGetChoicesStop(OUT &ppChoiceStop);
	while (ppChoice != ppChoiceStop)
		delete *ppChoice++;
	_CEventBallotVote ** ppVoteStop;
	_CEventBallotVote ** ppVote = m_arraypaVotes.PrgpGetVotesStop(OUT &ppVoteStop);
	while (ppVote != ppVoteStop)
		delete *ppVote++;
	}

//	IEventBallot::IEvent::XmlUnserializeCore()
void
IEventBallot::XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const
	{
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chAPIa_Ballot_strTitle, m_strTitle);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chAPIa_Ballot_strDescription, m_strDescription);

	pbinXmlAttributes->BinAppendText("><" d_szAPIe_BallotEntries ">");
	_CEventBallotChoice ** ppChoiceStop;
	_CEventBallotChoice ** ppChoice = m_arraypaChoices.PrgpGetChoicesStop(OUT &ppChoiceStop);
	while (ppChoice != ppChoiceStop)
		{
		_CEventBallotChoice * pChoice = *ppChoice++;
		pbinXmlAttributes->BinAppendTextSzv_VE("<" d_szAPIe_BallotChoice_s "/>", &pChoice->m_strQuestion);
		}
	pbinXmlAttributes->BinAppendText("</" d_szAPIe_BallotEntries ">");

	pbinXmlAttributes->BinAppendText("><" d_szAPIe_BallotVotes ">");
	_CEventBallotVote ** ppVoteStop;
	_CEventBallotVote ** ppVote = m_arraypaVotes.PrgpGetVotesStop(OUT &ppVoteStop);
	while (ppVote != ppVoteStop)
		{
		_CEventBallotVote * pVote = *ppVote++;
		pbinXmlAttributes->BinAppendTextSzv_VE("<" d_szAPIe_BallotVote " v='$x'", pVote->m_ukmChoices);
		pbinXmlAttributes->BinAppendXmlAttributeOfContactIdentifier(d_szAPIa_BallotVote_pContact, pVote->m_pContact);;
		pbinXmlAttributes->BinAppendTextSzv_VE("/>");
		}
	pbinXmlAttributes->BinAppendText("</" d_szAPIe_BallotVotes ">");
	}

//	IEventBallot::IEvent::XmlUnserializeCore()
void
IEventBallot::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	pXmlNodeElement->UpdateAttributeValueCStr(d_chAPIa_Ballot_strTitle, OUT_F_UNCH &m_strTitle);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chAPIa_Ballot_strDescription, OUT_F_UNCH &m_strDescription);

	const CXmlNode * pXmlNodeEntities = pXmlNodeElement->PFindElement(d_szAPIe_BallotEntries);
	if (pXmlNodeEntities != NULL)
		{
		const CXmlNode * pXmlNodeChoice = pXmlNodeEntities->m_pElementsList;
		while (pXmlNodeChoice != NULL)
			{
			_CEventBallotChoice * pChoice = PAllocateNewChoice();
			pXmlNodeChoice->UpdateAttributeValueCStr(d_chAPIa_BallotChoice_strText, OUT_F_UNCH &pChoice->m_strQuestion);
			pXmlNodeChoice = pXmlNodeChoice->m_pNextSibling;
			}
		}
	const CXmlNode * pXmlNodeVotes = pXmlNodeElement->PFindElement(d_szAPIe_BallotVotes);
	if (pXmlNodeVotes != NULL)
		{
		const CXmlNode * pXmlNodeVote = pXmlNodeVotes->m_pElementsList;
		while (pXmlNodeVote != NULL)
			{
			_CEventBallotVote * pVote = PAllocateNewVote();
			_XmlUnserializeAttributeOfContactIdentifier(d_szAPIa_BallotVote_pContact, OUT &pVote->m_pContact, IN pXmlNodeVote);
			pXmlNodeVote->UpdateAttributeValueUIntHexadecimal(d_chAPIa_BallotReceived_uVotedChoice, OUT_F_UNCH &pVote->m_ukmChoices);
			pXmlNodeVote = pXmlNodeVote->m_pNextSibling;
			}
		}
	}

void
IEventBallot::XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply)
	{
	MessageLog_AppendTextFormatCo(d_coRed, "IEventBallot::XcpExtraDataRequest()\n");
	}

void
IEventBallot::XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, CBinXcpStanzaType * pbinXcpStanzaReply)
	{
	MessageLog_AppendTextFormatCo(d_coRed, "IEventBallot::XcpExtraDataArrived()\n");
	}

#define d_chActionForEventBallot_Vote			'v'

#define d_coBallot		0xCCFF99		// Display the ballot with a light green color

//	IMPLEMENTATION NOTES
//	I would like to use the HTML <li></li> however I am unable to set the background color, nor proper indenting, so I use plain HTML.
void
IEventBallot::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	_BinHtmlInitWithTime(OUT &g_strScratchBufferStatusBar);
	BOOL fBallotSent = Event_FIsEventTypeSent();
	g_strScratchBufferStatusBar.BinAppendTextSzv_VE(fBallotSent ? "Sending Ballot: <b>^S</b>" : "Please vote on the following: <b>^S</b>", &m_strTitle);
	if (!m_strDescription.FIsEmptyString())
		g_strScratchBufferStatusBar.BinAppendTextSzv_VE("<br/><i>$S</i>", &m_strDescription);
	_CEventBallotChoice ** ppChoiceStop;
	_CEventBallotChoice ** ppChoiceFirst = m_arraypaChoices.PrgpGetChoicesStop(OUT &ppChoiceStop);
	_CEventBallotChoice ** ppChoice = ppChoiceFirst;
	while (ppChoice != ppChoiceStop)
		{
		_CEventBallotChoice * pChoice = *ppChoice++;
		g_strScratchBufferStatusBar.BinAppendTextSzv_VE("<br/> ^_^_ &#149; ^S", &pChoice->m_strQuestion);
		}
	// &#8226;, 249	// Other HTML bullets, U+2219 bullet operator
	const int cChoices = m_arraypaChoices.GetSize();
	_CEventBallotVote ** ppVoteStop;
	_CEventBallotVote ** ppVote = m_arraypaVotes.PrgpGetVotesStop(OUT &ppVoteStop);
	while (ppVote != ppVoteStop)
		{
		_CEventBallotVote * pVote = *ppVote++;
		g_strScratchBufferStatusBar.BinAppendTextSzv_VE("<br/>- ^j voted: ", pVote->m_pContact);
		UINT_BALLOT_CHOICES ukfChoiceMask = 0x00000001;	// Initialize the first bit
		int iChoice = 0;
		while (iChoice < cChoices)
			{
			if (pVote->m_ukmChoices & ukfChoiceMask)
				{
				_CEventBallotChoice * pChoice = ppChoiceFirst[iChoice];
				g_strScratchBufferStatusBar.BinAppendTextSzv_VE(" ^S", &pChoice->m_strQuestion);
				}
			iChoice++;
			ukfChoiceMask <<= 1;
			}
		} // while

	if (!fBallotSent)
		{
		g_strScratchBufferStatusBar.BinAppendText("<br/>");
		_BinHtmlAppendHyperlinkAction(INOUT &g_strScratchBufferStatusBar, d_chActionForEventBallot_Vote, "Vote");
		}
	//MessageLog_AppendTextFormatCo(d_coBlack, "$S", &g_strScratchBufferStatusBar);
	poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, QBrush(d_coBallot));
	}

//	IEventBallot::IEvent::HyperlinkGetTooltipText()
//
//	This virtual method shows the tooltips for a ballot.
void
IEventBallot::HyperlinkGetTooltipText(PSZUC pszActionOfHyperlink, IOUT CStr * pstrTooltipText)
	{
	Assert(pszActionOfHyperlink[0] == d_chActionForEventBallot_Vote);
	pstrTooltipText->Format("Cast your vote on this ballot!");
	}

//	IEventBallot::IEvent::HyperlinkClicked()
void
IEventBallot::HyperlinkClicked(PSZUC pszActionOfHyperlink, OCursor *)
	{
	Assert(pszActionOfHyperlink[0] == d_chActionForEventBallot_Vote);
	((CEventBallotReceived *)this)->DisplayDialogBallotVote();
	}


_CEventBallotChoice *
IEventBallot::PAllocateNewChoice()
	{
	_CEventBallotChoice * pChoice = new _CEventBallotChoice;
	m_arraypaChoices.Add(PA_CHILD pChoice);
	return pChoice;
	}

_CEventBallotVote *
IEventBallot::PAllocateNewVote()
	{
	_CEventBallotVote * pVote = new _CEventBallotVote;
	m_arraypaVotes.Add(PA_CHILD pVote);
	return pVote;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventBallotSent::CEventBallotSent(const TIMESTAMP * ptsEventID) : IEventBallot(ptsEventID)
	{
	}

void
CEventBallotSent::XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply)
	{
	const UINT_BALLOT_CHOICES ukmChoices = pXmlNodeExtraData->UFindAttributeValueHexadecimal_ZZR(d_chAPIa_BallotReceived_uVotedChoice);
	//MessageLog_AppendTextFormatCo(d_coRed, "CEventBallotSent::XcpExtraDataArrived() - $x\n", ukmChoices);

	TContact * pContact = pbinXcpStanzaReply->m_pContact; // Get the contact who voted
	// Search if the contact already voted
	_CEventBallotVote * pVote;
	_CEventBallotVote ** ppVoteStop;
	_CEventBallotVote ** ppVote = m_arraypaVotes.PrgpGetVotesStop(OUT &ppVoteStop);
	while (ppVote != ppVoteStop)
		{
		pVote = *ppVote++;
		if (pVote->m_pContact == pContact)
			{
			MessageLog_AppendTextFormatSev(eSeverityComment, "Updating vote by ^j from 0x$x to 0x$x\n", pContact, pVote->m_ukmChoices, ukmChoices);
			goto UpdateChoices;
			}
		}
	// The contact never voted, therefore create a new entry
	MessageLog_AppendTextFormatSev(eSeverityComment, "New vote by ^j: 0x$x\n", pContact, ukmChoices);
	pVote = PAllocateNewVote();
	pVote->m_pContact = pContact;
	UpdateChoices:
	pVote->m_ukmChoices = ukmChoices;
	m_pVaultParent_NZ->SetModified();
	// TODO: Update the GUI
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventBallotReceived::CEventBallotReceived(const TIMESTAMP * ptsEventID) : IEventBallot(ptsEventID)
	{
	m_ukmChoices = 0;
	m_tsChoicesReceived = d_ts_zNA;
	}

void
CEventBallotReceived::XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const
	{
	pbinXmlAttributes->BinAppendXmlAttributeUIntHexadecimal(d_chAPIa_BallotReceived_uVotedChoice, m_ukmChoices);	// Save the attribute before the rest of the ballot, because the ballot includes XML elements
	pbinXmlAttributes->BinAppendXmlAttributeTimestamp(d_chAPIa_BallotReceived_tsConfirmation, m_tsChoicesReceived);
	IEventBallot::XmlSerializeCore(IOUT pbinXmlAttributes);
	}

void
CEventBallotReceived::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	pXmlNodeElement->UpdateAttributeValueUIntHexadecimal(d_chAPIa_BallotReceived_uVotedChoice, OUT_F_UNCH &m_ukmChoices);
	pXmlNodeElement->UpdateAttributeValueTimestamp(d_chAPIa_BallotReceived_tsConfirmation, OUT_F_UNCH &m_tsChoicesReceived);
	IEventBallot::XmlUnserializeCore(pXmlNodeElement);
	}

void
CEventBallotReceived::SetChoices(UINT_BALLOT_CHOICES ukmChoices)
	{
	if (ukmChoices == m_ukmChoices)
		return;	// Nothing changed
	m_ukmChoices = ukmChoices;
	m_pVaultParent_NZ->SetModified();
//	Event_UpdateWidgetWithinParentChatLog();	// BUGGY!
	// Send the selected choices to the ballot creator
	CBinXcpStanzaTypeInfo binXcpStanza(this);
	binXcpStanza.BinAppendTextSzv_VE("<" d_szXCP_"x" _tsI " v='$u'/>", m_tsOther, m_ukmChoices);	// TODO: rewrite this with more elegant code!
	binXcpStanza.XcpSendStanzaToContact(m_pContactGroupSender_YZ);
	}

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "IEventBallot.h"

#define d_chAPIa_Ballot_strTitle					't'
#define d_chAPIa_Ballot_strDescription				'd'
#define d_chAPIa_BallotReceived_uxVotedChoice		'v'		// What was voted (which choice(s) the user selected)
#define d_szAPIa_BallotReceived_uxVotedChoice_ux	" v='$x'"
#define d_chAPIa_BallotReceived_tsConfirmation		'c'		// Timestamp where the confirmation

#define d_chAPIe_BallotChoices						'C'
#define d_szAPIe_BallotChoices						"C"
#define d_chAPIe_BallotChoice						'C'
#define d_szAPIe_BallotChoice_S						"C t='^S'"
#define d_chAPIa_BallotChoice_strText				't'				// Text to appear on the ballot choice
#define d_chAPIa_BallotChoice_strGroup				'g'				// NYI: Which sub-group to automatically create

#define d_chAPIe_BallotVotes						'V'
#define d_szAPIe_BallotVotes						"V"
#define d_chAPIe_BallotVote							'V'
#define d_szAPIe_BallotVote_p_u_ts					"V f='^i' v='$x' t='$t'"
#define d_chAPIa_BallotVote_pContactFrom			'f'
#define d_chAPIa_BallotVote_uxVotedChoice			'v'
#define d_chAPIa_BallotVote_tsTime					't'
#define d_chAPIa_BallotVote_strComment				'c'				// NYI: Feedback comment of the user who casted the vote


IEventBallot::IEventBallot(const TIMESTAMP * ptsEventID) : IEvent(ptsEventID)
	{
	}

IEventBallot::~IEventBallot()
	{
	m_arraypaChoices.DeleteAllChoices();
	m_arraypaVotes.DeleteAllVotes();
	}

void
CArrayPtrBallotChoices::DeleteAllChoices()
	{
	_CEventBallotChoice ** ppChoiceStop;
	_CEventBallotChoice ** ppChoice = PrgpGetChoicesStop(OUT &ppChoiceStop);
	while (ppChoice != ppChoiceStop)
		delete *ppChoice++;
	RemoveAllElements();
	}

void
CArrayPtrBallotVotes::DeleteAllVotes()
	{
	_CEventBallotVote ** ppVoteStop;
	_CEventBallotVote ** ppVote = PrgpGetVotesStop(OUT &ppVoteStop);
	while (ppVote != ppVoteStop)
		delete *ppVote++;
	RemoveAllElements();
	}

//	IEventBallot::IEvent::XmlUnserializeCore()
void
IEventBallot::XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const
	{
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chAPIa_Ballot_strTitle, m_strTitle);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chAPIa_Ballot_strDescription, m_strDescription);

	pbinXmlAttributes->BinAppendText("><" d_szAPIe_BallotChoices ">");
	_CEventBallotChoice ** ppChoiceStop;
	_CEventBallotChoice ** ppChoice = m_arraypaChoices.PrgpGetChoicesStop(OUT &ppChoiceStop);
	while (ppChoice != ppChoiceStop)
		{
		_CEventBallotChoice * pChoice = *ppChoice++;
		pbinXmlAttributes->BinAppendTextSzv_VE("<" d_szAPIe_BallotChoice_S "/>", &pChoice->m_strQuestion);
		}
	pbinXmlAttributes->BinAppendText("</" d_szAPIe_BallotChoices ">");

	if (pbinXmlAttributes->FSerializingEventToDisk())
		{
		// Do not transmit the votes to the contacts; only serialize the vote to disk.
		pbinXmlAttributes->BinAppendText("><" d_szAPIe_BallotVotes ">");
		_CEventBallotVote ** ppVoteStop;
		_CEventBallotVote ** ppVote = m_arraypaVotes.PrgpGetVotesStop(OUT &ppVoteStop);
		while (ppVote != ppVoteStop)
			{
			_CEventBallotVote * pVote = *ppVote++;
			pbinXmlAttributes->BinAppendTextSzv_VE("<" d_szAPIe_BallotVote_p_u_ts "/>",  pVote->m_pContact, pVote->m_ukmChoices, pVote->m_tsVote);
			}
		pbinXmlAttributes->BinAppendText("</" d_szAPIe_BallotVotes ">");
		}
	}

//	IEventBallot::IEvent::XmlUnserializeCore()
void
IEventBallot::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	pXmlNodeElement->UpdateAttributeValueCStr(d_chAPIa_Ballot_strTitle, OUT_F_UNCH &m_strTitle);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chAPIa_Ballot_strDescription, OUT_F_UNCH &m_strDescription);

	const CXmlNode * pXmlNodeChoices = pXmlNodeElement->PFindElement(d_chAPIe_BallotChoices);
	if (pXmlNodeChoices != NULL)
		{
		m_arraypaChoices.DeleteAllChoices();	// Delete any previous choice(s) before adding new ones
		const CXmlNode * pXmlNodeChoice = pXmlNodeChoices->m_pElementsList;
		while (pXmlNodeChoice != NULL)
			{
			_CEventBallotChoice * pChoice = PAllocateNewChoice();
			pXmlNodeChoice->UpdateAttributeValueCStr(d_chAPIa_BallotChoice_strText, OUT_F_UNCH &pChoice->m_strQuestion);
			pXmlNodeChoice = pXmlNodeChoice->m_pNextSibling;
			}
		}
	const CXmlNode * pXmlNodeVotes = pXmlNodeElement->PFindElement(d_chAPIe_BallotVotes);
	if (pXmlNodeVotes != NULL)
		{
		m_arraypaVotes.DeleteAllVotes();	// Delete any previous vote(s) results before adding new ones
		const CXmlNode * pXmlNodeVote = pXmlNodeVotes->m_pElementsList;
		while (pXmlNodeVote != NULL)
			{
			_CEventBallotVote * pVote = PAllocateNewVote();
			_XmlUnserializeAttributeOfContactIdentifier(d_chAPIa_BallotVote_pContactFrom, OUT &pVote->m_pContact, IN pXmlNodeVote);
			pXmlNodeVote->UpdateAttributeValueUIntHexadecimal(d_chAPIa_BallotVote_uxVotedChoice, OUT_F_UNCH &pVote->m_ukmChoices);
			pXmlNodeVote->UpdateAttributeValueTimestamp(d_chAPIa_BallotVote_tsTime, OUT_F_UNCH &pVote->m_tsVote);
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
	CBin * pbinHtmlBallot = &g_strScratchBufferStatusBar;
	_BinHtmlInitWithTime(OUT pbinHtmlBallot);
	BOOL fBallotSent = Event_FIsEventTypeSent();
	pbinHtmlBallot->BinAppendTextSzv_VE(fBallotSent ? "Sending Ballot: <b>^S</b>" : "Please vote on the following: <b>^S</b>", &m_strTitle);
	if (!m_strDescription.FIsEmptyString())
		pbinHtmlBallot->BinAppendTextSzv_VE("<br/><i>$S</i>", &m_strDescription);

	UINT rgcVotes[d_cBallotChoicesMax];	// Count how many votes for each choice
	InitToZeroes(OUT rgcVotes, sizeof(rgcVotes));
	UINT cChoices = m_arraypaChoices.GetSize();
	if (cChoices > LENGTH(rgcVotes))
		cChoices = LENGTH(rgcVotes);

	_CEventBallotVote ** ppVoteStop;
	_CEventBallotVote ** ppVoteFirst = m_arraypaVotes.PrgpGetVotesStop(OUT &ppVoteStop);
	_CEventBallotVote ** ppVote = ppVoteFirst;
	while (ppVote != ppVoteStop)
		{
		_CEventBallotVote * pVote = *ppVote++;
		UINT_BALLOT_CHOICES ukfChoiceMask = 0x00000001;	// Initialize the first bit
		UINT iChoice = 0;
		while (iChoice < cChoices)
			{
			if (pVote->m_ukmChoices & ukfChoiceMask)
				rgcVotes[iChoice]++;
			iChoice++;
			ukfChoiceMask <<= 1;
			}
		}

	UINT iChoice = 0;
	_CEventBallotChoice ** ppChoiceStop;
	_CEventBallotChoice ** ppChoiceFirst = m_arraypaChoices.PrgpGetChoicesStop(OUT &ppChoiceStop);
	_CEventBallotChoice ** ppChoice = ppChoiceFirst;
	while (ppChoice != ppChoiceStop)
		{
		_CEventBallotChoice * pChoice = *ppChoice++;
		pbinHtmlBallot->BinAppendTextSzv_VE("<br/> ^_^_ $I ^S", rgcVotes[iChoice], &pChoice->m_strQuestion);
		// &#149;, &#8226;, 249	// Other HTML bullets, U+2219 bullet operator
		iChoice++;
		}

	ppVote = ppVoteFirst;
	while (ppVote != ppVoteStop)
		{
		_CEventBallotVote * pVote = *ppVote++;
		TIMESTAMP_DELTA dts = pVote->m_tsVote - m_tsEventID;	// Calculate how long it took for the user to respond (vote)
		pbinHtmlBallot->BinAppendTextSzv_VE("<br/>[$T] ^j voted: ", dts, pVote->m_pContact);
		m_arraypaChoices.BinHtmlAppendVoteChoices(INOUT pbinHtmlBallot, pVote->m_ukmChoices);
		/*
		UINT_BALLOT_CHOICES ukfChoiceMask = 0x00000001;	// Initialize the first bit
		UINT iChoice = 0;
		while (iChoice < cChoices)
			{
			if (pVote->m_ukmChoices & ukfChoiceMask)
				{
				_CEventBallotChoice * pChoice = ppChoiceFirst[iChoice];
				pbinHtmlBallot->BinAppendTextSzv_VE(" ^S", &pChoice->m_strQuestion);
				}
			iChoice++;
			ukfChoiceMask <<= 1;
			}
		*/
		} // while

	if (!fBallotSent)
		{
		Assert(EGetEventClass() == CEventBallotReceived::c_eEventClass);
		pbinHtmlBallot->BinAppendText("<br/>");
		_BinHtmlAppendHyperlinkAction(INOUT pbinHtmlBallot, d_chActionForEventBallot_Vote, "Vote");
		UINT_BALLOT_CHOICES ukmChoices = ((CEventBallotReceived *)this)->m_ukmChoices;
		if (ukmChoices != 0)
			{
			pbinHtmlBallot->BinAppendText(" &nbsp; You voted: ");
			m_arraypaChoices.BinHtmlAppendVoteChoices(INOUT pbinHtmlBallot, ukmChoices);
			}
		}
	//MessageLog_AppendTextFormatCo(d_coBlack, "$S", pbinHtmlBallot);
	poCursorTextBlock->InsertHtmlBin(*pbinHtmlBallot, QBrush(d_coBallot));
	}

void
CArrayPtrBallotChoices::BinHtmlAppendVoteChoices(INOUT CBin * pbinHtml, UINT_BALLOT_CHOICES ukmChoices) const
	{
	Assert(pbinHtml != NULL);
	if (m_paArrayHdr == NULL)
		return;
	UINT_BALLOT_CHOICES ukfChoiceMask = 0x00000001;	// Initialize the first bit
	int iChoice = 0;
	while (iChoice < m_paArrayHdr->cElements)
		{
		if (ukmChoices & ukfChoiceMask)
			{
			_CEventBallotChoice * pChoice = (_CEventBallotChoice *)m_paArrayHdr->rgpvData[iChoice];
			pbinHtml->BinAppendTextSzv_VE(" ^S", &pChoice->m_strQuestion);
			}
		iChoice++;
		ukfChoiceMask <<= 1;
		}
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
	InitToGarbage(OUT pVote, sizeof(*pVote));
	return pVote;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventBallotSent::CEventBallotSent(const TIMESTAMP * ptsEventID) : IEventBallot(ptsEventID)
	{
	}

void
CEventBallotSent::XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanzaType * pbinXcpStanzaReply)
	{
	const UINT_BALLOT_CHOICES ukmChoices = pXmlNodeExtraData->UFindAttributeValueHexadecimal_ZZR(d_chAPIa_BallotReceived_uxVotedChoice);
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
			if (pVote->m_ukmChoices == ukmChoices)
				return;		// This vote is a duplicate
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
	pVote->m_tsVote = Timestamp_GetCurrentDateTime();
	m_pVaultParent_NZ->SetModified();
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
	pbinXmlAttributes->BinAppendXmlAttributeUIntHexadecimal(d_chAPIa_BallotReceived_uxVotedChoice, m_ukmChoices);	// Save the attribute before the rest of the ballot, because the ballot includes XML elements
	pbinXmlAttributes->BinAppendXmlAttributeTimestamp(d_chAPIa_BallotReceived_tsConfirmation, m_tsChoicesReceived);
	IEventBallot::XmlSerializeCore(IOUT pbinXmlAttributes);
	}

void
CEventBallotReceived::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	pXmlNodeElement->UpdateAttributeValueUIntHexadecimal(d_chAPIa_BallotReceived_uxVotedChoice, OUT_F_UNCH &m_ukmChoices);
	pXmlNodeElement->UpdateAttributeValueTimestamp(d_chAPIa_BallotReceived_tsConfirmation, OUT_F_UNCH &m_tsChoicesReceived);
	IEventBallot::XmlUnserializeCore(pXmlNodeElement);
	}

PSZUC
CEventBallotReceived::PszGetTextOfEventForSystemTray(OUT_IGNORE CStr * pstrScratchBuffer) const
	{
	return pstrScratchBuffer->Format("Please vote: $S\n$S", &m_strTitle, &m_strDescription);
	}

void
CEventBallotReceived::SetChoices(UINT_BALLOT_CHOICES ukmChoices)
	{
	if (ukmChoices == m_ukmChoices)
		return;	// Nothing changed
	m_ukmChoices = ukmChoices;
	m_pVaultParent_NZ->SetModified();
	ChatLog_UpdateEventWithinSelectedChatLogFromNavigationTree();

	// Send the selected choices to the ballot creator
	CBinXcpStanzaTypeInfo binXcpStanza(this);
	binXcpStanza.BinAppendTextSzv_VE("<" d_szXCP_"x" _tsI d_szAPIa_BallotReceived_uxVotedChoice_ux "/>", m_tsOther, m_ukmChoices);	// TODO: rewrite this with more elegant code!
	binXcpStanza.XcpSendStanzaToContact(PGetContactForReply_YZ());
	}

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "IEventBallot.h"

#define d_chAPIa_IEventBallot_strTitle							't'		// Ballot title / headline
#define d_chAPIa_IEventBallot_strDescription					'd'		// Description of what/why the vote
#define d_chAPIa_IEventBallot_uxFlagsBallot						'b'		// Various options for the ballot

#define d_chAPIa_CEventBallotReceived_uxVotedChoice				'v'		// What was voted (which choice(s) the user selected)
#define d_szAPIa_CEventBallotReceived_uxVotedChoice_ux			" v='$x'"
#define d_chAPIa_CEventBallotReceived_tsConfirmationReceipt		'r'		// Timestamp confirming vote was successfully receivede by the ballot sender
#define d_chAPIa_CEventBallotReceived_strNote					'n'
#define d_szAPIa_CEventBallotReceived_strNote					" n='$S'"

#define d_chAPIe_BallotChoices						'C'
#define d_szAPIe_BallotChoices						"C"
#define d_chAPIe_BallotChoice						'C'
#define d_szAPIe_BallotChoice_S						"C t='^S'"
#define d_chAPIa_BallotChoice_strText				't'				// Text to appear for one ballot choice
#define d_chAPIa_BallotChoice_strGroup				'g'				// NYI: Which sub-group to automatically create

#define d_chAPIe_BallotVotes						'V'
#define d_szAPIe_BallotVotes						"V"
#define d_chAPIe_BallotVote							'V'
//#define d_szAPIe_BallotVote_p_u_ts					"V f='^i' v='$x' t='$t'"
#define d_szAPIe_BallotVote_p_u_ts_str				"V f='^i' v='$x' t='$t' n='^S'"	//  ^{n$S}
#define d_chAPIa_BallotVote_pContactFrom			'f'
#define d_chAPIa_BallotVote_uxVotedChoice			'v'
#define d_chAPIa_BallotVote_tsTime					't'
#define d_chAPIa_BallotVote_strNote					'n'				// NYI: Feedback comment of the user who casted the vote


IEventBallot::IEventBallot(const TIMESTAMP * ptsEventID) : IEvent(ptsEventID)
	{
	m_uFlagsBallot = 0;
	}

IEventBallot::~IEventBallot()
	{
	DeleteChoices();
	}

void
IEventBallot::DeleteChoices()
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

//	IEventBallot::IEvent::XmlUnserializeCoreE()
EXml
IEventBallot::XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const
	{
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chAPIa_IEventBallot_strTitle, m_strTitle);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chAPIa_IEventBallot_strDescription, m_strDescription);
	pbinXmlAttributes->BinAppendXmlAttributeUIntHexadecimal(d_chAPIa_IEventBallot_uxFlagsBallot, m_uFlagsBallot);

	pbinXmlAttributes->BinAppendText("><" d_szAPIe_BallotChoices ">");
	_CEventBallotChoice ** ppChoiceStop;
	_CEventBallotChoice ** ppChoice = m_arraypaChoices.PrgpGetChoicesStop(OUT &ppChoiceStop);
	while (ppChoice != ppChoiceStop)
		{
		_CEventBallotChoice * pChoice = *ppChoice++;
		pbinXmlAttributes->BinAppendTextSzv_VE("<" d_szAPIe_BallotChoice_S "/>", &pChoice->m_strQuestion);
		}
	pbinXmlAttributes->BinAppendText("</" d_szAPIe_BallotChoices ">");

    //if (pbinXmlAttributes->FSerializingEventToDisk())
		{
		// Do not transmit the votes to the contacts; only serialize the vote to disk.
		pbinXmlAttributes->BinAppendText("><" d_szAPIe_BallotVotes ">");
		_CEventBallotVote ** ppVoteStop;
		_CEventBallotVote ** ppVote = m_arraypaVotes.PrgpGetVotesStop(OUT &ppVoteStop);
		while (ppVote != ppVoteStop)
			{
			_CEventBallotVote * pVote = *ppVote++;
			pbinXmlAttributes->BinAppendTextSzv_VE("<" d_szAPIe_BallotVote_p_u_ts_str "/>",  pVote->m_pContact, pVote->m_ukmChoices, pVote->m_tsVote, &pVote->m_strComment);
			}
		pbinXmlAttributes->BinAppendText("</" d_szAPIe_BallotVotes ">");
		}
	return eXml_fElementPresent;	// There may be an element present
	}

//	IEventBallot::IEvent::XmlUnserializeCore()
void
IEventBallot::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	pXmlNodeElement->UpdateAttributeValueCStr(d_chAPIa_IEventBallot_strTitle, OUT_F_UNCH &m_strTitle);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chAPIa_IEventBallot_strDescription, OUT_F_UNCH &m_strDescription);
	pXmlNodeElement->UpdateAttributeValueUIntHexadecimal(d_chAPIa_IEventBallot_uxFlagsBallot, OUT_F_UNCH &m_uFlagsBallot);

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
			pXmlNodeVote->UpdateAttributeValueCStr(d_chAPIa_BallotVote_strNote, OUT_F_UNCH &pVote->m_strComment);
			pXmlNodeVote = pXmlNodeVote->m_pNextSibling;
			}
		}
	} // XmlUnserializeCore()

void
IEventBallot::XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanza * pbinXcpStanzaReply)
	{
	MessageLog_AppendTextFormatCo(d_coRed, "IEventBallot::XcpExtraDataRequest()\n");
	}

void
IEventBallot::XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, CBinXcpStanza * pbinXcpStanzaReply)
	{
	MessageLog_AppendTextFormatCo(d_coRed, "IEventBallot::XcpExtraDataArrived()\n");
	}

//	Some of the HTML entities do are not displayed properly
//const char c_szHtmlBulletSmall[]		= "&#8729;";	// Tiny dot
//const char c_szHtmlBulletLarge[]		= "&#8226;";	// Bullet
const char c_szHtmlBulletSmall[]		= "&#8226;";	// Bullet
const char c_szHtmlBulletLarge[]		= "&#9679;";	// Black circle
//const char c_szHtmlBulletLarge[]		= "&#11044;";	// Black large circle
//const char c_szHtmlBulletLarge[]		= "&#10687;";	// Circled bullet
const char c_szHtmlCheckboxEmpty[]		= "&#9744;";	// Ballot box
//const char c_szHtmlCheckboxSelected[]	= "&#9745;";	// Ballot with a checkmark
const char c_szHtmlCheckboxSelected[]	= "&#9746;";	// Ballot with a x

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
	pbinHtmlBallot->BinAppendTextSzv_VE(fBallotSent ? "Sending Ballot: <b>^S</b>" : "Please vote on the following:<br/><b>^S</b>", &m_strTitle);
	if (!m_strDescription.FIsEmptyString())
		pbinHtmlBallot->BinAppendTextSzv_VE("<br/><i>$S</i>", &m_strDescription);

	UINT rgcVotes[d_cBallotChoicesMax];	// Count how many votes for each choice
	InitToZeroes(OUT rgcVotes, sizeof(rgcVotes));
	UINT cChoices = m_arraypaChoices.GetSize();
	if (cChoices > LENGTH(rgcVotes))
		cChoices = LENGTH(rgcVotes);

	// Count how many votes per choice
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

	// Display each choice
	CHU szuVotes[16];
	UINT iChoice = 0;
	_CEventBallotChoice ** ppChoiceStop;
	_CEventBallotChoice ** ppChoiceFirst = m_arraypaChoices.PrgpGetChoicesStop(OUT &ppChoiceStop);
	_CEventBallotChoice ** ppChoice = ppChoiceFirst;
	while (ppChoice != ppChoiceStop)
		{
		_CEventBallotChoice * pChoice = *ppChoice++;
		PSZAC pszHtmlBullet;
		if (fBallotSent)
			{
			IntegerToString(OUT szuVotes, rgcVotes[iChoice], ITS_mskfThousandSeparator);
			pszHtmlBullet = (PSZAC)szuVotes;	// Instead of displaying a bullet, display the count of votes
			}
		else
			{
			const BOOL fuChoiceSelectd = ((CEventBallotReceived *)this)->m_ukmChoices & (1 << iChoice);
			if (m_uFlagsBallot & FB_kfAllowMultipleChoices)
				pszHtmlBullet = fuChoiceSelectd ? c_szHtmlCheckboxSelected : c_szHtmlCheckboxEmpty;
			else
				pszHtmlBullet = fuChoiceSelectd ? c_szHtmlBulletLarge : c_szHtmlBulletSmall;
			}
		pbinHtmlBallot->BinAppendTextSzv_VE("<br/> ^_^_ $s ^S", pszHtmlBullet, &pChoice->m_strQuestion);
		iChoice++;
		} // while

	// Display who voted
	ppVote = ppVoteFirst;
	while (ppVote != ppVoteStop)
		{
		_CEventBallotVote * pVote = *ppVote++;
		TIMESTAMP_DELTA dts = pVote->m_tsVote - m_tsEventID;	// Calculate how long it took for the user to respond (vote)
		pbinHtmlBallot->BinAppendTextSzv_VE("<br/>[$T] ^j voted: ", dts, pVote->m_pContact);
		m_arraypaChoices.BinHtmlAppendVoteChoices(INOUT pbinHtmlBallot, pVote->m_ukmChoices, pVote->m_strComment);
		} // while

	if (!fBallotSent)
		{
		// Display what the user voted
		Assert(EGetEventClass() == CEventBallotReceived::c_eEventClass);
		pbinHtmlBallot->BinAppendText("<br/>");
		_BinHtmlAppendHyperlinkAction(INOUT pbinHtmlBallot, d_chActionForEventBallot_Vote, "Vote");
		UINT_BALLOT_CHOICES ukmChoices = ((CEventBallotReceived *)this)->m_ukmChoices;
		if (ukmChoices != 0)
			{
			pbinHtmlBallot->BinAppendText(" &nbsp; You voted: ");
			m_arraypaChoices.BinHtmlAppendVoteChoices(INOUT pbinHtmlBallot, ukmChoices, ((CEventBallotReceived *)this)->m_strComment);
			}
		}
	//MessageLog_AppendTextFormatCo(d_coBlack, "$S", pbinHtmlBallot);
	poCursorTextBlock->InsertHtmlBin(*pbinHtmlBallot, QBrush(d_coBallot));
	} // ChatLogUpdateTextBlock()

void
CArrayPtrBallotChoices::BinHtmlAppendVoteChoices(INOUT CBin * pbinHtml, UINT_BALLOT_CHOICES ukmChoices, const CStr & strComment) const
	{
	Assert(pbinHtml != NULL);
	if (m_paArrayHdr == NULL)
		return;
	BOOL fPreviousChoices = FALSE;
	UINT_BALLOT_CHOICES ukfChoiceMask = 0x00000001;	// Initialize the first bit
	int iChoice = 0;
	while (iChoice < m_paArrayHdr->cElements)
		{
		if (ukmChoices & ukfChoiceMask)
			{
			if (fPreviousChoices)
				pbinHtml->BinAppendText(" +");	// Display multiple choices with a + sign
			fPreviousChoices = TRUE;
			_CEventBallotChoice * pChoice = (_CEventBallotChoice *)m_paArrayHdr->rgpvData[iChoice];
			pbinHtml->BinAppendTextSzv_VE(" ^S", &pChoice->m_strQuestion);
			}
		iChoice++;
		ukfChoiceMask <<= 1;
		}
	if (!strComment.FIsEmptyString())
		pbinHtml->BinAppendTextSzv_VE("<br/><b><i>^S</i></b>", &strComment);
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

//	IEventBallot::IEvent::DetachFromObjectsAboutBeingDeleted()
void
IEventBallot::DetachFromObjectsAboutBeingDeleted()
	{
	_CEventBallotVote ** ppVoteStop;
	_CEventBallotVote ** ppVote = m_arraypaVotes.PrgpGetVotesStop(OUT &ppVoteStop);
	while (ppVote != ppVoteStop)
		{
		_CEventBallotVote * pVote = *ppVote++;
		Assert(pVote->m_pContact != NULL);
//		pVote->m_pContact->Contact_UpdateFlagCannotBeDeleted();	// Prevent any contact in use by the ballot to be deleted
		}
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

void
IEventBallot::SetChoices(const QStringList & lsChoices)
	{
	DeleteChoices();	// Delete any previous choice(s)
	foreach(const QString & sChoice, lsChoices)
		{
		PAllocateNewChoice()->m_strQuestion = sChoice;
		}
	}

QStringList
IEventBallot::LsGetChoices() const
	{
	QStringList lsChoices;
	_CEventBallotChoice ** ppChoiceStop;
	_CEventBallotChoice ** ppChoice = m_arraypaChoices.PrgpGetChoicesStop(OUT &ppChoiceStop);
	while (ppChoice != ppChoiceStop)
		{
		_CEventBallotChoice * pChoice = *ppChoice++;
		lsChoices.append(pChoice->m_strQuestion);
		}
	return lsChoices;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventBallotSent::CEventBallotSent(const TIMESTAMP * ptsEventID) : IEventBallot(ptsEventID)
	{
	}

void
CEventBallotSent::XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanza * pbinXcpStanzaReply)
	{
	const UINT_BALLOT_CHOICES ukmChoices = pXmlNodeExtraData->UFindAttributeValueHexadecimal_ZZR(d_chAPIa_CEventBallotReceived_uxVotedChoice);
	CStr strComment = pXmlNodeExtraData->PszuFindAttributeValue(d_chAPIa_CEventBallotReceived_strNote);
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
			if (pVote->m_ukmChoices == ukmChoices && pVote->m_strComment.FCompareBinary(strComment))
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
	pVote->m_strComment = strComment;
	pVote->m_tsVote = Timestamp_GetCurrentDateTime();
	m_pVaultParent_NZ->SetModified();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventBallotReceived::CEventBallotReceived(const TIMESTAMP * ptsEventID) : IEventBallot(ptsEventID)
	{
	m_ukmChoices = 0;
	m_tsConfirmationReceipt = d_ts_zNA;
	}

//  CEventBallotReceived::IEvent::XmlSerializeCoreE()
EXml
CEventBallotReceived::XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const
	{
	pbinXmlAttributes->BinAppendXmlAttributeUIntHexadecimal(d_chAPIa_CEventBallotReceived_uxVotedChoice, m_ukmChoices);	// Save the attribute before the rest of the ballot, because the ballot includes XML elements
	pbinXmlAttributes->BinAppendXmlAttributeTimestamp(d_chAPIa_CEventBallotReceived_tsConfirmationReceipt, m_tsConfirmationReceipt);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chAPIa_CEventBallotReceived_strNote, m_strComment);
	return IEventBallot::XmlSerializeCoreE(IOUT pbinXmlAttributes);
	}

//  CEventBallotReceived::IEvent:XmlUnserializeCore()
void
CEventBallotReceived::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	pXmlNodeElement->UpdateAttributeValueUIntHexadecimal(d_chAPIa_CEventBallotReceived_uxVotedChoice, OUT_F_UNCH &m_ukmChoices);
	pXmlNodeElement->UpdateAttributeValueTimestamp(d_chAPIa_CEventBallotReceived_tsConfirmationReceipt, OUT_F_UNCH &m_tsConfirmationReceipt);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chAPIa_CEventBallotReceived_strNote, OUT_F_UNCH &m_strComment);
	IEventBallot::XmlUnserializeCore(pXmlNodeElement);
	}

PSZUC
CEventBallotReceived::PszGetTextOfEventForSystemTray(OUT_IGNORE CStr * pstrScratchBuffer) const
	{
	return pstrScratchBuffer->Format("Please vote: $S\n$S", &m_strTitle, &m_strDescription);
	}

void
CEventBallotReceived::UpdateBallotChoices(UINT_BALLOT_CHOICES ukmChoices, WEditTextArea * pwEditComments)
	{
	CStr strComments = *pwEditComments;
	if (strComments.FCompareBinary(m_strComment) && ukmChoices == m_ukmChoices)
		return;	// Nothing changed
	m_ukmChoices = ukmChoices;
	m_strComment = strComments;
	m_pVaultParent_NZ->SetModified();
	ChatLog_UpdateEventWithinSelectedChatLogFromNavigationTree();

	// Send the selected choices to the ballot creator
	CBinXcpStanzaTypeInfo binXcpStanza(this);
	binXcpStanza.BinAppendTextSzv_VE("<" d_szXCP_"x" _tsI d_szAPIa_CEventBallotReceived_uxVotedChoice_ux d_szAPIa_CEventBallotReceived_strNote "/>", m_tsOther, m_ukmChoices, &m_strComment);	// TODO: rewrite this with more elegant code!
	binXcpStanza.XcpSendStanzaToContact(PGetContactForReply_YZ());
}


CEventBallotPoll::CEventBallotPoll(const TIMESTAMP *ptsEventID) : CEventBallotSent ( ptsEventID )
    {
    m_tsStarted = d_ts_zNA;
    m_tsStopped = d_ts_zNA;
    m_cSecondsPollLength = d_zNA;
    }

//  CEventBallotPoll::IEvent::XmlSerializeCoreE()
EXml
CEventBallotPoll::XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const
    {
    pbinXmlAttributes->BinAppendXmlAttributeTimestamp('z', m_tsStarted);
    pbinXmlAttributes->BinAppendXmlAttributeTimestamp('Z', m_tsStopped);
    pbinXmlAttributes->BinAppendXmlAttributeInt('l', m_cSecondsPollLength);

    return CEventBallotSent::XmlSerializeCoreE(IOUT pbinXmlAttributes);
    }

//  CEventBallotPoll::IEvent:XmlUnserializeCore()
void
CEventBallotPoll::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
    {
    pXmlNodeElement->UpdateAttributeValueTimestamp('z', OUT_F_UNCH &m_tsStarted);
    pXmlNodeElement->UpdateAttributeValueTimestamp('Z', OUT_F_UNCH &m_tsStopped);
    pXmlNodeElement->UpdateAttributeValueInt('l', &m_cSecondsPollLength);
    CEventBallotSent::XmlUnserializeCore(pXmlNodeElement);
    }

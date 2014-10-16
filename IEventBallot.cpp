//	IEventBallot.cpp

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#define d_chXMLa_IEventBallot_strTitle							't'		// Ballot title / headline
#define d_chXMLa_IEventBallot_strDescription					'd'		// Description of what/why the vote
#define d_chXMLa_IEventBallot_uxFlagsBallot						'b'		// Various options for the ballot
#define d_chXMLa_IEventBallot_strButtonSubmit					'S'		// Customize the button text to submit a vote
#define d_chXMLa_IEventBallot_strButtonDismiss					'D'		// Customize the button text to dismiss a vote

#define d_chXMLa_CEventBallotReceived_uxVotedChoice				'v'		// What was voted (which choice(s) the user selected)
#define d_szXMLa_CEventBallotReceived_uxVotedChoice_ux			" v='$x'"
#define d_chXMLa_CEventBallotReceived_tsConfirmationReceipt		'r'		// Timestamp confirming vote was successfully receivede by the ballot sender
#define d_chXMLa_CEventBallotReceived_strNote					'n'
#define d_szXMLa_CEventBallotReceived_strNote					" n='$S'"

#define d_chXMLa_CEventBallotPoll_tsStarted						's'
#define d_chXMLa_CEventBallotPoll_tsStopped						'S'
#define d_chXMLa_CEventBallotPoll_cSecondsPollLength			'l'
#define d_chXMLa_CEventBallotPoll_strTargetId					'T'

#define d_chXMLe_BallotChoices						'C'
#define d_szXMLe_BallotChoices						"C"
#define d_chXMLe_BallotChoice						'C'
#define d_szXMLe_BallotChoice_S_ux					"C t='^S' f='$x'"
#define d_szXMLe_BallotChoice_S_ux_i				"C t='^S' f='$x' c='$i'"
#define d_chXMLa_BallotChoice_strText				't'				// Text to appear for one ballot choice
#define d_chXMLa_BallotChoice_uxFlags				'f'
#define d_chXMLa_BallotChoice_cVotes				'c'				// Number of votes for the choice
#define d_chXMLa_BallotChoice_strGroup				'g'				// NYI: Which sub-group to automatically create

#define d_chXMLe_BallotVotes						'V'
#define d_szXMLe_BallotVotes						"V"
#define d_chXMLe_BallotVote							'V'
//#define d_szXMLe_BallotVote_p_u_ts					"V f='^i' v='$x' t='$t'"
#define d_szXMLe_BallotVote_p_u_ts_str				"V f='^i' v='$x' t='$t' n='^S'"	//  ^{n$S}
#define d_chXMLa_BallotVote_pContactFrom			'f'
#define d_chXMLa_BallotVote_uxVotedChoice			'v'
#define d_chXMLa_BallotVote_tsTime					't'
#define d_chXMLa_BallotVote_strNote					'n'				// NYI: Feedback comment of the user who casted the vote




IEventBallot::IEventBallot(const TIMESTAMP * ptsEventID) : IEvent(ptsEventID)
	{
	m_uFlagsBallot = 0;
	}

IEventBallot::~IEventBallot()
	{
	DeleteChoicesAndVotes();
	}

void
IEventBallot::DeleteChoicesAndVotes()
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
	Empty();
	}

void
CArrayPtrBallotVotes::DeleteAllVotes()
	{
	_CEventBallotVote ** ppVoteStop;
	_CEventBallotVote ** ppVote = PrgpGetVotesStop(OUT &ppVoteStop);
	while (ppVote != ppVoteStop)
		delete *ppVote++;
	Empty();
	}

//	IEventBallot::IEvent::XmlUnserializeCoreE()
EXml
IEventBallot::XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const
	{
	pbinXmlAttributes->BinAppendXmlAttributeUIntHexadecimal(d_chXMLa_IEventBallot_uxFlagsBallot, m_uFlagsBallot);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chXMLa_IEventBallot_strTitle, m_strTitle);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chXMLa_IEventBallot_strDescription, m_strDescription);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chXMLa_IEventBallot_strButtonSubmit, m_strButtonSubmit);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chXMLa_IEventBallot_strButtonDismiss, m_strButtonDismiss);

	const BOOL fSerializeVotes = pbinXmlAttributes->FuSerializingEventToDisk();	// Serialize the votes only when saving to disk.  Transmitting via XOSP or cloning does not serialize the votes.
	PSZAC pszFmtTemplateBallotChoice_S_ux_i = fSerializeVotes ? "<" d_szXMLe_BallotChoice_S_ux_i "/>" : "<" d_szXMLe_BallotChoice_S_ux "/>";
	pbinXmlAttributes->BinAppendText("><" d_szXMLe_BallotChoices ">");	// This will close the XML attribute and open an XML element
	_CEventBallotChoice ** ppChoiceStop;
	_CEventBallotChoice ** ppChoice = m_arraypaChoices.PrgpGetChoicesStop(OUT &ppChoiceStop);
	while (ppChoice != ppChoiceStop)
		{
		_CEventBallotChoice * pChoice = *ppChoice++;
		pbinXmlAttributes->BinAppendText_VE(pszFmtTemplateBallotChoice_S_ux_i, &pChoice->m_strQuestion, pChoice->m_uFlags, pChoice->m_cVotes);
		}
	pbinXmlAttributes->BinAppendText("</" d_szXMLe_BallotChoices ">");

	if (fSerializeVotes)
		{
		// Do not transmit the votes to the contacts; only serialize the vote to disk.
		pbinXmlAttributes->BinAppendText("<" d_szXMLe_BallotVotes ">");
		_CEventBallotVote ** ppVoteStop;
		_CEventBallotVote ** ppVote = m_arraypaVotes.PrgpGetVotesStop(OUT &ppVoteStop);
		while (ppVote != ppVoteStop)
			{
			_CEventBallotVote * pVote = *ppVote++;
			pbinXmlAttributes->BinAppendText_VE("<" d_szXMLe_BallotVote_p_u_ts_str "/>",  pVote->m_pContact, pVote->m_ukmChoices, pVote->m_tsVote, &pVote->m_strComment);
			}
		pbinXmlAttributes->BinAppendText("</" d_szXMLe_BallotVotes ">");
		}
	return eXml_ElementPresent;	// There may be an element present
	}

//	IEventBallot::IEvent::XmlUnserializeCore()
void
IEventBallot::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	pXmlNodeElement->UpdateAttributeValueUIntHexadecimal(d_chXMLa_IEventBallot_uxFlagsBallot, OUT_F_UNCH &m_uFlagsBallot);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chXMLa_IEventBallot_strTitle, OUT_F_UNCH &m_strTitle);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chXMLa_IEventBallot_strDescription, OUT_F_UNCH &m_strDescription);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chXMLa_IEventBallot_strButtonSubmit, OUT_F_UNCH &m_strButtonSubmit);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chXMLa_IEventBallot_strButtonDismiss, OUT_F_UNCH &m_strButtonDismiss);

	const CXmlNode * pXmlNodeChoices = pXmlNodeElement->PFindElement(d_chXMLe_BallotChoices);
	if (pXmlNodeChoices != NULL)
		{
		m_arraypaChoices.DeleteAllChoices();	// Delete any previous choice(s) before adding new ones
		const CXmlNode * pXmlNodeChoice = pXmlNodeChoices->m_pElementsList;
		while (pXmlNodeChoice != NULL)
			{
			_CEventBallotChoice * pChoice = PAllocateNewChoice();
			pXmlNodeChoice->UpdateAttributeValueCStr(d_chXMLa_BallotChoice_strText, OUT_F_UNCH &pChoice->m_strQuestion);
			pXmlNodeChoice->UpdateAttributeValueUIntHexadecimal(d_chXMLa_BallotChoice_uxFlags, OUT &pChoice->m_uFlags);
			pXmlNodeChoice->UpdateAttributeValueInt(d_chXMLa_BallotChoice_cVotes, OUT &pChoice->m_cVotes);
			pXmlNodeChoice = pXmlNodeChoice->m_pNextSibling;
			}
		}
	const CXmlNode * pXmlNodeVotes = pXmlNodeElement->PFindElement(d_chXMLe_BallotVotes);
	if (pXmlNodeVotes != NULL)
		{
		m_arraypaVotes.DeleteAllVotes();	// Delete any previous vote(s) results before adding new ones
		const CXmlNode * pXmlNodeVote = pXmlNodeVotes->m_pElementsList;
		while (pXmlNodeVote != NULL)
			{
			_CEventBallotVote * pVote = PAllocateNewVote();
			_XmlUnserializeAttributeOfContactIdentifier(d_chXMLa_BallotVote_pContactFrom, OUT &pVote->m_pContact, IN pXmlNodeVote);
			pXmlNodeVote->UpdateAttributeValueUIntHexadecimal(d_chXMLa_BallotVote_uxVotedChoice, OUT_F_UNCH &pVote->m_ukmChoices);
			pXmlNodeVote->UpdateAttributeValueTimestamp(d_chXMLa_BallotVote_tsTime, OUT_F_UNCH &pVote->m_tsVote);
			pXmlNodeVote->UpdateAttributeValueCStr(d_chXMLa_BallotVote_strNote, OUT_F_UNCH &pVote->m_strComment);
			pXmlNodeVote = pXmlNodeVote->m_pNextSibling;
			}
		}
	} // XmlUnserializeCore()


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

void
IEventBallot::_AppendHtmlForChatLog(IOUT CBin * pbinHtmlBallot) CONST_MCC
	{
	BOOL fBallotSent = Event_FIsEventTypeSent();
	pbinHtmlBallot->BinAppendText_VE(fBallotSent ? "Sending Ballot: <b>^S</b>" : "Please vote on the following:<br/><b>^S</b>", &m_strTitle);
	if (!m_strDescription.FIsEmptyString())
		pbinHtmlBallot->BinAppendText_VE("<br/><i>$S</i>", &m_strDescription);

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
		pbinHtmlBallot->BinAppendText_VE("<br/> ^_^_ $s ^S", pszHtmlBullet, &pChoice->m_strQuestion);
		iChoice++;
		} // while

	// Display who voted
	ppVote = ppVoteFirst;
	while (ppVote != ppVoteStop)
		{
		_CEventBallotVote * pVote = *ppVote++;
		TIMESTAMP_DELTA dts = pVote->m_tsVote - m_tsEventID;	// Calculate how long it took for the user to respond (vote)
		pbinHtmlBallot->BinAppendText_VE("<br/>[$T] ^j voted: ", dts, pVote->m_pContact);
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
	} // _AppendHtmlForChatLog()

#ifdef COMPILE_WITH_CHATLOG_HTML
void
IEventBallot::AppendHtmlForChatLog(IOUT CBin * pbinHtml) CONST_MCC
	{
	_AppendHtmlForChatLog(IOUT pbinHtml);
	}
#else
//	IMPLEMENTATION NOTES
//	I would like to use the HTML <li></li> however I am unable to set the background color, nor proper indenting, so I use plain HTML.
void
IEventBallot::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	CBin * pbinHtmlBallot = &g_strScratchBufferStatusBar;
	_BinHtmlInitWithTime(OUT pbinHtmlBallot);
	_AppendHtmlForChatLog(IOUT pbinHtmlBallot);

	//MessageLog_AppendTextFormatCo(d_coBlack, "$S", pbinHtmlBallot);
	poCursorTextBlock->InsertHtmlBin(*pbinHtmlBallot, QBrush(d_coBallot));
	} // ChatLogUpdateTextBlock()
#endif

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
			pbinHtml->BinAppendText_VE(" ^S", &pChoice->m_strQuestion);
			}
		iChoice++;
		ukfChoiceMask <<= 1;
		}
	if (!strComment.FIsEmptyString())
		pbinHtml->BinAppendText_VE("<br/><b><i>^S</i></b>", &strComment);
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
#ifdef COMPILE_WITH_CHATLOG_HTML
EGui
IEventBallot::HyperlinkClickedE(PSZUC pszActionOfHyperlink)
	{
	Assert(pszActionOfHyperlink[0] == d_chActionForEventBallot_Vote);
	((CEventBallotReceived *)this)->DisplayDialogBallotVote();
	return eGui_NoUpdate;
	}

#else
//	IEventBallot::IEvent::HyperlinkClicked()
void
IEventBallot::HyperlinkClicked(PSZUC pszActionOfHyperlink, OCursor *)
	{
	Assert(pszActionOfHyperlink[0] == d_chActionForEventBallot_Vote);
	((CEventBallotReceived *)this)->DisplayDialogBallotVote();
	}
#endif

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

_CEventBallotChoice::_CEventBallotChoice()
	{
	m_uFlags = 0;
	m_cVotes = 0;		// So far, nobody voted for this choice!
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
	pVote->m_paoJapiPollResultsComment = NULL;
	m_arraypaVotes.Add(PA_CHILD pVote);
	return pVote;
	}


//	This method is essentially the same as m_arraypaChoices.PrgpGetChoicesStop() except it make sure the votes have been tallied/counted
_CEventBallotChoice **
IEventBallot::PrgpGetChoicesStopWithTally(OUT _CEventBallotChoice *** pppChoiceStop) CONST_MCC
	{
	_CEventBallotChoice ** prgpChoices = m_arraypaChoices.PrgpGetChoicesStop(OUT pppChoiceStop);
	if ((m_uFlagsBallot & FB_kfVotesTailied) == 0)
		{
		m_uFlagsBallot |= FB_kfVotesTailied;
		//MessageLog_AppendTextFormatCo(d_coRed, "PrgpGetChoicesStopWithTally() - tsEventID = $t\n", m_tsEventID);
		//	Clear the previous votes
		const int cChoices = *pppChoiceStop - prgpChoices;
		_CEventBallotChoice ** ppChoice = prgpChoices;
		while (ppChoice != *pppChoiceStop)
			(*ppChoice++)->m_cVotes = 0;

		// Calculate the votes
		_CEventBallotVote ** ppVoteStop;
		_CEventBallotVote ** ppVote = m_arraypaVotes.PrgpGetVotesStop(OUT &ppVoteStop);
		while (ppVote != ppVoteStop)
			{
			_CEventBallotVote * pVote = *ppVote++;
			int iChoice = 0;
			while (iChoice < cChoices)
				{
				if (pVote->m_ukmChoices & (1 << iChoice))
					prgpChoices[iChoice]->m_cVotes++;
				iChoice++;
				} // while
			} // while
		}
	else
		{
		// MessageLog_AppendTextFormatCo(d_coGreen, "PrgpGetChoicesStopWithTally() - tsEventID $t is up-to-date\n", m_tsEventID);
		}
	return prgpChoices;
	}
/*
void
IEventBallot::SetChoices(const QStringList & lsChoices)
	{
	DeleteChoicesAndVotes();	// Delete any previous choice(s)
	foreach(const QString & sChoice, lsChoices)
		{
		PAllocateNewChoice()->m_strQuestion = sChoice;
		}
	}
*/
void
IEventBallot::SetChoices(const QVariantList & lsChoices)
	{
	DeleteChoicesAndVotes();	// Delete any previous choice(s)
	foreach(const QVariant & vChoice, lsChoices)
		{
		PAllocateNewChoice()->m_strQuestion = vChoice.toString();
		}
	}

QVariantList
IEventBallot::LsGetChoices() CONST_MCC
	{
	QVariantList oList;
	_CEventBallotChoice ** ppChoiceStop;
	_CEventBallotChoice ** ppChoice = PrgpGetChoicesStopWithTally(OUT &ppChoiceStop);
	while (ppChoice != ppChoiceStop)
		{
		_CEventBallotChoice * pChoice = *ppChoice++;
		oList.append(QVariant::fromValue(new OJapiPollOption(pChoice)));	// This causes a memory... need to be fixed
		}
	return oList;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventBallotSent::CEventBallotSent(const TIMESTAMP * ptsEventID) : IEventBallot(ptsEventID)
	{
	}

//	CEventBallotSent::IEvent::XospDataE()
//
//	This virtual method is called every time someone casts a vote.
//	This method records the vote of the user and update the Chat Log.
EGui
CEventBallotSent::XospDataE(const CXmlNode * pXmlNodeData, INOUT CBinXcpStanza * pbinXospReply)
	{
	const UINT_BALLOT_CHOICES ukmChoices = pXmlNodeData->UFindAttributeValueHexadecimal_ZZR(d_chXMLa_CEventBallotReceived_uxVotedChoice);
	CStr strComment = pXmlNodeData->PszuFindAttributeValue(d_chXMLa_CEventBallotReceived_strNote);
	//MessageLog_AppendTextFormatCo(d_coRed, "CEventBallotSent::XospDataE() - $x\n", ukmChoices);

	TContact * pContact = pbinXospReply->m_pContact; // Get the contact who voted
	Assert(pContact != NULL);
	if (m_uFlagsBallot & FB_kfStopAcceptingVotes)
		{
		MessageLog_AppendTextFormatSev(eSeverityWarning, "Vote from ^j is rejected because the poll stopped\n", pContact);
		return eGui_NoUpdate;
		}
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
				return eGui_NoUpdate;		// This vote is a duplicate
			MessageLog_AppendTextFormatSev(eSeverityComment, "Ballot $t: Updating vote by ^j from 0x$x to 0x$x\n", m_tsEventID, pContact, pVote->m_ukmChoices, ukmChoices);
			goto UpdateChoices;
			}
		}
	// The contact never voted, therefore create a new entry
	MessageLog_AppendTextFormatSev(eSeverityComment, "Ballot $t: New vote by ^j: 0x$x\n", m_tsEventID, pContact, ukmChoices);
	pVote = PAllocateNewVote();
	pVote->m_pContact = pContact;
	UpdateChoices:
	pVote->m_ukmChoices = ukmChoices;
	pVote->m_strComment = strComment;
	pVote->m_tsVote = Timestamp_GetCurrentDateTime();
	m_pVaultParent_NZ->SetModified();
	m_uFlagsBallot &= ~FB_kfVotesTailied;	// Force a recount of the votes

	if (m_uFlagsBallot & FB_kfFromBallotmaster)
		{
		// The ballot sent was created by the Ballotmaster, therefore notify every Ballotmaster instance
		OJapiAppBallotmaster * pBallotmaster = OJapiAppBallotmaster::s_plistBallotmasters;
		while (pBallotmaster != NULL)
			{
			pBallotmaster->OnEventVoteReceived(this);
			pBallotmaster = pBallotmaster->m_pNext;
			}
		}
	// pContact->PGetProfile()->BallotMaster_onEventVoteReceived(this);

	return eGui_zUpdate;
	} // XospDataE()

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
	pbinXmlAttributes->BinAppendXmlAttributeUIntHexadecimal(d_chXMLa_CEventBallotReceived_uxVotedChoice, m_ukmChoices);	// Save the attribute before the rest of the ballot, because the ballot includes XML elements
	pbinXmlAttributes->BinAppendXmlAttributeTimestamp(d_chXMLa_CEventBallotReceived_tsConfirmationReceipt, m_tsConfirmationReceipt);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chXMLa_CEventBallotReceived_strNote, m_strComment);
	return IEventBallot::XmlSerializeCoreE(IOUT pbinXmlAttributes);
	}

//  CEventBallotReceived::IEvent:XmlUnserializeCore()
void
CEventBallotReceived::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	pXmlNodeElement->UpdateAttributeValueUIntHexadecimal(d_chXMLa_CEventBallotReceived_uxVotedChoice, OUT_F_UNCH &m_ukmChoices);
	pXmlNodeElement->UpdateAttributeValueTimestamp(d_chXMLa_CEventBallotReceived_tsConfirmationReceipt, OUT_F_UNCH &m_tsConfirmationReceipt);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chXMLa_CEventBallotReceived_strNote, OUT_F_UNCH &m_strComment);
	IEventBallot::XmlUnserializeCore(pXmlNodeElement);
	}

PSZUC
CEventBallotReceived::PszGetTextOfEventForSystemTray(OUT_IGNORE CStr * pstrScratchBuffer) const
	{
	return pstrScratchBuffer->Format("Please vote: $S\n$S", &m_strTitle, &m_strDescription);
	}

#include "XcpApi.h"

/*
//	Update the GUI and send the ballot via XOSP
void
CEventBallotReceived::UpdateBallotChoices(UINT_BALLOT_CHOICES ukmChoices, WEditTextArea * pwEditComments)
	{
	SubmitVoteViaXosp(*pwEditComments;
	if (strComments.FCompareBinary(m_strComment) && ukmChoices == m_ukmChoices)
		return;	// Nothing changed
	m_ukmChoices = ukmChoices;
	m_strComment = strComments;
	m_pVaultParent_NZ->SetModified();
	ChatLog_UpdateEventWithinSelectedChatLogFromNavigationTree();
	}
*/

BOOL
CEventBallotReceived::SubmitVoteViaXospF(UINT_BALLOT_CHOICES ukmChoices, const CStr & strFeedbackComments)
	{
	if (strFeedbackComments.FCompareBinary(m_strComment) && (ukmChoices != d_uzBallotChoiceInvalid) && (ukmChoices == m_ukmChoices))
		return FALSE;	// Nothing changed
	m_ukmChoices = ukmChoices;
	m_strComment = strFeedbackComments;
	m_pVaultParent_NZ->SetModified();
	// Send the selected choices to the ballot creator
	CBinXcpStanza binXcpStanza;
	binXcpStanza.XcpApi_SendDataToEvent_VE(this, "<v" d_szXMLa_CEventBallotReceived_uxVotedChoice_ux d_szXMLa_CEventBallotReceived_strNote "/>", m_ukmChoices, &m_strComment);
	return TRUE;
	}

CEventBallotAttatchment *
CEventBallotPoll::PAllocateNewAttatchment()
	{
	CEventBallotAttatchment *pAttatchment = new CEventBallotAttatchment(this);
	m_arraypaAtattchments.Add(PA_CHILD pAttatchment);
	return pAttatchment;
	}

CEventBallotPoll::CEventBallotPoll(const TIMESTAMP * ptsEventID) : CEventBallotSent(ptsEventID)
	{
	m_tsStarted = d_ts_zNA;
	m_tsStopped = d_ts_zNA;
	m_cSecondsPollLength = d_zNA;
	m_pEventBallotSent = NULL;
	m_uFlagsBallot |= FB_kfAllowNoComments;	// By default, do not allow feedback comments
	}

//  CEventBallotPoll::IEvent::XmlSerializeCoreE()
EXml
CEventBallotPoll::XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const
    {
	pbinXmlAttributes->BinAppendXmlAttributeTimestamp(d_chXMLa_CEventBallotPoll_tsStarted, m_tsStarted);
	pbinXmlAttributes->BinAppendXmlAttributeTimestamp(d_chXMLa_CEventBallotPoll_tsStopped, m_tsStopped);
	pbinXmlAttributes->BinAppendXmlAttributeInt(d_chXMLa_CEventBallotPoll_cSecondsPollLength, m_cSecondsPollLength);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chXMLa_CEventBallotPoll_strTargetId, m_strTargetIdentifier);

	EXml eXml = CEventBallotSent::XmlSerializeCoreE(IOUT pbinXmlAttributes);	// Serialize the base class and its XML elements
	Assert(eXml == eXml_ElementPresent);

	#if 1
	// BEGIN serialize attatchments
	pbinXmlAttributes->BinAppendText("<A>");
	CEventBallotAttatchment **ppBallotAttatchmentStop;
	CEventBallotAttatchment **ppBallotAttatchment = m_arraypaAtattchments.PrgpGetAttatchmentsStop(&ppBallotAttatchmentStop);
	while ( ppBallotAttatchment != ppBallotAttatchmentStop)
		{
		CEventBallotAttatchment *pBallotAttatchment = *ppBallotAttatchment++;
		pbinXmlAttributes->BinAppendText_VE("<a n='^S' m='^S' b='{B|}' />", &pBallotAttatchment->m_strName, &pBallotAttatchment->m_strMimeType, &pBallotAttatchment->m_binContent );
		}
	pbinXmlAttributes->BinAppendText("</A>");
	// END serialize attatchments
	#endif

	return eXml_ElementPresent;
    }

//  CEventBallotPoll::IEvent:XmlUnserializeCore()
void
CEventBallotPoll::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
    {
	pXmlNodeElement->UpdateAttributeValueTimestamp(d_chXMLa_CEventBallotPoll_tsStarted, OUT_F_UNCH &m_tsStarted);
	pXmlNodeElement->UpdateAttributeValueTimestamp(d_chXMLa_CEventBallotPoll_tsStopped, OUT_F_UNCH &m_tsStopped);
	pXmlNodeElement->UpdateAttributeValueInt(d_chXMLa_CEventBallotPoll_cSecondsPollLength, OUT_F_UNCH &m_cSecondsPollLength);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chXMLa_CEventBallotPoll_strTargetId, OUT_F_UNCH &m_strTargetIdentifier);

	const CXmlNode * pXmlNodeAttatchments = pXmlNodeElement->PFindElement('A');
	if (pXmlNodeAttatchments != NULL)
		{
		const CXmlNode * pXmlNodeAttatchment = pXmlNodeAttatchments ->m_pElementsList;
		while (pXmlNodeAttatchment != NULL)
			{
			CEventBallotAttatchment *pAttatchment = PAllocateNewAttatchment();
			pXmlNodeAttatchment->UpdateAttributeValueCStr('n', OUT_F_UNCH &pAttatchment->m_strName);
			pXmlNodeAttatchment->UpdateAttributeValueCStr('m', OUT_F_UNCH &pAttatchment->m_strMimeType);
			pXmlNodeAttatchment->UpdateAttributeValueCBin('b', OUT_F_UNCH &pAttatchment->m_binContent);
			pXmlNodeAttatchment = pXmlNodeAttatchment->m_pNextSibling;
			}
		}
    CEventBallotSent::XmlUnserializeCore(pXmlNodeElement);
	}


CEventBallotAttatchment::CEventBallotAttatchment(CEventBallotPoll *pPollParent)
	{
	m_pPollParent = pPollParent;
	m_paoJapiAttatchment = NULL;
	}

CEventBallotAttatchment::CEventBallotAttatchment(CEventBallotPoll * pPollParent, const CBin & /*binContent*/, const CStr & strName, const CStr & strMimeType)
	{
	m_pPollParent = pPollParent;
	m_strName = strName;
	m_strMimeType = strMimeType;
	m_paoJapiAttatchment = NULL;
	}

CEventBallotAttatchment::~CEventBallotAttatchment()
	{
	}

POJapi
CEventBallotAttatchment::POJapiGet()
	{
	if (m_paoJapiAttatchment == NULL)
		m_paoJapiAttatchment = new OJapiPollAttatchment(this);
	return m_paoJapiAttatchment;
	}



#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TApplicationBallotmaster.h"
#include "WLayoutBrowser.h"


void
DisplayApplicationBallotMaster()
	{
	TProfile * pProfile = NavigationTree_PGetSelectedTreeItemMatchingInterfaceTProfile();
	if (pProfile == NULL)
		return;
	/*
	TApplicationBallotmaster * pApplication = pProfile->PGetApplicationBallotmaster_NZ();
	pApplication->TreeItemW_SelectWithinNavigationTree();
	*/

	CStr strPathApplication = "file:///" + pProfile->m_pConfigurationParent->SGetPathOfFileName("Apps/Ballotmaster/default.htm");
	TBrowser * pBrowser = new TBrowser(pProfile);
	pProfile->m_arraypaBrowsers.Add(PA_CHILD pBrowser);
	pBrowser->SetIconNameAndUrl(eMenuAction_DisplayBallotMaster, "Ballots", strPathApplication);
	pBrowser->TreeItemBrowser_DisplayWithinNavigationTree();
	pBrowser->TreeItemW_SelectWithinNavigationTree();
	}


CServiceBallotmaster::CServiceBallotmaster(TProfile * pProfileParent) : IService(pProfileParent), m_oVaultBallots(pProfileParent->PGetContactDummy_NZ())
	{
	}

//	CServiceBallotmaster::IXmlExchange::XmlExchange()
void
CServiceBallotmaster::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	IService::XmlExchange(pXmlExchanger);
	m_oVaultBallots.XmlExchange("Ballots", INOUT pXmlExchanger);
	}

//	CServiceBallotmaster::IService::DetachFromObjectsAboutBeingDeleted()
void
CServiceBallotmaster::DetachFromObjectsAboutBeingDeleted()
	{

	}

CEventBallotPoll *
CServiceBallotmaster::PAllocateBallot(const IEventBallot * pEventBallotTemplate)
	{
	CEventBallotPoll * paEventBallot = new CEventBallotPoll;
	paEventBallot->m_pVaultParent_NZ = &m_oVaultBallots;
	if (pEventBallotTemplate != NULL)
		{
		MessageLog_AppendTextFormatSev(eSeverityNoise, "CServiceBallotmaster::PAllocateBallot() - Template $t\n", pEventBallotTemplate->m_tsEventID);
		Assert(pEventBallotTemplate->m_pVaultParent_NZ != NULL);
		CBinXcpStanzaEventCopier binXcpStanzaCopier(m_oVaultBallots.m_pParent);
		binXcpStanzaCopier.EventCopy(IN pEventBallotTemplate, OUT paEventBallot);
		paEventBallot->m_tsStarted = paEventBallot->m_tsStopped = d_zNA;
		paEventBallot->m_uFlagsEvent |= IEvent::FE_kfEventDeleted;
		}
	m_oVaultBallots.m_arraypaEvents.Add(PA_CHILD paEventBallot);
	return paEventBallot;
	}

//	Make a copy of an existing ballot and add it to the Ballotmaster
void
CServiceBallotmaster::EventBallotAddAsTemplate(IEventBallot * pEventBallot)
	{
	Assert(pEventBallot != NULL);
	(void)PAllocateBallot(pEventBallot);
	}

void
TProfile::BallotMaster_EventBallotAddAsTemplate(IEventBallot * pEventBallot)
	{
	PGetServiceBallotmaster_NZ()->EventBallotAddAsTemplate(pEventBallot);
	}

void
TProfile::BallotMaster_OnEventNewBallotReceived(CEventBallotReceived * pEventBallotReceived)
	{
	Assert(pEventBallotReceived->EGetEventClass() == CEventBallotReceived::c_eEventClass);
	m_arraypEventsRecentBallots.AddEvent(pEventBallotReceived);
	Dashboard_NewEventRelatedToBallot(pEventBallotReceived);

	OJapiAppBallotmaster * pBallotmaster = OJapiAppBallotmaster::s_plistBallotmasters;
	while (pBallotmaster != NULL)
		{
		pBallotmaster->OnEventBallotReceived(pEventBallotReceived);
		pBallotmaster = pBallotmaster->m_pNext;
		}
	}

/*
void
TProfile::BallotMaster_onEventVoteReceived(CEventBallotSent * pEventBallotSent)
	{
	Assert(pEventBallotSent->EGetEventClass() == CEventBallotSent::c_eEventClass);
	}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
OJapiBallot::OJapiBallot(IEventBallot * pEventBallot)
	{
	Assert(pEventBallot != NULL);
	m_pEventBallot = pEventBallot;
	}

QString
OJapiBallot::id() const
	{
	return Timestamp_ToStringBase85(m_pEventBallot->m_tsEventID);
	}

QString
OJapiBallot::type() const
	{
	return (m_pEventBallot->EGetEventClass() == CEventBallotReceived::c_eEventClass) ? "ballot" : "poll";
	}

//	submit(), slot
//	This method should be called only for received ballots to send the vote back to the poll sender.
void
OJapiBallot::submit(int kmOptions, const QString & sComment)
	{
	//MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "OJapiBallot::submit() - Event ID $t: $x $Q\n", m_pEventBallot->m_tsEventID, kmOptions, &sComment);
	if (m_pEventBallot->EGetEventClass() == CEventBallotReceived::c_eEventClass)
		{
		((CEventBallotReceived *)m_pEventBallot)->SubmitVoteViaXospF(kmOptions, sComment);
		return;
		}
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "OJapiBallot::submit() - Event ID $t is not allowed to vote\n", m_pEventBallot->m_tsEventID);
	}

QString
OJapiBallot::originator() const
	{
	if (m_pEventBallot->EGetEventClass() == CEventBallotReceived::c_eEventClass)
		{
		TContact * pContactSender = m_pEventBallot->PGetContactForReply_YZ();
		if (pContactSender != NULL)
			return pContactSender->TreeItem_SGetNameDisplay();
		}
	return m_pEventBallot->PGetProfile()->m_strNameProfile;	// At the moment, the originator is the profile/role who sent the ballot
	}

QString
OJapiBallot::status() const
	{
	Assert(m_pEventBallot != NULL);
	const EEventClass eEventClass = m_pEventBallot->EGetEventClass();
	if (eEventClass == CEventBallotReceived::c_eEventClass)
		{
		return ((((CEventBallotReceived *)m_pEventBallot)->m_ukmChoices) == 0) ? "unvoted" : "voted";
		}
	Assert(eEventClass == CEventBallotPoll::c_eEventClass);
	if (eEventClass == CEventBallotPoll::c_eEventClass)
		{
		return ((CEventBallotPoll *)m_pEventBallot)->PszGetStatus();
		}
	MessageLog_AppendTextFormatCo(d_coBlue, "OJapiBallot::OJapiBallot() - returning an empty string for Event ID $t\n", m_pEventBallot->m_tsEventID);
	return c_sEmpty;
	}

PSZAC
CEventBallotPoll::PszGetStatus() const
	{
	if (m_tsStopped != d_ts_zNA)
		return "stopped";
	if (m_tsStarted != d_ts_zNA)
		return "started";
	return (m_uFlagsEvent & FE_kfEventDeleted) ? "unsaved" : "unstarted";
	}

QString
OJapiBallot::title() const
	{
	return m_pEventBallot->m_strTitle;
	}
void
OJapiBallot::title(const QString & sTitle)
	{
	m_pEventBallot->m_strTitle = sTitle;
	}

QString
OJapiBallot::description() const
	{
	return m_pEventBallot->m_strDescription;
	}
void
OJapiBallot::description(const QString & sDescription)
	{
	m_pEventBallot->m_strDescription = sDescription;
	}

QVariantList
OJapiBallot::options() const
	{
	Assert(m_pEventBallot != NULL);
	IEventBallot * pEvent = m_pEventBallot;
	if (pEvent->EGetEventClass() == CEventBallotPoll::c_eEventClass)
		{
		// Fetching the options is a bit more complex because we also want the vote tally/counts.
		// Therefore we have to see if there has been a ballot sent, and get the options from there.
		pEvent = ((CEventBallotPoll *)pEvent)->PGetEventBallotSend_YZ();
		if (pEvent == NULL)
			pEvent = m_pEventBallot;	// Use the poll template
		}
	return pEvent->LsGetChoices();
	}

void
OJapiBallot::options(const QVariantList & lsOptions)
	{
	return m_pEventBallot->SetChoices(lsOptions);
	}
bool
OJapiBallot::allowComments() const
	{
	return ((m_pEventBallot->m_uFlagsBallot & IEventBallot::FB_kfAllowNoComments) == 0);
	}
void
OJapiBallot::allowComments(bool fAllowComments)
	{
	if (fAllowComments)
		m_pEventBallot->m_uFlagsBallot &= ~IEventBallot::FB_kfAllowNoComments;
	else
		m_pEventBallot->m_uFlagsBallot |= IEventBallot::FB_kfAllowNoComments;
	}

bool
OJapiBallot::allowMultipleChoices() const
	{
	return ((m_pEventBallot->m_uFlagsBallot & IEventBallot::FB_kfAllowMultipleChoices) != 0);
	}
void
OJapiBallot::allowMultipleChoices(bool fAllowMultipleChoices)
	{
	if (fAllowMultipleChoices)
		m_pEventBallot->m_uFlagsBallot |= IEventBallot::FB_kfAllowMultipleChoices;
	else
		m_pEventBallot->m_uFlagsBallot &= ~IEventBallot::FB_kfAllowMultipleChoices;
	}

bool
OJapiBallot::isTemplate()
	{
	return ((m_pEventBallot->m_uFlagsBallot & IEventBallot::FB_kfBallotmasterTemplate) != 0);
	}
void
OJapiBallot::isTemplate(bool fIsTemplate)
	{
	if (fIsTemplate)
		m_pEventBallot->m_uFlagsBallot |= IEventBallot::FB_kfBallotmasterTemplate;
	else
		m_pEventBallot->m_uFlagsBallot &= ~IEventBallot::FB_kfBallotmasterTemplate;
	}
bool
OJapiBallot::isSelected()
	{
	return ((m_pEventBallot->m_uFlagsBallot & IEventBallot::FB_kfBallotmasterTemplate) != 0);
	}
void
OJapiBallot::isSelected(bool fIsSelected)
	{
	if (fIsSelected)
		m_pEventBallot->m_uFlagsEvent |= IEvent::FE_kfEventSelected;
	else
		m_pEventBallot->m_uFlagsEvent &= ~IEvent::FE_kfEventSelected;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
OJapiPollCore::OJapiPollCore(CEventBallotPoll * pBallot) : OJapiBallot(pBallot)
	{
	Assert(pBallot != NULL);
    Assert(pBallot->EGetEventClass() == eEventClass_eBallotPoll);
	m_pBallot = pBallot;
	}

int
OJapiPollCore::pollTimeLength() const
	{
	//MessageLog_AppendTextFormatCo(d_coBlue, "OPoll::pollTimeLength get");
	return m_pBallot->m_cSecondsPollLength;
	}

void
OJapiPollCore::pollTimeLength(int cSeconds)
	{
	//MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "OPoll::pollTimeLength set");
	m_pBallot->m_cSecondsPollLength = cSeconds;
	}

QString
OJapiPollCore::dismissText() const
	{
	return m_pBallot->m_strButtonDismiss;
	}
void
OJapiPollCore::dismissText(const QString & sText)
	{
	m_pBallot->m_strButtonDismiss = sText;
	}

QString
OJapiPollCore::submitText() const
	{
	return m_pBallot->m_strButtonSubmit;
	}
void
OJapiPollCore::submitText(const QString & sText)
	{
	m_pBallot->m_strButtonSubmit = sText;
	}

void
OJapiPollCore::pollTargetId(const CString & sTargetId)
	{
	m_pBallot->m_strTargetIdentifier = sTargetId;
	}
QString
OJapiPollCore::pollTargetId() const
	{
	return m_pBallot->m_strTargetIdentifier;
	}

QDateTime
OJapiPollCore::dateStarted() const
	{
    return Timestamp_ToQDateTime(m_pBallot->m_tsStarted);
	}
QDateTime
OJapiPollCore::dateStopped() const
	{
    return Timestamp_ToQDateTime(m_pBallot->m_tsStopped);
	}

OJapiPollResultsStats::OJapiPollResultsStats(CEventBallotPoll * pBallot)
    {
    m_pBallot = pBallot;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////
OJapiPollOption::OJapiPollOption(_CEventBallotChoice * pChoice)
	{
	Assert(pChoice != NULL);
	m_pChoice = pChoice;
	}

QString
OJapiPollOption::text() const
	{
	return m_pChoice->m_strQuestion;
	}
int
OJapiPollOption::count() const
	{
	return m_pChoice->m_cVotes;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
OJapiPollResultsComment::OJapiPollResultsComment(_CEventBallotVote * pComment )
	{
    m_pComment = pComment;
	}

QDateTime
OJapiPollResultsComment::date()
    {
    return Timestamp_ToQDateTime(m_pComment->m_tsVote);
    }

QString
OJapiPollResultsComment::comment()
	{
    return m_pComment->m_strComment;
	}

QString
OJapiPollResultsComment::name()
	{
    return m_pComment->m_pContact->m_strJidBare;
	}


OJapiPollResults::OJapiPollResults(CEventBallotPoll * pBallot) : OJapiPollCore(pBallot), m_oStats(pBallot)
	{
	Assert(pBallot != NULL);
    }

//	Return a list of comments from those who voted
QVariant
OJapiPollResults::comments() const
    {
	//MessageLog_AppendTextFormatCo(d_coBlue, "OJapiPollResults::comments($t) m_pEventBallotSent=0x$p\n", &m_pBallot->m_tsEventID, m_pEventBallotSent);
    QVariantList lvComments;
	CEventBallotSent * pEventBallotSent = m_pBallot->PGetEventBallotSend_YZ();
	if (pEventBallotSent != NULL)
		{
		_CEventBallotVote ** ppVoteStop;
		_CEventBallotVote ** ppVote = pEventBallotSent->m_arraypaVotes.PrgpGetVotesStop(OUT &ppVoteStop);
		while (ppVote != ppVoteStop)
			{
			_CEventBallotVote * pVote = *ppVote++;
			//MessageLog_AppendTextFormatCo(d_coBlue, "OJapiPollResults::comments($t) - tsVote=$t, $S: $S\n", m_pBallot->m_tsEventID, pVote->m_tsVote, &pVote->m_pContact->m_strJidBare, &pVote->m_strComment);
			if (pVote->m_strComment.FIsEmptyString())
				continue;
			#if 1
			if (pVote->m_paoJapiPollResultsComment == NULL)
				pVote->m_paoJapiPollResultsComment = new OJapiPollResultsComment(pVote);
			lvComments.append(QVariant::fromValue(pVote->m_paoJapiPollResultsComment));
			#else
			lvComments.append(QVariant::fromValue(new OJapiPollResultsComment(pVote)));
			#endif
			}
		}
    return lvComments;
    }

QVariant
OJapiPollResults::counts() const
    {
    MessageLog_AppendTextFormatCo(d_coBlue, "OJapiPollResults::counts()\n");
    QVariantList list;
	CEventBallotSent * pEventBallotSent = m_pBallot->PGetEventBallotSend_YZ();
	if (pEventBallotSent != NULL)
		{
		_CEventBallotChoice ** ppChoiceStop;
		_CEventBallotChoice ** ppChoice = pEventBallotSent->PrgpGetChoicesStopWithTally(OUT &ppChoiceStop);
		while (ppChoice != ppChoiceStop)
			{
			_CEventBallotChoice * pChoice = *ppChoice++;
			list.append(pChoice->m_cVotes);
			}
		}
    return list;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////
OJapiPoll::OJapiPoll(CEventBallotPoll * pBallot) : OJapiPollCore(pBallot)
	{
	m_paoJapiPollResults = NULL;
	}

OJapiPoll::~OJapiPoll()
	{
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "OJapiPoll::~OJapiPoll() 0x$p\n", this);
	delete m_paoJapiPollResults;
	}

bool
OJapiPoll::save()
	{
	m_pBallot->m_uFlagsEvent &= ~IEvent::FE_kfEventDeleted;
	m_pBallot->PGetAccount_NZ()->PGetConfiguration()->XmlConfigurationSaveToFile();	// Force a save to make sure if the machine crashes, the poll have been saved

	QString sStatus = status();
	MessageLog_AppendTextFormatCo(d_coBlue, "OPoll::save($S) - status=$Q\n", &m_pBallot->m_strTitle, &sStatus);
	return true;
	}

void
OJapiPoll::destroy()
	{
	m_pBallot->m_uFlagsEvent |= IEvent::FE_kfEventDeleted;
	}

//	This method return a pointer to a group or a contact.
//	The name of the method uses the word 'group' because most polls are sent to groups.
ITreeItemChatLogEvents *
CEventBallotPoll::PGetGroupTarget_YZ()
	{
	PSZUC pszGroupIdentifier = m_strTargetIdentifier;
	Assert(pszGroupIdentifier != NULL);
	TAccountXmpp * pAccount = PGetAccount();
	ITreeItemChatLogEvents * pContactOrGroup = pAccount->Contact_PFindByJID(pszGroupIdentifier, eFindContact_zDefault);
	if (pContactOrGroup == NULL)
		pContactOrGroup = pAccount->Group_PFindByIdentifier_YZ(pszGroupIdentifier);
	if (pContactOrGroup != NULL)
		return pContactOrGroup;
	if (*pszGroupIdentifier != '\0')
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "CEventBallotPoll::PGetGroupTarget_YZ() - Invalid pollTargetId '$s'\n", pszGroupIdentifier);
	return NULL;
	}

CEventBallotSent *
CEventBallotPoll::PGetEventBallotSend_YZ() CONST_MCC
	{
	if ((m_pEventBallotSent == NULL) && (m_uFlagsEvent & FE_kfEventError) == 0)
		{
		// Attempt to find the ballot
		ITreeItemChatLogEvents * pGroup = PGetGroupTarget_YZ();
		if (pGroup != NULL)
			{
			m_pEventBallotSent = (CEventBallotSent *)pGroup->Vault_PGet_NZ()->PFindEventByID(m_tsStarted);
			if (m_pEventBallotSent == NULL || m_pEventBallotSent->EGetEventClass() != CEventBallotSent::c_eEventClass)
				{
				MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "CEventBallotPoll::PGetEventBallotSend_YZ($t) - Unable to find m_pEventBallotSent matching $t\n", m_tsEventID, m_tsStarted);
				m_pEventBallotSent = NULL;
				m_uFlagsEvent |= FE_kfEventError;
				}
			}
		}
	return m_pEventBallotSent;
	}


//	Return true if the poll was successfully started.
bool
CEventBallotPoll::FStartPoll()
	{
	m_tsStopped = d_ts_zNA;		// Clear this variable in case it is a resume of a poll
	m_uFlagsBallot &= ~FB_kfStopAcceptingVotes;
	if (m_tsStarted == d_ts_zNA)
		{
		Assert(m_pEventBallotSent == NULL);
		ITreeItemChatLogEvents * pGroup = PGetGroupTarget_YZ();
		if (pGroup == NULL)
			return false;
		m_tsStarted = Timestamp_GetCurrentDateTime();

		// To send a poll, we need to clone the template and add it to the Chat Log of the group (or contact)
		m_pEventBallotSent = new CEventBallotSent(IN &m_tsStarted);		// Create a ballot with the same Event ID
		m_pEventBallotSent->Event_InitFromDataOfEvent(this);
		m_pEventBallotSent->m_uFlagsBallot |= FB_kfFromBallotmaster;
		pGroup->Vault_AddEventToChatLogAndSendToContacts(PA_CHILD m_pEventBallotSent);
		}
	return true;
	}

void
CEventBallotPoll::StopPoll()
	{
	if (m_tsStarted != d_ts_zNA)
		m_tsStopped = Timestamp_GetCurrentDateTime();
	m_uFlagsBallot |= FB_kfStopAcceptingVotes;
	if (PGetEventBallotSend_YZ() != NULL)
		m_pEventBallotSent->m_uFlagsBallot |= FB_kfStopAcceptingVotes;
	}

bool
OJapiPoll::start()
	{
	return m_pBallot->FStartPoll();
	}

void
OJapiPoll::stop()
	{
	m_pBallot->StopPoll();
	}

//	Send the ballot to a group, or to a contact
//	This method is for debugging
bool
OJapiPoll::send(const QString & sGroupId)
	{
	// In order to send the poll to the group, we need to find the TGroup object
	CStr strGroupId = sGroupId;
	TAccountXmpp * pAccount = m_pBallot->PGetAccount();
	ITreeItemChatLogEvents * pContactOrGroup = pAccount->Contact_PFindByJID(strGroupId, eFindContact_zDefault);
	if (pContactOrGroup == NULL)
		pContactOrGroup = pAccount->Group_PFindByIdentifier_YZ(strGroupId);
	MessageLog_AppendTextFormatCo(d_coBlack, "OJapiPoll::send($Q) - pContactOrGroup = 0x$p\n", &sGroupId, pContactOrGroup);
	if (pContactOrGroup != NULL)
		{
		// To send the poll, we need to clone it and add it to the group (this code is a quite ugly at the moment, however it is for proof of concept)
		CEventBallotPoll * paEventBallot = new CEventBallotPoll;
		paEventBallot->m_pVaultParent_NZ = m_pBallot->m_pVaultParent_NZ;
		CBinXcpStanzaEventCopier binXcpStanzaCopier(m_pBallot->m_pVaultParent_NZ->m_pParent);
		binXcpStanzaCopier.EventCopy(IN m_pBallot, OUT paEventBallot);
		paEventBallot->m_pVaultParent_NZ = NULL;
		pContactOrGroup->Vault_AddEventToChatLogAndSendToContacts(PA_CHILD paEventBallot);
		#if 0
		paEventBallot->m_uFlagsBallot |= CEventBallotPoll::FB_kfStopAcceptingVotes; // Testing the stop()
		stop();
		//start();
		#endif
		return true;
		}
	return false;
	}

//	Submit a vote back to the sender of the poll
bool
OJapiPoll::submit()
	{
	return true;
	}


void OJapiAppBallotmaster::open()
    {
	//LaunchBrowser("Ballotmaster", "html5-pollmaster/index.html");
	LaunchApplication("Ballotmaster");
	//CStr strUrl = (PSZUC)"file:///C:/Users/Cesar/.Cambrian/Apps/Ballotmaster/default.htm";
	//MessageLog_AppendTextFormatCo(d_coGreen, "OJapiAppBallotmaster::go($S)", &strUrl);
	//m_pBallotmaster->m_pawLayoutBrowser->NavigateToAddress(strUrl);
    }

POJapiPollResults
OJapiPoll::getResults() CONST_MCC
	{
	if (m_paoJapiPollResults == NULL)
		m_paoJapiPollResults = new OJapiPollResults(m_pBallot);
	return m_paoJapiPollResults;
	}

QVariantList OJapiPoll::listAttatchments()
	{
	QVariantList list;
	CEventBallotAttatchment **ppBallotAttatchmentStop;
	CEventBallotAttatchment **ppBallotAttatchment = m_pBallot->m_arraypaAtattchments.PrgpGetAttatchmentsStop(&ppBallotAttatchmentStop);
	while ( ppBallotAttatchment != ppBallotAttatchmentStop)
		{
		CEventBallotAttatchment *pBallotAttatchment = *ppBallotAttatchment++;
		list.append(QVariant::fromValue(pBallotAttatchment->POJapiGet()));
		}

	return list;
	}

void OJapiPoll::addAttatchment(const QString &strName, const QString &strContentBase64, const QString strMimeType)
	{
	CEventBallotAttatchment *pAttatchment = m_pBallot->PAllocateNewAttatchment();
	pAttatchment->m_strMimeType = strMimeType;
	pAttatchment->m_strName = strName;
	pAttatchment->m_binContent.BinAppendBinaryDataFromBase64(strContentBase64);
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
OJapiAppBallotmaster * OJapiAppBallotmaster::s_plistBallotmasters;	// Declare the static variable

OJapiAppBallotmaster::OJapiAppBallotmaster(OJapiCambrian * poCambrian, const SApplicationHtmlInfo *pApplicationInfo) : OJapiAppInfo(pApplicationInfo)
	{
	Assert(poCambrian != NULL);
	m_pServiceBallotmaster = poCambrian->m_pProfile->PGetServiceBallotmaster_NZ();
	Assert(m_pServiceBallotmaster->EGetRuntimeClass() == RTI_SZ(CServiceBallotmaster));
	m_pNext = s_plistBallotmasters;	// Insert the new objec at the beginning of the list
	s_plistBallotmasters = this;
	}

OJapiAppBallotmaster::~OJapiAppBallotmaster()
	{
	MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "OJapiAppBallotmaster::~OJapiAppBallotmaster() 0x$p\n", this);	// For debugging, display in the Error Log when the object is destroyed (to make sure we have not missed it)
	// Remove the Ballotmaster from the list
	OJapiAppBallotmaster ** ppBallotmasterRemove = &s_plistBallotmasters;
	OJapiAppBallotmaster * pBallotmaster = s_plistBallotmasters;
	while (TRUE)
		{
		if (pBallotmaster == this)
			{
			*ppBallotmasterRemove = m_pNext;
			break;
			}
		ppBallotmasterRemove = &pBallotmaster->m_pNext;
		pBallotmaster = pBallotmaster->m_pNext;
		Assert(pBallotmaster != NULL && "The OJapiAppBallotmaster should be in the list");
		}
	}

CEventBallotPoll*
OJapiAppBallotmaster::PFindPollByID(TIMESTAMP tsIdPoll) const
	{
	//MessageLog_AppendTextFormatSev(eSeverityNoise, "OJapiAppBallotmaster::PFindPollByID($t)\n", tsIdPoll);
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_pServiceBallotmaster->m_oVaultBallots.m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		CEventBallotPoll * pEvent = (CEventBallotPoll *)*ppEvent++;
		Assert(pEvent->EGetEventClass() == CEventBallotPoll::c_eEventClass);
		if (pEvent->m_tsEventID == tsIdPoll && (pEvent->m_uFlagsEvent & IEvent::FE_kfEventDeleted) == 0)
			{
			//MessageLog_AppendTextFormatSev(eSeverityNoise, "OJapiAppBallotmaster::PFindPollByID($t) -> 0x$p $t\n", tsIdPoll, pEvent, pEvent->m_tsEventID);
			return pEvent;
			}
		}
	return NULL;
	}

CEventBallotPoll *
OJapiAppBallotmaster::PFindPollByID(const QString & sIdPoll) const
	{
	return PFindPollByID(Timestamp_FromStringW_ML(sIdPoll));
	}

CEventBallotPoll *
OJapiAppBallotmaster::PFindPollByTimeStarted(TIMESTAMP tsStarted) const
	{
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_pServiceBallotmaster->m_oVaultBallots.m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		CEventBallotPoll * pEvent = (CEventBallotPoll *)*--ppEventStop;
		Assert(pEvent->EGetEventClass() == CEventBallotPoll::c_eEventClass);
		if (pEvent->m_tsStarted == tsStarted)
			return pEvent;
		}
	MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "OJapiAppBallotmaster::PFindPollByTimeStarted() - Unable to find poll matching tsStarted $t\n", tsStarted);
	return NULL;
	}

void
OJapiAppBallotmaster::OnEventBallotReceived(CEventBallotReceived * pEventBallotReceived)
	{
	Assert(pEventBallotReceived != NULL);
	MessageLog_AppendTextFormatSev(eSeverityInfoTextBlack, "emit onEventBallotReceived($t)\n", pEventBallotReceived->m_tsEventID);
	emit onEventBallotReceived(PGetOJapiBallot(pEventBallotReceived));
	}

void
OJapiAppBallotmaster::OnEventVoteReceived(const CEventBallotSent * pEventBallotSent)
	{
	Assert(pEventBallotSent != NULL);
	if (pEventBallotSent->PGetProfile() == m_pServiceBallotmaster->m_pProfileParent)
		{
		// Find the event matching the Event ID
		CEventBallotPoll * pPoll = PFindPollByTimeStarted(pEventBallotSent->m_tsEventID);
		if (pPoll != NULL)
			{
			MessageLog_AppendTextFormatSev(eSeverityInfoTextBlack, "emit onEventVoteReceived($t)\n", pPoll->m_tsEventID);
			emit onEventVoteReceived(PGetOJapiPoll(pPoll));
			}
		}
	}

POJapiPoll
OJapiAppBallotmaster::PGetOJapiBallot(CEventBallotReceived * pBallot)
	{
	if (pBallot != NULL)
		return new OJapiBallot(pBallot);	//	This code causes a memory leak.  Until Qt has a reference counter of QObjects, Cambrian must provide a mechanism to minimize the memory leaks.
	return NULL;
	}

//	Return the JavaScript object for a ballot
POJapiPoll
OJapiAppBallotmaster::PGetOJapiPoll(CEventBallotPoll * pBallot)
	{
	if (pBallot != NULL)
		return new OJapiPoll(pBallot);	//	This code causes a memory leak.  Until Qt has a reference counter of QObjects, Cambrian must provide a mechanism to minimize the memory leaks.
	return NULL;
	}

POJapiPoll
OJapiAppBallotmaster::PCreateNewPollFromTemplate(CEventBallotPoll * pPollTemplate)
	{
	CEventBallotPoll * pPollNew = m_pServiceBallotmaster->PAllocateBallot(pPollTemplate);
	Assert(pPollNew != NULL);
	pPollNew->m_uFlagsEvent |= IEvent::FE_kfEventDeleted;	// When allocating a new poll, assume it has never been saved
	OJapiPoll * poJapiPoll = (OJapiPoll *)PGetOJapiPoll(pPollNew);
	Assert(poJapiPoll->status() == "unsaved");
	return poJapiPoll;
	}


//	build(), slot
//
//	Create a new poll
//
//	Ideally the name new() should be used, however since new() is a C++ operator (reserved keyword), we cannot use it.
//	Originally the name build() was chosen, however I think the name create() would be more appropriate.
POJapiPoll
OJapiAppBallotmaster::build()
	{
	//MessageLog_AppendTextFormatCo(d_coBlue, "OPolls::build()\n");
	return PCreateNewPollFromTemplate(NULL);
	}

//	Create a new poll based on a template
POJapiPoll
OJapiAppBallotmaster::build(QObject * pObjectPollTemplate)
	{
	OJapiPoll * pPollTemplate = qobject_cast<OJapiPoll *>(pObjectPollTemplate);	// Make sure we received an object of proper type
	return PCreateNewPollFromTemplate((pPollTemplate != NULL) ? pPollTemplate->m_pBallot : NULL);
	}

POJapiPoll
OJapiAppBallotmaster::build(const QString & sIdPollTemplate)
	{
	return PCreateNewPollFromTemplate(PFindPollByID(sIdPollTemplate));
	}

//	get(), slot
//	Return the poll matching the ID
POJapiPoll
OJapiAppBallotmaster::get(const QString & sIdPoll)
	{
	return PGetOJapiPoll(PFindPollByID(sIdPoll));
	}

//	getList(), slot
//	Return a list of polls and ballots.
QVariant
OJapiAppBallotmaster::getList()
	{
	Assert(m_pServiceBallotmaster != NULL);
	QVariantList oList;

	TProfile * pProfile = m_pServiceBallotmaster->m_pProfileParent;
	IEvent ** ppEventStop;
	IEvent ** ppEvent = pProfile->m_arraypEventsRecentBallots.PrgpGetEventsStopLast(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		CEventBallotReceived * pEventBallot = (CEventBallotReceived *)*ppEvent++;
		if (pEventBallot->EGetEventClass() == CEventBallotReceived::c_eEventClass)
			oList.append(QVariant::fromValue(PGetOJapiBallot(pEventBallot)));
		}

	ppEvent = m_pServiceBallotmaster->m_oVaultBallots.m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
        CEventBallotPoll * pEvent = (CEventBallotPoll *)*ppEvent++;
		if (pEvent->EGetEventClass() != eEventClass_eBallotPoll)
            continue;
		if ((pEvent->m_uFlagsEvent & IEvent::FE_kfEventDeleted) == 0)
			oList.append(QVariant::fromValue(PGetOJapiPoll(pEvent)));	// List only non-deleted polls
		}
	MessageLog_AppendTextFormatCo(d_coBlue, "OPolls::list() - $i elements\n", oList.size());
	return QVariant::fromValue(oList);
	}

/*
//	list(), slot
//
//	Return an XML of all the polls available
QString
OPolls::list() const
	{
	CBin binBallots;
	m_pBallotmaster->ApiBallotsList(OUT &binBallots);
	return binBallots.ToQString();
	}

//	save(), slot
//	Save the polls
void
OPolls::save(QString sXmlPolls)
	{
	CStr strXmlPolls = sXmlPolls;	// Convert to UTF-8
	m_pBallotmaster->ApiBallotSave(strXmlPolls);
	}
*/

OJapiAppInfo * PaAllocateJapiGeneric(SApplicationHtmlInfo * pInfo)
	{
	return new OJapiAppInfo(pInfo);
	}

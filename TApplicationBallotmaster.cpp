#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TApplicationBallotmaster.h"
#include "IEventBallot.h"
#include "WLayoutBrowser.h"

void
DisplayApplicationBallotMaster()
	{
	TProfile * pProfile = NavigationTree_PGetSelectedTreeItemMatchingInterfaceTProfile();
	if (pProfile == NULL)
		return;
	TApplicationBallotmaster * pApplication = pProfile->PGetApplicationBallotmaster_NZ();
	pApplication->TreeItemW_SelectWithinNavigationTree();
	/*
	CStr strPathApplication = "file:///" + g_oConfiguration.SGetPathOfFileName("Apps/Ballotmaster/default.htm");
	TBrowser * pBrowser = new TBrowser(&g_oConfiguration);
	g_oConfiguration.m_arraypaBrowsers.Add(PA_CHILD pBrowser);
	pBrowser->SetIconNameAndUrl(eMenuAction_DisplayBallotMaster, "Ballotmaster", strPathApplication);
	pBrowser->TreeItemBrowser_DisplayWithinNavigationTree();
	pBrowser->TreeItemW_SelectWithinNavigationTree();
	*/
	}


//	This function must have the same interface as PFn_PaAllocateApplication()
IApplication *
PaAllocateApplicationBallotmaster(TProfile * pProfileParent)
	{
	return new TApplicationBallotmaster(pProfileParent);
	}

TApplicationBallotmaster *
TProfile::PGetApplicationBallotmaster_NZ()
	{
	TApplicationBallotmaster * pApplication = (TApplicationBallotmaster *)m_arraypaApplications.PFindRuntimeObject(RTI(TApplicationBallotmaster));
	if (pApplication == NULL)
		{
		pApplication = (TApplicationBallotmaster *)PaAllocateApplicationBallotmaster(this);
		m_arraypaApplications.Add(PA_CHILD pApplication);
		pApplication->TreeItemApplication_DisplayWithinNavigationTree();
		}
	return pApplication;
	}


CBinXcpStanzaEventCopier::CBinXcpStanzaEventCopier(ITreeItemChatLogEvents * pContactOrGroup) : CBinXcpStanza(eStanzaType_zInformation)
	{
	TAccountXmpp * pAccount = pContactOrGroup->m_pAccount;
	m_pContact = m_paContact = new TContact(pAccount);	// We will serialize using a dummy contact
	m_pContact->m_strNameDisplayTyped = m_pContact->m_pAccount->m_pProfileParent->m_strNameProfile;	// Use the profile name as the contact so the preview looks like someone is receiving the event from the sender
	}

CBinXcpStanzaEventCopier::~CBinXcpStanzaEventCopier()
	{
	delete m_paContact;
	}

void
CBinXcpStanzaEventCopier::EventCopy(IN const IEvent * pEventSource, OUT IEvent * pEventDestination)
	{
	Assert(pEventSource != NULL);
	Assert(pEventSource->m_pVaultParent_NZ != NULL);
	BinXmlSerializeEventForXcpCore(pEventSource, d_ts_zNA);	// TODO: Need to use XmlSerializeCore() instead
	MessageLog_AppendTextFormatCo(d_coOrange, "CBinXcpStanzaEventCopier::EventCopy(): $B\n", this);

	CXmlTree oXmlTree;
	(void)oXmlTree.EParseFileDataToXmlNodesCopy_ML(IN *this);
	pEventDestination->m_pVaultParent_NZ = pEventSource->m_pVaultParent_NZ;	// Use the same vault as the source event
	pEventDestination->XmlUnserializeCore(IN &oXmlTree);
	}

IEvent *
CBinXcpStanzaEventCloner::PaEventClone(IEvent * pEventToClone)
	{
	Assert(pEventToClone != NULL);

	/*
		CVaultEvents * pVault = m_pContact->Vault_PGet_NZ();	// Get an empty vault from the dummy contact
	paEventSent->m_pVaultParent_NZ = pVault;				// We need a valid pointer because the event may need to access the vault, contact or account
	*/
	return 0;
	}

void
TProfile::BallotMaster_EventBallotAddAsTemplate(IEventBallot * pEventBallot)
	{
	PGetApplicationBallotmaster_NZ()->EventBallotAddAsTemplate(pEventBallot);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
TApplicationBallotmaster::TApplicationBallotmaster(TProfile * pProfileParent) : IApplication(pProfileParent, eMenuIconVote)
	{
	m_paVaultBallots = NULL;
	m_paContactDummy = NULL;
	m_pawLayoutBrowser = NULL;
	}

TApplicationBallotmaster::~TApplicationBallotmaster()
	{
	delete m_pawLayoutBrowser;
	delete m_paContactDummy;
	}

void
TApplicationBallotmaster::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	IApplication::XmlExchange(INOUT pXmlExchanger);
	pXmlExchanger->XmlExchangeStr("URL", INOUT &m_strUrlAddress);
	CVaultEvents * pVault = PGetVault_NZ();
	pVault->XmlExchange("Ballots", INOUT pXmlExchanger);
	}

const EMenuActionByte c_rgzeActionsMenuApplication[] =
	{
	eMenuAction_ApplicationHide,
	ezMenuActionNone
	};

//	TApplicationBallotmaster::ITreeItem::TreeItem_MenuAppendActions()
void
TApplicationBallotmaster::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionsAdd(c_rgzeActionsMenuApplication);
	}

//	TApplicationBallotmaster::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TApplicationBallotmaster::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_ApplicationHide:
		TreeItemW_Hide();
		return ezMenuActionNone;
	default:
		return IApplication::TreeItem_EDoMenuAction(eMenuAction);
		}
	}

//	TApplicationBallotmaster::ITreeItem::TreeItem_GotFocus()
void
TApplicationBallotmaster::TreeItem_GotFocus()
	{
	if (m_strUrlAddress.FIsEmptyString())
		m_strUrlAddress = "file:///" + m_pProfileParent->m_pConfigurationParent->SGetPathOfFileName("Apps/Ballotmaster/default.htm");
	if (m_pawLayoutBrowser == NULL)
		m_pawLayoutBrowser = new WLayoutBrowser(m_pProfileParent, INOUT_LATER &m_strUrlAddress);
	MainWindow_SetCurrentLayout(IN m_pawLayoutBrowser);
	}


//	This method is somewhat a hack to create a vault to store events. At the moment, Cambrian needs something to store the ballots.
CVaultEvents *
TApplicationBallotmaster::PGetVault_NZ()
	{
	if (m_paVaultBallots == NULL)
		{
		if (m_paContactDummy == NULL)
			{
			TAccountXmpp * pAccount = (TAccountXmpp *)m_pProfileParent->m_arraypaAccountsXmpp.PvGetElementFirst_YZ();
			if (pAccount == NULL)
				pAccount = new TAccountXmpp(m_pProfileParent);	// This will cause a memory leak.  At the moment, I just want the code to work as a proof of concept
			m_paContactDummy = new TContact(pAccount);
			}
		SHashSha1 hashFileName;
		InitToZeroes(OUT &hashFileName, sizeof(hashFileName));	// This also needs to be fixed
		m_paVaultBallots = new CVaultEvents(m_paContactDummy, IN &hashFileName);
		}
	return m_paVaultBallots;

	}
CEventBallotPoll *
TApplicationBallotmaster::PAllocateBallot(const IEventBallot * pEventBallotTemplate)
	{
	CVaultEvents * pVault = PGetVault_NZ();	// Get the vault first because it will initialize m_paContactDummy
    CEventBallotPoll * paEventBallot = new CEventBallotPoll;
	paEventBallot->m_pVaultParent_NZ = pVault;
	if (pEventBallotTemplate != NULL)
		{
		Assert(pEventBallotTemplate->m_pVaultParent_NZ != NULL);
		CBinXcpStanzaEventCopier binXcpStanzaCopier(m_paContactDummy);
		binXcpStanzaCopier.EventCopy(IN pEventBallotTemplate, OUT paEventBallot);
		}
    //Assert(paEventBallot->m_pVaultParent_NZ == pVault);
	pVault->m_arraypaEvents.Add(PA_CHILD paEventBallot);
	//MessageLog_AppendTextFormatSev(eSeverityNoise, "TApplicationBallotmaster::PAllocateBallot(0x$p) - Returning $t\n", pEventBallotTemplate, paEventBallot->m_tsEventID);
	return paEventBallot;
	}

//	Make a copy of an existing ballot and add it to the Ballotmaster
void
TApplicationBallotmaster::EventBallotAddAsTemplate(IEventBallot * pEventBallot)
	{
	Assert(pEventBallot != NULL);
	(void)PAllocateBallot(pEventBallot);
	/*
	CBinXcpStanzaEventCopier binXcpStanzaCopier(m_paContactDummy);
	CEventBallotSent * paEventBallotTemplate = new CEventBallotSent();	// When adding a template, always use the 'sent' ballot
	binXcpStanzaCopier.EventCopy(IN pEventBallot, OUT paEventBallotTemplate);
	paEventBallotTemplate->m_pVaultParent_NZ = pVault;
	pVault->m_arraypaEvents.Add(PA_CHILD paEventBallotTemplate);
	*/
	}

void
TApplicationBallotmaster::ApiBallotSave(IN PSZUC pszXmlBallot)
	{
	CVaultEvents * pVault = PGetVault_NZ();
	pVault->EventsUnserialize(pszXmlBallot);
	}

void
TApplicationBallotmaster::ApiBallotsList(OUT CBin * pbinXmlBallots)
	{
	CBinXcpStanzaTypeInfo binXmlEvents;
	CVaultEvents * pVault = PGetVault_NZ();
	pVault->EventsSerializeForMemory(IOUT &binXmlEvents);
	pbinXmlBallots->BinInitFromCBinStolen(INOUT &binXmlEvents);
	}



///////////////////////////////////////////////////////////////////////////////////////////////////
OJapiPollCore::OJapiPollCore(CEventBallotPoll * pBallot)
	{
	Assert(pBallot != NULL);
    Assert(pBallot->EGetEventClass() == eEventClass_eBallotPoll);
	m_pBallot = pBallot;
	}

QString
OJapiPollCore::id() const
	{
    Assert(m_pBallot->EGetEventClass() == eEventClass_eBallotPoll);
	QString s = Timestamp_ToStringBase85(m_pBallot->m_tsEventID);
	//MessageLog_AppendTextFormatCo(d_coBlue, "OPoll::id($t) returns $Q\n", m_pBallot->m_tsEventID, &s);
	return s;
	}

QString
OJapiPollCore::status() const
	{
    if ( m_pBallot->m_tsStopped != d_ts_zNA )
        return "stopped";

    if ( m_pBallot->m_tsStarted != d_ts_zNA )
        return "started";

	//return (m_pBallot->m_uFlagsEvent & IEvent::FE_kfEventDeleted) ? "deleted" : "unstarted";
	return "unsaved";
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
OJapiPollCore::title() const
	{
	return m_pBallot->m_strTitle;
	}
void
OJapiPollCore::title(const QString & sTitle)
	{
	MessageLog_AppendTextFormatCo(d_coBlue, "OPoll::title($Q)\n", &sTitle);
	m_pBallot->m_strTitle = sTitle;
	}

QString
OJapiPollCore::description() const
	{
	return m_pBallot->m_strDescription;
	}
void
OJapiPollCore::description(const QString & sDescription)
	{
	m_pBallot->m_strDescription = sDescription;
	}
QStringList
OJapiPollCore::options() const
	{
	return m_pBallot->LsGetChoices();
	}
void
OJapiPollCore::options(const QStringList & lsChoices)
	{
	return m_pBallot->SetChoices(lsChoices);
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


OJapiPollResultsComment::OJapiPollResultsComment (_CEventBallotVote * pComment )
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

    }

QVariant
OJapiPollResults::comments() const
    {
    //MessageLog_AppendTextFormatCo(d_coBlue, "OJapiPollResults::comments($t)\n", &m_pBallot->m_tsEventID);
    QVariantList lvComments;

    _CEventBallotVote ** ppVoteStop;
    _CEventBallotVote ** ppVote = m_pBallot->m_arraypaVotes.PrgpGetVotesStop(OUT &ppVoteStop);
    while (ppVote != ppVoteStop)
        {
        _CEventBallotVote * pVote = *ppVote++;
        MessageLog_AppendTextFormatCo(d_coBlue, "comment: $t $S $S\n", pVote->m_tsVote, &pVote->m_pContact->m_strJidBare, &pVote->m_strComment);
        if ( pVote->m_strComment.FIsEmptyString() )
            continue;
        lvComments.append(QVariant::fromValue(new OJapiPollResultsComment(pVote)) );
        }

    return lvComments;
    }

QVariant
OJapiPollResults::counts() const
    {
    MessageLog_AppendTextFormatCo(d_coBlue, "OJapiPollResults::counts()\n");
    QVariantList list;
    //list.append(123);
    //list.append(456);
    return list;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////
OJapiPoll::OJapiPoll(CEventBallotPoll * pBallot) : OJapiPollCore(pBallot), m_oResults(pBallot)
	{
	}

OJapiPoll::~OJapiPoll()
	{
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "OJapiPoll::~OJapiPoll() 0x$p\n", this);
	}

bool
OJapiPoll::save()
	{
	MessageLog_AppendTextFormatCo(d_coBlue, "OPoll::save($S)\n", &m_pBallot->m_strTitle);
	m_pBallot->m_uFlagsEvent &= ~IEvent::FE_kfEventDeleted;
	m_pBallot->PGetAccount_NZ()->PGetConfiguration()->XmlConfigurationSaveToFile();	// Force a save to make sure if the machine crashes, the poll have been saved
	return true;
	}

void
OJapiPoll::destroy()
	{
	m_pBallot->m_uFlagsEvent |= IEvent::FE_kfEventDeleted;
	}

void
OJapiPoll::start()
	{
    m_pBallot->m_tsStarted = Timestamp_GetCurrentDateTime();
    m_pBallot->m_tsStopped = d_ts_zNA;
	}

void
OJapiPoll::stop()
	{
    if ( m_pBallot->m_tsStarted != d_ts_zNA )
        m_pBallot->m_tsStopped = Timestamp_GetCurrentDateTime();
	}

POJapiPollResults
OJapiPoll::getResults() CONST_MCC
	{
	return &m_oResults;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
OJapiPolls::OJapiPolls(OJapiCambrian * poCambrian)
	{
	Assert(poCambrian != NULL);
	m_pBallotmaster = poCambrian->m_pProfile->PGetApplicationBallotmaster_NZ();
	Assert(m_pBallotmaster->EGetRuntimeClass() == RTI(TApplicationBallotmaster));
	}

OJapiPolls::~OJapiPolls()
	{
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "OPolls::~OPolls() 0x$p\n", this);	// Display in the Error Log (to make sure we have not missed it)
	}

CEventBallotPoll*
OJapiPolls::PFindPollByID(TIMESTAMP tsIdPoll) const
	{
	//MessageLog_AppendTextFormatSev(eSeverityNoise, "OJapiPolls::PFindPollByID($t)\n", tsIdPoll);
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_pBallotmaster->PGetVault_NZ()->m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
        CEventBallotPoll* pEvent = (CEventBallotPoll *)*ppEvent++;
        //Assert(pEvent->EGetEventClass() == CEventBallotPoll::c_eEventClass);
		if (pEvent->m_tsEventID == tsIdPoll && (pEvent->m_uFlagsEvent & IEvent::FE_kfEventDeleted) == 0)
			{
			//MessageLog_AppendTextFormatSev(eSeverityNoise, "OJapiPolls::PFindPollByID($t) -> 0x$p $t\n", tsIdPoll, pEvent, pEvent->m_tsEventID);
			return pEvent;
			}
		}
	return NULL;
	}

CEventBallotPoll *
OJapiPolls::PFindPollByID(const QString & sIdPoll) const
	{
	return PFindPollByID(Timestamp_FromStringW_ML(sIdPoll));
	}

//	Return the JavaScript object for a ballot
POJapiPoll
OJapiPolls::PGetOJapiPoll(CEventBallotPoll * pBallot)
	{
	if (pBallot != NULL)
		return new OJapiPoll(pBallot);	//	This code causes a memory leak.  Until Qt has a reference counter of QObjects, Cambrian must provide a mechanism to minimize the memory leaks.
	return NULL;
	}

POJapiPoll
OJapiPolls::PCreateNewPollFromTemplate(CEventBallotPoll * pPollTemplate)
	{
    CEventBallotPoll * pPollNew = m_pBallotmaster->PAllocateBallot(pPollTemplate);
	Assert(pPollNew != NULL);
	pPollNew->m_uFlagsEvent |= IEvent::FE_kfEventDeleted;	// When allocating a new poll, assume it has never been saved
	return PGetOJapiPoll(pPollNew);
	}


//	build(), slot
//
//	Create a new poll
//
//	Ideally the name new() should be used, however since new() is a C++ operator (reserved keyword), we cannot use it.
//	Originally the name build() was chosen, however I think the name create() would be more appropriate.
POJapiPoll
OJapiPolls::build()
	{
	//MessageLog_AppendTextFormatCo(d_coBlue, "OPolls::build()\n");
	return PCreateNewPollFromTemplate(NULL);
	}

//	Create a new poll based on a template
POJapiPoll
OJapiPolls::build(QObject * pObjectPollTemplate)
	{
	OJapiPoll * pPollTemplate = qobject_cast<OJapiPoll *>(pObjectPollTemplate);	// Make sure we received an object of proper type
	return PCreateNewPollFromTemplate((pPollTemplate != NULL) ? pPollTemplate->m_pBallot : NULL);
	}

POJapiPoll
OJapiPolls::build(const QString & sIdPollTemplate)
	{
	return PCreateNewPollFromTemplate(PFindPollByID(sIdPollTemplate));
	}

//	get(), slot
//	Return the poll matching the ID
POJapiPoll
OJapiPolls::get(const QString & sIdPoll)
	{
	return PGetOJapiPoll(PFindPollByID(sIdPoll));
	}

//	getList(), slot
//	Return a list of polls.
QVariant
OJapiPolls::getList()
	{
	CVaultEvents * pVaultPolls = m_pBallotmaster->PGetVault_NZ();
	#if 0
	#define d_cDebugLists		10000
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "OPolls::getList() [Debug=$I iterations]\n", d_cDebugLists);
	for (int i = 0; i < d_cDebugLists - 1; i++)
		{
		QVariantList oList;
		IEvent ** ppEventStop;
		IEvent ** ppEvent = pVaultPolls->m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		//QScopedArrayPointer <OPoll *>oList(new OPoll *[10]);
		//QScopedPointerArrayDeleter <QObject *>oList;
		while (ppEvent != ppEventStop)
			{
			CEventBallotSent * pEvent = (CEventBallotSent *)*ppEvent++;
			/*
			QSharedPointer<OPoll> oPoll(new OPoll(pEvent));
			oList.append(QVariant::fromValue(oPoll));
			*/
			/*
			QScopedPointer<OPoll> p(new OPoll(pEvent));
			oList.append(p);
			*/
			/*
			QPointer<OPoll> p(new OPoll(pEvent));
			oList.append(p);
			*/
			//oList[0] = new OPoll(pEvent);
			//oList.append(QVariant::fromValue(PA_CHILD QScopedPointer(new OPoll(pEvent))));
			oList.append(QVariant::fromValue(PA_CHILD new OPoll(pEvent)));
			}
		}
	#endif
	//MessageLog_AppendTextFormatCo(d_coBlue, "OPolls::list() - $i elements\n", oList.size());
	QVariantList oList;
	IEvent ** ppEventStop;
	IEvent ** ppEvent = pVaultPolls->m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
        CEventBallotPoll * pEvent = (CEventBallotPoll *)*ppEvent++;
        if ( pEvent->EGetEventClass() != eEventClass_eBallotPoll )
            continue;
		if ((pEvent->m_uFlagsEvent & IEvent::FE_kfEventDeleted) == 0)
			{
			//OPoll oPoll(pEvent);
			//oList.append(QVariant::fromValue(&oPoll));
			oList.append(QVariant::fromValue(PGetOJapiPoll(pEvent)));	// List only non-deleted polls
            }
		}
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

//	Widget to display a web browser.
//	Also contains code for JavaScript APIs (JAPI).  Sample tests are available at http://download.cambrian.org/tests/

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WLayoutBrowser.h"
#include "TBrowserTabs.h"
#include "WLayoutTabbedBrowser.h"
#include "TApplicationBallotmaster.h"

//	Colors to display debugging information in the Message Log
#define d_coBrowserDebug			d_coGreen
#define d_coBrowserDebugWarning		d_coRed		// Display a warning (typically a minor error)

#include <QtScript/QScriptEngine>
//#include <QtScript/QScriptContext>
//#include <QScriptValue>
//#include <QtScript/
//#include <QAxObject>

/*
CVariant::CVariant(const QObject * poObject)
	{
	setValue(poObject);	// This code does not compile
	}
*/

OSettings::OSettings(OJapiCambrian * poCambrian) : QObject(poCambrian)
	{
	}

OSettings::~OSettings()
	{
	//MessageLog_AppendTextFormatCo(d_coYellowDirty, "OSettings::~OSettings()\n");
	}

bool
OSettings::AudioEnabled() const
	{
	return ((g_uPreferences & P_kfDontPlaySoundWhenNewMessageArrive) == 0);
	}

void
OSettings::AudioEnabled(bool fEnable)
	{
	MessageLog_AppendTextFormatCo(d_coBrowserDebug, "OSettings::AudioEnabled($i)\n", fEnable);
	if (fEnable)
		g_uPreferences &= ~P_kfDontPlaySoundWhenNewMessageArrive;
	else
		g_uPreferences |= P_kfDontPlaySoundWhenNewMessageArrive;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
OJapiCambrian::OJapiCambrian(TProfile * pProfile, QObject * pParent) : OJapi(pParent), m_oSettings(this), m_oApps(this), m_oGroupList(this), m_oAppChat(this), m_oMe(this)
	{
	m_pProfile = pProfile;
	m_paAppBallotmaster = NULL;
	}

OJapiCambrian::~OJapiCambrian()
	{
	//MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "OJapiCambrian::~OJapiCambrian()\n");
	m_arraypaTemp.DeleteAllRuntimeObjects();// Delete all temporary object
	delete m_paAppBallotmaster;
	}

QVariant
OJapiCambrian::Settings()
	{
	/*
	QAxObject o(this);
	IDispatch * paDispatch;
	o.queryInterface(IID_IUnknown, OUT (void **)&paDispatch);
	*/
	//QScriptValue qv;
	//QObject::property()
	//MessageLog_AppendTextFormatCo(d_coRed, "OJapiCambrian::Settings(paDispatch=0x$p)\n", paDispatch);
	QVariant v;
	//v.setValue(new OSettings(this));
	v.setValue(&m_oSettings);
	return v;
	}

POJapiAppBallotmaster
OJapiCambrian::polls()
	{
	if (m_paAppBallotmaster == NULL)
		m_paAppBallotmaster = new OJapiAppBallotmaster(this, PGetApplicationHtmlInfoBallotmaster());
	return m_paAppBallotmaster;
	/*
	if (FDeniedBySecurityPolicy())
		return NULL;
	*/
	/*
	OPolls * p = new OPolls(this);
	p->dumpObjectTree();
	return QVariant::fromValue(p);
	*/
	//QSharedDataPointer<OPolls> oPolls(new OPolls(this));
	//return QVariant::fromValue(oPolls);
	/*
	QSharedPointer<OPolls> oPolls(new OPolls(this));
	return QVariant::fromValue(oPolls.data());
	*/
	//return QVariant::fromValue(new OPolls(this));
	//return QVariant::fromValue(m_paPolls);

	/*
	QScopedPointer <OPolls *>p;
	p.reset(&m_paPolls);
	//qVariantFromValue
	return QVariant::fromValue(p);
	*/
	//
	/*
	QVariant v;
	v.setValue(m_paPolls);
	return v;
	*/
	}

POJapiAppChat
OJapiCambrian::chatApp()
	{
	return &m_oAppChat;
	}

POJapiMe
OJapiCambrian::me()
	{
	return &m_oMe;
	}

POJapiGroupList
OJapiCambrian::groups()
	{
	return &m_oGroupList;
	}

POCapiRootGUI
OJapiCambrian::capi()
	{
	return &m_capi;
	}

POJapiUtil
OJapiCambrian::util()
	{
	return &m_oUtil;
	}

void
OJapiCambrian::SendBitcoin(int n)
	{
	MessageLog_AppendTextFormatCo(d_coRed, "Send $i Bitcoin\n", n);
	//EMessageBoxQuestion("Send Bitcoin?");
	}

void
OJapiCambrian::MessageSendTo(const QString & sContactTo, const QString & sMessage)
	{
	MessageLog_AppendTextFormatSev(eSeverityComment, "Sending message to $Q: $Q\n", &sContactTo, &sMessage);
	CStr strContactTo = sContactTo;	// Typecast
	CStr strMessage = sMessage;		// Typecast
	// Find the contact to send the message
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = g_arraypAccounts.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		TContact ** ppContactStop;
		TContact ** ppContact = pAccount->m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
		while (ppContact != ppContactStop)
			{
			TContact * pContact = *ppContact++;
			if (strContactTo.FCompareStringsNoCase(pContact->m_strNameDisplayTyped) ||
				strContactTo.FCompareStringsNoCaseUntilCharacter(pContact->m_strJidBare, '@') || // Compare the username
				strContactTo.FCompareStringsNoCase(pContact->m_strJidBare))
				{
				pContact->Xmpp_SendEventMessageText(strMessage);
				return;
				}
			} // while
		} // while
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t Unable to find peer '$Q'\n", &sContactTo);
	}

/*
QVariant
OJapiCambrian::set()
	{
	MessageLog_AppendTextFormatCo(d_coBrowserDebug, "OJapiCambrian::set()\n");
	QVariant v;
	v.setValue(new OSettings(this));
	return v;
	}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
WLayoutBrowser::WLayoutBrowser(TProfile * pProfile, CStr * pstrUrlAddress_NZ)
	{
	m_pProfile = pProfile;
	Assert(pstrUrlAddress_NZ != NULL);
	m_pstrUrlAddress_NZ = pstrUrlAddress_NZ;
	m_pwEdit = NULL;
	m_pwButtonBack = NULL;
	m_pwButtonForward = NULL;

	QWidget * pWidget = new QWidget(this);
	pWidget->setContentsMargins(0, 0, 0, 0);
	pWidget->setMaximumHeight(24);
	OLayoutHorizontal * poLayout = new OLayoutHorizontal(pWidget);
	poLayout->setContentsMargins(2, 2, 2, 0);
	m_pwButtonBack = new WButtonIconForToolbar(eMenuIcon_GoBack, "Go Back");
	m_pwButtonForward = new WButtonIconForToolbar(eMenuIcon_GoForward, "Go Forward");
	poLayout->addWidget(m_pwButtonBack);
	poLayout->addWidget(m_pwButtonForward);

	m_pwEdit = new WEdit(*pstrUrlAddress_NZ);
	m_pwEdit->Edit_SetWatermark("Enter web address");
	m_pwEdit->setParent(this);
	poLayout->addWidget(m_pwEdit);

	connect(m_pwEdit, SIGNAL(returnPressed()), this, SLOT(SL_NavigateToAddress()));
	connect(m_pwButtonBack, SIGNAL(clicked()), this, SLOT(SL_GoBack()));
	connect(m_pwButtonForward, SIGNAL(clicked()), this, SLOT(SL_GoForward()));

	m_pwWebView = new QWebView(this);
	connect(m_pwWebView, SIGNAL(urlChanged(QUrl)), this, SLOT(SL_UrlChanged(QUrl)));
	NavigateToAddress(*pstrUrlAddress_NZ);

//	QScriptEngine

	m_paCambrian = new OJapiCambrian(pProfile, this);

	QWebPage * poPage = m_pwWebView->page();
	m_poFrame = poPage->mainFrame();
	SL_InitJavaScript();
	connect(m_poFrame, SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(SL_InitJavaScript()));
	poPage->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);	// Enable the context menu item named "Inspect".  This is useful for debugging web pages.

	#if 0
	QAxObject o(poPage);
	IDispatch * paDispatch;
	o.queryInterface(IID_IUnknown, OUT (void **)&paDispatch);
	MessageLog_AppendTextFormatCo(d_coBrowserDebug, "IID_IUnknown=0x$p\n", paDispatch);
	#endif
	}

WLayoutBrowser::~WLayoutBrowser()
	{
	TRACE1("WLayoutBrowser::~WLayoutBrowser(0x$p)\n", this);
	delete m_paCambrian;	// Maybe use deleteLater()
	}

void
WLayoutBrowser::SL_InitJavaScript()
	{
	//MessageLog_AppendTextFormatCo(COX_MakeBold(d_coBlueDark), "SL_InitJavaScript()\n");
	m_paCambrian->m_arraypaTemp.DeleteAllRuntimeObjects();// Delete any previous temporary object
	m_poFrame->addToJavaScriptWindowObject("Cambrian", m_paCambrian); // , QWebFrame::ScriptOwnership);
	}

void
WLayoutBrowser::NavigateToAddress(const CStr & strAddress)
	{
    Assert(m_pwWebView != NULL);
	m_pwWebView->stop();
	QUrl url(strAddress);
	if (url.scheme().isEmpty())
		url.setScheme("http");
	m_pwWebView->load(url);
	}

//	This slot is called when the user presses the Enter key
void
WLayoutBrowser::SL_NavigateToAddress()
	{
	CStr strAddress = *m_pwEdit;
	NavigateToAddress(strAddress);
	}

void
WLayoutBrowser::SL_GoBack()
	{
	m_pwWebView->back();
	}

void
WLayoutBrowser::SL_GoForward()
	{
	m_pwWebView->forward();
	}

void
WLayoutBrowser::SL_UrlChanged(const QUrl & url)
	{
	if (m_pwEdit != NULL)
		{
		QString sUrl = url.toString();
		m_pwEdit->setText(sUrl);
		*m_pstrUrlAddress_NZ = sUrl;	// Remember the last URL
		}
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	TBrowser::ITreeItem::TreeItem_GotFocus()
void
TBrowser::TreeItem_GotFocus()
	{
	if (m_pawLayoutBrowser == NULL)
		m_pawLayoutBrowser = new WLayoutBrowser(m_pProfile, INOUT_LATER &m_strUrl);
	MainWindow_SetCurrentLayout(IN m_pawLayoutBrowser);
	}

TBrowser::~TBrowser()
	{
	TRACE3("TBrowser::~TBrowser(0x$p, $S) - $S", this, &m_strNameDisplayTyped, &m_strUrl);
	MainWindow_DeleteLayout(PA_DELETING m_pawLayoutBrowser);
	//delete m_pawLayoutBrowser;
	}


/*void
LaunchBrowser(const QString & sName, const QString & sUrlRelative)
    {
    //EMessageBoxInformation("opening page $Q", &sUrl);
	MessageLog_AppendTextFormatCo(d_coBlueDark, "LaunchBrowser($Q, $Q)\n", &sName, &sUrlRelative);

    TProfile * pProfile = NavigationTree_PGetSelectedTreeItemMatchingInterfaceTProfile();
	//MessageLog_AppendTextFormatCo(d_coBlack, "LaunchBrowser($p)\n", pProfile);
    if (pProfile == NULL)
        return;

	//MessageLog_AppendTextFormatCo(d_coAqua, "pProfile ($p)\n", pProfile);
	//MessageLog_AppendTextFormatCo(d_coAqua, "pProfile->m_pConfigurationParent ($p)\n", pProfile->m_pConfigurationParent);

	CStr strUrlRelative(sUrlRelative);
	CStr strUrl = "file:///" + pProfile->m_pConfigurationParent->SGetPathOfFileName(strUrlRelative);//"Apps/Test/index.htm");

	// find a browser opened with the same name
	TBrowser ** ppBrowserStop;
	TBrowser ** ppBrowser = pProfile->m_arraypaBrowsers.PrgpGetBrowsersStop(OUT &ppBrowserStop);
	while (ppBrowser != ppBrowserStop)
		{
		TBrowser * pBrowser = *ppBrowser++;
		if ( pBrowser->m_strUrl.FStringBeginsWith(strUrl.PszaGetUtf8NZ()))
		//if ( qUrlAbs.host() == qUrlCurrent.host() && qUrlAbs.path() == qUrlCurrent.path() )
		//if (pBrowser->m_strUrl.FCompareStringsExactCase(strUrl))
			{
			// There is already an opened browser with the same URL, therefore select it
			pBrowser->TreeItemW_SelectWithinNavigationTree();
			return;
			}
		}

	// No browser yet, therefore allocate one
	TBrowser * pBrowser = new TBrowser(pProfile);
	pProfile->m_arraypaBrowsers.Add(PA_CHILD pBrowser);
	pBrowser->SetNameAndUrl(sName, strUrl);
	pBrowser->TreeItemBrowser_DisplayWithinNavigationTree();
    pBrowser->TreeItemW_SelectWithinNavigationTree();
	}*/

CStr
StrGetUrlForApplication(PSZAC pszNameApplication)
	{
	CStr strUrl;
	const SApplicationHtmlInfo *pInfo = PGetApplicationHtmlInfo(pszNameApplication);
	Assert(pInfo != NULL && "Application doesn't exist");
	if (pInfo != NULL)
		{
		strUrl = MainWindow_SGetUrlPathOfApplication(pInfo->pszLocation);
		QUrl qurlAbsolute(strUrl);
		// Use the page 'Under Construction' if the application does not exist
		if (qurlAbsolute.isLocalFile())
			{
			QString strTest = qurlAbsolute.toString();
			QFile oFile(qurlAbsolute.toLocalFile());
			if (!oFile.exists())
				{
				MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "The HTML application located at '$S' cannot be found.  Therefore SocietyPro will display the HTML application 'Under Construction'\n", &strUrl);
				pInfo = PGetApplicationHtmlInfo("Underconstruction");
				Assert(pInfo != NULL && "Underconstruction app doesn't exist");
				if (pInfo != NULL)
					strUrl = MainWindow_SGetUrlPathOfApplication(pInfo->pszLocation);
				}
			}
		}
	return strUrl;
	}

void
LaunchApplication(const QString & sNameApplication)
	{
	CStr strNameApplication = sNameApplication;
	LaunchApplication((PSZAC)strNameApplication);
	}

//	The query string begins with a '?'
void
LaunchApplication(PSZAC pszNameApplication, PSZUC pszQueryString)
	{
	CStr strUrlAbsolute = StrGetUrlForApplication(pszNameApplication);
	strUrlAbsolute.AppendTextU(pszQueryString);
	LaunchBrowser(strUrlAbsolute);
	}

void
LaunchApplicationWithIdentifier(PSZAC pszNameApplication, PSZUC pszIdentifier)
	{
	CStr strQueryString;
	if (pszIdentifier != NULL && pszIdentifier[0] != '\0')
		strQueryString.Format("?id={s%}", pszIdentifier);
	LaunchApplication(pszNameApplication, strQueryString);
	}

void
LaunchApplicationWithIdentifierGroup(PSZAC pszNameApplication, TGroup * pGroup)
	{
	CHU szGroupIdentifier[32];
	szGroupIdentifier[0] = '\0';
	if (pGroup != NULL)
		pGroup->Group_GetIdentifier(OUT szGroupIdentifier);
	LaunchApplicationWithIdentifier(pszNameApplication, szGroupIdentifier);
	}

//	Launch the Ballotmaster with the parameter to vote for a specific poll
void
LaunchApplication_BallotmasterVote(CEventBallotReceived * pEventBallotReceived_NZ)
	{
	Assert(pEventBallotReceived_NZ != NULL);
	CStr strQueryString;
	LaunchApplication(d_szNameApplicationHtml_Ballotmaster, IN strQueryString.Format("?idBallot={t_}", pEventBallotReceived_NZ->m_tsEventID));
	}
void
LaunchApplication_Ballotmaster(TGroup * pGroupSelect_YZ)
	{
	LaunchApplicationWithIdentifierGroup(d_szNameApplicationHtml_Ballotmaster, pGroupSelect_YZ);
	}
void
LaunchApplication_Channels(TGroup * pChannelToSelect_YZ)
	{
	LaunchApplicationWithIdentifierGroup(d_szNameApplicationHtml_Channels, pChannelToSelect_YZ);
	}
void
LaunchApplication_GroupManager(TGroup * pGroupToSelect_YZ)
	{
	LaunchApplicationWithIdentifierGroup(d_szNameApplicationHtml_GroupManager, pGroupToSelect_YZ);
	}
void
LaunchApplication_Corporations(TGroup * pCorporationToSelect_YZ)
	{
	LaunchApplicationWithIdentifierGroup(d_szNameApplicationHtml_Corporations, pCorporationToSelect_YZ);
	}
void
LaunchApplication_PeerManager(TContact * UNUSED_PARAMETER(pContactToSelect_YZ))
	{
	LaunchApplication(d_szNameApplicationHtml_PeerManager);
	}

TBrowserTabs *
TProfile::BrowserTabs_PCreateAndDisplayBrowserTabs()
	{
	TBrowserTabs * pBrowserTabs = new TBrowserTabs(this);
	m_arraypaBrowsersTabbed.Add(PA_CHILD pBrowserTabs);
	pBrowserTabs->m_strNameDisplayTyped.InitFromStringA("Web Browser");
	pBrowserTabs->TreeItemBrowser_DisplayWithinNavigationTree();
	return pBrowserTabs;
	}

void
LaunchBrowser(const CStr & strUrlAbsolute)
	{
	//EMessageBoxInformation("opening page $Q", &sUrl);
	MessageLog_AppendTextFormatCo(d_coBlueDark, "LaunchBrowser($S)\n", &strUrlAbsolute);

	TProfile * pProfile = NavigationTree_PGetSelectedTreeItemMatchingInterfaceTProfile();
	if (pProfile == NULL)
		return;

	// find browser or open a new one
	TBrowserTabs * pBrowserTabs = pProfile->m_arraypaBrowsersTabbed.PGetBrowserTabsFirst_YZ();
	if (pBrowserTabs == NULL)
		pBrowserTabs = pProfile->BrowserTabs_PCreateAndDisplayBrowserTabs();

	// find an open tab for the selected url
	TBrowserTab ** ppBrowserTabStop;
	TBrowserTab ** ppBrowserTab;
	ppBrowserTab = pBrowserTabs->m_arraypaTabs.PrgpGetBrowserTabStop(OUT &ppBrowserTabStop);
	while (ppBrowserTab != ppBrowserTabStop)
		{
		TBrowserTab *pBrowserTab = *ppBrowserTab++;
		if ( pBrowserTab->m_strUrl.FStringBeginsWith(strUrlAbsolute.PszaGetUtf8NZ()))
			{
			pBrowserTabs->TreeItemW_SelectWithinNavigationTree();
			pBrowserTab->Show();
			return;
			}
		}

	/*
	CStr strUrlRelative(sUrlRelative);
	CStr strUrl = "file:///" + pProfile->m_pConfigurationParent->SGetPathOfFileName(strUrlRelative);//"Apps/Test/index.htm");
	*/
	// add a new tab
	pBrowserTabs->PBrowserTabAdd(strUrlAbsolute);
	pBrowserTabs->TreeItemW_SelectWithinNavigationTree();

	/*
	// find a browser with tabs already opened
	TBrowserTabs ** ppBrowserStop;
	TBrowserTabs ** ppBrowser = pProfile->m_arraypaBrowsersTabbed.PrgpGetBrowsersStop(OUT &ppBrowserStop);
	while (ppBrowser != ppBrowserStop)
		{
		// There is already an opened browser
		TBrowserTabs * pBrowser = *ppBrowser++;

		// add new tab
		pBrowser->AddTab(strUrl);
		pBrowser->TreeItemW_SelectWithinNavigationTree();
		return;
		}
	*/
	}

void
NavigationTree_NewTabbedBrowser()
	{
	TProfile * pProfile = NavigationTree_PGetSelectedTreeItemMatchingInterfaceTProfile();
	//MessageLog_AppendTextFormatCo(d_coBlack, "LaunchBrowser($p)\n", pProfile);
	if (pProfile == NULL)
		return;
	pProfile->BrowserTabs_PCreateAndDisplayBrowserTabs()->TreeItemW_SelectWithinNavigationTree();
	}

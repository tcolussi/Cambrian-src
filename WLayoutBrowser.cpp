//	Widget to display a web browser.
//	Also contains code for JavaScript APIs (JAPI).  Sample tests are available at http://download.cambrian.org/tests/

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WLayoutBrowser.h"
#include <QWebFrame>
#include "TBrowserTabs.h"
#include "WLayoutTabbedBrowser.h"

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
    MessageLog_AppendTextFormatCo(d_coYellowDirty, "OSettings::~OSettings()\n");
	}

/*
void
OSettings::AudioEnabled(bool fEnable)
	{
	}
*/
bool
OSettings::AudioEnabled() const
	{
	return true;
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
OJapiProfile::OJapiProfile(OJapiCambrian * poCambrian) : QObject(poCambrian)
	{
	}

OJapiProfile::~OJapiProfile()
	{
	MessageLog_AppendTextFormatCo(d_coYellowDirty, "OJapiProfile::~OJapiProfile()\n");
	}

QString
OJapiProfile::CreateNew(const QString & sNameProfile)
	{
	MessageLog_AppendTextFormatCo(d_coBrowserDebug, "OJapiProfile::CreateNew($Q)\n", &sNameProfile);
	TProfile * pProfile = new TProfile(&g_oConfiguration);	// At the moment, there is only one configuration object, so use the global variable
	g_oConfiguration.m_arraypaProfiles.Add(PA_CHILD pProfile);
	pProfile->m_strNameProfile = sNameProfile;
	pProfile->GenerateKeys();
	pProfile->TreeItemProfile_DisplayContactsWithinNavigationTree();
	pProfile->TreeItemW_EnsureVisible();
	return sNameProfile;	// Until we have the keys generated, return the name fo the profile as its identifier (this is good enough for proof of concept)
	}

void
OJapiProfile::SwitchTo(const QString & sIdProfile)
	{
	MessageLog_AppendTextFormatCo(d_coBrowserDebug, "OJapiProfile::SwitchTo($Q)\n", &sIdProfile);
	}

//	Return the profile data, such as email, telephone, picture and so on.
//	The profile data is stored in the configuration file.
QString
OJapiProfile::DataGet(const QString & sIdProfile)
	{
	MessageLog_AppendTextFormatCo(d_coBrowserDebug, "OJapiProfile::DataGet($Q)\n", &sIdProfile);
	TProfile * pProfile = PFindProfileByID(sIdProfile);
	if (pProfile != NULL)
		return pProfile->m_strData;
	return c_sEmpty;
	}

void
OJapiProfile::DataUpdate(const QString & sIdProfile, const QString & sDataProfile)
	{
	MessageLog_AppendTextFormatCo(d_coBrowserDebug, "OJapiProfile::DataUpdate($Q): $Q\n", &sIdProfile, &sDataProfile);
	TProfile * pProfile = PFindProfileByID(sIdProfile);
	if (pProfile != NULL)
		pProfile->m_strData = sDataProfile;
	}

TProfile *
OJapiProfile::PFindProfileByID(const QString & sIdProfile) const
	{
	// At the moment, the profile identifier is the profile name
	CStr strNameProfile = sIdProfile;
	TProfile ** ppProfileStop;
	TProfile ** ppProfile = g_oConfiguration.m_arraypaProfiles.PrgpGetProfilesStop(OUT &ppProfileStop);
	while (ppProfile != ppProfileStop)
		{
		TProfile * pProfile = *ppProfile++;
		Assert(pProfile->EGetRuntimeClass() == RTI(TProfile));
		if (pProfile->m_strNameProfile.FCompareStringsNoCase(strNameProfile))
			return pProfile;
		}
	MessageLog_AppendTextFormatCo(d_coBrowserDebugWarning, "OJapiProfile::PFindProfileByID($Q) - Unable to find profile matching identifier!\n", &sIdProfile);
	return NULL;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
OJapiCambrian::OJapiCambrian(TProfile * pProfile, QObject * pParent) : OJapi(pParent), m_oSettings(this), m_oProfile(this), m_oApps(this), m_oMe(this)
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

QVariant
OJapiCambrian::Profile()
	{
	//MessageLog_AppendTextFormatCo(d_coRed, "OJapiCambrian::Profile()\n");
	QVariant v;
	v.setValue(&m_oProfile);
	return v;
	}

POJapiAppBallotmaster
OJapiCambrian::polls()
	{
	if (m_paAppBallotmaster == NULL)
		m_paAppBallotmaster = new OJapiAppBallotmaster(this);
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

POJapiMe
OJapiCambrian::me()
	{
	return &m_oMe;
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
	m_pwButtonBack = new WButtonIconForToolbar(eMenuIconGoBack, "Go Back");
	m_pwButtonForward = new WButtonIconForToolbar(eMenuIconGoForward, "Go Forward");
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
WLayoutBrowser::SL_UrlChanged(QUrl url)
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

void
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

	// find a browser with tabs already opened
	TBrowserTabs ** ppBrowserStop;
	TBrowserTabs ** ppBrowser = pProfile->m_arraypaBrowsersTabbed.PrgpGetBrowsersStop(OUT &ppBrowserStop);
	while (ppBrowser != ppBrowserStop)
		{
		// There is already an opened browser with the same URL, therefore select it
		TBrowserTabs * pBrowser = *ppBrowser++;

		// find an open tab for the selected url
		TBrowserTab **ppBrowserTabStop;
		TBrowserTab **ppBrowserTab;
		ppBrowserTab = pBrowser->m_arraypaTabs.PrgpGetBrowserTabStop(&ppBrowserTabStop);
		while ( ppBrowserTab != ppBrowserTabStop)
			{
			TBrowserTab *pBrowserTab = *ppBrowserTab++;
			if ( pBrowserTab->m_url.FStringBeginsWith(strUrl.PszaGetUtf8NZ()))
				{
				pBrowserTab->Show();
				pBrowser->TreeItemW_SelectWithinNavigationTree();
				return;
				}
			}

		// add new tab
		pBrowser->AddTab(strUrl);
		return;
		}

	// add new browser
	CStr sTreeItemName("Web Browser");

	TBrowserTabs * pBrowser = new TBrowserTabs(pProfile);
	pBrowser->SetIconAndName(eMenuAction_DisplaySecureWebBrowsing, sTreeItemName);
	pBrowser->TreeItemBrowser_DisplayWithinNavigationTree();
	pBrowser->TreeItemW_SelectWithinNavigationTree();
	pBrowser->AddTab(strUrl);

	pProfile->m_arraypaBrowsersTabbed.Add(PA_CHILD pBrowser);
	}


void
NavigationTree_NewTabbedBrowser()
{
	TProfile * pProfile = NavigationTree_PGetSelectedTreeItemMatchingInterfaceTProfile();
	//MessageLog_AppendTextFormatCo(d_coBlack, "LaunchBrowser($p)\n", pProfile);
	if (pProfile == NULL)
		return;

	CStr sName("Web Browser");
	//CStr url1("file:///C:/Users/Cesar/.Cambrian/Apps/html5-pollmaster/index.html");
	//CStr url2("file:///C:/Users/Cesar/.Cambrian/Apps/html5-pomodoro/index.html");

	TBrowserTabs * pBrowser = new TBrowserTabs(pProfile);
	pBrowser->SetIconAndName(eMenuAction_DisplaySecureWebBrowsing, sName);
	pBrowser->TreeItemBrowser_DisplayWithinNavigationTree();
	pBrowser->TreeItemW_SelectWithinNavigationTree();
	//pBrowser->AddTab(url1);
	//pBrowser->AddTab(url2);

	pProfile->m_arraypaBrowsersTabbed.Add(PA_CHILD pBrowser);
}

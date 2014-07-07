//	Widget to display a web browser.
//	Also contains code for JavaScript APIs (JAPI).  Sample tests are available at http://download.cambrian.org/tests/

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WLayoutBrowser.h"
#include <QWebFrame>

//	Colors to display debugging information in the Message Log
#define d_coBrowserDebug			d_coGreen
#define d_coBrowserDebugWarning		d_coRed		// Display a warning (typically a minor error)

//#include <QtScript/QScriptEngine>
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

OSettings::OSettings(OCambrian * poCambrian) : QObject(poCambrian)
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
OProfile::OProfile(OCambrian * poCambrian) : QObject(poCambrian)
	{
	}

OProfile::~OProfile()
	{
	MessageLog_AppendTextFormatCo(d_coYellowDirty, "OProfile::~OProfile()\n");
	}

QString
OProfile::CreateNew(const QString & sNameProfile)
	{
	MessageLog_AppendTextFormatCo(d_coBrowserDebug, "OProfile::CreateNew($Q)\n", &sNameProfile);
	TProfile * pProfile = new TProfile(&g_oConfiguration);	// At the moment, there is only one configuration object, so use the global variable
	g_oConfiguration.m_arraypaProfiles.Add(PA_CHILD pProfile);
	pProfile->m_strNameProfile = sNameProfile;
	pProfile->GenerateKeys();
	pProfile->TreeItemProfile_DisplayWithinNavigationTree();
	pProfile->TreeItemWidget_EnsureVisible();
	return sNameProfile;	// Until we have the keys generated, return the name fo the profile as its identifier (this is good enough for proof of concept)
	}

void
OProfile::SwitchTo(const QString & sIdProfile)
	{
	MessageLog_AppendTextFormatCo(d_coBrowserDebug, "OProfile::SwitchTo($Q)\n", &sIdProfile);
	}

//	Return the profile data, such as email, telephone, picture and so on.
//	The profile data is stored in the configuration file.
QString
OProfile::DataGet(const QString & sIdProfile)
	{
	MessageLog_AppendTextFormatCo(d_coBrowserDebug, "OProfile::DataGet($Q)\n", &sIdProfile);
	TProfile * pProfile = PFindProfileByID(sIdProfile);
	if (pProfile != NULL)
		return pProfile->m_strData;
	return c_sEmpty;
	}

void
OProfile::DataUpdate(const QString & sIdProfile, const QString & sDataProfile)
	{
	MessageLog_AppendTextFormatCo(d_coBrowserDebug, "OProfile::DataUpdate($Q): $Q\n", &sIdProfile, &sDataProfile);
	TProfile * pProfile = PFindProfileByID(sIdProfile);
	if (pProfile != NULL)
		pProfile->m_strData = sDataProfile;
	}

TProfile *
OProfile::PFindProfileByID(const QString & sIdProfile) const
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
	MessageLog_AppendTextFormatCo(d_coBrowserDebugWarning, "OProfile::PFindProfileByID($Q) - Unable to find profile matching identifier!\n", &sIdProfile);
	return NULL;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
OCambrian::OCambrian(QObject * pParent) : QObject(pParent), m_oSettings(this), m_oProfile(this)
	{
	}

OCambrian::~OCambrian()
	{
	}

QVariant
OCambrian::Settings()
	{
	/*
	QAxObject o(this);
	IDispatch * paDispatch;
	o.queryInterface(IID_IUnknown, OUT (void **)&paDispatch);
	*/
	//QScriptValue qv;
	//QObject::property()
	//MessageLog_AppendTextFormatCo(d_coRed, "OCambrian::Settings(paDispatch=0x$p)\n", paDispatch);
	QVariant v;
	//v.setValue(new OSettings(this));
	v.setValue(&m_oSettings);
	return v;
	}

QVariant
OCambrian::Profile()
	{
	//MessageLog_AppendTextFormatCo(d_coRed, "OCambrian::Profile()\n");
	QVariant v;
	v.setValue(&m_oProfile);
	return v;
	}

void
OCambrian::SendBitcoin(int n)
	{
	MessageLog_AppendTextFormatCo(d_coRed, "Send $i Bitcoin\n", n);
	//EMessageBoxQuestion("Send Bitcoin?");
	}

void
OCambrian::MessageSendTo(const QString & sContactTo, const QString & sMessage)
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
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t Unable to find contact '$Q'\n", &sContactTo);
	}

/*
QVariant
OCambrian::set()
	{
	MessageLog_AppendTextFormatCo(d_coBrowserDebug, "OCambrian::set()\n");
	QVariant v;
	v.setValue(new OSettings(this));
	return v;
	}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
WLayoutBrowser::WLayoutBrowser(TBrowser * pBrowser)
	{
	Assert(pBrowser != NULL);
	m_pBrowser = pBrowser;
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

	m_pwEdit = new WEdit(pBrowser->m_strUrl);
	m_pwEdit->Edit_SetWatermark("Enter web address");
	m_pwEdit->setParent(this);
	poLayout->addWidget(m_pwEdit);

	connect(m_pwEdit, SIGNAL(returnPressed()), this, SLOT(SL_NavigateToAddress()));
	connect(m_pwButtonBack, SIGNAL(clicked()), this, SLOT(SL_GoBack()));
	connect(m_pwButtonForward, SIGNAL(clicked()), this, SLOT(SL_GoForward()));

	m_pwWebView = new QWebView(this);
	connect(m_pwWebView, SIGNAL(urlChanged(QUrl)), this, SLOT(SL_UrlChanged(QUrl)));
	NavigateToAddress(pBrowser->m_strUrl);

//	QScriptEngine

	m_paCambrian = new OCambrian(this);

	QWebPage * poPage = m_pwWebView->page();
	m_poFrame = poPage->mainFrame();
	SL_InitJavaScript();
	connect(m_poFrame, SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(SL_InitJavaScript()));

	#if 0
	QAxObject o(poPage);
	IDispatch * paDispatch;
	o.queryInterface(IID_IUnknown, OUT (void **)&paDispatch);
	MessageLog_AppendTextFormatCo(d_coBrowserDebug, "IID_IUnknown=0x$p\n", paDispatch);
	#endif
	}

void
WLayoutBrowser::SL_InitJavaScript()
	{
	m_poFrame->addToJavaScriptWindowObject("Cambrian", m_paCambrian);
	}

void
WLayoutBrowser::NavigateToAddress(const CStr & strAddress)
	{
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
		m_pBrowser->m_strUrl = sUrl;	// Remember the last URL
		}
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	TBrowser::ITreeItem::TreeItem_GotFocus()
void
TBrowser::TreeItem_GotFocus()
	{
	if (m_pawLayoutBrowser == NULL)
		m_pawLayoutBrowser = new WLayoutBrowser(this);
	MainWindow_SetCurrentLayout(IN m_pawLayoutBrowser);
	}
TBrowser::~TBrowser()
	{
	delete m_pawLayoutBrowser;
	}



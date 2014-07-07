///////////////////////////////////////////////////////////////////////////////////////////////////
//	WLayoutCertificate.cpp
//
//	Various widgets to pin and display properties of encryption certificates.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WLayoutCertificate.h"

#if 0
WLayoutWelcome::WLayoutWelcome()
	{
	#if 0
	//QGridLayout * poLayoutGrid = new QGridLayout(this);
	OLayoutHorizontal * pLayoutH = new OLayoutHorizontal(this);
	//poLayoutGrid->addLayout(pLayoutH, 0, 0);
	pLayoutH->addWidget(new WLabelSelectable("If you are new to Cambrian, you are welcome to register an account."), 0, 0);
	pLayoutH->addWidget(new WButtonTextWithIcon("Register...|Create a new chat account on an XMPP server", eMenuIconXmpp), 1, 0);
//	setLayout(poLayout);
	#endif

	OLayoutForm * poLayout = Splitter_PoAddGroupBoxAndLayoutForm_VE("Welcome to Cambrian");
	WButtonTextWithIcon * pwButton = poLayout->Layout_PwAddRowLabelAndPushButton("If you are new to Cambrian, you are welcome to register an account.", "Register...|Create a new chat account on an XMPP server", eMenuIconXmpp);
	connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_AccountRegister()));
	pwButton = poLayout->Layout_PwAddRowLabelAndPushButton("If you already have an existing chat account, feel free to login.", "Login...|Connect to an XMPP server using an existing", eMenuIconXmpp);
	connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_AccountLogin()));
	pwButton = poLayout->Layout_PwAddRowLabelAndPushButton("If you have received an invitation by email (or other messaging sytem), use the following button to paste your invitation.", "Invitation...|Paste your invitation to create your XMPP account", eMenuIconXmpp);
	connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_AccountAcceptInvitation()));


	#if 0
	OLayoutVerticalAlignTop * poLayoutVertical = Splitter_PoAddGroupBoxAndLayoutVertical_VE("Welcome to Cambrian");
	WButtonTextWithIcon * pwButtonRegister = new WButtonTextWithIcon("Register...|Create a new chat account on an XMPP server", eMenuIconXmpp);
	poLayoutVertical->Layout_PwAddRowLabelAndButton("If you are new to Cambrian, you are welcome to register an account.", pwButtonRegister);
	WButtonTextWithIcon * pwButtonLogin = new WButtonTextWithIcon("Login...|Connect to an XMPP server using an existing", eMenuIconXmpp);
	poLayoutVertical->Layout_PwAddRowLabelAndButton("If you already have an existing chat account, feel free to login.", pwButtonLogin);
	#endif

	/*
	OLayoutHorizontal * poLayout = Splitter_PoAddGroupBoxAndLayoutHorizontal_VE("Welcome to Cambrian");

	WButtonTextWithIcon * pwButtonRegister = new WButtonTextWithIcon("Register...|Create a new chat account on an XMPP server", eMenuIconXmpp);
	poLayout->Layout_AddLabelAndWidgetH_PA("If you are new to Cambrian, you are welcome to register an account.", pwButtonRegister);

	WButtonTextWithIcon * pwButtonLogin = new WButtonTextWithIcon("Login...|Connect to an XMPP server using an existing", eMenuIconXmpp);
	poLayout->Layout_AddLabelAndWidgetH_PA("If you already have an existing chat account, feel free to login.", pwButtonLogin);
	*/
	Splitter_AddWidgetSpacer();
	}

//	WLayoutWelcome::WLayout::Layout_NoticeDisplay()
void
WLayoutWelcome::Layout_NoticeDisplay(IN INotice * piwNotice)
	{
	Assert(piwNotice != NULL);
	Assert(piwNotice->EGetRuntimeClass() != RTI(Null));	// Make sure the vtable is still valid
	insertWidget(1, PA_CHILD piwNotice);	// Insert the notice before the spacer
	}

void DisplayDialogAccountNew(EMenuAction eMenuAction);

void
WLayoutWelcome::SL_AccountRegister()
	{
	DisplayDialogAccountNew(eMenuAction_AccountRegister);
	}

void
WLayoutWelcome::SL_AccountLogin()
	{
	DisplayDialogAccountNew(eMenuAction_AccountLogin);
	}

void
WLayoutWelcome::SL_AccountAcceptInvitation()
	{
	DisplayDialogAccountNew(eMenuAction_AccountAcceptInvitation);
	}


CTreeItemWelcome::CTreeItemWelcome()
	{
	m_strNameDisplay.BinAppendStringWithNullTerminator("Welcome");
	TreeItem_DisplayWithinNavigationTree(NULL);
	extern QIcon * g_pIconCambrian;
	m_paTreeWidgetItem->setIcon(0, *g_pIconCambrian);
	}

//	CTreeItemWelcome::ITreeItem::TreeItem_GotFocus()
void
CTreeItemWelcome::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayoutAutoDelete(PA_CHILD new WLayoutWelcome);
	}
#endif

//	TTreeItemMyInbox::ITreeItem::TreeItem_GotFocus()
void
TTreeItemMyInbox::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayout(NULL);
	}

class CTableAccounts : public WTable
{
public:
	CTableAccounts(PA_PARENT OLayoutForm * poLayoutParent, const CArrayPtrAccountsXmpp & arraypAccounts);
};

CTableAccounts::CTableAccounts(PA_PARENT OLayoutForm * poLayoutParent, const CArrayPtrAccountsXmpp & arraypAccounts) : WTable(PA_PARENT poLayoutParent)
	{
	SetColumns_VEZ("Account Name", "Resource", "Contacts", /*"Tasks",*/ NULL); // Errors
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = arraypAccounts.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
		OTableRow oRow;
		oRow.AddData(pAccount->m_strJID);
		oRow.AddData(pAccount->m_strJIDwithResource.PchFindCharacter('/'));
		oRow.AddInt(pAccount->Contacts_UGetCount());
//		oRow.AddInt(pAccount->SocketTask_UGetTasksPending());
		AppendRow(oRow);
		}
	}

WLayoutCertificateServerName::WLayoutCertificateServerName(TCertificateServerName * pServer)
	{
	BOOL fuIsConnected = FALSE;
	CSocketXmpp * pSocket = pServer->PGetSocket_YZ();
	if (pSocket != NULL)
		fuIsConnected = pSocket->Socket_FuIsReadyToSendMessages();
	OLayoutForm * pLayout = Splitter_PoAddGroupBoxAndLayoutForm_VE("Server: $s", pServer->TreeItem_PszGetNameDisplay());
	pLayout->Layout_PwAddRowLabelEditReadOnly(fuIsConnected ? "IP Address" : "Last Known IP Address:", pServer->m_strLastKnownIP);
	if (pSocket != NULL)
		{
		QSslCipher oCipher = pSocket->Socket_GetSessionCipher();
		if (!oCipher.isNull())
			{
			pLayout->Layout_PwAddRowLabelEditReadOnly("Cipher Protocol:", oCipher.protocolString());
			pLayout->Layout_PwAddRowLabelEditReadOnly("Cipher Name:", oCipher.name());
			pLayout->Layout_PwAddRowLabelEditReadOnly("Cipher Key Exchange Method:", oCipher.keyExchangeMethod());
			pLayout->Layout_PwAddRowLabelEditReadOnly("Authentication Method:", oCipher.authenticationMethod());
			pLayout->Layout_PwAddRowLabelEditReadOnly("Encryption Method:", oCipher.encryptionMethod());
			pLayout->Layout_PwAddRowLabelEditReadOnlyInt("Bits Used:", oCipher.usedBits());
			pLayout->Layout_PwAddRowLabelEditReadOnlyInt("Bits Supported:", oCipher.supportedBits());
			fuIsConnected = TRUE;
			}
		}
	if (!fuIsConnected && pSocket != NULL)
		{
		pLayout = Splitter_PoAddGroupBoxAndLayoutFormError_V("Socket connection error with server $s", pServer->TreeItem_PszGetNameDisplay());
		pLayout->Layout_AddRowLabelError("Error Message:", pSocket->errorString());
		}
	pLayout = Splitter_PoAddGroupBoxAndLayoutForm_VE("XMPP accounts currently connected to server $s using certificate '$S'", pServer->TreeItem_PszGetNameDisplay(), &pServer->m_pCertificateParent->m_strCertificateName);
	(void)new CTableAccounts(PA_PARENT pLayout, pServer->m_arraypAccounts);

	BOOL fOtherServers = FALSE;
	CArrayPtrAccountsXmpp arraypAccountsXmppOther;
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = g_arraypAccounts.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		if (pAccount->m_strServerName.FCompareStringsNoCase(pServer->m_strServerName))
			{
			// We have an account matching the server name, so check if it is not already in pServer->m_arraypAccounts
			if (!pServer->m_arraypAccounts.FindElementF(pAccount))
				{
				arraypAccountsXmppOther.Add(pAccount);
				fOtherServers = TRUE;
				}
			}
		}
	if (fOtherServers)
		{
		pLayout = Splitter_PoAddGroupBoxAndLayoutForm_VE("Other XMPP accounts using server $s", pServer->TreeItem_PszGetNameDisplay());
		(void)new CTableAccounts(PA_PARENT pLayout, arraypAccountsXmppOther);
		}
	Splitter_AddWidgetSpacer();
	} // WLayoutCertificateServerName()

void
OLayoutForm::Layout_AddFormCertificate(const CSslCertificate & oCertificate)
	{
	Layout_PwAddRowLabelEditReadOnly("Issued By:", oCertificate.issuerInfo(QSslCertificate::Organization).join(QLatin1Char(' ')));
	Layout_PwAddRowLabelEditReadOnly("Valid Since:", oCertificate.effectiveDate().toString(Qt::DefaultLocaleLongDate));
	Layout_PwAddRowLabelEditReadOnly("Valid Until:", oCertificate.expiryDate().toString(Qt::DefaultLocaleLongDate));
	Layout_PwAddRowLabelEditReadOnlyFingerprint("Fingerprint MD5:", oCertificate.digest(QCryptographicHash::Md5));
	Layout_PwAddRowLabelEditReadOnlyFingerprint("Fingerprint SHA-1:", oCertificate.digest(QCryptographicHash::Sha1));
	Layout_PwAddRowLabelEditReadOnlyFingerprint("Fingerprint SHA 256:", oCertificate.digest(QCryptographicHash::Sha256));
	Layout_PwAddRowLabelEditReadOnlyFingerprint("Fingerprint SHA 512:", oCertificate.digest(QCryptographicHash::Sha512));
	const QSslKey oKey = oCertificate.publicKey();
	const QByteArray arraybKey = oKey.toDer();
	QSsl::KeyAlgorithm eAlgorithm = oKey.algorithm();
	
	Layout_AddRowLabelFormat_VE_Gsb("$s $i bit Public Key:", (eAlgorithm == QSsl::Rsa) ? "RSA" : (eAlgorithm == QSsl::Dsa) ? "DSA" : "Unknown", oKey.length());
	WSplitter * pSplitterPublicKey = Layout_PwAddSplitterHorizontal();
	WEditTextAreaReadOnly * pwEditPublicKey = pSplitterPublicKey->Splitter_PwAddEditTextAreaReadOnly(NULL, CString(arraybKey.toHex()));
	OLayoutForm * pLayoutPublicKeys = pSplitterPublicKey->Splitter_PoAddWidgetAndLayoutForm();
	pLayoutPublicKeys->Layout_PwAddRowLabelEditReadOnlyFingerprint("MD5:", QCryptographicHash::hash(arraybKey, QCryptographicHash::Md5));
	pLayoutPublicKeys->Layout_PwAddRowLabelEditReadOnlyFingerprint("SHA-1:", QCryptographicHash::hash(arraybKey, QCryptographicHash::Sha1));
	pLayoutPublicKeys->Layout_PwAddRowLabelEditReadOnlyFingerprint("SHA 256:", QCryptographicHash::hash(arraybKey, QCryptographicHash::Sha256));
	pLayoutPublicKeys->Layout_PwAddRowLabelEditReadOnlyFingerprint("SHA 512:", QCryptographicHash::hash(arraybKey, QCryptographicHash::Sha512));
	pwEditPublicKey->Widget_SetHeightEqualToLayout(IN pLayoutPublicKeys);

	WSplitter * pSplitter = Layout_PwAddSplitterHorizontal();
	#if 1
	pSplitter->Splitter_PwAddEditTextAreaReadOnly("PEM", QString(oCertificate.toPem()));
	pSplitter->Splitter_PwAddEditTextAreaReadOnly("Text", oCertificate.toText());
	#else
	pSplitter->Splitter_PwAddEditTextAreaReadOnly("PEM", QString(oCertificate.toPem()))->Widget_SetHeightEqualToLayout(IN pLayoutPublicKeys);
	pSplitter->Splitter_PwAddEditTextAreaReadOnly("Text", oCertificate.toText())->Widget_SetHeightEqualToLayout(IN pLayoutPublicKeys);
	#endif
	} // Layout_AddFormCertificate()

void
OLayoutForm::Layout_AddFormCertificate(const TCertificate * pCertificate)
	{
	if (pCertificate != NULL)
		{
		QList<QSslCertificate> listSslCertificates = QSslCertificate::fromData(pCertificate->m_binDataBinary.ToQByteArrayShared(), QSsl::Der);
		const QSslCertificate & oCertificate = listSslCertificates.at(0);
		Layout_AddFormCertificate((const CSslCertificate &)oCertificate);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
TAccountXmpp::Certificate_FRegisterCertificateChainFromSocketAndAcceptAllErrors(const QList<QSslError> & listErrorsSsl)
	{
	Certificate_RegisterCertificateChainFromSocket();
	Assert(m_pCertificateServerName != NULL);
	if (m_pCertificateServerName != NULL)
		{
		Assert(m_pCertificateServerName->m_pCertificateParent != NULL);
		if (Certificate_FIsCertificateAuthenticated())
			{
			MessageLog_AppendTextFormatSev(eSeverityNoise, "\t The certificate '$S' has been previously authenticated, therefore it is safe to ignore the $i errors...\n", &m_pCertificateServerName->m_pCertificateParent->m_strCertificateName, listErrorsSsl.size());
			return TRUE;	// The pinned fingerprint matches the certificate fingerprint, therefore the certificate errors are automatically accepted
			}
		// Store the errors into the object so they may be displayed to the user
		foreach (const QSslError & oErrorSsl, listErrorsSsl)
			m_pCertificateServerName->m_arraypaszErrors.AddStringAllocateUnique(oErrorSsl.errorString());

		MessageLog_AppendTextFormatSev(eSeverityWarning, "\t The certificate '$S' is NOT authenticated for ^J!\n", &m_pCertificateServerName->m_pCertificateParent->m_strCertificateName, this);
		NoticeListRoaming_AddNoticeCertificateNotAuthenticated();
		}
	else
		{
		Assert(FALSE && "This case must be handled!");
		}
	#if 0
	QString sCertificateName = "Certificate: " + oCertificate.subjectInfo(QSslCertificate::Organization).join(QLatin1Char(' '));
	new WDockingCertificate(this, roCertificate, sCertificateName);
	#endif
	return FALSE;	// Wait until the user manually accepts the certificate
	}

//	Register the certificate chain from the socket related to the chat account
void
TAccountXmpp::Certificate_RegisterCertificateChainFromSocket()
	{
	//NoticeListRoaming_AddMessageWarning_VE(this, "IP Address Changed", "The IP address of server <b>^S</b> changed from <b>1.2.3.4</b> to <b>5.6.7.8</b>", &m_strServerName);
	Assert(m_paSocket != NULL);
	if (m_pCertificateServerName != NULL)
		return;	// The certificate chain has already bbeen registered
//	m_pProfileParent->m_pConfigurationParent
	TCertificate * pCertificate = g_oConfiguration.PCertificatesRegister(m_paSocket);	// Register all the certificate
//	Assert(pCertificate != NULL);
	if (pCertificate != NULL)
		{
		m_pCertificateServerName = pCertificate->PFindServerOrAllocate(m_strServerName);
		(void)m_pCertificateServerName->m_arraypAccounts.AddUniqueF(this);

		CStr strPreviousIP = m_pCertificateServerName->m_strLastKnownIP;
		m_paSocket->Socket_GetIpAddress(OUT &m_pCertificateServerName->m_strLastKnownIP);
		if (!m_pCertificateServerName->m_strLastKnownIP.FCompareBinary(strPreviousIP))
			{
			if (!strPreviousIP.FIsEmptyString())
				NoticeListRoaming_AddMessageWarning_VE(m_pCertificateServerName, "IP Address Changed", "The IP address of server <b>^S</b> changed from <b>^S</b> to <b>^S</b>", &m_strServerName, &strPreviousIP, &m_pCertificateServerName->m_strLastKnownIP);
			}
		}
	} // Certificate_RegisterCertificateChainFromSocket()

//	Return TRUE if the certificate has been approved for the account.
//	If the fingerprint of the account is empty, this method will search for any another account sharing the same server and copy its fingerprint.
BOOL
TAccountXmpp::Certificate_FIsCertificateAuthenticated()
	{
	if (m_pCertificateServerName == NULL)
		{
		// This is a rare situation, typically just after having registered/created a new account where no server has been assigned yet
		Certificate_RegisterCertificateChainFromSocket();
		if (m_pCertificateServerName == NULL)
			{
			MessageLog_AppendTextFormatSev(eSeverityWarning, "No certificate found for account '$S'... therefore assuming 'certificate' is authentic...\n", &m_strJID);
			return TRUE;
			}
		Assert(m_pCertificateServerName != NULL);
		}
	if (m_pCertificateServerName != NULL)
		{
		const TCertificate * pCertificate = m_pCertificateServerName->m_pCertificateParent;
		if (m_binFingerprintCertificate.FIsEmptyBinary())
			{
			// We have an empty fingerprint, so attempt to find an account having a valid fingerprint
			TAccountXmpp * pAccount = pCertificate->PFindAccountAuthenticating();
			if (pAccount == NULL)
				{
				// There is no account authenticating the certificate, so it is the very first time we see this certificate
				if ((g_uPreferences & P_kfManualCertificateAuthentication) == 0)
					{
					Certificate_PinFingerprintSha1(pCertificate);	// Automatically authenticate and pin the certificate using SHA-1 (we could also use another hash algorithm, however SHA-1 is safer than MD5 and efficient to compute)
					MessageLog_AppendTextFormatSev(eSeverityNoise, "The encryption certificate for account ^J was automatically authenticated with fingerprint SHA-1 ({BG}).\n", this, &m_binFingerprintCertificate);
					}
				}
			else
				{
				MessageLog_AppendTextFormatSev(eSeverityNoise, "Copying fingerprint {BG} from account ^J to ^J.\n", &pAccount->m_binFingerprintCertificate, pAccount, this);
				Certificate_PinFingerprint(pAccount->m_binFingerprintCertificate);	// Copy the fingerprint of another account
				}
			}
		return Certificate_FCompareFingerprints(pCertificate);
		}
	return FALSE;
	} // Certificate_FIsCertificateAuthenticated()

//	Compare the fingerprint of the account with the fingerprint the certificate.
BOOL
TAccountXmpp::Certificate_FCompareFingerprints(const TCertificate * pCertificate)
	{
	Assert(pCertificate->EGetRuntimeClass() == RTI(TCertificate));
	return m_binFingerprintCertificate.FCompareFingerprintWithCertificateBinaryData(pCertificate->m_binDataBinary);
	}

void
TAccountXmpp::Certificate_PinFingerprintSha1(const TCertificate * pCertificate)
	{
	Assert(pCertificate->EGetRuntimeClass() == RTI(TCertificate));
	m_binFingerprintCertificate.BinInitFromCalculatingHashSha1(pCertificate->m_binDataBinary);
	}

TCertificate *
TAccountXmpp::Certificate_PGet_YZ() const
	{
	if (m_pCertificateServerName != NULL)
		{
		Assert(m_pCertificateServerName->EGetRuntimeClass() == RTI(TCertificateServerName));
		return m_pCertificateServerName->m_pCertificateParent;
		}
	return NULL;
	}

void
TAccountXmpp::NoticeListRoaming_AddNoticeCertificateNotAuthenticated()
	{
	NoticeListRoaming_AddNotice(PA_DELETING new WNoticeWarningCertificateNotAuthenticated(this));	// Display a notice to the user so he/she may authenticate the certificate
	}


WCertificateAuthenticator::WCertificateAuthenticator(TCertificate * pCertificate) // : WGroupBox(NULL)
	{
	m_pCertificate = pCertificate;
	const CBin * pbinFingerprint = &c_strEmpty;
	TAccountXmpp * pAccount = pCertificate->PGetRuntimeInterface_TAccountXmpp();
	if (pAccount != NULL)
		{
		// The certificate has a single account associated
		Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
		pbinFingerprint = pAccount->Certificate_PGetBinaryFingerprint();
		}
	//OLayoutHorizontal * pLayoutHorizontal = Splitter_PoAddGroupBoxAndLayoutHorizontal_VE("Please Authenticate Certificate: $S", &pCertificate->m_strCertificateName);
	Widget_SetTitleFormat_VE_Gsb("Please Authenticate Certificate: $S", &pCertificate->m_strCertificateName);
	m_pwEditFingerprint = new WEditFingerprint(*pbinFingerprint,
		"To authenticate a certificate, please enter a valid fingerprint.\n\n"
		"Before blindly copying & pasting a fingerprint from below,\n"
		"please do some research to ensure the encryption certificate is indeed legitimate.\n"
		"To accomplish this, you may contact a friend or visit a website listing the fingerprints of valid certificates.");
	m_pwEditFingerprint->Edit_SetWatermark("Enter any fingerprint (MD5, SHA-1, SHA-256 or SHA-512) from below");
	QPushButton * pwButtonAuthenticate = new WButtonTextWithIcon("Authenticate |Confirm the certificate is genuine based on its fingerprint", eMenuIconAccept);
	OLayoutHorizontal * poLayout = new OLayoutHorizontal(PA_PARENT this);
	poLayout->Layout_AddWidgetsH_VEZA(new WButtonIconForToolbar(NULL, eMenuIconQuestion), new WLabelSelectable("Certificate Fingerprint:"), m_pwEditFingerprint, pwButtonAuthenticate, NULL);
	connect(m_pwEditFingerprint, SIGNAL(returnPressed()), this, SLOT(SL_Authenticate()));
	connect(pwButtonAuthenticate, SIGNAL(clicked()), this, SLOT(SL_Authenticate()));
//	m_pwEditFingerprint->setFocus();
	}

void
WCertificateAuthenticator::SL_Authenticate()
	{
	// Get the fingerprint the user typed in hexadecimal
	CBin binFingerprint;
	if (!binFingerprint.BinAppendBinaryFingerprintF_MB(m_pwEditFingerprint))
		goto SetFocus;
	if (!binFingerprint.FCompareFingerprintWithCertificateBinaryData(m_pCertificate->m_binDataBinary))
		{
		(void)EMessageBoxInformation("The fingerprint '{BG}' does not match any fingerprint of the certificate.", &binFingerprint);
		SetFocus:
		m_pwEditFingerprint->setFocus();
		return;
		}
	// We have a matching fingerprint, so approve all accounts using this certificate
	CArrayPsz arraypszServersAuthenticated;
	TCertificateServerName ** ppServerStop;
	TCertificateServerName ** ppServer = m_pCertificate->m_arraypaServers.PrgpGetServersStop(OUT &ppServerStop);
	while (ppServer != ppServerStop)
		{
		TCertificateServerName * pServer = *ppServer++;
		Assert(pServer != NULL);
		arraypszServersAuthenticated.Add(pServer->m_strServerName.PszuGetDataNZ());
		pServer->Approve();
		TAccountXmpp ** ppAccountStop;
		TAccountXmpp ** ppAccount = pServer->m_arraypAccounts.PrgpGetAccountsStop(OUT &ppAccountStop);
		while (ppAccount != ppAccountStop)
			{
			TAccountXmpp * pAccount = *ppAccount++;
			MessageLog_AppendTextFormatSev(eSeverityNoise, "Assigning fingerprint '{BG}' to account '$S'\n", &binFingerprint, &pAccount->m_strJID);
			pAccount->Certificate_PinFingerprint(binFingerprint);
			}
		Assert(pServer->FIsApproved());
		} // while
	const int cServersAuthenticated = arraypszServersAuthenticated.GetSize();
	if (cServersAuthenticated == 1)
		(void)EMessageBoxInformation("Success! The certificate <b>^S</b> is now securing all communications with server <b>^s</b>.", &m_pCertificate->m_strCertificateName, arraypszServersAuthenticated.PvGetElementFirst_YZ());
	else
		(void)EMessageBoxInformation("Success! The certificate <b>^S</b> is now securing all communications with the following $i servers:\n\n^L", &m_pCertificate->m_strCertificateName, cServersAuthenticated, &arraypszServersAuthenticated);
	NoticeListRoaming_RefreshDisplay();		// Hide any notice regarding the authentication of the certificate
	deleteLater(PA_DELETING);			// Delete the authenticator
	Configuration_NetworkReconnectIfDisconnected();	// Reconnect all disconnected accounts
	} // SL_Authenticate()

void
WSplitter::Splitter_AddCertificateInfo(TCertificate * pCertificate)
	{
	if (pCertificate->PFindAccountAuthenticating() == NULL)
		{
		// The certificate is not authenticated by any account, therefore add the 'Authenticator'
		addWidget(PA_CHILD new WCertificateAuthenticator(pCertificate));
		}

	OLayoutForm * pLayout = Splitter_PoAddGroupBoxAndLayoutForm_VE("Certificate: $S", &pCertificate->m_strCertificateName);
	pLayout->Layout_AddFormCertificate(pCertificate);

	TCertificateServerName ** ppServerStop;
	TCertificateServerName ** ppServer = pCertificate->m_arraypaServers.PrgpGetServersStop(OUT &ppServerStop);
	while (ppServer != ppServerStop)
		{
		TCertificateServerName * pServer = *ppServer++;
		Assert(pServer != NULL);
		if (pServer->m_arraypaszErrors.FIsEmpty())
			continue;
		pLayout = Splitter_PoAddGroupBoxAndLayoutFormError_V("Certificate errors with server $s:", pServer->TreeItem_PszGetNameDisplay());
		pLayout->Layout_AddRowErrors(IN pServer->m_arraypaszErrors);
		}
	Splitter_AddWidgetSpacer();
	} // Splitter_AddCertificateInfo()


WDockingCertificate::WDockingCertificate(TAccountXmpp * pAccount)
	{
	Assert(pAccount != NULL);
	TCertificate * pCertificate = pAccount->Certificate_PGet_YZ();
	Assert(pCertificate != NULL);
	Widget_SetTitleFormat_VE_Gsb("Certificate: $S", &pCertificate->m_strCertificateName);
	m_pSplitter->Splitter_AddCertificateInfo(pCertificate);
	Widget_ResizeTocontentOfSplitter();
	setFloating(true);
	g_pwMainWindow->addDockWidget(Qt::RightDockWidgetArea, PA_CHILD this);
	/*
	setVisible(true);
	setFocusPolicy(Qt::StrongFocus);
	setFocus();
	raise();
	*/
	}

WLayoutCertificate::WLayoutCertificate(TCertificate * pCertificate)
	{
	Assert(pCertificate != NULL);
	Splitter_AddCertificateInfo(pCertificate);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WLayoutCertificates::WLayoutCertificates()
	{
	Splitter_SetMarginForCustomLayout();

	CArrayPtrCertificateServers arraypServers;
	CArrayPtrCertificates arraypCertificates;
	arraypCertificates.Copy(IN &g_oConfiguration.m_oTreeItemCertificates.m_arraypaCertificates);
	g_oConfiguration.m_oTreeItemCertificates.m_arraypaCertificates.GetAllCertificatesAndServers(IOUT &arraypCertificates, IOUT &arraypServers);

	WTable * pTableCertificates = new WTable(this);
	pTableCertificates->SetColumns_VEZ("Certificate Name", "Fingerprint SHA-1", "Issued By", "Valid Since", "Valid Until", "Bits", "Public Key SHA-1", "Serial Number", NULL);

	TCertificate ** ppCertificateStop;
	TCertificate ** ppCertificate = arraypCertificates.PrgpGetCertificatesStop(OUT &ppCertificateStop);
	while (ppCertificate != ppCertificateStop)
		{
		TCertificate * pCertificate = *ppCertificate++;
		Assert(pCertificate->EGetRuntimeClass() == RTI(TCertificate));
		QList<QSslCertificate> listSslCertificates = QSslCertificate::fromData(pCertificate->m_binDataBinary.ToQByteArrayShared(), QSsl::Der);
		foreach (const QSslCertificate & oCertificateSsl, listSslCertificates)
			{
			OTableRow oRow;
			oRow.AddData(oCertificateSsl.subjectInfo(QSslCertificate::Organization).join(QLatin1Char(' ')));
			oRow.AddDataFingerprint(oCertificateSsl.digest(QCryptographicHash::Sha1));
			oRow.AddData(oCertificateSsl.issuerInfo(QSslCertificate::Organization).join(QLatin1Char(' ')));
			oRow.AddDate(oCertificateSsl.effectiveDate());
			oRow.AddDate(oCertificateSsl.expiryDate());
			QSslKey oKey = oCertificateSsl.publicKey();
			oRow.AddInt(oKey.length());
			oRow.AddDataFingerprint(QCryptographicHash::hash(oKey.toDer(), QCryptographicHash::Sha1));
			oRow.AddData(oCertificateSsl.serialNumber());
			pTableCertificates->AppendRow(oRow);
			} // foreach
		} // while

	// Now display the servers using the certificates
	WTable * pTableServers = new WTable(this);
	pTableServers->SetColumns_VEZ("Server Name", "Encrypted With Certificate", "Protocol", "IP Address", "Date Approved", "Date Revoked", NULL);
	TCertificateServerName ** ppServerStop;
	TCertificateServerName ** ppServer = arraypServers.PrgpGetServersStop(OUT &ppServerStop);
	while (ppServer != ppServerStop)
		{
		TCertificateServerName * pServer = *ppServer++;
		Assert(pServer->EGetRuntimeClass() == RTI(TCertificateServerName));
		QString sProtocol;
		CSocketXmpp * pSocket = pServer->PGetSocket_YZ();
		if (pSocket != NULL)
			{
			QSslCipher oCipher = pSocket->Socket_GetSessionCipher();
			sProtocol = oCipher.protocolString();
			//oCipher.usedBits();
			}
		OTableRow oRow;
		oRow.AddData(pServer->TreeItem_PszGetNameDisplay());
		oRow.AddData(pServer->m_pCertificateParent->TreeItem_PszGetNameDisplay());
		oRow.AddData(sProtocol);
		oRow.AddData(pServer->m_strLastKnownIP);
		oRow.AddDate(pServer->m_dtuCertificateApproved);
		oRow.AddDate(pServer->m_dtuCertificateRevoked);
		pTableServers->AppendRow(oRow);
		} // while
	} // WLayoutCertificates


///////////////////////////////////////////////////////////////////////////////////////////////////
WNoticeWarningCertificateNotAuthenticated::WNoticeWarningCertificateNotAuthenticated(TAccountXmpp * pAccount) //  : WNoticeWarning(NULL)
	{
	Assert(pAccount != NULL);
	m_pAccount = pAccount;
	setFixedHeight(160);
	m_pwGroupBox->setTitle("Security Warning: Certificate Changed!");
	m_pwGroupBox->Widget_SetTitleColorError();

	//WButtonTextWithIcon * pButtonAuthenticate = new WButtonTextWithIcon("Authenticate...|Secure the server communication by authenticating the encryption certificate.", eMenuIconAccept);
	WButtonTextWithIcon * pButtonCertificateApprove = new WButtonTextWithIcon("Approve Certificate |Accept the certificate as genuine", eMenuIconAccept);
	WButtonTextWithIcon * pButtonCertificateView = new WButtonTextWithIcon("View Certificate...|Inspect the certificate before approving it", eMenuIconCertificate);
	m_pButtonConnectNow = new WButtonTextWithIcon("Connect Now!", eMenuAction_AccountReconnect);
	m_poLayoutBody->Layout_PoAddLayoutVerticalWithWidgets_VEZA(pButtonCertificateApprove, pButtonCertificateView, m_pButtonConnectNow, NULL);

	TCertificate * pCertificate = PGetCertificate_YZ();
	if (pCertificate != NULL)
		{
		SHashSha1 hashCertificate;
		HashSha1_CalculateFromCBin(OUT &hashCertificate, pCertificate->m_binDataBinary);	// By default, use SHA-1 for fingerprints.  The user many always authenticate a certificate using another hash, such as MD5, SHA-256 or SHA-512, however for simplicity of a one-click cert authentication, Cambrian uses SHA-1.
		//NoticeWarning_SetTextFormat_VE("The certificate encrypting the communication with server <b>^S</b> has not been authenticated.", &pAccount->m_strServerName);
		const CBin * pbinFingerprint = pAccount->Certificate_PGetBinaryFingerprint();
		NoticeWarning_SetTextFormat_VE(
			//"You are seeing this warning because a security certificate change has been detected for site <b>^S</b>.<br/><br/>"
			"You are seeing this warning because Cambrian detected a change of security certificate for site <b>^S</b>.<br/><br/>"
			"The expected certificate fingerprint was: <b>{BG}</b><br/>"
			"The current certificate fingerprint is now: <b>{hG}</b>", &m_pAccount->m_strServerName, pbinFingerprint, &hashCertificate);
		connect(pButtonCertificateApprove, SIGNAL(clicked()), this, SLOT(SL_CertificateApprove()));
		}
	else
		NoticeWarning_SetTextFormat_VE("The communication with server <b>^S</b> is not encrypted.", &pAccount->m_strServerName);

	connect(pButtonCertificateView, SIGNAL(clicked()), this, SLOT(SL_CertificateView()));
	connect(m_pButtonConnectNow, SIGNAL(clicked()), this, SLOT(SL_ConnectNow()));
	} // WNoticeWarningCertificateNotAuthenticated()

//	WNoticeWarningCertificateNotAuthenticated::INotice::ETreeItemGotFocus()
//
//	Display the notice only for Tree Items related to the certificate, however not for the certificate itself: the certificate already displays a widget for the user to authenticate the certificate.
//
ENoticeAction
WNoticeWarningCertificateNotAuthenticated::ETreeItemGotFocus(IN ITreeItem * piTreeItemFocus)
	{
	MessageLog_AppendTextFormatCo(d_coBlack, "WNoticeWarningCertificateNotAuthenticated::ETreeItemGotFocus(0x$p)\n", piTreeItemFocus);
	if (m_pAccount->Certificate_FIsCertificateAuthenticated())
		return eNoticeActionDelete;	// The certificate is (now) authenticated, therefore there is no need to display the notice anymore
	RTI_ENUM rtiTreeItemFocus = piTreeItemFocus->EGetRuntimeClass();
	if (rtiTreeItemFocus == RTI(TCertificate))
		return eNoticeActionKeepHidden;	// The certificate already displays a widget to authenticate itself
	// Customize the widget based on what Tree Item is having the focus
	TCertificate * pCertificate = PGetCertificate_YZ();
	if (pCertificate == NULL)
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "WNoticeWarningCertificateNotAuthenticated has no certificate\n");
		return eNoticeActionDelete;	// Don't attempt to authenticate a certificate which does not exist.  The code is likely to crash down the stack
		}
	NoticeStatus_Hide();
	m_pButtonConnectNow->hide();
	if (rtiTreeItemFocus == RTI(TContact))
		{
		TContact * pContactFocus = (TContact *)piTreeItemFocus;		// Contact having the focus. This way we may display a customized message to the user and display the button "Connect Now"
		PSZUC pszContactName = pContactFocus->TreeItem_PszGetNameDisplay();
		if (!m_pAccount->Socket_FIsConnected())
			{
			Widget_SetToolTipFormat_VE_Gsb(INOUT m_pButtonConnectNow, "Force an immediate unsecure connection with the server.  You may wish to connect now to chat with <b>^s</b> helping you to authenticate the certificate.", pszContactName);
			m_pButtonConnectNow->show();
			}
		else
			{
			if (pCertificate != NULL)
				{
				PSZUC pszServerContact = pContactFocus->m_strJidBare.PchFindCharacter('@');
				if (pszServerContact != NULL)
					{
					pszServerContact++;	// Skip the '@'
					if (!m_pAccount->m_strServerName.FCompareStringsNoCase(pszServerContact))
						{
						NoticeStatus_SetTextFormat_VE("<b>^s</b> uses server <b>^s</b> which may be unable to help you authenticating the certificate.", pszContactName, pszServerContact);
						return eNoticeActionDisplay;
						}
					}
				NoticeStatus_SetTextFormat_VE("You are now connected to server <b>^S</b>.  Please take this opportunity to chat with <b>^s</b> to authenticate the certificate.", &m_pAccount->m_strServerName, pszContactName);
				return eNoticeActionDisplay;
				}
			} // if...else
		} // if (contact)
	NoticeStatus_SetTextFormat_VE("Please confirm with friends or the site operators to verify this certificate is correct.");
	switch (rtiTreeItemFocus)
		{
	case RTI(TTreeItemMyInbox):
	case RTI(TCertificates):		// Since we are displaying all certificates, why not displaying notices of all unauthenticated certificates
		return eNoticeActionDisplay;
	default:
		return (ENoticeAction)(PGetRuntimeInterfaceOf_TCertificate(piTreeItemFocus) == PGetCertificate_YZ());
		}
	} // ETreeItemGotFocus()

//	One-click button to approve the certificate
void
WNoticeWarningCertificateNotAuthenticated::SL_CertificateApprove()
	{
	m_pAccount->Certificate_PinFingerprintSha1(PGetCertificate_YZ());
	m_pAccount->Socket_Connect();
	}

//	Display the dialog to view (and manually authenticate) the certificate
void
WNoticeWarningCertificateNotAuthenticated::SL_CertificateView()
	{
	new WDockingCertificate(m_pAccount);
	}

// Discard any certificate error and connect to the server anyway.
void
WNoticeWarningCertificateNotAuthenticated::SL_ConnectNow()
	{
	NoticeStatus_SetTextFormat_VE("Connecting to server <b>^S</b>...", &m_pAccount->m_strServerName);
	m_pAccount->Socket_ConnectUnsecured();
	}

BOOL
WNoticeWarningCertificateNotAuthenticated::FDeleteDuplicateNotice(INOUT INotice * piwNoticeNew)
	{	
	Assert(piwNoticeNew->EGetRuntimeClass() == RTI(WNoticeWarningCertificateNotAuthenticated));
	return (PGetRuntimeInterfaceOf_TCertificate(((WNoticeWarningCertificateNotAuthenticated *)piwNoticeNew)->m_pAccount) == PGetCertificate_YZ());	// There should be only one one notice per certificate.  Therefore if two notices point to the same certificate, they are considered duplicates
	}


TCertificate *
WNoticeWarningCertificateNotAuthenticated::PGetCertificate_YZ()
	{
	return m_pAccount->Certificate_PGet_YZ();	// May return NULL
	}

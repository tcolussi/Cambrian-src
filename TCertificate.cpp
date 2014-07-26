///////////////////////////////////////////////////////////////////////////////////////////////////
//	TCertificate.cpp
//
//	Classes for 'pinning' the certificates.
//	If Cambrian detects a new encryption certificate, then it notifies the user about it.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WLayoutCertificate.h"

void
CSslCertificate::GetCertificateName(CStr * pstrName) const
	{
	*pstrName = subjectInfo(QSslCertificate::Organization).join(QLatin1Char(' '));
	}

PSZUC
CSslCertificate::GetCertificateIssuer(OUT CStr * pstrIssuer) const
	{
	*pstrIssuer = issuerInfo(QSslCertificate::Organization).join(QLatin1Char(' '));
	return pstrIssuer->PszuGetDataNZ();
	}

QByteArray
CSslCertificate::GetHashValue(EHashAlgorithm eHashAlgorithm) const
	{
	return QCryptographicHash::hash(toDer(), (QCryptographicHash::Algorithm)eHashAlgorithm);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
ICertificate::ICertificate()
	{
	}

ICertificate::~ICertificate()
	{
	}

void
ICertificate::NavigationTree_DisplayCertificates(ICertificate * pCertificateParent)
	{
	TreeItemW_DisplayWithinNavigationTree(IN pCertificateParent);
	TreeItemW_SetIcon(eMenuIconCertificate);
	TCertificate ** ppCertificateStop;
	TCertificate ** ppCertificate = m_arraypaCertificates.PrgpGetCertificatesStop(OUT &ppCertificateStop);
	while (ppCertificate != ppCertificateStop)
		{
		TCertificate * pCertificate = *ppCertificate++;
		pCertificate->NavigationTree_DisplayCertificatesAndServers(this);
		}
	}

void
TCertificate::NavigationTree_DisplayCertificatesAndServers(ICertificate * pCertificateParent)
	{
	NavigationTree_DisplayCertificates(pCertificateParent);
	TCertificateServerName ** ppServerStop;
	TCertificateServerName ** ppServer = m_arraypaServers.PrgpGetServersStop(OUT &ppServerStop);
	while (ppServer != ppServerStop)
		{
		TCertificateServerName * pServer = *ppServer++;
		Assert(pServer != NULL);
		Assert(pServer->m_pCertificateParent == this);
		pServer->NavigationTree_DisplayServer();
		} // while
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
IXmlExchange *
TCertificate::S_PaAllocateCertificate(PVOID pCertificateParentIssuer)
	{
	return new TCertificate((TCertificate *)pCertificateParentIssuer);
	}

TCertificate::TCertificate(TCertificate * pCertificateParentIssuer)
	{
	Assert(PGetRuntimeInterfaceOf_ITreeItem(pCertificateParentIssuer) == pCertificateParentIssuer);	// A root certificate allocated by S_PaAllocateCertificate() will have a pointer to an interface ICertificate/TCertificates rather to the class TCertificate.  This is why it is important to call InitializeAsRootCertificates() after loading the root certificates.
	m_pCertificateParentIssuer = pCertificateParentIssuer;
	}

TCertificate::~TCertificate()
	{
	m_arraypaServers.DeleteAllTreeItems();
	}

TCertificateServerName *
TCertificate::PFindServerOrAllocate(const CStr & strServerName)
	{
	TCertificateServerName * pServer;
	TCertificateServerName ** ppServerStop;
	TCertificateServerName ** ppServer = m_arraypaServers.PrgpGetServersStop(OUT &ppServerStop);
	while (ppServer != ppServerStop)
		{
		pServer = *ppServer++;
		Assert(pServer != NULL);
		Assert(pServer->m_pCertificateParent == this);
		Assert(pServer->EGetRuntimeClass() == RTI(TCertificateServerName));
		if (pServer->m_strServerName.FCompareBinary(strServerName))
			goto Done;
		} // while
	pServer = new TCertificateServerName(this);
	m_arraypaServers.Add(PA_CHILD pServer);
	pServer->m_strServerName = strServerName;
	if (m_paTreeItemW_YZ != NULL)
		pServer->NavigationTree_DisplayServer();	// The certificate is already visible in the Navigation Tree, therefore add the Server as well
	Done:
	return pServer;
	}

//	Return the pointer of the first account authenticating the certificate.
TAccountXmpp *
TCertificate::PFindAccountAuthenticating() const
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = g_arraypAccounts.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
		if (pAccount->Certificate_FCompareFingerprints(this))
			return pAccount;
		}
	return NULL;
	}

void
CArrayPtrCertificates::GetAllCertificatesAndServers(IOUT CArrayPtrCertificates * parraypCertificates, IOUT CArrayPtrCertificateServers * parraypServers) const
	{
	TCertificate ** ppCertificateStop;
	TCertificate ** ppCertificate = PrgpGetCertificatesStop(OUT &ppCertificateStop);
	while (ppCertificate != ppCertificateStop)
		{
		const TCertificate * pCertificate = *ppCertificate++;
		pCertificate->m_arraypaCertificates.GetAllCertificatesAndServers(IOUT parraypCertificates, IOUT parraypServers);
		parraypCertificates->Append(IN &pCertificate->m_arraypaCertificates);
		parraypServers->Append(IN &pCertificate->m_arraypaServers);
		}
	}

//	ICertificate::IXmlExchange::XmlExchange()
void
ICertificate::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	ITreeItem::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeObjects('C', INOUT_F_UNCH_S &m_arraypaCertificates, TCertificate::S_PaAllocateCertificate, this);
	}

//	TCertificate::IXmlExchange::XmlExchange()
void
TCertificate::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	ICertificate::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeStr("CertificateName", INOUT_F_UNCH_S &m_strCertificateName);
	pXmlExchanger->XmlExchangeBin("Data", INOUT_F_UNCH_S &m_binDataBinary);
	pXmlExchanger->XmlExchangeObjects('S', INOUT_F_UNCH_S &m_arraypaServers, TCertificateServerName::S_PaAllocateServer, this);
	}

//	TCertificate::ITreeItem::TreeItem_PszGetNameDisplay()
PSZUC
TCertificate::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	return _PszGetDisplayNameOr(m_strCertificateName);
	}

//	TCertificate::IRuntimeObject::PGetRuntimeInterface()
//
//	Enable the TCertificate object to respond to additional interfaces, typically TProfile and TAccountXmpp.
POBJECT
TCertificate::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	POBJECT pObject = ICertificate::PGetRuntimeInterface(rti, piParent);
	if (pObject != NULL)
		return pObject;
	return PGetRuntimeInterfaceOf_(rti, m_arraypaServers.PGetObjectUnique_YZ());	// Use the first server (if any) to fetch the interface.
	}

//	TCertificate::ITreeItem::TreeItem_GotFocus()
void
TCertificate::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayoutAutoDelete(PA_CHILD new WLayoutCertificate(this));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
TCertificateServerName::TCertificateServerName(TCertificate * pCertificateParent)
	{
	Assert(pCertificateParent != NULL);
	m_pCertificateParent = pCertificateParent;
	}

TCertificateServerName::~TCertificateServerName()
	{
	}

//	Retrun one of the socket from one of the account.
//	Since multiple accounts may exist on the same server, the socket object may be different, however
//	the encrytion should be the same regardless of the account.
CSocketXmpp *
TCertificateServerName::PGetSocket_YZ() const
	{
	TAccountXmpp * pAccount = (TAccountXmpp *)m_arraypAccounts.PvGetElementFirst_YZ();	// Pick the first active account
	if (pAccount != NULL)
		{
		Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
		return pAccount->PGetSocket_YZ();
		}
	return NULL;	// Returning NULL is not a bug; it happens when the object TCertificateServerName has been unserialized and the socket connection was never initialized, or failed to connect with the server because the server is unreachable.
	}

void
TCertificateServerName::Approve()
	{
	m_dtuCertificateApproved = QDateTime::currentDateTimeUtc();
	}

BOOL
TCertificateServerName::FIsApproved() const
	{
	return (m_dtuCertificateApproved.isValid() && !m_dtuCertificateRevoked.isValid());
	}

//	TCertificateServerName::IRuntimeObject::PGetRuntimeInterface()
//
//	Enable the TCertificateServerName object to respond to the interface of its parent TCertificate as well as
//	the additional interfaces of TAccountXmpp (if unique)
POBJECT
TCertificateServerName::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Report(piParent == NULL);
	/*
	if (rti == RTI(TCertificate))
		return m_pCertificateParent;
	if (rti == RTI(TAccountXmpp))
		return m_arraypAccounts.PGetObjectUnique_YZ();	// May return NULL
	*/
	POBJECT pObject = ITreeItem::PGetRuntimeInterface(rti, m_pCertificateParent);
	if (pObject != NULL)
		return pObject;
	return PGetRuntimeInterfaceOf_(rti, m_arraypAccounts.PGetObjectUnique_YZ());
	}

//	TCertificateServerName::IXmlExchange::XmlExchange()
void
TCertificateServerName::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	Assert(pXmlExchanger != NULL);
	ITreeItem::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeStr("Name", INOUT_F_UNCH_S &m_strServerName);
	pXmlExchanger->XmlExchangeStr("IP", INOUT_F_UNCH_S &m_strLastKnownIP);
	pXmlExchanger->XmlExchangeDateTime("DateApproved", INOUT_F_UNCH_S &m_dtuCertificateApproved);
	pXmlExchanger->XmlExchangeDateTime("DateRevoked", INOUT_F_UNCH_S &m_dtuCertificateRevoked);
//	m_strServerName.InitOnlyIfEmpty(IN m_strNameDisplay);	// Make sure the server name is not empty
	}

//	TCertificateServerName::ITreeItem::TreeItem_PszGetNameDisplay()
PSZUC
TCertificateServerName::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	return _PszGetDisplayNameOr(m_strServerName);
	}

//	TCertificateServerName::ITreeItem::TreeItem_MenuAppendActions()
void
TCertificateServerName::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	if (!m_dtuCertificateApproved.isValid())
		pMenu->ActionAdd(eMenuIconServerSecure);
	else
		pMenu->ActionAdd(eMenuIconServerDeny);
	//pMenu->ActionsAdd(c_rgzeActionsMenuAccount);
	}

//	TCertificateServerName::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TCertificateServerName::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuIconServerSecure:
		Approve();
		break;
	default:
		return ITreeItem::TreeItem_EDoMenuAction(eMenuAction);
		}
	NavigationTree_IconUpdate();
	return ezMenuActionNone;
	}

//	TCertificateServerName::ITreeItem::TreeItem_GotFocus()
void
TCertificateServerName::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayoutAutoDelete(PA_CHILD new WLayoutCertificateServerName(this));
	}

void
TCertificateServerName::NavigationTree_DisplayServer()
	{
	TreeItemW_DisplayWithinNavigationTree(m_pCertificateParent);
	NavigationTree_IconUpdate();
	}

void
TCertificateServerName::NavigationTree_IconUpdate()
	{
	EMenuAction eMenuActionIcon = eMenuIconServerSecure; // eMenuIconServerWarning;
	if (m_dtuCertificateApproved.isValid())
		{
		eMenuActionIcon = m_dtuCertificateRevoked.isValid() ? eMenuIconServerDeny : eMenuIconServerSecure;
		}
	TreeItemW_SetIcon(eMenuActionIcon);
	}

IXmlExchange *
TCertificateServerName::S_PaAllocateServer(PVOID pvCertificateParent)
	{
	Assert(pvCertificateParent != NULL);
	return new TCertificateServerName((TCertificate *)pvCertificateParent);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
TCertificates::TCertificates()
	{
	}

TCertificates::~TCertificates()
	{
	}

void
TCertificates::FlushDataAndCauseMemoryLeak()
	{
	m_paTreeItemW_YZ = NULL;
	m_arraypaCertificates.RemoveAllElements();
	}

void
TCertificates::InitializeAsRootCertificates()
	{
	TCertificate ** ppCertificateStop;
	TCertificate ** ppCertificate = m_arraypaCertificates.PrgpGetCertificatesStop(OUT &ppCertificateStop);
	while (ppCertificate != ppCertificateStop)
		{
		TCertificate * pCertificate = *ppCertificate++;
		Assert(pCertificate->m_pCertificateParentIssuer == NULL || (ICertificate *)pCertificate->m_pCertificateParentIssuer == this);
		pCertificate->m_pCertificateParentIssuer = NULL;
		}
	}

//	TCertificates::ITreeItem::TreeItem_PszGetNameDisplay()
PSZUC
TCertificates::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	return (PSZUC)"Certificates";
	}

//	TCertificates::ITreeItem::TreeItem_GotFocus()
void
TCertificates::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayoutAutoDelete(PA_CHILD new WLayoutCertificates);
	}

//	TCertificates::IRuntimeObject::PGetRuntimeInterface()
//
//	Enable the TCertificates object to respond to the interface of a TAccountXmpp if unique (TBD)
POBJECT
TCertificates::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Report(piParent == NULL);
	/*
	if (rti == RTI(TAccountXmpp))
		return g_arraypAccounts.PvGetElementUnique_YZ();	// May return NULL
	*/
	return ITreeItem::PGetRuntimeInterface(rti, piParent);
	}

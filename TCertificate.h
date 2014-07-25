///////////////////////////////////////////////////////////////////////////////////////////////////
//	TCertificate.h
//
//	Classes for 'pinning' the certificates.
//	If Cambrian detects a new encryption certificate, then it notifies the user about it.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef TCERTIFICATE_H
#define TCERTIFICATE_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
class WLayoutCertificate;
class WLayoutCertificates;
class WLayoutCertificateServerName;

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Class holding the name of a server using a given encryption certificate.
//	If a server change its encryption certificate, then a new instance of this class will be created and will be stored as a child of the TCertificate.
class TCertificateServerName : public ITreeItem
{
public:
	TCertificate * m_pCertificateParent;		// Pointer of the encryption certificate the server is using.
	CArrayPtrAccountsXmpp m_arraypAccounts;		// Accounts using this server (typically there is one account per server, however there may be multiple XMPP accounts sharing the same server)
	CArrayPasz m_arraypaszErrors;				// All the certificate errors related to the server (of course this is not the most elegant solution as many accounts may share the same server, however it is better than not reporting any error at all)

	// The following values are serialized
	CStr m_strServerName;						// Name of the server.  By default, this value is identical to m_strNameDisplay, however the user may wish to rename the server name to something more friendly, especially if the host is an IP address.
	CStr m_strLastKnownIP;						// Value of the last known IP address of the server the server.  This value is a good backup in case the DNS fails, or the domain has been seized by gangsters.
	QDateTime m_dtuCertificateApproved;			// Date when the certificate was approved by the user for the specific server
	QDateTime m_dtuCertificateRevoked;			// Date when the certificate was revoked by the user (this is very rare)

public:
	TCertificateServerName(TCertificate * pCertificateParent);
	~TCertificateServerName();
	CSocketXmpp * PGetSocket_YZ() const;

	void Approve();
	BOOL FIsApproved() const;

	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piObjectSecondary) const;					// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);					// From IXmlExchange
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;								// From ITreeItem
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);					// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);			// From ITreeItem
	virtual void TreeItem_GotFocus();												// From ITreeItem

	void NavigationTree_DisplayServer();
	void NavigationTree_IconUpdate();
public:
	static IXmlExchange * S_PaAllocateServer(PVOID pvCertificateParent);	// This static method must be compatible with interface PFn_PaAllocateXmlObject()
	RTI_IMPLEMENTATION(TCertificateServerName)
}; // TCertificateServer

class CArrayPtrCertificateServers : public CArrayPtrTreeItems
{
public:
	inline TCertificateServerName ** PrgpGetServersStop(OUT TCertificateServerName *** pppServerStop) const { return (TCertificateServerName **)PrgpvGetElementsStop(OUT (void ***)pppServerStop); }
};

class CArrayPtrCertificates : public CArrayPtrTreeItems
{
public:
	inline TCertificate ** PrgpGetCertificatesStop(OUT TCertificate *** pppCertificateStop) const { return (TCertificate **)PrgpvGetElementsStop(OUT (void ***)pppCertificateStop); }
	void GetAllCertificatesAndServers(IOUT CArrayPtrCertificates * parraypCertificates, IOUT CArrayPtrCertificateServers * parraypServers) const;
};

//	Interface representing a certificate (each certificate may issue children certificates)
class ICertificate : public ITreeItem
{
protected:
	CArrayPtrCertificates m_arraypaCertificates;	// All the children certificates issued by the current certificate
protected:
	ICertificate();
	~ICertificate();
	void NavigationTree_DisplayCertificates(ICertificate * pCertificateParent);
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);	// From IXmlExchange
	friend class CArrayPtrCertificates;
	friend class CChatConfiguration;
	friend class WLayoutCertificates;
}; // ICertificate

class TCertificate : public ICertificate
{
public:
	CStr m_strCertificateName;	// Name of the certificate, extracted from the binary data.  The value ITreeItem::m_strNameDisplayTyped also contains the name, however the user may change it to something more descriptive.
	CBin m_binDataBinary;		// Certificate in binary format
	TCertificate * m_pCertificateParentIssuer;		// Pointer to the parent certificate who issued the current certificate (if this pointer is NULL, it means the certificate is a root certificate)
	CArrayPtrCertificateServers m_arraypaServers;	// List of servers using this certificate
public:
	explicit TCertificate(TCertificate * pCertificateParentIssuer);
	~TCertificate();
	TCertificateServerName * PFindServerOrAllocate(const CStr & strServerName);
	void GetSslCertificate(OUT CSslCertificate * pSslCertificate) const;
	TAccountXmpp * PFindAccountAuthenticating() const;

	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piObjectSecondary) const;		// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);		// From IXmlExchange
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;					// From ITreeItem
//	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);		// From ITreeItem
	virtual void TreeItem_GotFocus();									// From ITreeItem
	void NavigationTree_DisplayCertificatesAndServers(ICertificate * pCertificateParent);

public:
	static IXmlExchange * S_PaAllocateCertificate(PVOID pCertificateParentIssuer);	// This static method must be compatible with interface PFn_PaAllocateXmlObject()
	RTI_IMPLEMENTATION(TCertificate)
}; // TCertificate


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Tree Item displaying the word "Certificates" so the user may view all the certificates
class TCertificates : public ICertificate
{
public:
	TCertificates();
	~TCertificates();
	void FlushDataAndCauseMemoryLeak();
	void InitializeAsRootCertificates();

	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piObjectSecondary) const;		// From IRuntimeObject
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;					// From ITreeItem
	virtual void TreeItem_GotFocus();									// From ITreeItem

	RTI_IMPLEMENTATION(TCertificates)
};

class CTreeItemWelcome : public ITreeItem
{
public:
	CTreeItemWelcome();
	virtual void TreeItem_GotFocus();		// From ITreeItem

	RTI_IMPLEMENTATION(CTreeItemWelcome)
};


#endif // TCERTIFICATE_H

///////////////////////////////////////////////////////////////////////////////////////////////////
//	WLayoutCertificate.h
//
//	Various widgets to pin and display properties of encryption certificates.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef WCERTIFICATE_H
#define WCERTIFICATE_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
/*
class WLayoutWelcome : public WLayout
{
	Q_OBJECT
public:
	WLayoutWelcome();
	virtual void Layout_NoticeDisplay(IN INotice * piwNotice);	// From WLayout

public slots:
	void SL_AccountRegister();
	void SL_AccountLogin();
	void SL_AccountAcceptInvitation();
};
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
class WLayoutCertificateServerName : public WLayout
{
public:
	WLayoutCertificateServerName(TCertificateServerName * pServer);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	The class WCertificateAuthenticator is shared by WDockingCertificate and WLayoutCertificate
class WCertificateAuthenticator : public WGroupBox
{
	Q_OBJECT
protected:
	TCertificate * m_pCertificate;
	WEdit * m_pwEditFingerprint;
public:
	explicit WCertificateAuthenticator(TCertificate * pCertificate);
public slots:
	void SL_Authenticate();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class WDockingCertificate : public WDockingSplitter
{
public:
	WDockingCertificate(TAccountXmpp * pAccount);
};

class WLayoutCertificate : public WLayout
{
public:
	explicit WLayoutCertificate(TCertificate * pCertificate);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Widget displaying all the certificates
class WLayoutCertificates : public WLayout
{
public:
	WLayoutCertificates();
};


#endif // WCERTIFICATE_H

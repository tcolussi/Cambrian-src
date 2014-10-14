#ifndef WLAYOUTBROWSER_H
#define WLAYOUTBROWSER_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
class WLayoutBrowser  : public WLayout
{
	Q_OBJECT
protected:
	TProfile * m_pProfile;		// Which profile the browser belongs to (this is important for sandboxing)
	CStr * m_pstrUrlAddress_NZ;	// INOUT: Address to initialize the browsing, and where to store the last known URL when closing the browsing session.
	WEdit * m_pwEdit;
	WButtonIconForToolbar * m_pwButtonBack;
	WButtonIconForToolbar * m_pwButtonForward;

	OJapiCambrian * m_paCambrian;

	QWebView * m_pwWebView;
	QWebFrame * m_poFrame;
public:
	WLayoutBrowser(TProfile * pProfile, CStr * pstrUrlAddress_NZ);
	~WLayoutBrowser();
	void NavigateToAddress(const CStr & strAddress);
public slots:
	void SL_InitJavaScript();
	void SL_NavigateToAddress();
	void SL_GoBack();
	void SL_GoForward();
	void SL_UrlChanged(const QUrl & url);
}; // WLayoutBrowser

void LaunchBrowser(const CStr & strUrlAbsolute);
void LaunchApplication(const QString & sNameApplication);
void LaunchApplication(PSZAC pszNameApplication, PSZUC pszQueryString = NULL);
void LaunchApplicationWithIdentifier(PSZAC pszNameApplication, PSZUC pszIdentifier);
void LaunchApplicationWithIdentifierGroup(PSZAC pszNameApplication, TGroup * pGroup);
CStr StrGetUrlForApplication(PSZAC pszNameApplication);

#endif // WLAYOUTBROWSER_H

#ifndef WLAYOUTBROWSER_H
#define WLAYOUTBROWSER_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class CVariant : public QVariant
{
public:
	CVariant(const QObject * poObject);
};

class OCambrian;

class OSettings : public QObject
{
	Q_OBJECT
public:
	OSettings(OCambrian * poCambrian);
	virtual ~OSettings();

	Q_PROPERTY(bool AudioEnabled READ AudioEnabled WRITE AudioEnabled)
	bool AudioEnabled() const;
	void AudioEnabled(bool fEnable);

public slots:
	//void AudioEnabled(int fEnable);
};


class OProfile : public QObject
{
	Q_OBJECT
public:
	OProfile(OCambrian * poCambrian);
	virtual ~OProfile();

public slots:
	QString CreateNew(const QString & sNameProfile);
	void SwitchTo(const QString & sIdProfile);
	QString DataGet(const QString & sIdProfile);
	void DataUpdate(const QString & sIdProfile, const QString & sDataProfile);

protected:
	TProfile * PFindProfileByID(const QString & sIdProfile) const;
};

class OCambrian : public QObject
{
protected:
	OSettings m_oSettings;
	OProfile m_oProfile;

public:
	Q_OBJECT
	Q_PROPERTY(QVariant Settings READ Settings)
	Q_PROPERTY(QVariant Profile READ Profile)
public:
	OCambrian(QObject * pParent);
	virtual ~OCambrian();
	QVariant Settings();
	QVariant Profile();

public slots:
	void SendBitcoin(int n);
	void MessageSendTo(const QString & sContactTo, const QString & sMessage);
	//QVariant Settings();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class WLayoutBrowser  : public WLayout
{
	Q_OBJECT
protected:
	TBrowser * m_pBrowser;
	WEdit * m_pwEdit;
	WButtonIcon * m_pwButtonBack;
	WButtonIcon * m_pwButtonForward;

	OCambrian * m_paCambrian;

	QWebView * m_pwWebView;
	QWebFrame * m_poFrame;
public:
	WLayoutBrowser(TBrowser * pBrowser);
	void NavigateToAddress(const CStr & strAddress);
public slots:
	void SL_InitJavaScript();
	void SL_NavigateToAddress();
	void SL_GoBack();
	void SL_GoForward();
	void SL_UrlChanged(QUrl url);
}; // WLayoutBrowser

#endif // WLAYOUTBROWSER_H

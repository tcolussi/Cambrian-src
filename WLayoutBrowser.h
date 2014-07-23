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

//	OPoll
//
//	JavaScript interface to interact with a ballot.
//
//	A poll and a ballot are the same.  In JavaScript Cambrian uses the word 'poll', and in C++ Cambrian uses the word 'ballot'
//
class OPoll : public QObject
{
	Q_OBJECT
protected:
	CEventBallotSent * m_pBallot;
	QString m_sTitle;
	QString m_sDescription;

public:
	OPoll(CEventBallotSent * pBallot);
	virtual ~OPoll();
	QString id() const;
	QString title() const;
	void title(QString & sTitle);
	QString description() const;
	void description(QString & sDescription);

	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString title READ title WRITE title)
	Q_PROPERTY(QString description READ description WRITE description)
public slots:
	void save();
};

//	Helper for the Ballotmaster
class OPolls : public QObject
{
	Q_OBJECT
protected:
	TApplicationBallotmaster * m_pBallotmaster;

public:
	OPolls(OCambrian * poCambrian);
	~OPolls();

public slots:
	QVariant build();
	QVariant list();

	/* Old stuff with XML
	QString list() const;
	void save(QString sXmlPolls);
	*/
};

class OCambrian : public QObject
{
public:
	TProfile * m_pProfile;
protected:
	OSettings m_oSettings;
	OProfile m_oProfile;
	OPolls * m_paPolls;

public:
	Q_OBJECT
	Q_PROPERTY(QVariant Settings READ Settings)
	Q_PROPERTY(QVariant Profile READ Profile)
	Q_PROPERTY(QVariant Polls READ Polls)
public:
	OCambrian(TProfile * pProfile, QObject * pParent);
	virtual ~OCambrian();
	QVariant Settings();
	QVariant Profile();
	QVariant Polls();

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
	TProfile * m_pProfile;		// Which profile the browser belongs to (this is important for sandboxing)
	CStr * m_pstrUrlAddress_NZ;	// INOUT: Address to initialize the browsing, and where to store the last known URL when closing the browsing session.
	WEdit * m_pwEdit;
	WButtonIconForToolbar * m_pwButtonBack;
	WButtonIconForToolbar * m_pwButtonForward;

	OCambrian * m_paCambrian;

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
	void SL_UrlChanged(QUrl url);
}; // WLayoutBrowser

#endif // WLAYOUTBROWSER_H

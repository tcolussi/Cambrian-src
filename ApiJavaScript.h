//	ApiJavaScript.h
//
//	Cambrian objects available in JavaScript
#ifndef APIJAVASCRIPT_H
#define APIJAVASCRIPT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class OJapiCambrian;

//	Every object offering a JavaScript API must inherit from OJapi
//
//	Eventually this object will implement the security necessary for HTML applications.
class OJapi : public QObject
{
protected:
//	CSecurityContect * m_pSecurity;
public:
	OJapi(QObject * pObjectParent = NULL) : QObject(pObjectParent) { }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Core object shared with OPoll and OPollResults
class OJapiPollCore : public OJapi
{
	Q_OBJECT
public:
	CEventBallotSent * m_pBallot;

public:
	OJapiPollCore(CEventBallotSent * pBallot);

	QString id() const;
	QString status() const;
	QString title() const;
	void title(const QString & sTitle);
	QString description() const;
	void description(const QString & sDescription);
	QStringList choices() const;
	void choices(const QStringList & lsChoices);
	QDateTime dateStarted() const;
	QDateTime dateStopped() const;

	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString status READ status)
	Q_PROPERTY(QString title READ title WRITE title)
	Q_PROPERTY(QString description READ description WRITE description)
	Q_PROPERTY(QStringList choices READ choices WRITE choices)
	Q_PROPERTY(QDateTime dateStarted READ dateStarted)
	Q_PROPERTY(QDateTime dateStopped READ dateStopped)

}; // OJapiPollCore


class OJapiPollResults : public OJapiPollCore
{
public:
	OJapiPollResults(CEventBallotSent * pBallot);

public slots:

}; // OJapiPollResults
//typedef QObject * POJapiPollResults;
#define POJapiPollResults QObject *

//	OJapiPoll
//
//	JavaScript interface to interact with a ballot.
//
//	A poll and a ballot are the same.  In JavaScript Cambrian uses the word 'poll', and in C++ Cambrian uses the word 'ballot'
//
class OJapiPoll : public OJapiPollCore
{
	Q_OBJECT
public:
	OJapiPollResults m_oResults;

public:
	OJapiPoll(CEventBallotSent * pBallot);

public slots:
	bool save();
	void destroy();
	void start();
	void stop();
	POJapiPollResults getResults() CONST_MCC;

public:
	static QVariant S_VariantFromValue(CEventBallotSent * pBallot);
};
//typedef QObject * POJapiPoll;	// In order for
#define POJapiPoll QObject *

//	Helper for the Ballotmaster
class OJapiPolls : public OJapi
{
	Q_OBJECT
protected:
	TApplicationBallotmaster * m_pBallotmaster;

public:
	OJapiPolls(OJapiCambrian * poCambrian);
	~OJapiPolls();
	POJapiPoll PGetOJapiPoll(CEventBallotSent * pBallot);
	POJapiPoll PCreateNewPollFromTemplate(CEventBallotSent * pPollTemplate);
	CEventBallotSent * PFindPollByID(TIMESTAMP tsIdPoll) const;
	CEventBallotSent * PFindPollByID(const QString & sIdPoll) const;

public slots:
	POJapiPoll build();
	POJapiPoll build(QObject * pObjectPollTemplate);
	POJapiPoll build(const QString & sIdPollTemplate);
	POJapiPoll get(const QString & sIdPoll);
	QVariant getList();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class OSettings : public QObject
{
	Q_OBJECT
public:
	OSettings(OJapiCambrian * poCambrian);
	virtual ~OSettings();

	Q_PROPERTY(bool AudioEnabled READ AudioEnabled WRITE AudioEnabled)
	bool AudioEnabled() const;
	void AudioEnabled(bool fEnable);

public slots:
	//void AudioEnabled(int fEnable);
};


///////////////////////////////////////////////////////////////////////////////////////////////////
class OJapiProfile : public QObject
{
	Q_OBJECT
public:
	OJapiProfile(OJapiCambrian * poCambrian);
	virtual ~OJapiProfile();

public slots:
	QString CreateNew(const QString & sNameProfile);
	void SwitchTo(const QString & sIdProfile);
	QString DataGet(const QString & sIdProfile);
	void DataUpdate(const QString & sIdProfile, const QString & sDataProfile);

protected:
	TProfile * PFindProfileByID(const QString & sIdProfile) const;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class OJapiCambrian : public OJapi
{
public:
	TProfile * m_pProfile;
protected:
	OSettings m_oSettings;
	OJapiProfile m_oProfile;
	OJapiPolls * m_paPolls;

public:
	Q_OBJECT
	Q_PROPERTY(QVariant Settings READ Settings)
	Q_PROPERTY(QVariant Profile READ Profile)
	Q_PROPERTY(QVariant polls READ polls)
public:
	OJapiCambrian(TProfile * pProfile, QObject * pParent);
	virtual ~OJapiCambrian();
	QVariant Settings();
	QVariant Profile();
	QVariant polls();

public slots:
	void SendBitcoin(int n);
	void MessageSendTo(const QString & sContactTo, const QString & sMessage);
	//QVariant Settings();
};

#endif // APIJAVASCRIPT_H

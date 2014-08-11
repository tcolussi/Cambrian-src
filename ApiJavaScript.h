//	ApiJavaScript.h
//
//	Cambrian objects available in JavaScript
#ifndef APIJAVASCRIPT_H
#define APIJAVASCRIPT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class CListVariants : public QVariantList
{
public:
	void AddContact(TContact * pContact);
	void AddContacts(const CArrayPtrContacts & arraypContacts);
	void AddGroupMatchingType(TGroup * pGroup, EGroupType eGroupType);
	void AddGroupsMatchingType(const CArrayPtrGroups & arraypGroups, EGroupType eGroupType) { }
	void AddGroup(TGroup * pGroup);
	void AddAudience(TGroup * pGroup);
};

class OJapi;	
	class OJapiCambrian;
	class OJapiApps;
	class OJapiAppBallotmaster;

	class OJapiPollCore;
		class OJapiPoll;
		class OJapiPollResults;
	class OJapiPollResultsStats;
	class OJapiPollResultsComment;

	class OJapiContact;
	class OJapiGroup;

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

#define OJapiList	QVariant	// Every list is stored as a QVariant, however it makes more sense to specify OJapiList than QVariant because QVariant may be used for other purposes

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Core object shared with OPoll and OPollResults
class OJapiPollCore : public OJapi
{
	Q_OBJECT
public:
    CEventBallotPoll * m_pBallot;

public:
    OJapiPollCore(CEventBallotPoll * pBallot);

	QString id() const;
	QString type() const { return c_sEmpty; }
	QString title() const;
	void title(const QString & sTitle);
	QString description() const;
	void description(const QString & sDescription);
	QStringList options() const;
	void options(const QStringList & lsChoices);
	QDateTime dateStarted() const;
	QDateTime dateStopped() const;
	QString status() const;
    int pollTimeLength() const;
    void pollTimeLength(int cSeconds);

	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString type READ type)
	Q_PROPERTY(QString title READ title WRITE title)
	Q_PROPERTY(QString description READ description WRITE description)
	Q_PROPERTY(QStringList options READ options WRITE options)
	Q_PROPERTY(QDateTime dateStarted READ dateStarted)
	Q_PROPERTY(QDateTime dateStopped READ dateStopped)
	Q_PROPERTY(QString status READ status)
	Q_PROPERTY(int pollTimeLength READ pollTimeLength WRITE pollTimeLength)

}; // OJapiPollCore


///////////////////////////////////////////////////////////////////////////////////////////////////
class OJapiPollResultsStats : public OJapi
{
    Q_OBJECT
public:
    CEventBallotPoll * m_pBallot;

public:
    OJapiPollResultsStats(CEventBallotPoll * pBallot);

    int pending() const { return 0 ; }
    int invalid() const { return 0; }
    int responded() const { return 0; }
    int sent() const { return 0; }

    Q_PROPERTY(int pending READ pending)
    Q_PROPERTY(int invalid READ invalid)
    Q_PROPERTY(int responded READ responded)
    Q_PROPERTY(int sent READ sent)
};
#define POJapiPollResultsStats		POJapi

///////////////////////////////////////////////////////////////////////////////////////////////////
class _CEventBallotVote;
class OJapiPollResultsComment : public OJapi
{
    Q_OBJECT
    _CEventBallotVote * m_pComment;

public:
    OJapiPollResultsComment (_CEventBallotVote * pComment );

    QDateTime date();
    QString comment();
    QString name();

    Q_PROPERTY(QDateTime date READ date)
    Q_PROPERTY(QString comment READ comment)
    Q_PROPERTY(QString name READ name)
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class OJapiPollResults : public OJapiPollCore
{
	Q_OBJECT
    OJapiPollResultsStats m_oStats;

public:
    OJapiPollResults(CEventBallotPoll * pBallot);

    QVariant comments() const;
    QVariant counts() const;
    POJapiPollResultsStats stats() { return &m_oStats; }

    Q_PROPERTY(QVariant comments READ comments)
    Q_PROPERTY(QVariant counts READ counts)
    Q_PROPERTY(POJapiPollResultsStats stats READ stats)

public slots:
}; // OJapiPollResults
#define POJapiPollResults	POJapi


///////////////////////////////////////////////////////////////////////////////////////////////////

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
    OJapiPoll(CEventBallotPoll * pBallot);
	~OJapiPoll();

public slots:
	bool save();
	void destroy();
	void start();
	void stop();
	POJapiPollResults getResults() CONST_MCC;
};
#define POJapiPoll		POJapi

class OJapiAppBallotmaster : public OJapi
{
	Q_OBJECT
protected:
	CServiceBallotmaster * m_pServiceBallotmaster;
//	TApplicationBallotmaster * m_pBallotmaster;

public:
	OJapiAppBallotmaster(OJapiCambrian * poCambrian);
	~OJapiAppBallotmaster();
    POJapiPoll PGetOJapiPoll(CEventBallotPoll * pBallot);
    POJapiPoll PCreateNewPollFromTemplate(CEventBallotPoll * pPollTemplate);
	CEventBallotPoll * PFindPollByID(TIMESTAMP tsIdPoll) const;
	CEventBallotPoll * PFindPollByID(const QString & sIdPoll) const;

public slots:
	POJapiPoll build();
	POJapiPoll build(QObject * pObjectPollTemplate);
	POJapiPoll build(const QString & sIdPollTemplate);
	POJapiPoll get(const QString & sIdPoll);
	QVariant getList();
    void go();
};
#define POJapiAppBallotmaster		POJapi

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
class OJapiApps : public OJapi
{
    OJapiCambrian * m_poCambrian;
public:
    OJapiApps(OJapiCambrian * poCambrian);
	POJapiAppBallotmaster ballotmaster();

	Q_OBJECT
	Q_PROPERTY(POJapiAppBallotmaster ballotmaster READ ballotmaster)
};
#define POJapiApps		POJapi

///////////////////////////////////////////////////////////////////////////////////////////////////
class OJapiMe : public OJapi
{
	OJapiCambrian * m_poCambrian;
public:
	OJapiMe(OJapiCambrian * poCambrian);
	OJapiList groups();
	OJapiList peerList();
	OJapiList peers();

	Q_OBJECT
	Q_PROPERTY(OJapiList groups READ groups)
	Q_PROPERTY(OJapiList peers READ peers)
};
#define POJapiMe		POJapi

class OJapiContact : public OJapi
{
	TContact * m_pContact;

public:
	OJapiContact(TContact * pContact);

	QString id();
	QString name();

	Q_OBJECT
	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString name READ name)
};
#define POJapiContact		POJapi

///////////////////////////////////////////////////////////////////////////////////////////////////
//	JavaScript wrapper for TGroup
class OJapiGroup : public OJapi
{
	TGroup * m_pGroup;

public:
	OJapiGroup(TGroup * pGroup);

	QString id();
	QString name();
	int count();
	void addPeer(POJapiContact pContactAdd);
	void removePeer(POJapiContact pContactRemove);

	Q_OBJECT
	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString name READ name)
	Q_PROPERTY(int count READ count)
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class OJapiCambrian : public OJapi
{
public:
	TProfile * m_pProfile;
protected:
	OSettings m_oSettings;
	OJapiProfile m_oProfile;
    OJapiApps m_oApps;
	OJapiMe m_oMe;
	OJapiAppBallotmaster * m_paAppBallotmaster;

public:
	OJapiCambrian(TProfile * pProfile, QObject * pParent);
	virtual ~OJapiCambrian();
	QVariant Settings();
	QVariant Profile();

	POJapiApps apps();
	POJapiAppBallotmaster polls();
	POJapiMe me();

public:
	Q_OBJECT
	Q_PROPERTY(QVariant Settings READ Settings)
	Q_PROPERTY(QVariant Profile READ Profile)
	Q_PROPERTY(POJapiAppBallotmaster polls READ polls)
	Q_PROPERTY(POJapiApps apps READ apps)
	Q_PROPERTY(POJapiMe me READ me)

public slots:
	void SendBitcoin(int n);
	void MessageSendTo(const QString & sContactTo, const QString & sMessage);
};

#endif // APIJAVASCRIPT_H

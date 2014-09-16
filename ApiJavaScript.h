//	ApiJavaScript.h //
//	Cambrian objects available in JavaScript
#ifndef APIJAVASCRIPT_H
#define APIJAVASCRIPT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include <QQuickImageProvider>
#include "TApplicationBallotmaster.h"

// HTML js API
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


// QML js API
class OCapiRootGUI;
class OJapiTabs;
class OJapiProfile;
class OJapiProfilesList;
class OJapiBrowserTab;
class OJapiBrowsersList;


class CListVariants : public QVariantList
{
	OJapiCambrian *m_poCambrian;

public:
	CListVariants(OJapiCambrian *poCambrian);
	void AddContact(TContact * pContact);
	void AddContacts(const CArrayPtrContacts & arraypContacts);
	void AddGroupMatchingType(TGroup * pGroup, EGroupType eGroupType);
	void AddGroupsMatchingType(const CArrayPtrGroups & arraypGroups, EGroupType eGroupType);
	void AddGroup(TGroup * pGroup);
	void AddAudience(TGroup * pGroup);
};


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


///////////////////////////////////////  CAPI   //////////////////////////////////////////////////////


class OJapiBrowserTab: public OJapi
{
	TBrowserTab *m_pTab;
	OJapiBrowsersList *m_pBrowsersListParent;
	Q_OBJECT

	const QString title();

public:
	OJapiBrowserTab(TBrowserTab *pTab, OJapiBrowsersList *pBrowsersList);

	Q_PROPERTY(QString title READ title)

public slots:
	void back();
	void forward();
	void reload();
	void close();
	void openApp(const QString &sAppName);
	void openUrl(const QString &url);
};
#define POJapiBrowserTab POJapi




class OJapiBrowsersList: public OJapi
{
	Q_OBJECT

protected:
	POJapiBrowserTab PGetCurrentTab_YZ();
	TBrowserTabs* PGetBrowser_YZ();

public:
	OJapiProfile *m_poJapiProfileParent_NZ;
	OJapiBrowsersList(OJapiProfile *poProfile);

	Q_PROPERTY(POJapiBrowserTab current READ PGetCurrentTab_YZ)

public slots:
	QVariantList listBrowsers();
	POJapiBrowserTab newBrowser();

};
#define POJapiBrowsersList POJapi



class OJapiProfile : public OJapi
{
	OJapiBrowsersList m_oBrowsersList;
	Q_OBJECT

public:
	TProfile *m_pProfile;

	OJapiProfile(TProfile *pProfile);
	QString id();
	QString name();
	POJapiBrowsersList browsers();

	Q_PROPERTY(QString name READ name)
	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(POJapiBrowsersList browsers READ browsers)

public slots:
	void destroy();
};
#define POJapiProfile	POJapi



class OJapiProfilesList : public OJapi
{
	Q_OBJECT
	OCapiRootGUI * m_pRootGui;

public:
	OJapiProfilesList(OCapiRootGUI *pRootGui);

	TProfile *PGetCurrentProfile();
	POJapiProfile currentProfile();
	void setCurrentProfile(POJapiProfile poJapiProfile);

	Q_PROPERTY(POJapiProfile current READ currentProfile WRITE setCurrentProfile)

public slots:
	QVariantList list();
	POJapiProfile create(const QString & name);

signals:
	void roleChanged ();
};
#define POJapiProfilesList	POJapi


class OJapiAppInfo : public OJapi
{
	Q_OBJECT

	const SApplicationHtmlInfo *m_pApplicationInfo;

public:
	OJapiAppInfo(const SApplicationHtmlInfo *pApplicationInfo);

	QString name();
	QString tooltip();
	QString launchUrl();
	QString iconUrl();

	Q_PROPERTY(QString name READ name )
	Q_PROPERTY(QString tooltip READ tooltip )
	Q_PROPERTY(QString launchUrl READ launchUrl )
	Q_PROPERTY(QString iconUrl READ iconUrl )
};

class OJapiNotification : public OJapi
{
	Q_OBJECT

public:
	OJapiNotification(IEvent *pEvent = 0);

	QString title();
	QString text();
	QDateTime date();
	QString cardLink();
	QString actionLabel();
	QString actionLink();

	Q_PROPERTY(QString title READ title)
	Q_PROPERTY(QString text READ text)
	Q_PROPERTY(QDateTime date READ date)
	Q_PROPERTY(QString cardLink READ cardLink)
	Q_PROPERTY(QString actionLabel READ actionLabel)
	Q_PROPERTY(QString actionLink READ actionLink)

public slots:
	void clear();
};

class OJapiNotificationsList : public OJapi
{
	Q_OBJECT

public:

public slots:
	QVariantList recent(int nMax);
	void clearRecent();
};
#define POJapiNotificationsList POJapi




class OJapiPeerRequestsList : public QObject
{
	Q_OBJECT
public:
	OJapiPeerRequestsList();
	~OJapiPeerRequestsList();


public slots:
	QVariantList list(int nMax);
};
#define POJapiPeerRequestsList POJapi




class OJapiPeerRequest : public QObject
{
	Q_OBJECT

public:
	QString id();
	QString name();

	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString name READ name)
};




class OCapiRootGUI : public OJapi
{
	Q_OBJECT
	OJapiProfilesList m_oProfiles;
	OJapiNotificationsList m_oNotificationsList;
	OJapiPeerRequestsList m_oPeerRequestsList;

public:
	OCapiRootGUI();
	QVariantList apps();
	POJapiProfilesList roles();
	POJapiNotificationsList notifications();
	POJapiPeerRequestsList peerRequests();

	Q_PROPERTY(QVariantList apps READ apps)
	Q_PROPERTY(POJapiProfilesList roles READ roles)
	Q_PROPERTY(POJapiNotificationsList notifications READ notifications)
	Q_PROPERTY(POJapiPeerRequestsList peerRequests READ peerRequests)
};
#define POCapiRootGUI	POJapi



class OCapiImageProvider : public QQuickImageProvider
{
public:
	OCapiImageProvider();
	QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
};



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
	bool send(const QString & sGroupId);
	POJapiPollResults getResults() CONST_MCC;

	// attatchments sections
	QVariantList listAttatchments();
	void addAttatchment(const QString & sName, const QString & sContentBase64, const QString sMimeType);
};
#define POJapiPoll		POJapi

class OJapiPollAttatchment : public OJapi
{
	Q_OBJECT
	CEventBallotAttatchment *m_pBallotAttatchment;

	QString name();
	QString mimeType();
	QString content();

public:
	OJapiPollAttatchment(CEventBallotAttatchment  *pBallotAttatchment);

	Q_PROPERTY(QString name READ name)
	Q_PROPERTY(QString type READ mimeType)
	Q_PROPERTY(QString content READ content)

public slots:
	void destroy();
};



class OJapiAppBallotmaster : public OJapiAppInfo
{
	Q_OBJECT
protected:
	CServiceBallotmaster * m_pServiceBallotmaster;
//	TApplicationBallotmaster * m_pBallotmaster;

public:
	OJapiAppBallotmaster(OJapiCambrian * poCambrian, const SApplicationHtmlInfo *pApplicationInfo);
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
	void open();
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
	OJapiList peers();
	//OJapiList peerLists();

	Q_OBJECT
	Q_PROPERTY(OJapiList groups READ groups)
	Q_PROPERTY(OJapiList peers READ peers)
	//Q_PROPERTY(OJapiList peerLists READ peerLists)

public slots:
	//POJapiGroup newPeerList();
	//JapiGroup newGroup();
	POJapiGroup newGroup(const QString & type);
	//POJapiGroup getPeerList(const QString & sId);
	POJapiGroup getGroup(const QString & sId);
};
#define POJapiMe		POJapi


class OJapiUtil : public OJapi
{
	Q_OBJECT
public slots:
	QString base64encode(const QString & sText);
	QString base64decode(const QString & sBase64);
};
#define POJapiUtil	POJapi


class OJapiContact : public OJapi
{

public:
	OJapiContact(TContact * pContact);

	TContact * m_pContact;

	QString id();
	QString name();

	Q_OBJECT
	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString name READ name)
};
#define POJapiContact		POJapi

///////////////////////////////////////////////////////////////////////////////////////////////////
//	JavaScript object to manage Groups
class OJapiGroupList : public OJapi
{
	OJapiCambrian * m_poCambrian;
	Q_OBJECT

public:
	OJapiGroupList(OJapiCambrian * poCambrian);


	//Q_PROPERTY(OJapiList list READ list)

public slots:
	POJapiGroup build(const QString & type);
	POJapiGroup get(const QString & sId);
	OJapiList list();
};
#define POJapiGroupList	POJapi


///////////////////////////////////////////////////////////////////////////////////////////////////
//	JavaScript wrapper for TGroup
class OJapiGroup : public OJapi
{
	TGroup * m_pGroup;
	OJapiCambrian * m_poCambrian;

public:
	OJapiGroup(TGroup * pGroup, OJapiCambrian * poCambrian);

	QString id();
	QString name();
	void name(const QString & sName);
	int count();
	OJapiList members();
	QString type();

	Q_OBJECT
	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString name READ name WRITE name)
	Q_PROPERTY(QString type READ type)
	Q_PROPERTY(int count READ count)
	Q_PROPERTY(OJapiList members READ members)

public slots:
	void addPeer(POJapiContact pContactAdd);
	void removePeer(POJapiContact pContactRemove);
	void save();
	void destroy();
};
#define POJapiGroup		POJapi

///////////////////////////////////////////////////////////////////////////////////////////////////
class OJapiCambrian : public OJapi
{
public:
	TProfile * m_pProfile;
	CArrayPtrRuntimeObjects m_arraypaTemp;

protected:
	OSettings m_oSettings;
    OJapiApps m_oApps;
	OJapiAppBallotmaster * m_paAppBallotmaster;
	OCapiRootGUI m_capi;
	OJapiGroupList m_oGroupList;
	OJapiUtil m_oUtil;

public:
	OJapiMe m_oMe;

	OJapiCambrian(TProfile * pProfile, QObject * pParent);
	virtual ~OJapiCambrian();
	QVariant Settings();

	POJapiApps apps();
	POJapiAppBallotmaster polls();
	POJapiMe me();
	POJapiGroupList groups();
	POCapiRootGUI capi();
	POJapiUtil util();

public:
	Q_OBJECT
	Q_PROPERTY(QVariant Settings READ Settings)
	Q_PROPERTY(POJapiAppBallotmaster polls READ polls)
	Q_PROPERTY(POJapiApps apps READ apps)
	Q_PROPERTY(POJapiMe me READ me)
	Q_PROPERTY(POJapiGroupList groups READ groups)
	Q_PROPERTY(POCapiRootGUI capi READ capi)
	Q_PROPERTY(POJapiUtil util READ util);


public slots:
	void SendBitcoin(int n);
	void MessageSendTo(const QString & sContactTo, const QString & sMessage);
};


#endif // APIJAVASCRIPT_H

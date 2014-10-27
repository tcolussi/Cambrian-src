//	ApiJavaScript.h
//	Cambrian objects available in JavaScript

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
	class OJapiPollAttatchment;

	class OJapiContact;
	class OJapiGroup;

	class OJapiEvent;
		class OJapiEventPollResults;


// QML js API
class OCapiRootGUI;
class OJapiTabs;
class OJapiProfile;
class OJapiProfilesList;
class OJapiBrowserTab;
class OJapiBrowsersList;
class OJapiJurisdiction;

#ifndef APIJAVASCRIPT_H
#define APIJAVASCRIPT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#ifdef TEST_QT_IMAGE_PROVIDER
#include <QQuickImageProvider>
#endif
#include "TApplicationBallotmaster.h"

class CListVariants : public QVariantList
{
	OJapiCambrian * m_poCambrian;

public:
	CListVariants(OJapiCambrian * poCambrian);
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



class OJapiEvent : public OJapi
{
	Q_OBJECT
public:
	IEvent * m_pEvent;

public:
	OJapiEvent(IEvent * pEvent) { m_pEvent = pEvent; }

};

//	This object returns the results of a sent ballot
class OJapiEventPollResults : public OJapiEvent
{
public:
	OJapiEventPollResults(CEventBallotSent * pEventBallotSent);

};

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





class OJapiJurisdiction : public OJapi
{
	OJapiProfile *m_pProfileParent;
	Q_OBJECT

	TProfile * PGetProfile_NZ();

public:
	OJapiJurisdiction(OJapiProfile *pProfileParent);
	void setCurrent(const QString & sJurisdiction);
	QString current();

	Q_PROPERTY(QString current READ current WRITE setCurrent)
};
#define POJapiJurisdiction POJapi


class OJapiProfile : public OJapi
{
	OJapiBrowsersList m_oBrowsersList;
	OJapiJurisdiction m_oJurisdiction;
	Q_OBJECT

public:
	TProfile *m_pProfile;

	OJapiProfile(TProfile *pProfile);
	QString id();
	QString name();
	POJapiBrowsersList browsers();
	POJapiJurisdiction jurisdiction();

	Q_PROPERTY(QString name READ name)
	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(POJapiBrowsersList browsers READ browsers)
	Q_PROPERTY(POJapiJurisdiction jurisdiction READ jurisdiction)

public slots:
    bool destroy();
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


class OJapiAppChat : public OJapi
{
private:
	Q_OBJECT
	OJapiCambrian * m_poCambrian;

public:
	OJapiAppChat(OJapiCambrian * poCambrian);

public slots:
	bool open(const QString & strJabberId);

}; // OJapiAppChat
#define POJapiAppChat		POJapi




///////////////////////////////////////////////////////////////////////////////////////////////////
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



///////////////////////////////////////////////////////////////////////////////////////////////////
class OJapiPeerRequestsList : public OJapi
{
	Q_OBJECT
public:
	OJapiPeerRequestsList();
	~OJapiPeerRequestsList();


public slots:
	QVariantList list(int nMax);
};
#define POJapiPeerRequestsList POJapi

class OJapiPeerRequest : public OJapi
{
	Q_OBJECT

public:
	QString id();
	QString name();

	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString name READ name)
};




class OJapiPeerMessagesList : public OJapi
{
	Q_OBJECT
	OCapiRootGUI *m_pParentCapiRoot;
public:

	TProfile * PGetProfileSelected_YZ();
	OJapiPeerMessagesList(OCapiRootGUI *pParentCapiRoot);

	int recentCount();

	Q_PROPERTY(int recentCount READ recentCount)

public slots:
	QVariantList recent(int nMax);
	void clearAll();
};
#define POJapiPeerMessagesList POJapi

class OJapiPeerMessage : public OJapi
{
	Q_OBJECT
	OJapiPeerMessagesList * m_pParent;
	IEventMessageText *m_pEventMessage;

	inline TContact * PGetContact_YZ() { return m_pEventMessage->PGetContactForReply_YZ(); }

public:
	OJapiPeerMessage(OJapiPeerMessagesList *pParent, IEventMessageText *pEventMessage);

	//QString peerName();
	//QString peerId();
	QString message();
	QDateTime date();
	POJapiContact peer();

	//Q_PROPERTY(QString peerName READ peerName)
	//Q_PROPERTY(QString peerId READ peerId)
	Q_PROPERTY(QString message READ message)
	Q_PROPERTY(QDateTime date READ date)
	Q_PROPERTY(POJapiContact peer READ peer)

public slots:
	void clear();
};
#define POJapiPeerMessage POJapi




class OCapiRootGUI : public OJapi
{
	Q_OBJECT
	OJapiProfilesList m_oProfiles;
	OJapiNotificationsList m_oNotificationsList;
	OJapiPeerRequestsList m_oPeerRequestsList;
	OJapiPeerMessagesList m_oPeerMessagesList;

public:
	OCapiRootGUI();
	QVariantList apps();
	POJapiProfilesList roles();
	POJapiNotificationsList notifications();
	POJapiPeerRequestsList peerRequests();
	POJapiPeerMessagesList peerMessages();

	Q_PROPERTY(QVariantList apps READ apps)
	Q_PROPERTY(POJapiProfilesList roles READ roles)
	Q_PROPERTY(POJapiNotificationsList notifications READ notifications)
	Q_PROPERTY(POJapiPeerRequestsList peerRequests READ peerRequests)
	Q_PROPERTY(POJapiPeerMessagesList peerMessages READ peerMessages)
};
#define POCapiRootGUI	POJapi


#ifdef TEST_QT_IMAGE_PROVIDER
class OCapiImageProvider : public QQuickImageProvider
{
public:
	OCapiImageProvider();
	QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
};
#endif

//	Base class for all ballots related JAPI
class OJapiBallot : public OJapi
{
	Q_OBJECT
protected:
	IEventBallot * m_pEventBallot;

public:
	OJapiBallot(IEventBallot * pEventBallot);

	QString id() const;
	QString type() const;
	QString originator() const;
	QString status() const;
	QString title() const;
	void title(const QString & sTitle);
	QString description() const;
	void description(const QString & sDescription);
	QVariantList options() const;
	void options(const QVariantList & lsOptions);
	bool allowComments() const;
	void allowComments(bool fAllowComments);
	bool allowMultipleChoices() const;
	void allowMultipleChoices(bool fAllowMultipleChoices);
	bool isTemplate();
	void isTemplate(bool fIsTemplate);
	bool isSelected();
	void isSelected(bool fIsSelected);

	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString type READ type)
	Q_PROPERTY(QString originator READ originator)
	Q_PROPERTY(QString status READ status)
	Q_PROPERTY(QString title READ title WRITE title)
	Q_PROPERTY(QString description READ description WRITE description)
	Q_PROPERTY(QVariantList options READ options WRITE options)
	Q_PROPERTY(bool allowComments READ allowComments WRITE allowComments)
	Q_PROPERTY(bool allowMultipleChoices READ allowMultipleChoices WRITE allowMultipleChoices)
	Q_PROPERTY(bool isTemplate READ isTemplate WRITE isTemplate)
	Q_PROPERTY(bool isSelected READ isSelected WRITE isSelected)

public slots:
	bool submit(const QString & sComment);
}; // OJapiBallot
#define POJapiBallot		POJapi

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Core object shared with OPoll and OPollResults
class OJapiPollCore : public OJapiBallot
{
	Q_OBJECT
public:
    CEventBallotPoll * m_pBallot;

public:
	OJapiPollCore(CEventBallotPoll * pBallot);
    int pollTimeLength() const;
    void pollTimeLength(int cSeconds);
	QString dismissText() const;
	void dismissText(const QString & sText);
	QString submitText() const;
	void submitText(const QString & sText);
	void pollTargetId(const CString & sTargetId);
	QString pollTargetId() const;
	QDateTime dateStarted() const;
	QDateTime dateStopped() const;

	Q_PROPERTY(int pollTimeLength READ pollTimeLength WRITE pollTimeLength)
	Q_PROPERTY(QString dismissText READ dismissText WRITE dismissText)
	Q_PROPERTY(QString submitText READ submitText WRITE submitText)
	Q_PROPERTY(QString pollTargetId READ pollTargetId WRITE pollTargetId)
	Q_PROPERTY(QDateTime dateStarted READ dateStarted)
	Q_PROPERTY(QDateTime dateStopped READ dateStopped)
}; // OJapiPollCore


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Statistics about a poll
struct SEventPollStatistics
	{
	int cSent;		// Number of recipients the poll was sent (this is typically the number of group members)
	int cResponded;	// Number of people who responded/voted to the poll
	int cPending;	// Those who did not responded/voted (cPending = cSent - cResponded)
	int cInvalid;	// Always zero (at the moment
	};


///////////////////////////////////////////////////////////////////////////////////////////////////
class OJapiPollResultsStats : public OJapi
{
    Q_OBJECT
public:
    CEventBallotPoll * m_pBallot;
	SEventPollStatistics m_statistics;

public:
    OJapiPollResultsStats(CEventBallotPoll * pBallot);

	int sent() const { return m_statistics.cSent; }
	int responded() const { return m_statistics.cResponded; }
	int pending() const { return m_statistics.cPending; }
	int invalid() const { return m_statistics.cInvalid; }

    Q_PROPERTY(int pending READ pending)
    Q_PROPERTY(int invalid READ invalid)
    Q_PROPERTY(int responded READ responded)
    Q_PROPERTY(int sent READ sent)
};
#define POJapiPollResultsStats		POJapi

class _CEventBallotChoice;
class OJapiPollOption : public OJapi	// Display an option/choice to vote
{
	Q_OBJECT
	_CEventBallotChoice * m_pChoice;

public:
	OJapiPollOption(_CEventBallotChoice * pChoice);
	QString text() const;
	bool isSelected();
	void isSelected(bool fIsSelected);
	int count() const;

	Q_PROPERTY(QString text READ text)
	Q_PROPERTY(bool isSelected READ isSelected WRITE isSelected)
	Q_PROPERTY(int count READ count)
};

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
	POJapiPollResultsStats stats();

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
private:
	OJapiPollResults * m_paoJapiPollResults;

public:
    OJapiPoll(CEventBallotPoll * pBallot);
	~OJapiPoll();

public slots:
	bool save(/*const QString & sDebugContext*/);
	void destroy();
	bool start();
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

public:
	OJapiAppBallotmaster(OJapiCambrian * poCambrian, const SApplicationHtmlInfo * pApplicationInfo);
	~OJapiAppBallotmaster();
	TProfile * PGetProfile() const;
	IEventBallot ** PrgpGetEventsBallotsStop(OUT IEventBallot *** pppEventBallotStop) const;
	POJapiPoll PGetOJapiBallot(CEventBallotReceived * pBallot);
	POJapiPoll PGetOJapiPoll(CEventBallotPoll * pEventBallotPoll);
    POJapiPoll PCreateNewPollFromTemplate(CEventBallotPoll * pPollTemplate);
	CEventBallotReceived * PFindBallotByID(TIMESTAMP tsIdBallot) const;
	CEventBallotPoll * PFindPollByID(TIMESTAMP tsIdPoll) const;
	CEventBallotPoll * PFindPollByID(const QString & sIdPoll) const;
	CEventBallotPoll * PFindPollByTimeStarted(TIMESTAMP tsStarted) const;
	void OnEventBallotReceived(CEventBallotReceived * pEventBallotReceived);
	void OnEventVoteReceived(const CEventBallotSent * pEventBallotSent);
	void OnEventPollStopped(CEventBallotPoll * pEventBallotPoll);

public slots:
	POJapiPoll build();
	POJapiPoll build(QObject * pObjectPollTemplate);
	POJapiPoll build(const QString & sIdPollTemplate);
	POJapiPoll get(const QString & sIdPoll);
	QVariant getList();
	void open();

signals:
	// Signals for JavaScript to update its HTML when an event occurs
	void onBallotReceived(POJapiBallot oBallot);	// This signal is emitted when a new ballot arrives from a contact.  The signal handler should display a 'card' to the user so he/she may vote.
	void onVoteReceived(POJapiPoll oPoll);				// This signal is emitted when a vote arrives from a contact.  The signal handler should update the poll to tally the new vote (typically updating the pie chart)
	void onPollSaved(POJapiPoll oPoll);
	void onPollStopped(POJapiPoll oPoll);
	void onPollDestroyed(POJapiPoll oPoll);

public:
	OJapiAppBallotmaster * m_pNext;						// Next Ballotmaster in the linked list.  This is important because every OJapiAppBallotmaster must be notified when a new ballot or vote arrives.
	static OJapiAppBallotmaster * s_plistBallotmasters;	// Pointer to the last ballotmaster

}; // OJapiAppBallotmaster
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
};


///////////////////////////////////////////////////////////////////////////////////////////////////
class OJapiApps : public OJapi
{
    OJapiCambrian * m_poCambrian;

public:
    OJapiApps(OJapiCambrian * poCambrian);
	POJapiAppBallotmaster ballotmaster();
	POJapiAppChat chat();


	Q_OBJECT
	Q_PROPERTY(POJapiAppBallotmaster ballotmaster READ ballotmaster)
	Q_PROPERTY(POJapiAppChat chat READ chat)
};
#define POJapiApps		POJapi

///////////////////////////////////////////////////////////////////////////////////////////////////
class OJapiMe : public OJapi
{
	OJapiCambrian * m_poCambrian;
public:
	OJapiMe(OJapiCambrian * poCambrian);
	OJapiList groups();
	OJapiList channels();
	OJapiList peers();
	//OJapiList peerLists();

	Q_OBJECT
	Q_PROPERTY(OJapiList groups READ groups)
	Q_PROPERTY(OJapiList channels READ channels)
	Q_PROPERTY(OJapiList peers READ peers)
	//Q_PROPERTY(OJapiList peerLists READ peerLists)

public slots:
	//POJapiGroup newPeerList();
	//JapiGroup newGroup();
	POJapiGroup newGroup(const QString & type);
	//POJapiGroup getPeerList(const QString & sId);
	POJapiGroup getGroup(const QString & sId);
	POJapiContact newPeer(const QString & sUsername);
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
	TContact * m_pContact;

public:
	OJapiContact(TContact * pContact);
	QString id();
	QString name();

	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString name READ name)
	Q_OBJECT

public slots:
	void openChat();
	void destroy();
	void recommend();
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
	QString channelName() const;
	void channelName(const QString & sNameChannel);
	QString purpose() const;
	void purpose(const QString & sPurpose);

	Q_OBJECT
	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString name READ name WRITE name)
	Q_PROPERTY(QString type READ type)
	Q_PROPERTY(QString channelName READ channelName WRITE channelName)
	Q_PROPERTY(QString purpose READ purpose WRITE purpose)
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
	OJapiAppChat m_oAppChat;

public:
	OJapiMe m_oMe;

	OJapiCambrian(TProfile * pProfile, QObject * pParent);
	virtual ~OJapiCambrian();
	QVariant Settings();

	POJapiApps apps();
	POJapiAppBallotmaster polls();
	POJapiAppChat chatApp();
	POJapiMe me();
	POJapiGroupList groups();
	OJapiList channelsAvailable();
	POCapiRootGUI capi();
	POJapiUtil util();

public:
	Q_OBJECT
	Q_PROPERTY(QVariant Settings READ Settings)
	Q_PROPERTY(POJapiAppBallotmaster polls READ polls)
	Q_PROPERTY(POJapiApps apps READ apps)
	Q_PROPERTY(POJapiMe me READ me)
	Q_PROPERTY(POJapiGroupList groups READ groups)
	Q_PROPERTY(OJapiList channelsAvailable READ channelsAvailable)
	Q_PROPERTY(POCapiRootGUI capi READ capi)
	Q_PROPERTY(POJapiUtil util READ util)


public slots:
	void SendBitcoin(int n);
	void MessageSendTo(const QString & sContactTo, const QString & sMessage);
}; // OJapiCambrian


//	Application Names Identifiers.  Those values are not meant to be localized, but to offer a mechanism to select an application by name.
//	Ideally it should be an enum to select applications.  It would be more performant and more reliable than comparing strings.
#define d_szNameApplicationHtml_Ballotmaster			"Ballotmaster"
#define d_szNameApplicationHtml_Channels				"Channels"
#define d_szNameApplicationHtml_GroupManager			"Group Manager"
#define d_szNameApplicationHtml_Corporations			"Corporations"
#define d_szNameApplicationHtml_PeerManager				"Peer Manager"

#endif // APIJAVASCRIPT_H

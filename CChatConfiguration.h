///////////////////////////////////////////////////////////////////////////////////////////////////
//	CChatConfiguration.h

#ifndef CCHATCONFIGURATION_H
#define CCHATCONFIGURATION_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

//	This is the main class holding the configuration data of the chat application.
//	This class is responsible of allocating and deleting objects when no longer needed.
//
//	The purpose of this class is providing an abstraction layer of the chat configuration to the GUI displaying the Navigation Tree.
//	The Navigation Tree currently uses QTreeWidgetItem, however this may change in the future.
//
//	This class is also responsible of serializing and deserializing itself.  To serialize objects according in the same order
//	as displayed in the navigation tree, the object CChatConfiguration may query the object WChatNavigation.
//
//	At the moment this class is a singleton, however in the future it will be possible to have multiple CChatConfiguration opened simultaneously.
singleton CChatConfiguration
{
public:
	TProfile * m_pProfileSelected;				// Which profile was selected by the user.  NULL means to display all profiles in the Navigation Tree.
	CArrayPtrProfiles m_arraypaProfiles;
	TCertificates m_oTreeItemCertificates;

protected:
	QString m_sPathFileSave;			// Full path where to save the configuration.  The motivation is there may be multiple configurations opened simultaneously, and each needs to know where to save itself
	SFileSizeAndMd5 m_hashMd5LastSave;	// Remember the size and MD5 of the configuration file at the moment it was last written to disk.  The motivation for this variable is avoiding unnecessary disk writes.
	SHashSha1 m_hashSalt;				// Random value serialized in Config.xml. This value is to ensure there is entropy/randomness when a new configuration file is created, however there is consistency between operations (such as adding and deleting accounts and contacts) within the same Config.xml.

public:
	CChatConfiguration();
	~CChatConfiguration();
	void Destroy();
	TCertificate * PCertificateRegister(TCertificate * pCertificateParent, const CSslCertificate & oCertificateSsl);
	TCertificate * PCertificatesRegister(QSslSocket * pSocket);
	void NavigationTree_DisplayAllCertificates();
	void NavigationTree_DisplayAllCertificatesToggle();
	void NavigationTree_DisplayWallet();

	enum { c_cbSalt = sizeof(SHashSha1) };
	inline const SHashSha1 * PGetSalt() const { return &m_hashSalt; }
	QString SGetPathOfFileName(PSZAC pszFileName);
	QString SGetPathOfFileName(IN const SHashSha1 * pHashFileName);

	void XmlConfigurationExchange(INOUT CXmlExchanger * pXmlExchanger);
	void XmlConfigurationLoadFromFile(const QString * psPathFileOpen = NULL);
	void XmlConfigurationSaveToFile(const QString * psPathFileSaveAs = NULL);

	friend class WMainWindow;
}; // CChatConfiguration
extern CChatConfiguration g_oConfiguration;
extern CArrayPtrAccountsXmpp g_arraypAccounts;
extern CArrayPtrContacts g_arraypContactsRecentMessages;

//	Flags for g_uPreferences
#define P_kmPresenceMask							0x000F
#define P_kfDontPlaySoundWhenNewMessageArrive		0x0010
#define P_kfManualCertificateAuthentication			0x0020	// To not automatically authenticate a new certificate
extern UINT g_uPreferences;

extern const QString c_sCambrianOrg;
class OSettingsRegistry : public QSettings
{
public:
	OSettingsRegistry();
};

BOOL Configuration_FIsWithoutAccounts();
TAccountXmpp * Configuration_PGetAccountSelectedOrFirstAccount();
ITreeItem * Configuration_PFindAccountOrContactByJID(PSZUC pszuContactJID);
EError Configuration_EFileRead(IN const SHashSha1 * pHashFileName, OUT CBin * pbinFile);
EError Configuration_EFileWrite_ML(IN const SHashSha1 * pHashFileName, IN const CBin * pbinFile);
void Configuration_FileAppend(IN const SHashSha1 * pHashFileName, IN const CBin * pbinAppend);
void Configuration_FileAppend(IN const SHashSha1 * pHashFileName, INOUT CMemoryAccumulator * pAccumulator);
void Configuration_Save();
void Configuration_SaveAs();
void Configuration_Restore();
void Configuration_GlobalSettingsPresenceUpdate(EMenuAction eMenuAction);
void Configuration_OnTimerNetworkIdle();
void Configuration_NetworkReconnectIfDisconnected();

#endif // CCHATCONFIGURATION_H

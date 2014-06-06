///////////////////////////////////////////////////////////////////////////////////////////////////
//	TWallet.h
//
//	Classes to store events (transactions) related to a wallet.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CWALLET_H
#define CWALLET_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

//	Each contact has its own vault of events, and this vault is decrypted when the contact is visible.
class CVaultEventsForContact	// (vault)
{
protected:
	TWallet * m_pWalletParent;
	TContact * m_pContactParent_YZ;	// Pointer of the contact within the Navigation Tree.  This pointer may be NULL when constructing a vault from a serialized wallet.
	SHashSha1 m_hashContact;			// Hash to link the data to a contact in the Navigation Tree
	CBin m_binEventsEncrypted;			// Encrypted events until the decryption key is found (to be implemented)
	CArrayPtrEvents m_arraypaEvents;	// All the (decrypted) events related to a contact

public:
	CVaultEventsForContact(TWallet * pWalletParent, TContact * pContactParent_NZ);
	CVaultEventsForContact(TWallet * pWalletParent, const CXmlNode * pXmlNodeUnserialize);
	~CVaultEventsForContact();
	void ContactBind(TContact * pContact);
	void ContactUnbindIfAboutBeingDeleted();
	inline TContact * PGetContact_YZ() const { return m_pContactParent_YZ; }

	UINT EventsEncryptCb();
	void EventsDecrypt();

	CEventWalletTransaction * PAllocateEventTransaction();
	void AppendAllEvents(IOUT CArrayPtrEvents * parraypEvents) CONST_MAY_CREATE_CACHE;
	void AppendAllEventsTransactionsMatchingViewFlags(IOUT CArrayPtrEvents * parraypEventsTransactions, EWalletViewFlags eWalletViewFlags) CONST_MAY_CREATE_CACHE;

	void XmlSerializeVault(IOUT CBin * pbinXml);
	inline void SortEventsByIDs() { m_arraypaEvents.SortEventsByIDs(); }
	friend class CHashTableVaults;
	friend class CArrayPtrVaults;
}; // CVaultEventsForContact

class CArrayPtrVaults : public CArray
{
public:
	inline CVaultEventsForContact ** PrgpGetVaultsStop(OUT CVaultEventsForContact *** pppVaultStop) const { return (CVaultEventsForContact **)PrgpvGetElementsStop(OUT (void ***)pppVaultStop); }
	void BindVaultMatchingContactHash(TContact * pContact, const SHashSha1 * pHashContact);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Class holding all the transactions related to a wallet.
//	The application may open multiple wallets at once, and each wallet may contain different currencies.
//
//	For privacy, each wallet is 'autonomous'. That is, a wallet may be opened and closed independently of other wallet(s) and configuration(s).
//
class TWallet : public ITreeItem
{
	RTI_IMPLEMENTATION(TWallet)
protected:
	TWallet * m_pNext;					// Next wallet in the linked list
	SHashKey256 m_hashKey;				// Key to encrypt and decrypt the wallet
	QString m_sPathFileSave;			// Full path where to save the wallet.  The motivation is there may be multiple wallets opened simultaneously, and each needs to know where to save itself.
	SFileSizeAndMd5 m_hashMd5LastSave;	// Remember the size and MD5 of the wallet file at the moment it was last written to disk.  The motivation for this variable is avoiding unnecessary disk writes.
	SHashSha1 m_hashSalt;				// Random value serialized with wallet. This value is link wallet transactions with a contact in Config.xml
public:
	CArrayPtrVaults m_arraypaVaults;	// All the contacts involved in the wallet (which includes the transactions and other events related to the contact)

public:
	TWallet(PSZUC pszPassword, const QString & sPathFileWallet, INOUT CBin * pbinWalletEncrypted);
	~TWallet();
	inline TWallet * PGetNextWallet() const { return m_pNext; }
	void GetHashOfContact(OUT SHashSha1 * pHashSha1, const TContact * pContact) const;
	CVaultEventsForContact * PFindVault(TContact * pContact);
	CEventWalletTransaction * PAllocateEventTransaction(TContact * pContact);
	void GenerateDummyTransactions();

	EError EUnserializeFromXml(INOUT CBin * pbinWalletXml);
	void SerializeXmlToDisk();

	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);			// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);	// From ITreeItem
	virtual void TreeItem_GotFocus();										// From ITreeItem

	enum { c_cbSalt = sizeof(SHashSha1) };
	inline const SHashSha1 * PGetSalt() const { return &m_hashSalt; }
	TContact * PFindContactFromTransactionHash(const SHashSha1 * pHashTransactionContract) CONST_MAY_CREATE_CACHE;

public:
	static QString  S_SGetPathFileNameFromPassword(PSZUC pszPassword);
	static void S_FileOpen_MB(PSZUC pszPassword);
	static void S_SaveAll();
	static void S_AccountsAboutBeingDeleted();
	static void S_DetachFromContactsAboutBeingDeleted();
	static void S_ContactAdded(TContact * pContact);
	static CEventWalletTransaction * S_PAllocateEventTransaction(TContact * pContact);
	static TWallet * s_palistWallet;	// Linked list of all the wallets opened (at the moment there is only one)
}; // TWallet


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Class displaying all transactions related to its filter and view flags.
//	The class TWalletView may display transactions from multiple wallets.
class TWalletView : public ITreeItem
{
	RTI_IMPLEMENTATION(TWalletView)
protected:
	ITreeItem * m_pTreeItemFilterBy;	// Pointer to a TWallet or TContact
	EWalletViewFlags m_eWalletViewFlags;

public:
	TWalletView(ITreeItem * pParentFilterBy, PSZAC pszName, EWalletViewFlags eWalletViewFlags);
	~TWalletView();
	virtual void TreeItem_GotFocus();										// From ITreeItem
};

// QIdentityProxyModel

#endif // CWALLET_H

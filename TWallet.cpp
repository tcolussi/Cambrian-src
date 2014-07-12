#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "CHashTable.h"

const char c_szaWallet[] = "w";

class CHashElementVault : public CHashElement
{
public:
	CVaultEventsForContact * m_pVault;	// Vault of the hash element
};

class CHashTableVaults : public CHashTable
{
public:
	CHashTableVaults();
	void AddVault(const CVaultEventsForContact * pVault);
	CVaultEventsForContact * PFindVaultRemove(const SHashSha1 * pHashContact);
public:
	static const SHashSha1 * S_PGetHashKey(const CHashElementVault * pHashElement) { return &pHashElement->m_pVault->m_hashContact; }		// This routine must have a compatible interface as PFn_PszGetHashKey()
};

CHashTableVaults::CHashTableVaults() : CHashTable((PFn_PszGetHashKey)S_PGetHashKey, eHashFunctionHashSha1)
	{
	SetHashElementSize(sizeof(CHashElementVault));
	}

void
CHashTableVaults::AddVault(const CVaultEventsForContact * pVault)
	{
	CHashElementVault * pHashElement = (CHashElementVault *)PAllocateHashElement();
	pHashElement->m_pVault = const_cast<CVaultEventsForContact *>(pVault);
	Add(pHashElement);
	}

//	Find the vault matching the contact hash.
//	If the vault is found, remove it from the hash table and return pointer to it.
//	Return NULL if there is no vault matching the contact hash.
CVaultEventsForContact *
CHashTableVaults::PFindVaultRemove(const SHashSha1 * pHashContact)
	{
	Assert(pHashContact != NULL);
	CHashElementVault * pHashElement = (CHashElementVault *)PFindHashElementAndRemove((PSZUC)pHashContact);
	if (pHashElement != NULL)
		return pHashElement->m_pVault;
	return NULL;
	}

TWallet * TWallet::s_palistWallet;

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Remove any reference of the deleted contact, so the application does not crash
void
TWallet::S_DetachFromContactsAboutBeingDeleted()
	{
	TWallet * pWallet = TWallet::s_palistWallet;
	while (pWallet != NULL)
		{
		CVaultEventsForContact ** ppVaultStop;
		CVaultEventsForContact ** ppVault = pWallet->m_arraypaVaults.PrgpGetVaultsStop(OUT &ppVaultStop);
		while (ppVault != ppVaultStop)
			{
			CVaultEventsForContact * pVault = *ppVault++;
			pVault->ContactUnbindIfAboutBeingDeleted();
			}
		pWallet = pWallet->m_pNext;
		} // while
	} // S_DetachFromContactsAboutBeingDeleted()

void
TWallet::S_AccountsAboutBeingDeleted()
	{
	S_DetachFromContactsAboutBeingDeleted();	// At the moment, there is no difference between account(s) and contact(s) being deleted
	}

void
CArrayPtrVaults::BindVaultMatchingContactHash(TContact * pContact, const SHashSha1 * pHashContact)
	{
	CVaultEventsForContact ** ppVaultStop;
	CVaultEventsForContact ** ppVault = PrgpGetVaultsStop(OUT &ppVaultStop);
	while (ppVault != ppVaultStop)
		{
		CVaultEventsForContact * pVault = *ppVault++;
		if (pVault->m_pContactParent_YZ != NULL)
			continue;
		if (HashSha1_FCompareEqual(&pVault->m_hashContact, pHashContact))
			{
			pVault->ContactBind(pContact);
			return;
			}
		} // while
	}

//	Notify all wallets a new contact was added, giving the opportunity of a vault to decrypt itself if its hash matches with the new contact (this is the case when a contact was deleted, and later re-added)
void
TWallet::S_ContactAdded(TContact * pContact)
	{
	Assert(pContact != NULL);
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	Assert(!pContact->m_strJidBare.FIsEmptyString());
	// Find the hash table matching the configuration
	TWallet * pWallet = TWallet::s_palistWallet;
	while (pWallet != NULL)
		{
		SHashSha1 hashContact;
		pWallet->GetHashOfContact(OUT &hashContact, IN pContact);
		pWallet->m_arraypaVaults.BindVaultMatchingContactHash(pContact, IN &hashContact);
		pWallet = pWallet->m_pNext;
		} // while
	} // S_ContactAdded()

//	Determine which is the best wallet to allocate a transaction.
IEventWalletTransaction *
TWallet::S_PAllocateEventTransaction(TContact * pContact)
	{
	// At the moment, use the first wallet in the list, or allocate a new one
	if (s_palistWallet == NULL)
		g_oConfiguration.NavigationTree_DisplayWallet();
	Assert(s_palistWallet != NULL);
	return s_palistWallet->PAllocateEventTransaction(pContact);
	}

void
TWallet::S_SaveAll()
	{
	TWallet * pWallet = TWallet::s_palistWallet;
	while (pWallet != NULL)
		{
		pWallet->SerializeXmlToDisk();
		pWallet = pWallet->m_pNext;
		}
	}

QString
TWallet::S_SGetPathFileNameFromPassword(PSZUC pszPassword)
	{
	SHashSha1 hashFileNameWallet;
	HashSha1_CalculateFromSaltedPassword(OUT &hashFileNameWallet, eSaltPrefix_Wallet, IN pszPassword);
	return g_oConfiguration.SGetPathOfFileName(IN &hashFileNameWallet);
	}

extern TTreeItemDemo * g_pBanking;	// Node to attach the wallet

//	Open a wallet given a password.  The password is used to determine the filename where the wallet is stored.
void
TWallet::S_FileOpen_MB(PSZUC pszPassword)
	{
	QString sPathFileName = S_SGetPathFileNameFromPassword(IN pszPassword);
	// Before attempting to load a wallet from a file, check if the wallet is already opened
	TWallet * pWallet = TWallet::s_palistWallet;
	while (pWallet != NULL)
		{
		if (pWallet->m_sPathFileSave == sPathFileName)
			{
			pWallet->TreeItemW_SelectWithinNavigationTree();	// Select the wallet so the user may see it
			return;
			}
		pWallet = pWallet->m_pNext;
		}
	CBin binWallet;
	MessageLog_AppendTextFormatSev(eSeverityNoise, "Opening wallet $Q\n", &sPathFileName);
	binWallet.BinFileReadE(sPathFileName);	// Attempt to read the file.  If the file is not found, create a new wallet
	pWallet = new TWallet(pszPassword, sPathFileName, INOUT &binWallet);	// Create a wallet object and let it decrypt and unserialize the transactions
	pWallet->EUnserializeFromXml(INOUT &binWallet);
	pWallet->TreeItemW_DisplayTransactionsBitcoin();
//	pWallet->TreeItem_SelectWithinNavigationTree();
	/*
	EMessageBoxInformation("Unable to open wallet $Q", &sPathFileName);
	delete pWallet;
	*/
	} // S_FileOpen_MB()

///////////////////////////////////////////////////////////////////////////////////////////////////
IEventWalletTransaction::IEventWalletTransaction(const TIMESTAMP * ptsEventID) : IEvent(ptsEventID)
	{
	m_amtQuantity = 0;
	}

#define d_chAttribute_nAmount		'a'
#define d_chAttribute_strValue		'v'
#define d_chAttribute_strComment	'c'

//	IEventWalletTransaction::IEvent::XmlSerializeCore()
void
IEventWalletTransaction::XmlSerializeCore(IOUT CBinXcpStanza * pbinXmlAttributes) const
	{
	pbinXmlAttributes->BinAppendXmlAttributeL64(d_chAttribute_nAmount, m_amtQuantity);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chAttribute_strValue, m_strValue);
	pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chAttribute_strComment, m_strComment);
	}

//	IEventWalletTransaction::IEvent::XmlUnserializeCore()
void
IEventWalletTransaction::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	pXmlNodeElement->UpdateAttributeValueL64(d_chAttribute_nAmount, OUT_F_UNCH &m_amtQuantity);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chAttribute_strValue, OUT_F_UNCH &m_strValue);
	pXmlNodeElement->UpdateAttributeValueCStr(d_chAttribute_strComment, OUT_F_UNCH &m_strComment);
	}

void
IEventWalletTransaction::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	_BinHtmlInitWithTime(OUT &g_strScratchBufferStatusBar);
	g_strScratchBufferStatusBar.BinAppendTextSzv_VE(
		Event_FIsEventTypeSent() ?
		"<img src=':/ico/Bitcoin' valign='bottom' style='valign:bottom'/> You sent <b>{Am}</b> to <b>$s</b>" :
		"<img src=':/ico/Bitcoin' valign='bottom' style='valign:bottom'/> You received <b>{Am}</b> from <b>$s</b>",
		-m_amtQuantity, ChatLog_PszGetNickNameOfContact());
	if (!m_strComment.FIsEmptyString())
		g_strScratchBufferStatusBar.BinAppendTextSzv_VE(": <i>$S</i>", &m_strComment);
	/*
	QTextFrameFormat oTextFrameFormat;
	oTextFrameFormat.setBorder(1);
	oTextFrameFormat.setPadding(2);
	oTextFrameFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
	oTextFrameFormat.setBorderBrush(QBrush(0xFF0000));
	poCursorTextBlock->currentFrame()->setFrameFormat(oTextFrameFormat);
	*/
	//poCursorTextBlock->block().setBorderBrush();
	poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, QBrush(0xFFFF99));
	}

BOOL
IEventWalletTransaction::FuIsTransactionMatchingViewFlags(EWalletViewFlags eWalletViewFlags) const
	{
	if (eWalletViewFlags == eWalletViewFlag_kmDisplayTransactionsAll)
		return TRUE;
	if (eWalletViewFlags & eWalletViewFlag_kfDisplayTransactionsSent)
		return (m_amtQuantity <= 0);
	if (eWalletViewFlags & eWalletViewFlag_kfDisplayTransactionsReceived)
		return (m_amtQuantity >= 0);
	return FALSE;
	}

EEventClass
CEventWalletTransactionSent::EGetEventClassForXCP() const
	{
	return CEventWalletTransactionReceived::c_eEventClass;
	}

void
CArrayPtrEvents::Wallets_AppendEventsTransactionsFor(ITreeItem * pFilterBy, EWalletViewFlags eWalletViewFlags)
	{
	Assert(pFilterBy != NULL);
	if (TWallet::s_palistWallet == NULL)
		return;	// There is no wallet opened, therefore no transactions

	const RTI_ENUM rti = pFilterBy->EGetRuntimeClass();
	if (rti == RTI(TWallet))
		{
		// Include all transactions related to a wallet, which means include all the vaults
		TWallet * pWallet = (TWallet *)pFilterBy;
		CVaultEventsForContact ** ppVaultStop;
		CVaultEventsForContact ** ppVault = pWallet->m_arraypaVaults.PrgpGetVaultsStop(OUT &ppVaultStop);
		while (ppVault != ppVaultStop)
			{
			CVaultEventsForContact * pVault = *ppVault++;
			pVault->AppendAllEventsTransactionsMatchingViewFlags(IOUT this, eWalletViewFlags);
			}
		}
	else if (rti == RTI(TContact))
		{
		// Include all transactions related to a contact
		TWallet * pWallet = TWallet::s_palistWallet;
		while (pWallet != NULL)
			{
			CVaultEventsForContact * pVault = pWallet->PFindVault((TContact *)pFilterBy);
			if (pVault != NULL)
				pVault->AppendAllEventsTransactionsMatchingViewFlags(IOUT this, eWalletViewFlags);
			pWallet = pWallet->PGetNextWallet();
			}
		} // if...else
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Constructor for adding a new vault.  A new vault is added when a new event transaction for a contact.
//	IMPLEMENTATION NOTES
//	This code must in sync with the constructor CHashTableWalletTransactionContactFromConfiguration() to calculate the hash value.
CVaultEventsForContact::CVaultEventsForContact(TWallet * pWalletParent, TContact * pContactParent_NZ)
	{
	Assert(pWalletParent->EGetRuntimeClass() == RTI(TWallet));
	Assert(pContactParent_NZ->EGetRuntimeClass() == RTI(TContact));
	m_pWalletParent = pWalletParent;
	m_pContactParent_YZ = pContactParent_NZ;
	pWalletParent->GetHashOfContact(OUT &m_hashContact, pContactParent_NZ);	// Calculate the hash value immediately; this is the only time it will be computed for the vault!
	}

void
TWallet::GetHashOfContact(OUT SHashSha1 * pHashSha1, const TContact * pContact) const
	{
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	TAccountXmpp * pAccount = pContact->m_pAccount;
	CChatConfiguration * pConfiguration = pAccount->m_pProfileParent->m_pConfigurationParent;
	g_strScratchBufferStatusBar.BinInitFromBinaryData(IN pConfiguration->PGetSalt(), CChatConfiguration::c_cbSalt);
	g_strScratchBufferStatusBar.BinAppendBinaryData(IN &m_hashSalt, sizeof(m_hashSalt));
	g_strScratchBufferStatusBar.BinAppendCBinLowercase(pAccount->m_strJID);
	g_strScratchBufferStatusBar.BinAppendCBinLowercase(pContact->m_strJidBare);
	HashSha1_CalculateFromCBin(OUT pHashSha1, g_strScratchBufferStatusBar);
	}

CVaultEventsForContact::~CVaultEventsForContact()
	{
	if (m_pContactParent_YZ != NULL)
		{
		Assert(m_pContactParent_YZ->EGetRuntimeClass() == RTI(TContact));
		m_pContactParent_YZ->m_uFlagsTreeItem |= TContact::FTI_kfChatLogEvents_RepopulateAll;	// Repopulate the Chat Log events which will remove the wallet transaction events
		}
	m_arraypaEvents.DeleteAllEvents();
	}

void
CVaultEventsForContact::ContactBind(TContact * pContact)
	{
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	Assert(m_pContactParent_YZ == NULL);
	m_pContactParent_YZ = pContact;
	m_pContactParent_YZ->m_uFlagsTreeItem |= TContact::FTI_kfChatLogEvents_RepopulateAll | TContact::FTI_kfChatLogEvents_IncludeFromWallet;
	}

//	This is typically when a contact is being deleted
void
CVaultEventsForContact::ContactUnbindIfAboutBeingDeleted()
	{
	if (m_pContactParent_YZ != NULL)
		{
		Assert(m_pContactParent_YZ->EGetRuntimeClass() == RTI(TContact));
		if (m_binEventsEncrypted.FIsEmptyBinary())
			EventsEncryptCb();
		if (m_pContactParent_YZ->m_uFlagsTreeItem & ITreeItem::FTI_kfTreeItem_AboutBeingDeleted)
			{
			m_pContactParent_YZ = NULL;
			m_arraypaEvents.DeleteAllEvents();	// The events are no longer available since its parent contact is about being deleted
			}
		}
	}

/*
IEventWalletTransaction *
CVaultEventsForContact::PAllocateEventTransaction()
	{
	Assert(m_pContactParent_YZ != NULL);
	Assert(m_pContactParent_YZ->EGetRuntimeClass() == RTI(TContact));
	m_binEventsEncrypted.Empty();	// We need to re-encrypt the events
	IEventWalletTransaction * pTransaction = new IEventWalletTransaction(m_pContactParent_YZ, NULL);
	m_arraypaEvents.Add(PA_CHILD pTransaction);
	Assert(pContactParent->EGetRuntimeClass() == RTI(TContact));
	Assert(FALSE && "This code needs to be re-implemented and will crash!");
	pContactParent->m_uFlagsTreeItem |= TContact::FTI_kfChatLogEventsRepopulateAll | TContact::FTI_kfChatLogEventsIncludeFromWallet;

	return pTransaction;
	}
*/

void
CVaultEventsForContact::AppendAllEventsTransactionsMatchingViewFlags(IOUT CArrayPtrEvents * parraypEventsTransactions, EWalletViewFlags eWalletViewFlags) CONST_MAY_CREATE_CACHE
	{
	Assert(parraypEventsTransactions != NULL);
	if (m_arraypaEvents.FIsEmpty())
		EventsDecrypt();
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEventWalletTransaction * pEvent = (IEventWalletTransaction *)*ppEvent++;
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		if (pEvent->EGetEventClass() != eEventClass_eWalletTransactionSent)
			continue;
		if (pEvent->FuIsTransactionMatchingViewFlags(eWalletViewFlags))
			parraypEventsTransactions->Add(pEvent);
		}
	}

#define d_chXmlAttributeSalt	's'
#define d_szXmlAttributeSalt	"s"
#define d_chXmlElementVault		'v'
#define d_szXmlElementVault		"v"
#define d_chXmlAttributeContact	'c'
#define d_szXmlAttributeContact	"c"

//	Constructor to unserialize a vault.
CVaultEventsForContact::CVaultEventsForContact(TWallet * pWalletParent, const CXmlNode * pXmlNodeUnserialize)
	{
	Assert(pWalletParent->EGetRuntimeClass() == RTI(TWallet));
	m_pWalletParent = pWalletParent;
	m_pContactParent_YZ = NULL;
	if (pXmlNodeUnserialize->UpdateAttributeValueHashSha1(d_chXmlAttributeContact, OUT_F_UNCH &m_hashContact))
		{
		// Extract the encrypted events (transactions) only if the SHA-1 appears as valid: the hash is necessary to decrypt the events, and without the this decryption key, this encrypted data remain encrypted forever, therefore polluting the database
		m_binEventsEncrypted.BinAppendBinaryDataFromBase85Szv_ML(pXmlNodeUnserialize->m_pszuTagValue);
		}
	}

void
CVaultEventsForContact::EventsDecrypt()
	{
	Assert(m_arraypaEvents.FIsEmpty());
	if (m_pContactParent_YZ != NULL)
		{
		//MessageLog_AppendTextFormatCo(d_coYellowDirty, "Vault Events for $S: {h|}\n$B\n", &m_pContactParent_YZ->m_strJidBare, &m_hashContact, &m_binEventsEncrypted);
		CXmlTree oXmlTree;
		if (oXmlTree.EParseFileDataToXmlNodesCopy_ML(IN m_binEventsEncrypted) == errSuccess)
			m_arraypaEvents.EventsUnserializeFromDisk(IN &oXmlTree, m_pContactParent_YZ);
		}
	}

UINT
CVaultEventsForContact::EventsEncryptCb()
	{
	Assert(m_binEventsEncrypted.FIsEmptyBinary());
	/*
	g_strScratchBufferStatusBar.Empty();
	m_arraypaEvents.EventsSerializeForDisk(INOUT &g_strScratchBufferStatusBar);
	int cbEvents = g_strScratchBufferStatusBar.CbGetData();
	if (cbEvents > 0)
		m_binEventsEncrypted.BinAppendBinaryData(g_strScratchBufferStatusBar.PvGetDataUnsafe(), cbEvents);
	Assert(cbEvents >= 0);
	return cbEvents;
	*/
	return 0;
	}

//	Serialize a vault only if it has events.
void
CVaultEventsForContact::XmlSerializeVault(IOUT CBin * pbinXml)
	{
	if (m_binEventsEncrypted.FIsEmptyBinary())
		{
		// We need to serialize and encrypt the events
		if (EventsEncryptCb() == 0)
			return;	// Nothing to serialize
		}
	pbinXml->BinAppendTextSzv_VE("<"d_szXmlElementVault" "d_szXmlAttributeContact"='{h|}'>{B|}</"d_szXmlElementVault">\n", &m_hashContact, &m_binEventsEncrypted);
	}

EError
TWallet::EUnserializeFromXml(INOUT CBin * pbinWalletXml)
	{
	// Check if the 'decoded' XML appears as a valid XML.  If not, it is likely the password was incorrect and therefore the decryption transformed the encrypted wallet to some random bytes of data.
	(void)pbinWalletXml->FAssignFileSizeAndMd5(INOUT_F_UNCH &m_hashMd5LastSave);	// Remember the MD5
	CXmlTree oXmlTree;
	if (oXmlTree.EParseFileDataToXmlNodesModify_ML(INOUT pbinWalletXml) == errSuccess)
		{
		//MessageLog_AppendTextFormatCo(d_coGreen, "Wallet:^N", &oXmlTree);
		const CXmlNode * pXmlNodeRoot = oXmlTree.PGetRootNodeValidate(c_szaWallet);
		if (pXmlNodeRoot != NULL)
			{
			// TBD: Get the name of the wallet
			if (pXmlNodeRoot->UpdateAttributeValueHashSha1(d_chXmlAttributeSalt, OUT_F_UNCH &m_hashSalt))
				{
				CHashTableVaults oHashTableVaults;
				const CXmlNode * pXmlNodeVault = pXmlNodeRoot->PFindElement(d_chXmlElementVault);
				while (pXmlNodeVault != NULL)
					{
					CVaultEventsForContact * pVault = new CVaultEventsForContact(this, pXmlNodeVault);
					m_arraypaVaults.Add(PA_CHILD pVault);
					oHashTableVaults.AddVault(IN pVault);
					pXmlNodeVault = pXmlNodeVault->m_pNextSibling;
					}
				// Bind each vault to a contact.  This is done by searching every contact and matching them to a vault.  Some vaults may have no contact because the contact may have been deleted.
				// In the future, for performance reasons, the method GetHashOfContact() could be unrolled from the loop.
				TAccountXmpp ** ppAccountStop;
				TAccountXmpp ** ppAccount = g_arraypAccounts.PrgpGetAccountsStop(OUT &ppAccountStop);
				while (ppAccount != ppAccountStop)
					{
					const TAccountXmpp * pAccount = *ppAccount++;
					Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
					TContact ** ppContactStop;
					TContact ** ppContact = pAccount->PrgpGetContactsStop(OUT &ppContactStop);
					while (ppContact != ppContactStop)
						{
						TContact * pContact = *ppContact++;
						Assert(pContact != NULL);
						Assert(pContact->EGetRuntimeClass() == RTI(TContact));
						Assert(pContact->m_pAccount == pAccount);
						SHashSha1 hashContact;
						GetHashOfContact(OUT &hashContact, IN pContact);
						CVaultEventsForContact * pVault = oHashTableVaults.PFindVaultRemove(IN &hashContact);
						if (pVault != NULL)
							pVault->ContactBind(pContact);
						if (oHashTableVaults.GetElementsCount() == 0)
							goto Done;	// If there are no more elements in the hash table, then we are done
						} // while
					} // while
				} // if
			} // if
		} // if
	Done:
	HashSha1_InitRandomIfEmpty(INOUT &m_hashSalt);
	return errSuccess;
	} // EUnserializeFromXml()

void
TWallet::SerializeXmlToDisk()
	{
	CBin binXml;
	binXml.BinAppendTextSzv_VE("<$s "d_szXmlAttributeSalt"='{h|}'>\n", c_szaWallet, &m_hashSalt); // Name of the wallet
	// Serialize each vault
	CVaultEventsForContact ** ppVaultStop;
	CVaultEventsForContact ** ppVault = m_arraypaVaults.PrgpGetVaultsStop(OUT &ppVaultStop);
	while (ppVault != ppVaultStop)
		{
		CVaultEventsForContact * pVault = *ppVault++;
		pVault->XmlSerializeVault(INOUT &binXml);
		}
	binXml.BinAppendTextSzv_VE("</$s>", c_szaWallet);
	if (binXml.FAssignFileSizeAndMd5(INOUT_F_UNCH &m_hashMd5LastSave))
		{
		// Encrypt the file with the password
		if (m_sPathFileSave.isEmpty())
			m_sPathFileSave = S_SGetPathFileNameFromPassword(m_strNameDisplayTyped);	// At the moment, use the display name as the password
		EError err = binXml.BinFileWriteE(m_sPathFileSave);
		Report(err == errSuccess);
		}
	} // SerializeXmlToDisk()


///////////////////////////////////////////////////////////////////////////////////////////////////
//	This constructor must be followed by a call to EUnserializeFromXml() to return an error while loading the wallet.
TWallet::TWallet(PSZUC pszPassword, const QString & sPathFileWallet, INOUT CBin * pbinWalletEncrypted)
	{
	Assert(pszPassword != NULL);
	Assert(!sPathFileWallet.isEmpty());
	Assert(pbinWalletEncrypted != NULL);
	m_pNext = s_palistWallet;
	s_palistWallet = this;	// Add the wallet to the global linked list
	m_sPathFileSave = sPathFileWallet;
	HashKey256_CalculateKeyFromPassword(OUT &m_hashKey, pszPassword);
	InitToZeroes(OUT &m_hashMd5LastSave, sizeof(m_hashMd5LastSave));
	HashSha1_InitEmpty(OUT &m_hashSalt);

	if (!pbinWalletEncrypted->FIsEmptyBinary())
		pbinWalletEncrypted->DataDecryptAES256(IN &m_hashKey);
	TreeItemW_DisplayWithinNavigationTreeExpand(g_pBanking, (PSZAC)pszPassword, eMenuIconBitcoin);	// Display the wallet in the navigation tree using the password as the name (in the future the name will be stored elsewhere)
	}

TWallet::~TWallet()
	{
	// Delete each vault
	CVaultEventsForContact ** ppVaultStop;
	CVaultEventsForContact ** ppVault = m_arraypaVaults.PrgpGetVaultsStop(OUT &ppVaultStop);
	while (ppVault != ppVaultStop)
		{
		CVaultEventsForContact * paVault = *ppVault++;
		delete paVault;
		}
	m_arraypaVaults.RemoveAllElements();

	// Remove the wallet from the global linked list
	Assert(s_palistWallet != NULL);
	if (s_palistWallet == this)
		s_palistWallet = m_pNext;
	else
		{
		TWallet * pWalletPrev = s_palistWallet;
		TWallet * pWallet = pWalletPrev->m_pNext;
		while (TRUE)
			{
			Assert(pWallet != NULL);
			if (pWallet == this)
				{
				pWalletPrev->m_pNext = m_pNext;
				break;
				}
			pWalletPrev = pWallet;
			pWallet = pWallet->m_pNext;
			if (pWallet == NULL)
				break;
			} // while
		} // if...else
	}



struct SBitcoinTransaction
	{
	PSZAC pszDescription;
	int m_nAmountUSD;
	};

const SBitcoinTransaction c_rgBitcoinTransactions[] =
{
	{ "Rent", -800 },
	{ "Coffee", -3 },
	{ "Web Hosting", -160 },
	{ "Domain Name", -13 },
	{ "20 hours Spanish Lessons", -200 },
	{ "Dental work", -60 },
	{ "Purchaded more Bitcoins", +1500 },
	{ "Garage Sale", +140 },
	{ "Sold Widget", +20 },
	{ "Sold BTC at Localbitcoins", -100 },
};

UINT g_uBitcoinValueUSD = 800;	// How many USD are necessary to buy 1 BTC

#define d_ts_cWeeks			604800000	// Number of weeks in a timestamp

void
TWallet::GenerateDummyTransactions()
	{
	CArrayPtrContacts arraypContacts;
	// Get all the contacts currently present
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = g_arraypAccounts.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		pAccount->Contacts_Append(IOUT &arraypContacts);
		}

	const TIMESTAMP tsLastWeek = Timestamp_GetCurrentDateTime() - d_ts_cWeeks;
	qsrand(tsLastWeek);	// Initialize the random generator with a different seed

	const int cContacts = arraypContacts.GetSize();
	if (cContacts == 0)
		return;	// There is not a single contact, therefore no transactions to add
	int cTransactions = cContacts * 10;	// Generate an average of 10 transactions per contact
	MessageLog_AppendTextFormatSev(eSeverityNoise, "Generating $i dummy transactions for wallet '$S'...\n", cTransactions, &m_strNameDisplayTyped);
	while (cTransactions-- > 0)
		{
		TContact * pContact = (TContact *)arraypContacts.PvGetElementAtSafeRandom_YZ();
		Assert(pContact != NULL);
		CVaultEvents * pVault = pContact->m_paVaultEvents;
		if (pVault == NULL)
			continue;
		IEvent * pEventLast = pVault->PGetEventLast_YZ();
		if (pEventLast == NULL)
			continue;
		TIMESTAMP tsTransactionEnd = pEventLast->m_tsEventID;
		if (tsTransactionEnd <= pContact->m_tsCreated)
			continue;
		TIMESTAMP tsTransactionBegin = pContact->m_tsCreated;
		if (tsTransactionBegin < tsLastWeek)
			tsTransactionBegin = tsLastWeek;
		IEventWalletTransaction * pTransaction = PAllocateEventTransaction(pContact);
		pTransaction->m_tsEventID = LGenerateRandomValueBetween(tsTransactionBegin, tsTransactionEnd);
		const SBitcoinTransaction * pDescription = &c_rgBitcoinTransactions[qrand() % LENGTH(c_rgBitcoinTransactions)];
		pTransaction->m_amtQuantity = (L64)pDescription->m_nAmountUSD * d_cSatoshisPerBitcoin / g_uBitcoinValueUSD;
		pTransaction->m_strValue.Format("$i USD", pDescription->m_nAmountUSD);
		pTransaction->m_strComment.BinInitFromStringWithNullTerminator(pDescription->pszDescription);

		g_uBitcoinValueUSD += (qrand() % 4) - 1;	// Make the exchange rate vary a bit
		} // while

	// Make sure the events of each vault are sorted, otherwise the unserializer will complain the events are out of order
	CVaultEventsForContact ** ppVaultStop;
	CVaultEventsForContact ** ppVault = m_arraypaVaults.PrgpGetVaultsStop(OUT &ppVaultStop);
	while (ppVault != ppVaultStop)
		{
		CVaultEventsForContact * pVault = *ppVault++;
		pVault->SortEventsByIDs();
		}
	TreeItem_GotFocus();	// Refresh the layout
	} // GenerateDummyTransactions()

CVaultEventsForContact *
TWallet::PFindVault(TContact * pContact)
	{
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	CVaultEventsForContact ** ppVaultStop;
	CVaultEventsForContact ** ppVault = m_arraypaVaults.PrgpGetVaultsStop(OUT &ppVaultStop);
	while (ppVault != ppVaultStop)
		{
		CVaultEventsForContact * pVault = *ppVault++;
		if (pVault->PGetContact_YZ() == pContact)
			return pVault;
		}
	return NULL;
	}

IEventWalletTransaction *
TWallet::PAllocateEventTransaction(TContact * pContact)
	{
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	// Find the vault where the transaction belongs
	CVaultEventsForContact * pVault;
	CVaultEventsForContact ** ppVaultStop;
	CVaultEventsForContact ** ppVault = m_arraypaVaults.PrgpGetVaultsStop(OUT &ppVaultStop);
	while (ppVault != ppVaultStop)
		{
		pVault = *ppVault++;
		if (pVault->PGetContact_YZ() == pContact)
			goto AddEventToVault;
		}
	pVault = new CVaultEventsForContact(this, pContact);
	m_arraypaVaults.Add(PA_CHILD pVault);
	AddEventToVault:
	Assert(FALSE && "NYI");
	return NULL; //pVault->PAllocateEventTransaction();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
TWalletView::TWalletView(ITreeItem * pParentFilterBy, PSZAC pszName, EWalletViewFlags eWalletViewFlags)
	{
	m_pTreeItemFilterBy = pParentFilterBy;
	m_eWalletViewFlags = eWalletViewFlags;
	TreeItemW_DisplayWithinNavigationTreeExpand(pParentFilterBy, pszName, eMenuIconBitcoin);
	}

TWalletView::~TWalletView()
	{
	}


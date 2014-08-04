#ifndef TAPPLICATIONBALLOTMASTER_H
#define TAPPLICATIONBALLOTMASTER_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class TApplicationBallotmaster : public IApplication
{
protected:
	CStr m_strUrlAddress;				// Address to start the application (of course, this solution is not 100% portable, because the HTML files for the application are stored into the "user folder", however remembering the last URL is better than nothing)
	CVaultEvents * m_paVaultBallots;	// Ballots are 'events' because they may be sent to other users, and therefore require a vault for their storage.
public:
    WLayoutBrowser * m_pawLayoutBrowser;

private:
	TContact * m_paContactDummy;		// Temporary hack to have a dummy contact as the 'parent' of the vault.  This is necessary because the vault was designed to have a contact as its parent, and need to be refactored.

public:
	TApplicationBallotmaster(TProfile * pProfileParent);
	~TApplicationBallotmaster();
	virtual PSZAC PszGetClassNameApplication() { return c_szaApplicationClass_Ballotmaster; }	// From IApplication
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);			// From IXmlExchange
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);			// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);	// From ITreeItem
	virtual void TreeItem_GotFocus();										// From ITreeItem

	CVaultEvents * PGetVault_NZ();

    CEventBallotPoll * PAllocateBallot(const IEventBallot * pEventBallotTemplate = NULL);
	void EventBallotAddAsTemplate(IEventBallot * pEventBallot);
	void ApiBallotSave(IN PSZUC pszXmlBallot);
	void ApiBallotsList(OUT CBin * pbinXmlBallots);
	RTI_IMPLEMENTATION(TApplicationBallotmaster)
};

//	Generic class to copy the data from an event to another event.
class CBinXcpStanzaEventCopier : public CBinXcpStanza
{
protected:
	TContact * m_paContact;		// We need a contact to clone the event, so use an empty contact so there is no interference with existing data
public:
	CBinXcpStanzaEventCopier(ITreeItemChatLogEvents * pContactOrGroup);
	~CBinXcpStanzaEventCopier();

	void EventCopy(IN const IEvent * pEventSource, OUT IEvent * pEventDestination);
};

//	Generic class to clone an event
class CBinXcpStanzaEventCloner : public CBinXcpStanzaEventCopier
{
public:
	IEvent * PaEventClone(IEvent * pEventToClone);
};

#endif // TAPPLICATIONBALLOTMASTER_H

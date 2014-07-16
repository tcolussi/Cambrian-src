#ifndef TAPPLICATIONBALLOTMASTER_H
#define TAPPLICATIONBALLOTMASTER_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class TApplicationBallotmaster : public IApplication
{
protected:
	CStr m_strUrlAddress;				// Address to start the application (of course, this solution is not 100% portable, because the HTML files for the application are stored into the user folder, however remembering the last URL is better than nothing)
	WLayoutBrowser * m_pawLayoutBrowser;
public:
	TApplicationBallotmaster(TProfile * pProfileParent);
	~TApplicationBallotmaster();
	virtual PSZAC PszGetClassNameApplication() { return c_szaApplicationClass_Ballotmaster; }	// From IApplication
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);			// From IXmlExchange
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);			// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);	// From ITreeItem
	virtual void TreeItem_GotFocus();										// From ITreeItem

	RTI_IMPLEMENTATION(TApplicationBallotmaster)
};

#endif // TAPPLICATIONBALLOTMASTER_H

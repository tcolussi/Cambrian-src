#ifndef TAPPLICATIONMAYANX_H
#define TAPPLICATIONMAYANX_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
class TApplicationMayanX : public IApplication
{
public:
	CStr m_strxUserID;
protected:
	class WLayoutApplicationMayanX * m_pawLayout;
public:
	TApplicationMayanX(TProfile * pProfileParent);
	~TApplicationMayanX();
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);	// From IXmlExchange
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);			// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);	// From ITreeItem
	virtual void TreeItem_GotFocus();									// From ITreeItem
	RTI_IMPLEMENTATION(TApplicationMayanX)
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class CInternetServerMayanX : public QNetworkAccessManager
{
public:
	CStr m_strxSessionID;
public:
	void RequestSend(CInternetRequestWebMethodXmlMayanX * pRequest);
};


#endif // TAPPLICATIONMAYANX_H

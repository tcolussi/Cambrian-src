#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TApplicationMayanX.h"

TApplicationMayanX::TApplicationMayanX(TProfile * pProfileParent) : IApplication(pProfileParent)
	{
	m_pawLayout = NULL;
	}

void
TApplicationMayanX::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	IApplication::XmlExchange(INOUT pXmlExchanger);
	pXmlExchanger->XmlExchangeStr("xUserID", INOUT_F_UNCH_S &m_strxUserID);
	}

const EMenuActionByte c_rgzeActionsMenuApplication[] =
	{
	eMenuAction_ApplicationDelete,
	ezMenuActionNone
	};

//	TApplicationMayanX::ITreeItem::TreeItem_MenuAppendActions()
void
TApplicationMayanX::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionsAdd(c_rgzeActionsMenuApplication);
	}

//	TApplicationMayanX::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TApplicationMayanX::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_ApplicationDelete:
		m_pProfileParent->TreeItem_SelectWithinNavigationTree();	// Select the parent profile so the Navigation Tree does not attempt to select a deleted Tree Item.
		m_pProfileParent->m_arraypaApplications.DeleteTreeItem(PA_DELETING this);
		return ezMenuActionNone;
	default:
		return IApplication::TreeItem_EDoMenuAction(eMenuAction);
		}
	}


//	This function must have the same interface as PFn_PaAllocateApplication()
IApplication *
PaAllocateApplicationMayanX(TProfile * pProfileParent)
	{
	return new TApplicationMayanX(pProfileParent);
	}

TApplicationMayanX *
TProfile::PAllocateApplicationMayanX()
	{
	TApplicationMayanX * pApplication = (TApplicationMayanX *)PaAllocateApplicationMayanX(this);
	m_arraypaApplications.Add(PA_CHILD pApplication);
	pApplication->TreeItem_DisplayWithinNavigationTreeExpand(this, "MayanX", eMenuIconCoffeeExchange);
	return pApplication;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if 1
const char c_szaUrlApiMayanX[] = "http://mayanx.webaweb.net/api.asmx";	// mayan.cambrian.org:8080
#else
const char c_szaUrlApiMayanX[] = "http://localhost:1506/api.asmx";	// For testing on the local machine
#endif

void
CInternetServerMayanX::RequestSend(CInternetRequestWebMethodXmlMayanX * pRequest)
	{
	pRequest->MethodNameClose();

	MessageLog_AppendTextFormatCo(d_coGreen, "[$@] Connecting to URL '$s' with the following POST content:\n", c_szaUrlApiMayanX);
	MessageLog_AppendTextFormatCo(d_coGrayDark, "$B\n\n", pRequest);

	QUrl url(c_szaUrlApiMayanX);
	QNetworkRequest oNetworkRequest(url);
	oNetworkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; charset=utf-8");
	/*
	Microsoft IIS puts the following value: Cookie: ASP.NET_SessionId=av231dqxfh50yvwhmy3cfefo
	#if 0
	oNetworkRequest.setRawHeader("xSessionID", m_strxSessionID.ToQByteArrayShared());
	#else
	QByteArray xSessionID = m_strxSessionID.ToQByteArrayDeepCopy();
	MessageLog_AppendTextFormatCo(d_coOrange, "xSessionID = '$Y'\n", &xSessionID);
//	oNetworkRequest.setRawHeader("xSessionID", xSessionID);
	#endif
	*/
	post(oNetworkRequest, pRequest->ToQByteArrayDeepCopy());	// Make a deep copy of the request (otherwise the HTTP post will not work - because the request is likely to be on the stack, and the execution of the method post() is asynchronous)
	}

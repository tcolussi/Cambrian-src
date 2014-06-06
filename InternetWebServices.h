#ifndef INTERNETWEBSERVICES_H
#define INTERNETWEBSERVICES_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class CInternetRequestWebMethodXml : public CBin	// (request)
{
public:
	PSZAC m_pszMethodName;		// Name of the method to call for the web service
	BOOL m_fMethodClosed;
public:
	CInternetRequestWebMethodXml();
	void MethodNameOpen(PSZAC pszFmtHeader, PSZAC pszMethodName);
	void MethodNameClose(PSZAC pszFmtFooter);
};

class CInternetRequestWebMethodXmlMayanX : public CInternetRequestWebMethodXml
{
public:
	CInternetRequestWebMethodXmlMayanX(PSZAC pszMethodName);
	void MethodNameClose();
};

class CInternetResponseWebMethodXmlSoap
{
protected:
	CXmlTree m_oXmlTreeResponse;
	CXmlNode * m_pNodeRootSoapBody;	// Where the element <soap:Body> begins
	CXmlNode * m_pNodeRootResponse;	// Root node of the response, skipping the <soap:Envelope> and <soap:Body>.
	CXmlNode * m_pNodeError;		// Pointer to the node containing the error code and error description.	 Since the error may be an "Out Parameter", the error node may be a sibling of the response root node.

public:
	CInternetResponseWebMethodXmlSoap(QNetworkReply * poNetworkReply);
	CXmlNode * PGetXmlNodeResponse(PSZAC pszMethodName);
	CXmlNode * PGetXmlNodeError();
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Class to handle connections with an internet server.
//
//	For simplicity, this class is a wrapper of QNetworkAccessManager which does the core work.  BTW, sizeof(QNetworkAccessManager) = 8 bytes.
class CInternetServer : public QNetworkAccessManager
{
public:
	CStr m_strUrlBase;
public:
	CInternetServer(QObject * poParentToConnect);
	void SetUrlBase(PSZAC pszUrlBase);
	void RequestSend(CInternetRequestWebMethodXml * pRequest);
};


#endif // INTERNETWEBSERVICES_H

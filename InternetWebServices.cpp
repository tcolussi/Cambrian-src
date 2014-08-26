#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

/*
CInternetRequestWebMethodXml::CInternetRequestWebMethodXml(PSZAC pszMethodName, PSZAC pszAttributes)
	{
	MethodNameCreate(pszMethodName, pszAttributes);
	}

void
CInternetRequestWebMethodXml::MethodNameCreate(PSZAC pszMethodName, PSZAC pszAttributes)
	{
	Assert(pszMethodName != NULL);
	Assert(*pszMethodName != '\0');

	(void)PvSizeAlloc(3000);	// Allocate 3 KB, enough for the typical request for a web method
	Assert(FIsEmptyBinary());
	BinAppendText_VE("<$s $s>", pszMethodName, pszAttributes);

	// Remember the method name, so ew may close the request
	m_pszMethodName = pszMethodName;
	} // MethodNameCreate()

void
CInternetRequestWebMethodXml::MethodNameClose()
	{
	Assert(m_pszMethodName != NULL && "Method name invalid");
	BinAppendText_VE("</$s>", m_pszMethodName);
	#ifdef DEBUG
	AssertValidUtf8(PbGetData());
	#endif
	} // MethodNameClose()
*/

CInternetRequestWebMethodXml::CInternetRequestWebMethodXml()
	{
	m_fMethodClosed = FALSE;
	}

void
CInternetRequestWebMethodXml::MethodNameOpen(PSZAC pszFmtHeader, PSZAC pszMethodName)
	{
	m_pszMethodName = pszMethodName;
	BinAppendText_VE(pszFmtHeader, pszMethodName);
	m_fMethodClosed = FALSE;
	}

void
CInternetRequestWebMethodXml::MethodNameClose(PSZAC pszFmtFooter)
	{
	if (m_fMethodClosed)
		return;
	m_fMethodClosed = TRUE;
	BinAppendText_VE(pszFmtFooter, m_pszMethodName);
	}


CInternetRequestWebMethodXmlMayanX::CInternetRequestWebMethodXmlMayanX(PSZAC pszMethodName)
	{
	MethodNameOpen(
		"<?xml version='1.0' encoding='utf-8'?>"
			"<soap:Envelope xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xsd='http://www.w3.org/2001/XMLSchema' xmlns:soap='http://schemas.xmlsoap.org/soap/envelope/'>"
			"<soap:Body>\n"
				"<$s xmlns='http://mayanx.com/'>\n", pszMethodName);
	}

void
CInternetRequestWebMethodXmlMayanX::MethodNameClose()
	{
	CInternetRequestWebMethodXml::MethodNameClose("</$s>\n</soap:Body></soap:Envelope>");
	}

CInternetResponseWebMethodXmlSoap::CInternetResponseWebMethodXmlSoap(QNetworkReply * poNetworkReply)
	{
	m_pNodeRootSoapBody = NULL;
	m_pNodeRootResponse = NULL;
	m_pNodeError = NULL;
	QByteArray arraybResponse = poNetworkReply->readAll(); // Use: read()

	#if 1
	QNetworkReply::NetworkError eError = poNetworkReply->error();
	QNetworkRequest::Attribute eAttribute = (QNetworkRequest::Attribute)poNetworkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	QByteArray arraybContentType = poNetworkReply->header(QNetworkRequest::ContentTypeHeader).toByteArray();
	if (eError == QNetworkReply::NoError)
		{
		QString sUrl = poNetworkReply->url().toString();
		MessageLog_AppendTextFormatCo(d_coGreenDarker, "[$@] URL '$Q' returned HTTP Status Code $i with the following content-type: $Y\n", &sUrl, eAttribute, &arraybContentType);
		}
	else
		MessageLog_AppendTextFormatCo(d_coRed, "[$@] Qt returned the network error code $i and the following content:\n", eError);
	MessageLog_AppendTextFormatCo(d_coBlack, "$Y\n\n", &arraybResponse);
	#endif

	// Extract the value for the response
	if (m_oXmlTreeResponse.EParseFileDataToXmlNodesCopy_ML(IN arraybResponse) == errSuccess)
		{
		CXmlNode * pNodeRoot = m_oXmlTreeResponse.PGetRootNodeValidate("soap:Envelope");
		if (pNodeRoot != NULL)
			m_pNodeRootSoapBody = pNodeRoot->PFindElement("soap:Body");
		}
	}

CXmlNode *
CInternetResponseWebMethodXmlSoap::PGetXmlNodeResponse(PSZAC pszMethodName)
	{
	Assert(pszMethodName != NULL);
	Assert(strlenU(pszMethodName) < 200);
	if (m_pNodeRootSoapBody != NULL)
		{
		char szaMethodNameResponse[220];
		// Find the node "response"
		const char * pchSrc = pszMethodName;
		char * pchDst = szaMethodNameResponse;
		while (TRUE)
			{
			CHS ch = *pchSrc++;
			if (ch == '\0')
				break;
			*pchDst++ = ch;
			}
		strcpy(OUT pchDst, "Response");
		CXmlNode * pXmlNodeResponse = m_pNodeRootSoapBody->PFindElement(szaMethodNameResponse);
		if (pXmlNodeResponse != NULL)
			{
			strcpy(OUT pchDst, "Result");
			return pXmlNodeResponse->PFindElement(szaMethodNameResponse);
			}
		} // if
	return NULL;
	} // PGetXmlNodeResponse()

CXmlNode *
CInternetResponseWebMethodXmlSoap::PGetXmlNodeError()
	{
	if (m_pNodeRootSoapBody != NULL)
		{
		// The error is typically burried under the element "*Response".  Since we do not the method name, we will search within the elements of "*Response"
		CXmlNode * pXmlNodeResponse = m_pNodeRootSoapBody->m_pElementsList;
		if (pXmlNodeResponse != NULL)
			{
			CXmlNode * pXmlNodeError = pXmlNodeResponse->PFindElement("Error");
			if (pXmlNodeError != NULL)
				return pXmlNodeError;	// This is the case of an error returned as an "out parameter"
			// The "*Response" does not have the error, therefore search within the "*Result"
			/*
			CXmlNode * pXmlNodeResult = pXmlNodeResponse->m_pElementsList;
			if (pXmlNodeResult != NULL)
				return pXmlNodeResult->PFindElement("Error");
			*/
			}
		}
	return NULL;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CInternetServer::CInternetServer(QObject * poParentToConnect) : QNetworkAccessManager(poParentToConnect)
	{
	}

void
CInternetServer::SetUrlBase(PSZAC pszUrlBase)
	{
	m_strUrlBase.BinInitFromStringWithNullTerminator(pszUrlBase);
	}
/*
void
CInternetServer::RequestSend(CInternetRequestWebMethodXml * pRequest)
	{
	CStr strUrl = m_strUrlBase;
	//strUrl.AppendTextU((PSZUC)pRequest->m_pszMethodName);

	// Dump the request into the message log, for debugging purpose
	MessageLog_AppendTextFormatCo(d_coGreen, "[$@] Connecting to URL '$S' with the following POST content:\n", &strUrl);
	MessageLog_AppendTextFormatCo(d_coGrayDark, "$B\n\n", pRequest);

	QUrl url(strUrl);
	QNetworkRequest oNetworkRequest(url);
	oNetworkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");
	post(oNetworkRequest, pRequest->ToQByteArrayDeepCopy());	// Make a deep copy of the request (otherwise the HTTP post will not work - because the request is likely to be on the stack, and the execution of the method post() is asynchronous)
	}
*/

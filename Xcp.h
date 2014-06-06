#ifndef XCP_H
#define XCP_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif


class IData;
class IDataFile;

class IEventData;
class IEventDataFile;
/*
#define d_cbData_FileNotFound

class IData
{
public:
	IEventData * m_pEventParent;
	IData * m_pNext;				// Next data item
	CStr m_strxIdentifier;			// Identifier of the data item.  This identifier can be anyting, and its interpretation depends on the classes inheriting from IData.
	L64 m_cbData;					// Total number of bytes for the data item (negative values mean there is an error)

	enum { c_cbData_FileNotFound = -1 };

public:
	IData(PA_PARENT IEventData * pEventParent);
	virtual ~IData();
	virtual void XmlSerialize(IOUT CBin * pbinXmlAttributes, const TContact * pContactToSerializeFor) const;
	virtual void XmlUnserialize(const CXmlNode * pXmlNodeElement);
	virtual void XcpDataWriteToSocket(L64 ibData, int cbData) = 0;
	virtual void XcpDataArrived(L64 ibData, int cbData, const void * pvData) = 0;
};

class IEventData : public IEvent
{
public:
	IData * m_palistData;
public:

};


///////////////////////////////////////////////////////////////////////////////////////////////////
class IDataFile : public IData
{
public:
	CFile * m_paFile;
	enum { c_cbDataMax = 16*1024 };	// Do not transmit more than 16 KiB at the time

public:
	IDataFile(IEventDataFile * pParent);
	~IDataFile();
	CFile * _PGetFile();
	virtual void XcpDataWriteToSocket(L64 ibData, int cbData);
	virtual void XcpDataArrived(L64 ibData, int cbData, const void * pvData);
};
*/
/*
class IEventDataFile : public IEvent
{
public:
	CStr m_strFileName;
	L64 m_cblFileSize;			// Size of the file (in bytes) to transfer
	CFile * m_paFile;
public:
	IEventDataFile(ITreeItemChatLogEvents * pContact, const TIMESTAMP * ptsEventID);
	~IEventDataFile();
	virtual void XmlSerializeCore(IOUT CBin * pbinXmlAttributes, const TContact * pContactToSerializeFor) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
};

//	The user is offering a file to the contact(s)
class CEventDataFileSent : public IEventDataFile
{
public:
	CEventDataFileSent(ITreeItemChatLogEvents * pContact, const TIMESTAMP * ptsEventID);
	virtual void XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData);
};

//	The user received a file offer from the contact.
class CEventDataFileReceived : public IEventDataFile
{
public:
	CEventDataFileReceived(ITreeItemChatLogEvents * pContact, const TIMESTAMP * ptsEventID);
	virtual void XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData);
};
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
typedef UINT UINT_VERSION;		// At the moment, use a 32-bit integer to store the version number

class CEventIdentityStream
{

};


///////////////////////////////////////////////////////////////////////////////////////////////////
//	An 'identity' is an object which is updated often.
class IEventIdentity : public IEvent
{
public:
	SHashSha1 m_hashIdentity;	// Identity of the object
	UINT_VERSION m_uVersion;	// Version of the object (each time the object is modified, the version is incremented)

public:
	virtual void XmlUnserializeStream(const CXmlNode * pXmlNodeStream) { Assert(pXmlNodeStream != NULL); }
};

class CEventGroupUpdate : public IEventIdentity
{
public:
	UINT_VERSION m_uVersionMembers;
	UINT_VERSION m_uVersionPicturePanoramic;

public:
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);

};

void
CEventGroupUpdate::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	Assert(pXmlNodeElement != NULL);

	}


#endif // XCP_H

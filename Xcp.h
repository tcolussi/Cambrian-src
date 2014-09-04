///////////////////////////////////////////////////////////////////////////////////////////////////
//	Xcp.h

#ifndef XCP_H
#define XCP_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#ifdef SUPPORT_XCP_VERSION_1
//	Class holding a the data for an event requiring a large amount of XML data to transmit.
class CDataXmlLargeEvent : public SListNode
{
public:
	TIMESTAMP m_tsEventID;			// Which event the stanza is associated
	CBin m_binXmlData;				// XML data to transmit.  This data is the caching of IEvent::XmlSerializeCore() and will be unserialized with XmlUnserializeCore().
	TIMESTAMP_MINUTES m_tsmLastAccessed;	// Remember when this stanza was last used (after about 15 minutes of inactivity, the object gets deleted because it is assumed the stanza is now idle)
	SHashSha1 m_hashXmlData;				// Reliable checksum of the data
};
#endif

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


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Helper object to synchronize a group

struct SGroupMemberOnline
{

};

/*
<I i='3U5Y/bI' o='3U5Y/c_' t='as a group member, I want to get ALL messages of others have been typing when I am away'/>
<I i='3U5Y\Ig' o='3U5Y\J3' t='when I become online,'/>
<I i='3U5Z18t' o='3U5Z19s' t='I query all group members how many messages I have been missing'/>
<I i='3U5Z2-l' o='3U5Z2_6' t='and I pick to synchronize with the &quot;most reliable&quot; sources'/>
<I i='3U5Z4U*' o='3U5Z4W\' t='in a nutshell, the group member with the most messages is likely to have most of them'/>
<I i='3U5Z5#S' o='3U5Z5@X' t='it is almost like the blockchain'/>
<I i='3U5Z70`' o='3U5Z71Y' t='where the longest chain is &quot;the one&quot;'/>
<I i='3U5Z8YM' o='3U5Z8*-' t='and I need to do this in the background'/>
<I i='3U5Za7j' o='3U5ZaaP' t='because I will be receiving messages from multiple sources'/>
<I i='3U5ZbF|' o='3U5ZbIi' t='what I plan to do is the following:'/>
<I i='3U5Zcq!' o='3U5ZcsP' t='1. Create a synchronization object'/>
<I i='3U5ZdBS' o='3U5ZdDZ' t='2. Query everyone online to get all their messages'/>
<I i='3U5Zgc\' o='3U5Zgd|' t='the most reliable way to get all YOUR messages is to ask YOU if I got all of them'/>
*/

//	This object is created when a group needs to be synchronized:
//	1. Build a list of every group member who is online.
//	2. Sync with everyone online.
//	3. Pick the 'most reliable' group member to download the remaining messages.
class CHelperGroupSynchronization
{
public:
	TGroup * m_pGroup;
	CVaultEvents * m_oVault;	// We need a vault to temporary store events while synchronizing

public:


};

#endif // XCP_H

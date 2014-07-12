///////////////////////////////////////////////////////////////////////////////////////////////////
//	GroupChat.h

#ifndef GROUPCHAT_H
#define GROUPCHAT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
class CEventGroupMemberJoin : public IEvent
{
	TContact * m_pMember;				// Which member joined
	TContact * m_pContactInvitedBy;		// Who invited the member to join the group
public:
	CEventGroupMemberJoin(const TIMESTAMP * ptsEventID);
	CEventGroupMemberJoin(TContact * pMember);
	virtual EEventClass EGetEventClass() const { return eEventClass_eGroupMemberJoins; }
	virtual void XmlSerializeCore(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Event containing group chat information.
//	This includes the name of the group, as well as the contacts who are members of the group.
class CEventGroupInfo : public IEvent
{
public:
	CEventGroupInfo();
	virtual EEventClass EGetEventClass() const { return eEventClass_eGroupInfo; }
	virtual void XmlSerializeCore(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
};

#endif // GROUPCHAT_H

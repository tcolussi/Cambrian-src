///////////////////////////////////////////////////////////////////////////////////////////////////
//	GroupChat.h

#ifndef GROUPCHAT_H
#define GROUPCHAT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

/*
///////////////////////////////////////////////////////////////////////////////////////////////////
class CEventGroupMemberJoin : public IEvent
{
	TGroupMember * m_pMember;
public:
	CEventGroupMemberJoin(TGroup * pGroupParentOwner, const TIMESTAMP * ptsEventID);
	CEventGroupMemberJoin(TGroupMember * pMember);
	virtual EEventClass EGetEventClass() const { return eEventClass_eGroupMemberJoins; }
	virtual void XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
};
*/
/*
class CEventGroupMessageReceived : public IEventMessageText
{
protected:
	TContact * m_pContactGroupSender;		// Contact who sent the message to the group
public:
	CEventGroupMessageReceived(TGroup * pGroupParentOwner, const TIMESTAMP * ptsEventID);
	virtual ~CEventGroupMessageReceived();
	virtual EEventClass EGetEventClass() const { return eEventClass_eGroupMessageTextReceived_class; }
	virtual EEventClass EGetEventClassForXCP(const TContact * pContactToSerializeFor) const;
	virtual void XmlSerializeCore(IOUT CBinXcpStanzaType * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
	virtual BOOL SystemTray_FDisplayEvent() const;
	virtual TContact ** PpGetContactGroupSender() { return &m_pContactGroupSender; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class CTaskGroupSendText : public ITask
{
public:
	CTaskGroupSendText(CEventGroupMessageSent * pEventParent, const OCursor * poCursorTextBlock);
	virtual ETaskCompletion EWriteDataToSocket();
};
*/

#endif // GROUPCHAT_H

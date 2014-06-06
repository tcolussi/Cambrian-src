///////////////////////////////////////////////////////////////////////////////////////////////////
//	GroupChat.cpp
//
//	File containing the events and tasks related to group chat.
//
//	Since group chat involves a lot of classes and has many lines of code, the modivation for having this file is to keep IEvent.cpp and SocketTasks.cpp for simple events and tasks.
//
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "GroupChat.h"

/*
///////////////////////////////////////////////////////////////////////////////////////////////////
CEventGroupMemberJoin::CEventGroupMemberJoin(TGroup * pGroupParentOwner, const TIMESTAMP * ptsEventID) : IEvent(pGroupParentOwner, ptsEventID)
	{
	Assert(pGroupParentOwner->EGetRuntimeClass() == RTI(TGroup));
	m_pMember = NULL;
	}

CEventGroupMemberJoin::CEventGroupMemberJoin(TGroupMember * pMember) : IEvent(pMember->m_pGroup, NULL)
	{
	Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
	m_pMember = pMember;
	}
*/

/*
CEventGroupMessageSent *
TGroup::Vault_PAllocateEventMessageToSendToGroup(const CStr & strMessage)
	{
	CEventGroupMessageSent * pEvent = new CEventGroupMessageSent(this, NULL);
	Vault_AddEventToVault(PA_CHILD pEvent);
	pEvent->m_strMessage = strMessage;
	return pEvent;
	}
*/

/*
///////////////////////////////////////////////////////////////////////////////////////////////////
#define d_chAttribute_strMemberJID		'j'

//	CEventGroupMemberJoin::IEvent::XmlSerializeCore()
void
CEventGroupMemberJoin::XmlSerializeCore(IOUT CBin * pbinXmlAttributes, const TContact *) const
	{
	if (m_pMember != NULL)
		pbinXmlAttributes->BinAppendXmlAttributeCStr(d_chAttribute_strMemberJID, m_pMember->m_pContact->m_strJidBare);
	}

//	CEventGroupMemberJoin::IEvent::XmlUnserializeCore()
void
CEventGroupMemberJoin::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	Assert(mu_parentowner.pGroup->EGetRuntimeClass() == RTI(TGroup));
	m_pMember = mu_parentowner.pGroup->Member_PFindOrAllocate_NZ(pXmlNodeElement->PszuFindAttributeValue(d_chAttribute_strMemberJID));
	}

//	CEventGroupMemberJoin::IEvent::ChatLogUpdateTextBlock()
void
CEventGroupMemberJoin::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	_BinHtmlInitWithTime(OUT &g_strScratchBufferStatusBar);
	if (m_pMember != NULL)
		g_strScratchBufferStatusBar.BinAppendTextSzv_VE("<b>$s</b> joined group", m_pMember->TreeItem_PszGetNameDisplay());
	poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, c_brushSilver);
	}
*/


//	Append an XML attribute representing the Contact Identifier to the binary object.
//	At the moment the Contact Identifier is the JID, however in the future it may be a hash of the public key.
//
//	SEE ALSO: Contact_PFindByIdentifier()
void
TContact::BinAppendXmlAttributeContactIdentifier(IOUT CBin * pbin, CHS chAttributeName) const
	{
	pbin->BinAppendXmlAttributeCStr(chAttributeName, m_strJidBare);
	}

//	Method complement to BinAppendXmlAttributeContactIdentifier().
//	There could be many identifiers.
//	TODO: Use a hash table to cache identifiers
TContact *
TAccountXmpp::Contact_PFindByIdentifier(PSZUC pszContactIdentifier) CONST_MCC
	{
	if (pszContactIdentifier != NULL)
		return Contacts_PFindContactByJID(pszContactIdentifier);
	return NULL;
	}


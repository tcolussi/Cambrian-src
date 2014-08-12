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


///////////////////////////////////////////////////////////////////////////////////////////////////
CEventGroupMemberJoin::CEventGroupMemberJoin(const TIMESTAMP * ptsEventID) : IEvent(ptsEventID)
	{
	m_pMember = NULL;
	m_pContactInvitedBy = NULL;
	}

CEventGroupMemberJoin::CEventGroupMemberJoin(TContact * pMember) : IEvent(NULL)
	{
	Assert(pMember->EGetRuntimeClass() == RTI(TContact));
	m_pMember = pMember;
	m_pContactInvitedBy = NULL;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
#define d_chAttribute_strMemberJID		'm'

//	CEventGroupMemberJoin::IEvent::XmlSerializeCoreE()
EXml
CEventGroupMemberJoin::XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const
	{
	pbinXmlAttributes->BinAppendXmlAttributeOfContactIdentifier(d_chAttribute_strMemberJID, m_pMember);
	return eXml_zAttributesOnly;
	}

//	CEventGroupMemberJoin::IEvent::XmlUnserializeCore()
void
CEventGroupMemberJoin::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	_XmlUnserializeAttributeOfContactIdentifier(d_chAttribute_strMemberJID, OUT &m_pMember, pXmlNodeElement);
	}

//	CEventGroupMemberJoin::IEvent::ChatLogUpdateTextBlock()
void
CEventGroupMemberJoin::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	_BinHtmlInitWithTime(OUT &g_strScratchBufferStatusBar);
	if (m_pMember != NULL)
		g_strScratchBufferStatusBar.BinAppendTextSzv_VE("invited <b>$s</b> to join the group", m_pMember->TreeItem_PszGetNameDisplay());
	poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, c_brushSilver);
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	IMPLEMENTATION NOTES
//	At the moment the Contact Identifier is the JID, however in the future it may be a hash of the contact's public key.
//
//	PERFORMANCE NOTES
//	Considering a single contact may have multiple identifier, such as having multiple JIDs, it will be wise in the future to use a hash table to cache the identifiers.
TContact *
TAccountXmpp::Contact_PFindByIdentifierOrCreate_YZ(const CXmlNode * pXmlNodeEvent, CHS chAttributeName, INOUT CBinXcpStanza * pbinXcpApiExtraRequest) CONST_MCC
	{
	PSZUC pszContactIdentifier = pXmlNodeEvent->PszuFindAttributeValue(chAttributeName);
	if (pszContactIdentifier != NULL)
		{
		Assert(pszContactIdentifier[0] != '\0');
		TContact * pContact = Contact_PFindByJID(pszContactIdentifier, eFindContact_zDefault);
		if (pContact != NULL)
			return pContact;
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to find peer identifier '$s' from XML node: ^N\n", pszContactIdentifier, pXmlNodeEvent);
		// Create a new contact
		if (pbinXcpApiExtraRequest != NULL)
			{

			}
		return TreeItemAccount_PContactAllocateNewToNavigationTree_NZ(IN pszContactIdentifier);
		}
	return NULL;
	}

//	Append to the blob an XML attribute representing the Contact Identifier.
//	This method is used to serialize a pointer to a contact when saving to vault of event.
//
//	IMPLEMENTATION NOTES
//	At the moment the Contact Identifier is the JID, however in the future it may be a hash of the contact's public key.
//
//	SEE ALSO: Contact_PFindByIdentifierOrCreate_YZ(), CBin ^i
void
CBin::BinAppendXmlAttributeOfContactIdentifier(CHS chAttributeName, const TContact * pContact)
	{
	if (pContact != NULL)
		{
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		BinAppendXmlAttributeCStr(chAttributeName, pContact->m_strJidBare);
		}
	}

TContact *
TAccountXmpp::Contact_PFindByIdentifierGroupSender_YZ(const CXmlNode * pXmlNodeEvent) CONST_MCC
	{
	return Contact_PFindByIdentifierOrCreate_YZ(pXmlNodeEvent, d_chXCPa_pContactGroupSender, NULL);
	}

void
IEvent::_XmlUnserializeAttributeOfContactIdentifier(CHS chAttributeName, OUT TContact ** ppContact, const CXmlNode * pXmlNodeElement) const
	{
	Assert(ppContact != NULL);
	Assert(pXmlNodeElement != NULL);
	Assert(m_pVaultParent_NZ != NULL);
	Assert(m_pVaultParent_NZ->m_pParent->m_pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	*ppContact = m_pVaultParent_NZ->m_pParent->m_pAccount->Contact_PFindByIdentifierOrCreate_YZ(pXmlNodeElement, chAttributeName, NULL);
	}

/*
void
TContact::BinAppendXmlAttributeOfContactIdentifier(IOUT CBin * pbin, CHS chAttributeName) const
	{
	pbin->BinAppendXmlAttributeCStr(chAttributeName, m_strJidBare);
	}
*/

void
CBinXcpStanza::BinXmlAppendAttributeOfContactIdentifierOfGroupSenderForEvent(const IEvent * pEvent)
	{
	AssertValidEvent(pEvent);
	Endorse(m_pContact == NULL);	// Saving to disk
	/*
	TContact * pContact = pEvent->m_pContactGroupSender_YZ;
	if (pContact != NULL && pContact != m_pContact)
		{
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		pContact->BinAppendXmlAttributeOfContactIdentifier(INOUT this, d_chXCPa_pContactGroupSender);
		}
	*/
	if (pEvent->m_pContactGroupSender_YZ != m_pContact)
		{
		Endorse(pEvent->m_pContactGroupSender_YZ == NULL);	// The event was sent, or was received on a 1-to-1 conversation.  If this pointer is NULL, then the method BinAppendXmlAttributeOfContactIdentifier() will ignore it
		BinAppendXmlAttributeOfContactIdentifier(d_chXCPa_pContactGroupSender, pEvent->m_pContactGroupSender_YZ);
		}
	}

void
CBinXcpStanza::BinXmlAppendAttributeUIntHexadecimalExcludeForXcp(CHS chAttributeName, UINT uAttributeValueHexadecimal, UINT kmFlagsExcludeForXcp)
	{
	if (FSerializingEventForXcp())
		uAttributeValueHexadecimal &= ~kmFlagsExcludeForXcp;	// Exclude some flags
	BinAppendXmlAttributeUIntHexadecimal(chAttributeName, uAttributeValueHexadecimal);
	}

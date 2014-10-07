///////////////////////////////////////////////////////////////////////////////////////////////////
//	WLayoutChatLog.cpp
//
//	Widget to display the chat history when a contact is selected.
//
//	This class could have been named WChatMessages, WChatMessageContent or WLayoutDetails,
//	however the name WLayoutChatLog was chosen for naming orthogonality: when a TContact is selected in the navigation tree, a WLayoutChatLog is selected as the layout.
//
//	See also WLayoutContactGroup when a chat occurs between multiple parties.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WFindText.h"
#include "DialogInvitations.h"

WLayoutChatLog *
ITreeItemChatLogEvents::ChatLog_PwGetLayout_NZ() CONST_MCC
	{
	if (m_uFlagsTreeItem & FTI_kfChatLogEvents_RepopulateAll)
		{
		m_uFlagsTreeItem &= ~FTI_kfChatLogEvents_RepopulateAll;
		if (m_pawLayoutChatLog != NULL)
			{
			m_pawLayoutChatLog->ChatLog_EventsRepopulateUpdateUI();
			return m_pawLayoutChatLog;
			}
		}
	if (m_pawLayoutChatLog == NULL)
		m_pawLayoutChatLog = new WLayoutChatLog(this);
	return m_pawLayoutChatLog;
	}

//	ITreeItemChatLogEvents::ITreeItem::TreeItem_GotFocus()
void
ITreeItemChatLogEvents::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayout(ChatLog_PwGetLayout_NZ());
	TreeItem_IconUpdateOnMessagesRead();
	}

void
TContact::Vault_GetEventsForChatLog(OUT CArrayPtrEvents * parraypEventsChatLog) CONST_MCC
	{
	Vault_PGet_NZ()->GetEventsForChatLog(OUT parraypEventsChatLog);
	if (m_uFlagsTreeItem & FTI_kfChatLogEvents_IncludeFromWallet)
		{
		parraypEventsChatLog->Wallets_AppendEventsTransactionsFor(this, eWalletViewFlag_kmDisplayTransactionsAll);
		parraypEventsChatLog->SortEventsByIDs();	// Merging the transactions requires a sort to make sure the events are displayed in chronological order
		}
	}

void
CArrayPtrContacts::RemoveAllContactsComposingWhoAreIdle()
	{
	if (m_paArrayHdr == NULL)
		return;
	TContact ** ppContactStop;
	TContact ** ppContactStart = PrgpGetContactsStop(OUT &ppContactStop);
	TContact ** ppContactDst = ppContactStart;
	TContact ** ppContactSrc = ppContactStart;
	while (ppContactSrc != ppContactStop)
		{
		TContact * pContact = *ppContactSrc++;
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		MessageLog_AppendTextFormatSev(eSeverityNoise, "\t[$@] $S ($s) is still typing for $i minutes...\n", &pContact->m_strJidBare, pContact->ChatLog_PszGetNickname(), g_tsmMinutesSinceApplicationStarted - pContact->m_tsmLastStanzaReceived);
		if (g_tsmMinutesSinceApplicationStarted - pContact->m_tsmLastStanzaReceived >= 4)
			pContact->ChatLogContact_ChatStateIconUpdateComposingStopped();	// If the user has not typed anything during the past 4 minutes, then assume he stopped typing.
		else
			*ppContactDst++ = pContact;
		}
	m_paArrayHdr->cElements = ppContactDst - ppContactStart;
	} // RemoveAllContactsComposingWhoAreIdle()

void
ITreeItemChatLogEvents::ChatLog_ScrollToLastMessage()
	{
	ChatLog_PwGetLayout_NZ()->ChatLog_ScrollToDisplayLastMessage();
	}

void
ITreeItemChatLogEvents::ChatLog_FindText()
	{
	ChatLog_PwGetLayout_NZ()->WidgetFindText_Show();
	}

WChatLog *
ITreeItemChatLogEvents::ChatLog_PwGet_YZ() const
	{
	return (m_pawLayoutChatLog != NULL) ? m_pawLayoutChatLog->m_pwChatLog_NZ : NULL;
	}

void
ITreeItemChatLogEvents::ChatLog_ChatStateIconUpdateComposingStopped(INOUT TContact * pContact)
	{
	if (m_pawLayoutChatLog != NULL)
		m_pawLayoutChatLog->m_pwChatLog_NZ->ChatLog_ChatStateTextUpdate(INOUT pContact, eChatState_Paused);
	}

void
ITreeItemChatLogEvents::ChatLog_ChatStateIconUpdate(EChatState eChatState, INOUT TContact * pContact)
	{
	Assert(pContact != NULL);
	if (m_pawLayoutChatLog != NULL)
		m_pawLayoutChatLog->m_pwChatLog_NZ->ChatLog_ChatStateTextUpdate(INOUT pContact, eChatState);
	else
		{
		// Make sure even though there is no Layout, the icons are displayed nevertheless in the Navigation Tree
		if (eChatState == eChatState_zComposing)
			pContact->TreeItemContact_UpdateIconComposingStarted(this);
		else
			pContact->TreeItemContact_UpdateIconComposingStopped(this);
		}
	}

void
TContact::TreeItemContact_UpdateIconComposingStarted(ITreeItemChatLogEvents * pContactOrGroup)
	{
	Assert(pContactOrGroup != NULL);
	m_pAccount->m_arraypContactsComposing.AddUniqueF(this);

	if (pContactOrGroup->EGetRuntimeClass() == RTI(TGroup))
		{
		((TGroup *)pContactOrGroup)->Member_PFindOrAddContact_NZ(this)->TreeItemGroupMember_SetIconComposingStarted();
		/*
		pContactOrGroup->TreeItemW_SetIcon(eMenuIconPencil_16x16);
		// Find the member to set its icon
		TGroupMember * pMember = ((TGroup *)pContactOrGroup)->Member_PFindOrAddContact_NZ(this);
		pMember->TreeItemW_SetIcon(eMenuIconPencil_10x10);
		*/
		}
	else
		{
		Assert(pContactOrGroup->EGetRuntimeClass() == RTI(TContact));
		Assert(pContactOrGroup == this);
		TreeItemW_SetIcon(eMenuIconPencil_10x10);	// Use a smaller icon for a contact
		}

	/*
	// Update the icon for every alias
	IContactAlias * pAlias = m_plistAliases;
	while (pAlias != NULL)
		{
		pAlias->TreeItem_SetIcon(eMenuIconPencil);	// This line needs to be fixed to handle special cases
		pAlias = pAlias->m_pNextAlias;
		}
	*/
	}

void
TContact::TreeItemContact_UpdateIconComposingStopped(ITreeItemChatLogEvents * pContactOrGroup)
	{
	// Do not remove the contact from m_pAccount->m_arraypContactsComposing
	if (pContactOrGroup->EGetRuntimeClass() == RTI(TGroup))
		{
		// Find the member
		TGroupMember * pMember = ((TGroup *)pContactOrGroup)->Member_PFindOrAddContact_NZ(this);
		//pMember->TreeItem_IconUpdate();
		pMember->TreeItemGroupMember_SetIconComposingStopped();
		}
	pContactOrGroup->TreeItem_IconUpdate();
	}

void
TContact::ChatLogContact_ChatStateIconUpdate(EChatState eChatState)
	{
	ChatLog_ChatStateIconUpdate(eChatState, INOUT this);
	}


//	Remove "x is typing" in the Chat Log and update the contact's icon
void
TContact::ChatLogContact_ChatStateIconUpdateComposingStopped()
	{
	ChatLogContact_ChatStateIconUpdate(eChatState_Paused);
	}

void
TContact::ChatLogContact_RemoveInvitationMessage()
	{
	Assert(m_uFlagsContact & FC_kfContactNeedsInvitation);
	m_uFlagsContact &= ~FC_kfContactNeedsInvitation;
	if (m_pawLayoutChatLog != NULL)
		m_pawLayoutChatLog->m_pwChatLog_NZ->ChatLog_ChatStateTextRefresh();	// The invitation message is appended with the 'Chat State'
	}


//	Display the stanza message to the GUI
void
TContact::ChatLogContact_DisplayStanzaToUI(const CXmlNode * pXmlNodeMessageStanza)
	{
	Assert(pXmlNodeMessageStanza != NULL);
	Assert(m_pAccount != NULL);
	Endorse(m_paTreeItemW_YZ == NULL);
	m_tsmLastStanzaReceived = g_tsmMinutesSinceApplicationStarted;

	#if 0
	if (m_uFlagsContact & (FC_kfContactNeedsInvitation | FC_kfCommentContainsInvitation | FC_kfContactUnsolicited))
		{
		// If we receive a stanza from a contact, it means there is no need to to send an invitation because the stanza response is the proof the contact was established
		if (m_uFlagsContact & FC_kfContactNeedsInvitation)
			{
			m_uFlagsContact &= ~FC_kfContactNeedsInvitation;
			NoticeListAuxiliary_DeleteAllNoticesRelatedToTreeItem(this);
			}
		if (m_uFlagsContact & (FC_kfCommentContainsInvitation | FC_kfContactUnsolicited))
			{
			// Compare if the message is the handshake of the invitation
			PSZUC pszMessageBody = pXmlNodeMessageStanza->PszuFindElementValue_NZ(c_sza_body);	// The body may contain the invitation message
			if (m_uFlagsContact & FC_kfCommentContainsInvitation)
				{
				if (m_strComment.FCompareStringsExactCase(pszMessageBody))
					{
					m_strComment.Empty();	// Remove the invitation
					m_uFlagsContact &= ~FC_kfCommentContainsInvitation;
					TreeItemContact_UpdateIcon();
					return;	// Don't display the message because it is the invitation encoded in Base64
					}
				}
			if (m_uFlagsContact & FC_kfContactUnsolicited)
				{
				// We have an 'unsolicited' contact, however this may be to an incomplete handshake
				/*
				TContact * pContactInvitation = m_pAccount->PFindContactByComment(pszMessageBody);
				if (pContactInvitation != NULL && pContactInvitation != this)
					{
					m_uFlagsContact &= ~(FC_kfContactUnsolicited | FC_kfCommentContainsInvitation);
					BOOL fSelectContact = (NavigationTree_PGetSelectedTreeItem() == pContactInvitation);
					m_pAccount->Contact_DeleteSafely(PA_DELETING pContactInvitation);
					TreeItemContact_UpdateIcon();
					if (fSelectContact)
						TreeItem_SelectWithinNavigationTree();
					return;
					}
				*/
				} // if
			}
		} // if
	#endif

	PSZUC pszuMessageBody = ChatLog_PwGetLayout_NZ()->ChatLog_DisplayStanzaToUser(pXmlNodeMessageStanza);
	TreeItemChatLog_IconUpdateOnNewMessageArrivedFromContact(pszuMessageBody, this);
	} // ChatLogContact_DisplayStanzaToUI()


void
ITreeItemChatLogEvents::TreeItemChatLog_IconUpdateOnNewMessageArrivedFromContact(PSZUC pszMessage, TContact * pContact, TGroupMember * pMember)
	{
	Assert(pContact != NULL);
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	if (pszMessage == NULL)
		return;
	Assert(pszMessage[0] != '\0');
	if (m_pawLayoutChatLog == NULL || !m_pawLayoutChatLog->FGotFocus())
		{
		m_cMessagesUnread++;				// The layout does not have the focus (or does not exist), so display a special icon to indicate a new message arrived from the contact
		if (pMember != NULL)
			{
			Assert(pMember->m_pGroup == this);
			Assert(pMember->m_pContact == pContact);
			pMember->TreeItemW_SetTextToDisplayMessagesUnread(++pMember->m_cMessagesUnread);	// The group member has unread messages as well as its parent group
			}
		}
	TreeItemChatLog_UpdateTextAndIcon();	// Always update the text and icon when a new message arrives.  This is important because before a message arrive, there is usually the 'composing' icon, and after the message arrives, this icon must be changed by either the online presence, or an icon indicating there is a new unread message.
	if (!m_pAccount->TreeItemW_FIsExpanded())
		{
		// The account node is collapsed, therefore the contact is not visible in the GUI.
		// To let know the user there is a new message, the GUI displays the 'chat icon' to Tree Item of the account.
		m_pAccount->IconUpdate_MessageArrivedFromContact(pContact);
		}
	MainWindow_SystemTrayNewMessageArrived(pContact, pszMessage);
	}

void
ITreeItemChatLogEvents::TreeItemChatLog_IconUpdateOnMessagesReadForContact(TContact * pContact)
	{
	Assert(pContact != NULL);
	if (m_cMessagesUnread > 0)
		{
		m_cMessagesUnread = 0;
		TreeItemChatLog_UpdateTextAndIcon();
		m_pAccount->IconUpdate_MessagesReadForContact(pContact);		// Notify the parent account regarding the user the message(s) of the contact
		}
	}

//	TContact::ITreeItem::TreeItem_IconUpdateOnMessagesRead()
//
//	Change the icon to indicate the message has been read
void
TContact::TreeItem_IconUpdateOnMessagesRead()
	{
	TreeItemChatLog_IconUpdateOnMessagesReadForContact(this);
	/*
	if (m_cMessagesUnread > 0)
		{
		m_cMessagesUnread = 0;
		TreeItemContact_UpdateTextAndIcon();
		m_pAccount->IconUpdate_MessagesReadForContact(this);		// Notify the parent account the user read the message
		if (g_arraypContactsRecentMessages.RemoveElementI(this) >= 0)
			{
			MessageLog_AppendTextFormatSev(eSeverityComment, "TContact::TreeItem_IconUpdateOnMessagesRead($S) - Destroying System Tray Icon...\n", &m_strJidBare);
			SystemTray_Destroy();
			}
		}
	*/
	Dashboard_UpdateContact(this);
	}

//	TGroup::ITreeItem::TreeItem_IconUpdateOnMessagesRead()
void
TGroup::TreeItem_IconUpdateOnMessagesRead()
	{
	if (m_cMessagesUnread <= 0)
		return;	// There are no unread messsages
	m_cMessagesUnread = 0;
	// Loop through every member and remove the count
	TGroupMember ** ppMemberStop;
	TGroupMember ** ppMember = m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
	while (ppMember != ppMemberStop)
		{
		TGroupMember * pMember = *ppMember++;
		Assert(pMember != NULL);
		Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
		if (pMember->m_cMessagesUnread > 0)
			{
			pMember->m_cMessagesUnread = 0;
			pMember->TreeItemW_UpdateText();
			pMember->TreeItem_IconUpdate();
			MainWindow_SystemTrayMessageReadForContact(pMember->m_pContact);
			}
		}
	TreeItemChatLog_UpdateTextAndIcon();
	Dashboard_UpdateGroup(this);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

#define SL_INoticeUnsolicited(_pfmName)					SL_INotice(_pfmName, WNoticeContactUnsolicited)

WNoticeContactUnsolicited::WNoticeContactUnsolicited(TContact * pContact) : INoticeWithIcon("Approve Peer?", eMenuIconQuestion)
	{
	Assert(pContact != NULL);
//	NoticeListAuxiliary_AddNotice(this);
	mu_sponsor.pContact = pContact;	// This line is necessary for the notice to delete itself if the contact is deleted

	NoticeMessage_SetText_VE("You received an unsolicited message from <b>{C/}</b>:<br/>If <b>$s</b> is a peer you wish to keep, please approve him/her to your peers list.", &pContact->m_strJidBare, pContact->TreeItem_PszGetNameDisplay());
	Notice_AddButton(PA_CHILD new WButtonTextWithIcon("Approve|Add the peer to your list of approved peers", eMenuAction_ContactApprove), SL_INoticeUnsolicited(SL_ButtonContactApprove_clicked));
	}

void
WNoticeContactUnsolicited::SL_ButtonContactApprove_clicked()
	{
	mu_sponsor.pContact->TreeItem_EDoMenuAction(eMenuAction_ContactApprove);
	NoticeListAuxiliary_DeleteNotice(PA_DELETING this);
	}

void
WNoticeContactUnsolicited::SL_ButtonContactReject_clicked()
	{

	}


///////////////////////////////////////////////////////////////////////////////////////////////////
#define SL_WNoticeContactInvite(_pfmName)			SL_INotice(_pfmName, WNoticeContactInvite)

WNoticeContactInvite::WNoticeContactInvite(TContact * pContact) : INoticeWithIcon("Invite Peer?", eMenuIconQuestion)
	{
	Assert(pContact != NULL);
	mu_sponsor.pContact = pContact;	// This line is necessary for the notice to delete itself if the contact is deleted
	TAccountXmpp * pAccount = pContact->m_pAccount;
	PSZUC pszNickNameContact = pContact->ChatLog_PszGetNickname();
	setMaximumHeight(300);

	CStr strInvitationXml;
	strInvitationXml.Format("<i u='{C@}' f='^S' p='$i' h='{B/}'/>", &pContact->m_strJidBare, &pAccount->m_strJID, pAccount->m_uServerPort, pAccount->Certificate_PGetBinaryFingerprint());
//	pContact->Invitation_InitFromXml(strInvitationXml);

	NoticeMessage_SetText_VE("Until now, you never received any message from <b>^S</b>.  The text below is a <i>canned invitation</i> you may wish to send <b>^s</b> by email or via other messaging system.", &pContact->m_strJidBare, pszNickNameContact);
	Notice_AddButton(PA_CHILD new WButtonTextWithIcon("Copy|Copy the invitation text below into the clipboard", eMenuAction_Copy), SL_WNoticeContactInvite(SL_ButtonCopy_clicked));

	CStr strInvitation;
	strInvitation.Format("Hi $s,\nThis is an invitation to create a chat account to communicate with $S.\n\n"
	"If you do not have SocietyPro installed, please click on this download link:\n"
	d_szUrlBaseDowloadInvitation "?Invitation={S/}\n\n"
	"Otherwise, just copy & paste the invitation below into SocietyPro:\n\n" d_szInvitationPrefix "{S/}" d_szInvitationPostfix "\n", pszNickNameContact, &pAccount->m_strJID, &strInvitationXml, &strInvitationXml);
	m_pwEditInvitation = new WEditTextArea(strInvitation);

	PSZAC pszaLabelAndToolTip = (PSZAC)strInvitation.Format("Suggested Invitation to Send:|"
		"<html>Copy and paste this invitation and send it to the person you wish to invite.<br/></br/>"
		"You may use email or other messaging system to send your invitation.<br/><br/>"
		"The content of<br/><br/>{S/}<br/><br/>is<br/><br/>^S<br/><br/>encoded in Base64.  Likewise h='{B/}' is the certificate fingerprint encoded in Base64.</html>", &strInvitationXml, &strInvitationXml, pAccount->Certificate_PGetBinaryFingerprint());
	//m_pwEditInvitationBase64->setMaximumHeight(100);
	m_poLayoutNotice->Layout_PoAddRowLabelsAndWidgets_VEZA(pszaLabelAndToolTip, m_pwEditInvitation, NULL);
	/*
	WEdit * pwEditEmail = new WEdit;
	m_poLayoutBody->Layout_AddLabelAndWidgetH_PA("Email Address:|Enter the email address of the peer you wish to send the invitation.", pwEditEmail);
	WButtonTextWithIcon * m_pButtonConnectNow = new WButtonTextWithIcon("Send Email", eMenuAction_ContactInvite);
	m_poLayoutBody->Layout_PoAddLayoutVerticalWithWidgets_VEZA(m_pButtonConnectNow, NULL);
	Notice_LayoutBodyMoveToBottom(); // Move the fields: Edit Email and the Send button at the bottom.  This is done by removing the layout, and adding it at the bottom
	*/
	}

BOOL
WNoticeContactInvite::FDeleteDuplicateNotice(INOUT INotice * piwNoticeNew)
	{
	Assert(piwNoticeNew->EGetRuntimeClass() == RTI(WNoticeContactInvite));
	return (mu_sponsor.piTreeItem == piwNoticeNew->mu_sponsor.piTreeItem);
	}

void
WNoticeContactInvite::SL_ButtonCopy_clicked()
	{
	CInvitation * paInvitation = Invitation_PazAllocateFromEdit(m_pwEditInvitation);
	if (paInvitation == NULL)
		{
		EMessageBoxWarning("The invitation you are about to copy in the clipboard is not valid, and therefore the peer <b>^s</b> will not be able to use it.", mu_sponsor.pContact->ChatLog_PszGetNickname());
		m_pwEditInvitation->setFocus();
		return;
		}
	paInvitation->m_strInvitationText.CopyStringToClipboard();	// Copy the invitation to the clipboard
	}

void
WLayoutChatLog::WidgetContactInvitation_Show()
	{
	if (m_pContactParent_YZ != NULL)
		Layout_NoticeAuxiliaryAdd(PA_DELETING new WNoticeContactInvite(m_pContactParent_YZ));
	}

void
WLayoutChatLog::Socket_WriteXmlChatState(EChatState eChatState) const
	{
	m_pContactOrGroup_NZ->Xmpp_WriteXmlChatState(eChatState);
	}

/*
void
WLayoutChatLog::TreeItem_UpdateIconMessageRead()
	{
	PGetContactOrGroup_NZ()->TreeItem_IconUpdateOnMessagesRead();
	}
*/

/*
class WWidgetMessageInputLayout : public QWidget
{
public:
	WWidgetMessageInputLayout(QWidget * pwParent) : QWidget(pwParent) { }
	virtual QSize minimumSizeHint() const
		{
		return QSize(0, fontMetrics().height() * 3 / 2);
		}
	virtual QSize sizeHint() const
		{
		return minimumSizeHint();
		}
};
*/

WLayoutChatLog::WLayoutChatLog(ITreeItemChatLogEvents * pContactOrGroupParent)
	{
	Assert(pContactOrGroupParent != NULL);
	m_pContactOrGroup_NZ = pContactOrGroupParent;
	m_pContactParent_YZ = (pContactOrGroupParent->EGetRuntimeClass() == RTI(TContact)) ? (TContact *)pContactOrGroupParent : NULL;
	m_pwFindText = NULL;
	m_pwChatLog_NZ = new WChatLog(this, pContactOrGroupParent);
	setStretchFactor(0, 5);
	/*
	if (pContactParent_YZ != NULL)
		{
		if (pContactParent_YZ->m_uFlagsContact & TContact::FC_kfContactNeedsInvitation)
			WidgetContactInvitation_Show();
		if (pContactParent_YZ->m_uFlagsContact & TContact::FC_kfContactUnsolicited)
			Layout_NoticeAuxiliaryAdd(PA_DELETING new WNoticeContactUnsolicited(pContactParent_YZ));
		}
	*/

	m_pwChatInput = new WChatInput(this);

	QWidget * pWidget = new QWidget(this);
//	pWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	OLayoutHorizontal * pLayoutMessageInput = new OLayoutHorizontal(pWidget);
	pLayoutMessageInput->setContentsMargins(0, 0, 5, 0);
	pLayoutMessageInput->addWidget(m_pwChatInput);
	OLayoutVertical * pLayoutButtons = new OLayoutVerticalAlignTop(pLayoutMessageInput);
	pLayoutButtons->setSpacing(0);
	m_pwButtonSendBitcoin = new WButtonIconForToolbar(eMenuIconBitcoin, "Send Bitcoins");
	pLayoutButtons->addWidget(m_pwButtonSendBitcoin);
	connect(m_pwButtonSendBitcoin, SIGNAL(clicked()), this, SLOT(SL_ButtonSendBitcoin()));

	WButtonIconForToolbar * pwButton = new WButtonIconForToolbar(eMenuAction_ContactSendFile, "Send File\n\nYou may drag and drop the file, or copy & paste the file from Windows Explorer");
	pLayoutButtons->addWidget(pwButton);
	connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonSendFile()));

	pwButton = new WButtonIconForToolbar(eMenuAction_ContactAdd, "Add people to the converstation");
	pLayoutButtons->addWidget(pwButton);
	connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonAddContacts()));

	pwButton = new WButtonIconForToolbar(eMenuAction_BallotSend, "Send a ballot to the group to vote");
	pLayoutButtons->addWidget(pwButton);
	connect(pwButton, SIGNAL(clicked()), this, SLOT(SL_ButtonSendBallot()));

	setChildrenCollapsible(false);		// Do not allow the widget WChatInput() to collapse; it would be very confusing to the user

//	addWidget(PA_CHILD m_pwChatLog);
	//addWidget(PA_CHILD new WFindText(m_pwChatLog->document()));
//	addWidget(PA_CHILD m_pwChatInput);
	setAcceptDrops(true);
	}
/*
void
WLayoutChatLog::OnEventFocusIn()
	{
	// When the focus goes to the layout (the Chat Log or Message Input), assume the messages have been read
	TreeItem_UpdateIconMessageRead();
	//m_pContact->TreeItemChatLog_UpdateIconMessageRead();
	}
*/

BOOL
WLayoutChatLog::FGotFocus()
	{
	if (hasFocus())
		{
		MessageLog_AppendTextFormatSev(eSeverityNoise, "WLayoutChatLog has the focus\n");
		return TRUE;
		}
	return m_pwChatInput->hasFocus() || m_pwChatLog_NZ->hasFocus();
	}

#pragma GCC diagnostic ignored "-Wswitch"

//	Return TRUE if the contact never received any message (which means the contact may need an invitation)
//	Return FALSE if there is at least one message received (which means both parties have communicated successfully)
BOOL
TContact::Contact_FIsInvitationRecommended()
	{
	if ((m_uFlagsContact & FC_kfContactNeedsInvitation) == 0)
		return FALSE;
	if (m_paVaultEvents != NULL)
		{
		// Search the entire Chat Log to find any message received.  Of course this is not the most optimal code, however the Chat Log should be rather short if there has been no communication between the two parties.
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_paVaultEvents->m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		while (ppEvent != ppEventStop)
			{
			IEvent * pEvent = *--ppEventStop;
			AssertValidEvent(pEvent);
			EEventClass eEventClass = pEvent->EGetEventClass();
			if ((eEventClass == eEventClass_eMessageTextSent && pEvent->Event_FHasCompleted()) ||	// If the message was sent successfully (with a confirmation receipt), then there is no need to send an invitation because the remote contact has the JID of the account
				(eEventClass & eEventClass_kfReceivedByRemoteClient))								// Anything received by the remote client means there has been a communication between the two contacts, and therefore there is no need to send an invitaiton
				{
				m_uFlagsContact &= ~FC_kfContactNeedsInvitation;	// Remove the invitation flag
				return FALSE;
				}
			} // while
		} // if
	return TRUE;
	}

void
TContact::Vault_XmppAllocateEventMessageReceivedAndDisplayToChatLog(const CXmlNode * pXmlNodeMessageStanza, PSZUC pszuMessageBody, WChatLog * pwChatLog)
	{
	Assert(pXmlNodeMessageStanza != NULL);
	Assert(pszuMessageBody != NULL);
	Assert(pwChatLog != NULL);
	// First thing is to determine if the message is a duplicate/update of another message
	TIMESTAMP tsMessageWritten = pXmlNodeMessageStanza->LFindAttributeValueIdTimestamp_ZZR();	// Get the timestamp of when the message was written
	if (tsMessageWritten > 0)
		{
		CEventMessageTextReceived * pEvent = Vault_PGet_NZ()->m_arraypaEvents.PFindEventMessageReceivedByTimestamp(tsMessageWritten);
		if (pEvent != NULL)
			{
			if (pEvent->m_strMessageText.FCompareStringsExactCase(pszuMessageBody))
				{
				// The event contains the exact same text as before, so assume it is a duplicate
				MessageLog_AppendTextFormatSev(eSeverityComment, "Ignoring duplicate message ID '$t': $s\n", tsMessageWritten, pszuMessageBody);
				return;
				}
			// The message is not a duplicate, so update it in the GUI only if NOT older than 10 minutes, otherwise display it as a new message
			TIMESTAMP_DELTA dtsDelaySinceMessageWasWritten = Timestamp_GetCurrentDateTime() - pEvent->m_tsEventID;
			Assert(dtsDelaySinceMessageWasWritten > 0);
			if (dtsDelaySinceMessageWasWritten < d_ts_cMinutes * 10)
				{
				MessageLog_AppendTextFormatSev(eSeverityComment, "Updating message ID '$t' which was written $L seconds ago.\n", pEvent->m_tsEventID, dtsDelaySinceMessageWasWritten / d_ts_cSeconds);
				pEvent->MessageUpdated(pszuMessageBody, INOUT pwChatLog);
				return;
				}
			MessageLog_AppendTextFormatSev(eSeverityWarning, "Ignoring the update of message ID '$t' because its timestamp is too old (the message was written $L minutes ago).\n", pEvent->m_tsEventID, dtsDelaySinceMessageWasWritten / d_ts_cMinutes);
			} // if (event)
		}
	CEventMessageTextReceived * pEvent = new CEventMessageTextReceived(NULL);
	if (tsMessageWritten != d_ts_zNULL)
		pEvent->m_tsOther = tsMessageWritten;
	else
		pEvent->m_tsOther = pEvent->m_tsEventID;	// Use the same timestamp
	pEvent->m_strMessageText = pszuMessageBody;
	const CXmlNode * pXmlNodeHtml = pXmlNodeMessageStanza->PFindElement("html");
	if (pXmlNodeHtml != NULL)
		{
		const CXmlNode * pXmlNodeBody = pXmlNodeHtml->PFindElement(c_sza_body);
		if (pXmlNodeBody != NULL)
			{
			// At the moment, the only element we are supporting is <p>, however if we discover other HTML formats, we will suppor them
			CXmlNode * pXmlNodeP = pXmlNodeBody->PFindElement("p");
			if (pXmlNodeP != NULL)
				{
				// At this point the HTML message has been parsed into XML nodes, so we have to serialize the nodes to get an HTML string.
				pXmlNodeP->m_pszuTagName = (PSZU)c_szuEmpty;	// Remove the name so the <p> </p> will not be serialized
				pEvent->m_strMessageText.Empty();
				pXmlNodeP->SerializeToBinUtf8(OUT &pEvent->m_strMessageText, CXmlNode::STBF_kfCRLF);
				pEvent->m_strMessageText.BinAppendNullTerminatorSz();
				pEvent->m_uFlagsMessage |= CEventMessageTextReceived::FM_kfMessageHtml;
				MessageLog_AppendTextFormatCo(d_coGray, "Html Message: $S\n", &pEvent->m_strMessageText);
				}
			}
		}
	//Vault_AddEventToChatLogAndSendToContacts(PA_CHILD pEvent);
	//pEvent->EventAddToVault(PA_PARENT Vault_PGet_NZ());
	Vault_PGet_NZ()->EventAdd(PA_CHILD pEvent);
	pwChatLog->ChatLog_EventDisplay(IN pEvent);
	} // Event_AllocateEventMessageReceivedAndDisplayToChatLog()

/*
CEventFileReceived *
TContact::Vault_PAllocateEventFileReceived(const CXmlNode * pXmlNodeStreamInitiation)
	{
	Assert(pXmlNodeStreamInitiation != NULL);
	CEventFileReceived * pEvent = new CEventFileReceived(this, pXmlNodeStreamInitiation);
	Vault_AddEventToVault(PA_CHILD pEvent);
	ChatLog_DisplayNewEvent(pEvent);
	return pEvent;
	}
*/

//	Display the content of the stanza to the local user.
//	Return pointer to the message body which was added to the chat history.
//	Return NULL if the message body was empty.
PSZUC
WLayoutChatLog::ChatLog_DisplayStanzaToUser(const CXmlNode * pXmlNodeMessageStanza)
	{
	Assert(pXmlNodeMessageStanza != NULL);
	if (m_pContactParent_YZ == NULL)
		return NULL;
	PSZUC pszuMessageBody = pXmlNodeMessageStanza->PszuFindElementValue_ZZ(c_sza_body);
	if (pszuMessageBody != NULL)
		{
		m_pwChatLog_NZ->ChatLog_ChatStateComposerRemovePointerOnly(m_pContactParent_YZ);	// When receiving a message, assume the remote user (contact) stopped typing (composing)
		//m_pwChatLog->ChatLog_EventDisplay(m_pContactParent_YZ->Event_PAllocateEventMessageReceived_YZ(pXmlNodeMessageStanza, pszuMessageBody));
		m_pContactParent_YZ->Vault_XmppAllocateEventMessageReceivedAndDisplayToChatLog(pXmlNodeMessageStanza, pszuMessageBody, m_pwChatLog_NZ);
		const CXmlNode  * pXmlNodeRequest = pXmlNodeMessageStanza->PFindElement("request");
		if (pXmlNodeRequest != NULL)
			{
			// The client sending the message stanza requested a confirmation receipt.  Therefore send the confirmation receipt
			//CSocketXmpp * pSocket = m_pContactParent_YZ->Xmpp_PGetSocketOnlyIfReady();
			//Endorse(pSocket == NULL);	// Although this may sound obvious the socket is ready to send messages, it is not always the case, as sometimes a message is received while connecting to a server (the server cached the message)
			CSocketXmpp * pSocket = m_pContactParent_YZ->m_pAccount->PGetSocket_YZ();
			Assert(pSocket != NULL);
			if (pSocket != NULL)
				#if 0
				pSocket->Socket_WriteXmlFormatted("<message to='$s'><received xmlns='urn:xmpp:receipts' id='$s'/></message>",
					pXmlNodeMessageStanza->PszFindAttributeValueFrom_NZ(), pXmlNodeMessageStanza->PszFindAttributeValueId_NZ());
				#else
				pSocket->Socket_WriteXmlFormatted("<message from='^s' to='$s'><received xmlns='urn:xmpp:receipts' id='$s'/></message>",
					pXmlNodeMessageStanza->PszFindAttributeValueTo_NZ(), pXmlNodeMessageStanza->PszFindAttributeValueFrom_NZ(), pXmlNodeMessageStanza->PszFindAttributeValueId_NZ());
				#endif
			}
		}
	else
		{
		// The stanza does not contain any element <body>, therefore search if is anything else useful within the message, such as a 'delivery receipt' or a 'chat state'
		const CXmlNode  * pXmlNodeReceived = pXmlNodeMessageStanza->PFindElement("received");
		if (pXmlNodeReceived != NULL)
			{
			TIMESTAMP tsMessageReceived = pXmlNodeReceived->LFindAttributeValueIdTimestamp_ZZR();
			CEventMessageTextSent * pEvent = (CEventMessageTextSent *)m_pContactParent_YZ->Vault_PFindEventByID(tsMessageReceived);	// TODO: Search within the socket, it will be much faster
			if (pEvent != NULL)
				{
				Assert(pEvent->EGetEventClass() == CEventMessageTextSent::c_eEventClass);
				if (pEvent->EGetEventClass() == eEventClass_eMessageTextSent)
					{
					pEvent->Event_SetCompletedAndUpdateWidgetWithinParentChatLog();
					if (m_pContactParent_YZ->m_uFlagsContact & TContact::FC_kfContactNeedsInvitation)
						m_pContactParent_YZ->ChatLogContact_RemoveInvitationMessage();
					}
				}
			}
		else
			{
			const CXmlNode * pXmlNodeChatState = pXmlNodeMessageStanza->PFindElementMatchingAttributeValueXmlns("http://jabber.org/protocol/chatstates");
			if (pXmlNodeChatState != NULL)
				m_pwChatLog_NZ->ChatLog_ChatStateTextUpdate(INOUT m_pContactParent_YZ, FCompareStrings(pXmlNodeChatState->m_pszuTagName, "composing") ? eChatState_zComposing : eChatState_Paused);
			}
		} // if...else
	return pszuMessageBody;
	} // ChatLog_DisplayStanzaToUser()

void
WLayoutChatLog::ChatLog_EventAppend(IEvent * pEvent)
	{
	m_pwChatLog_NZ->ChatLog_EventDisplay(pEvent);
	}

void
WLayoutChatLog::ChatLog_EventsRepopulateUpdateUI()
	{
	m_pwChatLog_NZ->ChatLog_EventsRepopulate();
	m_pwChatInput->ChatInput_UpdateWatermarkText();
	}

void
WLayoutChatLog::ChatLog_ScrollToDisplayLastMessage()
	{
	Widget_ScrollToEnd(m_pwChatLog_NZ);
	}

void
WLayoutChatLog::WidgetFindText_Show()
	{
	if (m_pwFindText == NULL)
		{
		m_pwFindText = new WFindText(m_pwChatLog_NZ->document(), m_pwChatInput);
		insertWidget(1, PA_CHILD m_pwFindText);	// Insert the widget between the Chat Log and the Text Input.
		}
	//ChatLog_ScrollToDisplayLastMessage();	// DNW: Make sure the last message of the Chat Log is visible
	m_pwFindText->Show();
	/*
	QTextCursor c(m_pwChatLog->document());
	c.movePosition(QTextCursor::End);
	m_pwChatLog->ensureCursorVisible();
	*/
	ChatLog_ScrollToDisplayLastMessage();
	}

void
WLayoutChatLog::WidgetFindText_Hide()
	{
	if (m_pwFindText != NULL)
		m_pwFindText->Hide();
	}

/*
void
WLayoutChatLog::paintEvent(QPaintEvent *)	// For debugging purpose
	{
	QPainter p(this);
	p.setPen(QColor(0, 255, 0));
	p.drawRect(0, 0, width() - 1, height() - 1);
	}
*/

void
WLayoutChatLog::dragEnterEvent(QDragEnterEvent * pDragEnterEvent)
	{
	pDragEnterEvent->accept();
	}

void
WLayoutChatLog::dropEvent(QDropEvent * pDropEvent)
	{
	ITreeItemChatLogEvents * pContactOrGroup = PGetContactOrGroup_NZ();
	CStr strFile;
	QList<QUrl> listUrls = pDropEvent->mimeData()->urls();
	foreach (QUrl url, listUrls)
		{
		if (!url.isLocalFile())
			continue;
		strFile = url.toLocalFile();
		pContactOrGroup->Xmpp_SendEventFileUpload(strFile);
		strFile = url.toString();
		MessageLog_AppendTextFormatCo(d_coBlack, "URL: $S\n", &strFile);
		}
	}

//	WLayoutChatLog::WLayout::Layout_NoticeDisplay()
void
WLayoutChatLog::Layout_NoticeDisplay(IN INotice * piwNotice)
	{
	Assert(piwNotice != NULL);
	Assert(piwNotice->EGetRuntimeClass() != RTI(Null));	// Make sure the vtable is still valid
	insertWidget(1, PA_CHILD piwNotice);	// Insert the notice between the Chat Log and the Text Input.
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
DDialogSendBitcoin::DDialogSendBitcoin() : DDialogOkCancelWithLayouts("Send Bitcoins", eMenuIconBitcoin)
	{
	m_pwEditQuantity = new WEditNumber;
	m_pwEditComment = new WEditTextArea;
	DialogBody_AddRowWidget_PA("Bitcoin Amount", m_pwEditQuantity);
	DialogBody_AddRowWidget_PA("Transaction Comment / Description", m_pwEditComment);

	QComboBox * pwList = new QComboBox();
	pwList->addItem("Haven");
	pwList->addItem("Judge Judy");
	pwList->addItem("Susanne");
	OLayoutHorizontalAlignLeft * pLayoutArbiration = new OLayoutHorizontalAlignLeft(m_poLayoutBody);
	pLayoutArbiration->Layout_AddLabelAndWidgetV_PA("Arbitrator (in case of a dispute)", pwList);
	pLayoutArbiration->addWidget(new WButtonTextWithIcon(" Help me Pick... ", eMenuAction_FindText), 0, Qt::AlignBottom);
	//DialogBody_AddRowWidget_PA();
	Dialog_AddButtonsOkCancel_RenameButtonOk(SL_DDialogSendBitcoin(SL_ButtonSendBitcoins), "Send|Send the Bitcoins", eMenuIconBitcoin);
	}

void
DDialogSendBitcoin::SL_ButtonSendBitcoins()
	{
	TContact * pContact = NavigationTree_PGetSelectedTreeItemMatchingInterfaceTContact();
	if (pContact == NULL)
		return;
	//IEventWalletTransaction * pEvent = TWallet::S_PAllocateEventTransaction(pContact);
	IEventWalletTransaction * pEvent = new CEventWalletTransactionSent(NULL);
	pEvent->m_amtQuantity = m_pwEditQuantity->text().toDouble() * -d_cSatoshisPerBitcoin;	// Use a negative value to indicate a withdraw
	pEvent->m_strComment = *m_pwEditComment;
	pContact->Vault_AddEventToChatLogAndSendToContacts(PA_CHILD pEvent);
	TWallet::S_SaveAll();		// Save all wallets (in case of a crash)
	pContact->TreeItem_GotFocus();	// Refresh the layout
	close();
	}

void
WLayoutChatLog::SL_ButtonSendBitcoin()
	{
	DDialogSendBitcoin dialog;
	dialog.FuExec();
	}

void
WLayoutChatLog::SL_ButtonSendFile()
	{
	PGetContactOrGroup_NZ()->DisplayDialogSendFile();
	}

void
WLayoutChatLog::SL_ButtonAddContacts()
	{
	PGetContactOrGroup_NZ()->DisplayDialogAddContactsToGroupFu();
	}

void
WLayoutChatLog::SL_ButtonSendBallot()
	{
	PGetContactOrGroup_NZ()->DisplayDialogBallotSend();
	}

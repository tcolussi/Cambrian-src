///////////////////////////////////////////////////////////////////////////////////////////////////
//	DialogInvitations.h
//
//	Dialogs to create and accept invitations.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef DIALOGINVITATIONS_H
#define DIALOGINVITATIONS_H
#include "DialogAccountNew.h"	// Definition of SInvitation
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"	// This include is AFTER DialogAccountNew.h to enable auto-complete and syntax highlighting with Qt Creator
#endif

/*
//	Create an invitation to send to a new contact
class DDialogInvitationCreate : public DDialogOkCancelWithLayouts
{
	Q_OBJECT
protected:
	TAccountXmpp * m_pAccount;
	WEdit * m_pwEditUsername;
//	WEdit * m_pwEditNote;
	WEditTextArea * m_pwEditInvitationBase64;
	WLabelSelectable * m_pwLabelInvitationXml;
	CStr m_strInvitationBase64;

public:
	explicit DDialogInvitationCreate(TAccountXmpp * pAccount);
	virtual ~DDialogInvitationCreate();
	void UpdateUI();

public slots:
	void SL_ButtonOK_clicked();
	void SL_Edit_textChanged(const QString & sText);

	SL_TYPEDEF(DDialogInvitationCreate)
	//SL_OBJECT(DDialogInvitationCreate)
}; // DDialogInvitationCreate
#define	SL_ButtonCreate(_pfmName)		SL_DDialogButtonClicked(_pfmName, DDialogInvitationCreate)
*/

/*
struct SInvitation
{
	PSZUC pszUsername;
	PSZUC pszPassword;
	PSZUC pszFrom;
	PSZUC pszServerName;
	UINT uServerPort;
	CBin * pbinFingerprint;
};

//	Accept the invitation to register (create) an account and automatically link to the sender
class DDialogInvitationAccept : public DDialogOkCancelWithLayouts
{
protected:
	WEditTextArea * m_pwEditInvitation;
	WEdit * m_pwEditUsername;
	WEdit * m_pwEditPassword;
	WEdit * m_pwEditFrom;
	WEdit * m_pwEditServerName;
	WEditNumber * m_pwEditServerPort;
	WEditFingerprint * m_pwEditFingerprint;
	CStr m_strInvitationText;
	CStr m_strInvitationBase64;
	CXmlTree m_oXmlTree;		// This variable must be a member of the dialog because the pointers of SInvitation points to data within the m_binXmlFileData of CXmlTree
	CBin m_binFingerprint;	// Same as m_oXmlTree
public:
	SInvitation m_invitation;

public:
	DDialogInvitationAccept();
	void UpdateUI();
protected slots:
	void SL_ButtonOK_clicked();
	void SL_EditInvitation_textChanged();
	SL_OBJECT(DDialogInvitationAccept)
}; // DDialogInvitationAccept
#define	SL_ButtonAccept(_pfmName)		SL_DDialogButtonClicked(_pfmName, DDialogInvitationAccept)
*/

//	Display a notice the message comes from an unsolicited contact, and give the opportunity for the user to approve the contact.
class WNoticeContactUnsolicited : public INoticeWithIcon
{
	Q_OBJECT
public:
	WNoticeContactUnsolicited(TContact * pContact);
public slots:
	void SL_ButtonContactApprove_clicked();
	void SL_ButtonContactReject_clicked();
};


//	Create a canned invitation to send to the contact to establish communication.
class WNoticeContactInvite : public INoticeWithIcon
{
	Q_OBJECT
protected:
	WEditTextArea * m_pwEditInvitation;
public:
	WNoticeContactInvite(TContact * pContact);
	virtual BOOL FDeleteDuplicateNotice(INOUT INotice * piwNoticeNew);

//	~WNoticeContactInvite() { }
public slots:
	void SL_ButtonCopy_clicked();
	RTI_IMPLEMENTATION(WNoticeContactInvite)
};

//	Object holding the content of an invitation
class CInvitation
{
public:
	CStr m_strInvitationText;	// Invitation text (typically from the clipboard)
	CStr m_strInvitationBase64;
	CStr m_strInvitationXml;	// Content of the invitation in its XML format (useful for debugging)
	// CStr m_strNameInvitationFrom; // NYI: Name of the contact who created the invitation (this field may be useful in case the name differs from the JID)
	CStr m_strJidInvitationFrom;	// The JID of the contact who created invitation
	CStr m_strUsername;				// Suggested username to create the account
	CStr m_strServerName;			// Server name to create the invitation
	UINT m_uServerPort;
	CBin m_binFingerprintCertificate;

public:
	CInvitation();
	~CInvitation();
	BOOL FInitFromBase64(const CStr & strInvitationBase64);
};

CInvitation * Invitation_PazAllocateFromBase64(const CStr & strInvitationBase64);
CInvitation * Invitation_PazAllocateFromText(const CStr & strInvitationText);
CInvitation * Invitation_PazAllocateFromEdit(WEditTextArea * pwEdit);
CInvitation * Invitation_PazAllocateFromTextOfClipboard();

//	Class to notify the user he/she has a pending invitation waiting to be accepted.
//	This invitation is created when the application starts if there is a file named "Invitation.txt"
class WNoticeInvitationPending : public INoticeWithIcon
{
	Q_OBJECT
protected:
	CInvitation * m_paInvitation;
public:
	WNoticeInvitationPending(PA_CHILD CInvitation * paInvitation);
	~WNoticeInvitationPending();
	virtual ENoticeAction ETreeItemGotFocus(IN ITreeItem * piTreeItemFocus);

public slots:
	void SL_ButtonInvitationAccept_clicked();
	void SL_ButtonInvitationReject_clicked();
};


//	Create a form to invite multiple contacts by email
class WNoticeInviteContactsByEmail : public INoticeWithIcon
{
	Q_OBJECT
protected:
	WEditTextArea * m_pwEditInvitation;
public:
	WNoticeInviteContactsByEmail(TAccountXmpp * pAccount);
/*
	virtual BOOL FDeleteDuplicateNotice(INOUT INotice * piwNoticeNew);
*/
public slots:
	void SL_ButtonSend_clicked();
};


#endif // DIALOGINVITATIONS_H

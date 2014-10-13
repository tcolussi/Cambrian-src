///////////////////////////////////////////////////////////////////////////////////////////////////
//	DialogAccountNew.h
//
//	Dialog to add an existing XMPP account or create a new XMPP account.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef DIALOGACCOUNTNEW_H
#define DIALOGACCOUNTNEW_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

//	Base class to accept aninvitation
class DDialogInvitationCore : public DDialogOkCancelWithLayouts
{
protected:
	CInvitation * m_pInvitationInit;	// Initial invitation during the construction of the object
	CInvitation * m_paInvitationEdit;	// Allocated invitation by the dialog, either from text from the clipboard or from text entered by the user

	WEditTextArea * m_pwEditInvitation;		// Control for the user to paste the invitation (only for eMenuAction_AccountAcceptInvitation)
	WLabel * m_pwLabelInvitationFrom;		// Display where the invitation comes from
	WEdit * m_pwEditPassword;				// This variable is mostly used by DDialogAccountAdd, however we need a variable to determine if the dialog is an invitation to create an account or add a contact.

	CStr m_strInvitationBase64;
	CStr m_strJidInvitationFrom;	// The JID of the person who created invitation (only for eMenuAction_AccountAcceptInvitation)

public:
	DDialogInvitationCore(CInvitation * pInvitationInit, PSZAC pszCaption, EMenuIcon eMenuIcon);
	~DDialogInvitationCore();
	void CreateEditInvitation();
	void CreateLabelInvitationFrom();
	void UpdateLabelInvitationFrom();
	CInvitation * PGetInvitation() const;
	BOOL FValidateInvitationNotFromSelf_MB();
	void AddContactInvitationToNavigationTree_MB(TAccountXmpp * pAccount);

protected slots:
	void SL_EditInvitation_textChanged();
	SL_OBJECT(DDialogInvitationCore)
}; // DDialogInvitationCore


//	Dialog to add a contact from an invitation
class DDialogInvitationContactAdd : public DDialogInvitationCore
{
public:
	DDialogInvitationContactAdd(CInvitation * pInvitationInit);

protected slots:
	void SL_ButtonOK_clicked();
	SL_OBJECT(DDialogInvitationContactAdd)
};

//	class DDialogAccountAdd
//
//	Dialog to add a new account.
//	This dialog supports three modes:
//		1. Register/create a new account
//		2. Login/link/add an existing account
//		3. Accept an invitation to create an account
//
//	Since most of the code for both functions are identical, the same dialog is used.
class DDialogAccountAdd : public DDialogInvitationCore, ISocketUI
{
	Q_OBJECT
	SL_TYPEDEF(DDialogAccountAdd)
protected:
	TProfile * m_pProfile;	// Parent profile to create the account
	EMenuAction m_eMenuAction;	// One of the following value: eMenuAction_AccountRegister, eMenuAction_AccountLogin or eMenuAction_AccountAcceptInvitation.
	WButtonText * m_pwButtonAdvanced;
	WEdit * m_pwEditUsername;
//	WEditTextArea * m_pwEditLog;

	CStr m_strUsername;
	CStr m_strServerName;
	CStr m_strJID;
	CBin m_binFingerprint;
	UINT m_uServerPort;
	TAccountXmpp * m_paAccount;		// Pointer to the newly created chat account

public:
	DDialogAccountAdd(TProfile * pProfile, EMenuAction eMenuAction, CInvitation * pInvitationInit);
	virtual ~DDialogAccountAdd();

protected:
	void UpdateInvitation();
	void UpdateJID();
	void GetJID();
	void SetFocusToUsername();
	void DoActionAddAccountNavigationTree();
	void RemoveStatusBarDisplayFunction();
//	virtual void reject();	// From QDialog
	virtual void SocketUI_OnSuccess();
	virtual void SocketUI_OnError();

protected slots:
	void SL_ButtonOK_clicked();
	void SL_ButtonAdvanced_clicked();
	void SL_EditUsername_textChanged(const QString & sText);
	void SL_EditInvitation_textChanged();
	friend class DDialogAccountAddAdvanced;
}; // DDialogAccountAdd


//	Advanced options to login/register/create a new account
class DDialogAccountAddAdvanced : public DDialogOkCancelWithLayouts
{
public:
	DDialogAccountAdd * m_pDialogParent;
	WEdit * m_pwEditServerName;
	WEdit * m_pwEditServerPort;
	WEdit * m_pwEditFingerprint;
	// TLS Connection: Smart, Automatic, Always

public:
	DDialogAccountAddAdvanced(DDialogAccountAdd * pDialogParent);
protected slots:
	void SL_ButtonOK_clicked();
	SL_OBJECT(DDialogAccountAddAdvanced)
}; // DDialogAccountAddAdvanced


//	####### NOTE ###########
//	The class DDialogInvitationCreate should be in DialogInvitations.h however the Qt compiler is too stupid to compile it there.
//	If I put the class DDialogInvitationCreate in DialogInvitations.h, I get the following error: undefined reference to `vtable for DDialogInvitationCreate'





///////////////////////////////////////////////////////////////////////////////////////////////////
//	Create a modeless dialog to display an assertion failure.
//	The motivation for this dialog is to prevent Cambrian to block (pause) when there is an assertion failure.
//	Blocking is useful when debugging, however may freeze (crash) the application if the assertion fails while processing a socket message,
//	as each sockets are serialized with a mutex, and blocking may result in a mutex deadlock.
class DDialogAssertionFailure : public DDialogOkCancelWithLayouts
{
	Q_OBJECT
public:
	DDialogAssertionFailure * m_pNext;
	CStr m_strAssert;		// Text representing the Assert() or Report().  This text may be copied into the clipboard.
	int m_cAsserts;			// Number of times this message was displayed

public:
	DDialogAssertionFailure(const CStr & strMessageHtml, const CStr & strAssert);
	virtual ~DDialogAssertionFailure();
	virtual void reject();
	void IncreaseAssert();

public slots:
	void SL_CopyToClipboard();
	void SL_ShowErrorLog();
};



#endif // DIALOGACCOUNTNEW_H



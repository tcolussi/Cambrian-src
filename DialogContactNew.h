#ifndef DIALOGCONTACTNEW_H
#define DIALOGCONTACTNEW_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class DialogContactNew : public DDialogOkCancelWithLayouts
{
	Q_OBJECT
private:
	TAccountXmpp * m_pAccount;	// IN: Pointer of the account to add the new contacts
	CArrayPasz m_arraypaszContacts;	// OUT: Array of contacts to add to the XMPP account

	WEdit * m_pwEditUsername;
	WEditTextAreaReadOnly * m_pwEditHtmlUsernames;
public:
	explicit DialogContactNew(TAccountXmpp * pAccount);
	~DialogContactNew();
	
private slots:
	void SL_EditUsername_textChanged(const QString & sUsernames);
	void SL_ButtonOK_clicked();
	SL_TYPEDEF(DialogContactNew)

}; // DialogContactNew

///////////////////////////////////////////////////////////////////////////////////////////////////
class WLayoutContactNew : public WLayout
{
	Q_OBJECT
private:
	TAccountXmpp * m_pAccount;
	WEdit * m_pwEditInvitation;
	WButtonTextWithIcon * m_pwButtonCopyInvitation;
	WEdit * m_pwEditUsername;
	WEditTextAreaReadOnly * m_pwEditHtmlUsernames;
	CArrayPasz m_arraypaszContacts;	// Array of contacts to add to the XMPP account
public:
	WLayoutContactNew(TAccountXmpp * pAccount);
	virtual void Layout_SetFocus();	// From WLayout

private slots:
	void SL_CopyInvitation();
	void SL_EditUsername_textChanged(const QString & sUsernames);
	void SL_ButtonAddContacts();
};

class CInvitationFromUrlQueryString
{
public:
	CStr m_strJID;
	CStr m_strNameDisplay;
public:
	CInvitationFromUrlQueryString(PSZUC pszJID);
};

#endif // DIALOGCONTACTNEW_H

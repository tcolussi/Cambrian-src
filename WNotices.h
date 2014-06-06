///////////////////////////////////////////////////////////////////////////////////////////////////
//	WNotices.h
//
//	Widgets displaying messages to the user
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef WMESSAGES_H
#define WMESSAGES_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#define SL_OBJECT(class_name)		SL_TYPEDEF(class_name)	Q_OBJECT	// Combination of macros SL_TYPEDEF() and Q_OBJECT

class CListNotices	// Linked list of notices
{
public:
	INotice * m_paiwNoticeList;

public:
	CListNotices() { m_paiwNoticeList = NULL; }
	UINT UGetLength() const;
	void Notice_Add(PA_CHILD INotice * paiwNoticeNew);
	INotice * Notice_PAddUnique(PA_DELETING INotice * paiwNoticeNew, ITreeItem * piTreeItemSelected = NULL);
	INotice * Notice_PazDetach(INotice * piwNotice);
	void Notice_Delete(PA_DELETING INotice * piwNotice);
	void Notices_DetachAllFromParent(QWidget * pwParent);
	void Notices_DeleteAll();
	void Notices_DeleteAllRelatedToTreeItem(const ITreeItem * piTreeItemDeleting);
	void Notices_DeleteAllRelatedToSponsor(const void * pvSponsor);
};

//	Linked list of allocated notices where its destructor will automatically delete all notices within the list.
class CListaNotices	: public CListNotices
{
public:
	~CListaNotices() { Notices_DeleteAll(); }
};

extern CListaNotices g_listaNoticesRoaming;
extern CListaNotices g_listaNoticesAuxiliary;

//	Roaming Notice List
void NoticeListRoaming_AddNotice(PA_DELETING INotice * paiwNoticeNew);
void NoticeListRoaming_AddMessageWarning_VE(ITreeItem * pSponsor, PSZAC pszTitle, PSZAC pszFmtTemplate, ...);
void NoticeListRoaming_TreeItemGotFocus(IN ITreeItem * piTreeItemFocus);
void NoticeListRoaming_RefreshDisplay();
void NoticeListRoaming_TreeItemDeleting(const ITreeItem * piTreeItemDeleting);
void NoticeListRoaming_FlushAndCauseMemoryLeak();

//	Auxiliary Notice List
void NoticeListAuxiliary_AddNotice(PA_CHILD INotice * paiwNoticeNew, ITreeItem * piTreeItem);
void NoticeListAuxiliary_DeleteNotice(PA_DELETING INotice * paiwNoticeDelete);
void NoticeListAuxiliary_DeleteAllNoticesRelatedToTreeItem(const ITreeItem * piTreeItemDeleting);


enum ENoticeAction	// May be casted from a boolean (true or false)
	{
	eNoticeActionKeepHidden = false,	// Keep the notice hidden because the notice is NOT relevant to Tree Item having the focus.
	eNoticeActionDisplay = true,		// Display the notice to the user because the notice IS relevant to the Tree Item having the focus.
	eNoticeActionDelete					// Delete the notice because the notice is no longer relevant (the state of the application changed and there is no need to keep this notice anymore)
	};

//	Base interface to create a linked list of notices to display to the user.
//	The name 'Notice' was chosen to give a clear distinction between a chat message and a notice/warning.
//	Each notice inherits
class INotice : public WWidget	// Hungarian: piw
{
public:
	CListaNotices * m_plistaParent;	// Pointer of the parent where the notice is allocated
	INotice * m_piwNoticeNext;		// Pointer to the next notice in the linked list
	union	// The following union make it easier to cast the pointer of the object sponsoring the notice.  By default, if the object sponsoring the notice is deleted, the notice is deleted as well.
		{
		void * pvSponsor;			// Generic pointer to the object which is sponsoring the notice
		ITreeItem * piTreeItem;		// Generic pointer of the Tree Item which created the notice
		TAccountXmpp * pAccount;
		TContact * pContact;
		} mu_sponsor;
	WGroupBox * m_pwGroupBox;		// Group box displaying the notice (for concistency every notice has a group box)
	WButtonIcon * m_pButtonClose;
public:
	INotice();
	virtual ~INotice();
	void Notice_SetTitle(PSZAC pszTitle);

	/*
	void paintEvent(QPaintEvent *);
	void resizeEvent(QResizeEvent *);
	*/
	void contextMenuEvent(QContextMenuEvent *);

	// The following 4 virtual methods offer a powerful and flexible mechanism to identify notices, detect duplicates, display them in the GUI and deleting them when no longer relevant.
	virtual RTI_ENUM EGetRuntimeClass() const = 0;	// Pure virtual to identify the notice class - this method may be implemented using macro RTI_IMPLEMENTATION()
	virtual BOOL FDeleteDuplicateNotice(INOUT INotice * piwNoticeNew);
	virtual ENoticeAction ETreeItemGotFocus(IN ITreeItem * piTreeItemFocus);
	virtual BOOL FDeleteNoticeBecauseDeletingTreeItem(const ITreeItem * piTreeItemDeleting);

	// Special methods to parametrize the slots
	SL_TYPEDEF(INotice)
	void ConnectButton(QAbstractButton * pwButton, PFmSlot pfmSlot);
	template<typename PFmSlot_T>
	void ConnectButton_T(QAbstractButton * pwButton, PFmSlot_T pfmSlot);

}; // INotice

#define SL_INotice(pfmSlot, dialog_class)		SL_CAST(pfmSlot, dialog_class, INotice)
#define SL_INoticeWithLayout(pfmSlot)			SL_INotice(pfmSlot, INoticeWithLayout)

//	Message containing an horizontal layout so the widgets inside the noticed are horizontally aligned/centered
class INoticeWithLayout : public INotice
{
protected:
	OLayoutVertical * m_poLayoutNotice;		// Core layout permitting the notice to display multiple widgets vertically
	OLayoutHorizontal * m_poLayoutBody;		// Body of the notice, typically displaying an icon, a message, and perhaps some buttons for the user to perform an action
	OLayoutVerticalAlignTop * m_poLayoutButtons;	// Layout to display the buttons (if any for the notice)

public:
	INoticeWithLayout();
	OLayoutHorizontal * Notice_PoAddLayoutHorizontal();
	WLabelSelectable * Notice_PwAddLayoutHorizontalWithLabel();
	void Notice_LayoutBodyMoveToBottom();
	OLayoutVertical * Notice_PoGetLayoutButtons_NZ();
	void Notice_AddButton(PA_CHILD WButtonText * pawButton, INotice::PFmSlot pfmSlotButton);
	void Notice_AddButtonIconClose();

public slots:
	void SL_ButtonClose_clicked();
	SL_OBJECT(INoticeWithLayout)
}; // INoticeWithLayout

//	Generic notice with an icon and a label to display a message to the user message.
class INoticeWithIcon : public INoticeWithLayout
{
protected:
	WButtonIcon * m_pwButtonIcon;	// Wiget displaying the icon.  This may be replaced with a WLabelSelectable.
	WLabelSelectable * m_pwLabelMessage;		// Core widget displaying the message text for the notice
	WLabelSelectable * m_pwLabelMessageExtra;	// Optional widget to display additional text just below the message text.  By default, this label is NULL (hidden).
public:
	INoticeWithIcon(PSZAC pszTitle, EMenuAction eMenuIcon);
	void Notice_SetIcon(EMenuAction eMenuActionIcon);
	void NoticeMessage_SetText_VE(PSZAC pszFmtTemplate, ...);
	void NoticeMessage_SetText_VL(PSZAC pszFmtTemplate, va_list vlArgs);
	void NoticeMessageExtra_SetText_VE(PSZAC pszFmtTemplate, ...);
	void NoticeMessageExtra_Hide();

	RTI_IMPLEMENTATION(INoticeWithIcon)	// This line enambles the inheritance (and a creation of an instance of INoticeWithIcon) without having to define a new runtime class
};

class WNoticeMessageSponsored : public INoticeWithIcon
{
public:
	WNoticeMessageSponsored(ITreeItem * pSponsor, PSZAC pszTitle, EMenuAction eMenuIcon);
	virtual BOOL FDeleteDuplicateNotice(INOUT INotice * piwNoticeNew);
	virtual ENoticeAction ETreeItemGotFocus(IN ITreeItem * piTreeItemFocus);
	RTI_IMPLEMENTATION(WNoticeMessageSponsored)
};

class WNoticeWarning : public INoticeWithLayout
{
protected:
	WLabelSelectable * m_pwLabelWarning;	// Label displaying the main warning message
	WLabelSelectable * m_pwLabelStatus;	// Label displaying the 'status' of the warning message (or to display extra information about the notice)

public:
	WNoticeWarning();
	void NoticeWarning_SetTextFormat_VE(PSZAC pszFmtTemplate, ...);
	void NoticeWarning_SetTextFormat_VL(PSZAC pszFmtTemplate, va_list vlArgs);
	void NoticeStatus_SetTextFormat_VE(PSZAC pszFmtTemplate, ...);
	void NoticeStatus_Hide();

	RTI_IMPLEMENTATION(WNoticeWarning)
}; // WNoticeWarning


class WNoticeWarningCertificateNotAuthenticated : public WNoticeWarning
{
	Q_OBJECT
protected:
	TAccountXmpp * m_pAccount;		// Account to which the certificate is not authenticated.  With this pointer, we can get both the certificate and the socket to connect
	WButtonTextWithIcon * m_pButtonConnectNow;	// Button displaying the
//	class WDockingCertificate * m_pwDocking;	// Pointer to the docking widget, to avoid displaying the same widget multiple times if the user clicks on the button
public:
	WNoticeWarningCertificateNotAuthenticated(TAccountXmpp * pAccount);
	virtual BOOL FDeleteDuplicateNotice(INOUT INotice * piwNoticeNew);
	virtual ENoticeAction ETreeItemGotFocus(IN ITreeItem * piTreeItemFocus);

protected:
	TCertificate * PGetCertificate_YZ();

protected slots:
	void SL_CertificateApprove();
	void SL_CertificateView();
	void SL_ConnectNow();

	RTI_IMPLEMENTATION(WNoticeWarningCertificateNotAuthenticated)
}; // WNoticeWarningCertificateNotAuthenticated


#endif // WMESSAGES_H

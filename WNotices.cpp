///////////////////////////////////////////////////////////////////////////////////////////////////
//	WNotices.cpp
//
//	Widgets displaying notices to the user.
//	A notice is a special message/warning to bring attention to the user about a situation, event or action to be taken.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Roaming Notices
//
//	A roaming notice may 'roam' from one Tree Item to another, offering a powerful and flexible mechanism
//	to display a message (notice) to the user without having to use a Message Box.
CListaNotices g_listaNoticesRoaming;

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Auxiliary Notices
//
//	An auxiliary notice is a message specific to a Tree Item.  The g_listaNoticesAuxiliary is a convenient storage location to avoid having each ITreeItem or WLayout to keep a list of notices.
//	When a Tree Item (or WLayout) is deleted, the notices related to the deleted object are automatically deleted as well.
//
//	The motive for having an auxiliary list is reducing the load on the global roming notice list.
//	Adding notices to the roaming list is more computing expensive as its entire list must be consulted each time a Tree Item has the focus, is being deleted or the Navigation Tree is being refreshed.
//	The Auxiliary List is much simpler, as it does not check for duplicates notices, nor attempt set the focus and/or delete them when no longer relevant.
//	If the parent (sponsor) no longer wishes its notice, then it must call NoticeListAuxiliary_Delete()
//
CListaNotices g_listaNoticesAuxiliary;

UINT
CListNotices::UGetLength() const
	{
	UINT uLength = 0;
	INotice * piwNotice = m_paiwNoticeList;
	while (piwNotice != NULL)
		{
		Assert(piwNotice->m_plistaParent == this);
		Assert(piwNotice->EGetRuntimeClass() != RTI(Null));	// Make sure the vtable is valid
		piwNotice = piwNotice->m_piwNoticeNext;
		uLength++;
		}
	return uLength;
	}

/*
void
CListNotices::Notice_Add(PA_CHILD INotice * paiwNoticeNew)
	{
	Assert(paiwNoticeNew != NULL);
	Assert(paiwNoticeNew->EGetRuntimeClass() != RTI(Null));
	Assert(paiwNoticeNew->m_piwNoticeNext == NULL);
	Assert(paiwNoticeNew->m_plistaParent == NULL);

	paiwNoticeNew->m_piwNoticeNext = m_paiwNoticeList;
	m_paiwNoticeList = paiwNoticeNew;	// Insert the notice at the beginning of the list
	paiwNoticeNew->m_plistaParent = (CListaNotices *)this;
	}
*/

//	Add a notice to the list, however delete the new notice if considered a duplicate.
//	Return NULL if the notice was deleted because it is a duplicate.
//	Return the pointer paiwNoticeNew if the notice has been added to the list.
INotice *
CListNotices::Notice_PAddUnique(PA_DELETING INotice * paiwNoticeNew, INOUT ITreeItem * piTreeItemSelected)
	{
	Assert(paiwNoticeNew != NULL);
	Assert(paiwNoticeNew->EGetRuntimeClass() != RTI(Null));
	Assert(paiwNoticeNew->m_piwNoticeNext == NULL);
	Assert(paiwNoticeNew->m_plistaParent == NULL);
	Endorse(piTreeItemSelected == NULL || PGetRuntimeInterfaceOf_ITreeItem(piTreeItemSelected) != NULL);

	// Before adding the notice to the global list, query each notice for duplicate
	BOOL fuDeleteNotice = FALSE;
	const RTI_ENUM rtiNoticeNew = paiwNoticeNew->EGetRuntimeClass();
	INotice * piwNotice = m_paiwNoticeList;
	while (piwNotice != NULL)
		{
		Assert(piwNotice->m_plistaParent == this);
		if (piwNotice->EGetRuntimeClass() == rtiNoticeNew)
			fuDeleteNotice |= piwNotice->FDeleteDuplicateNotice(INOUT paiwNoticeNew);
		piwNotice = piwNotice->m_piwNoticeNext;
		}
	if (fuDeleteNotice)
		{
		MessageLog_AppendTextFormatSev(eSeverityNoise, "Deleting notice 0x$p because it is considered a duplicate\n", paiwNoticeNew);
		DeleteNotice:
		Assert(paiwNoticeNew->m_plistaParent == NULL);
		delete paiwNoticeNew;	// The notice is a duplicate, so delete it
		return NULL;
		}
	// Give an opportunity for the notice to display/attach itself to the current selected Tree Item
	if (piTreeItemSelected != NULL)
		{
		ENoticeAction eNoticeAction = paiwNoticeNew->ETreeItemGotFocus(piTreeItemSelected);
		if (eNoticeAction == eNoticeActionDisplay)
			MainWindow_DisplayNoticeWithinCurrentLayout(IN paiwNoticeNew);
		else if (eNoticeAction == eNoticeActionDelete)
			{
			paiwNoticeNew->m_plistaParent = NULL;
			goto DeleteNotice;
			}
		}
	paiwNoticeNew->m_piwNoticeNext = m_paiwNoticeList;
	m_paiwNoticeList = paiwNoticeNew;	// Insert the notice at the beginning of the list
	paiwNoticeNew->m_plistaParent = (CListaNotices *)this;
	return paiwNoticeNew;
	} // Notice_PAddUnique()

//	Detach (remove) the notice from the list and hide it from its parent widget.
//	The pointer piwNoticeDetach may point to a notice being deleted, so its vtable may no longer be valid.
INotice *
CListNotices::Notice_PazDetach(INotice * piwNoticeDetach)
	{
	Assert(piwNoticeDetach != NULL);
	Endorse(piwNoticeDetach->parent() != NULL);	// The notice may still have a parent, however if this is the case, the parent will delete it.  What is important is having the notice removed from the list to avoid a dangling pointer
	INotice * piwNoticePrevious = NULL;
	INotice * piwNotice = m_paiwNoticeList;
	while (piwNotice != NULL)
		{
		Assert(piwNotice->m_plistaParent == this);
		INotice * piwNoticeNext = piwNotice->m_piwNoticeNext;
		if (piwNotice == piwNoticeDetach)
			{
			if (piwNoticePrevious == NULL)
				m_paiwNoticeList = piwNoticeNext;
			else
				piwNoticePrevious->m_piwNoticeNext = piwNoticeNext;
			//piwNoticeDetach->Widget_DetachFromParent(); // setParent(NULL);	// Detach (and hide) the notice from its parent widget.
			piwNoticeDetach->m_plistaParent = NULL;
			return PA_CAST_FROM_P piwNoticeDetach;
			}
		Assert(piwNotice->EGetRuntimeClass() != RTI(Null));	// Make sure the vtable of other notices are still valid
		piwNoticePrevious = piwNotice;
		piwNotice = piwNoticeNext;
		} // while
	Assert(FALSE && "Notice not found in list");
	return NULL;
	}

void
CListNotices::Notice_Delete(PA_DELETING INotice * piwNotice)
	{
	delete Notice_PazDetach(piwNotice);
	}

//	Detach (and hide) all notices related to a parent widget.
//	This method will prevent the notices from being deleted along with the parent.
//	This method is useful for roaming notices.
void
CListNotices::Notices_DetachAllFromParent(QWidget * pwParent)	// Notices_DetachAllFromParentWidget()
	{
	INotice * piwNotice = m_paiwNoticeList;
	while (piwNotice != NULL)
		{
		Assert(piwNotice->m_plistaParent == this);
		Assert(piwNotice->EGetRuntimeClass() != RTI(Null));	// Make sure the vtable is still valid
		if (piwNotice->parent() == pwParent)
			piwNotice->setParent(NULL);		// Detach (and hide) the notice from its parent widget.
		piwNotice = piwNotice->m_piwNoticeNext;
		} // while
	}

void
CListNotices::Notices_DeleteAll()
	{
	while (m_paiwNoticeList != NULL)
		{
		Assert(m_paiwNoticeList->m_plistaParent == this);
		Assert(m_paiwNoticeList->EGetRuntimeClass() != RTI(Null));	// Make sure the vtable is still valid
		INotice * piwNoticeNext = m_paiwNoticeList->m_piwNoticeNext;
		m_paiwNoticeList->m_plistaParent = NULL;
		delete m_paiwNoticeList;
		m_paiwNoticeList = piwNoticeNext;
		}
	}

void
CListNotices::Notices_DeleteAllRelatedToTreeItem(const ITreeItem * piTreeItemDeleting)
	{
	Assert(piTreeItemDeleting != NULL);
	Assert(PGetRuntimeInterfaceOf_ITreeItem(piTreeItemDeleting) == piTreeItemDeleting);	// The vtable of the Tree Item should still be valid
	INotice * piwNoticePrevious = NULL;
	INotice * piwNotice = m_paiwNoticeList;
	while (piwNotice != NULL)
		{
		Assert(piwNotice->m_plistaParent == this);
		INotice * piwNoticeNext = piwNotice->m_piwNoticeNext;
		Assert(piwNotice->EGetRuntimeClass() != RTI(Null));	// Make sure the vtable is still valid
		if (piwNotice->FDeleteNoticeBecauseDeletingTreeItem(IN piTreeItemDeleting))
			{
			MessageLog_AppendTextFormatSev(eSeverityNoise, "Deleting notice 0x$p because Tree Item 0x$p ($s) is being deleted\n", piwNotice, piTreeItemDeleting, const_cast<ITreeItem *>(piTreeItemDeleting)->TreeItem_PszGetNameDisplay());
			if (piwNoticePrevious == NULL)
				m_paiwNoticeList = piwNoticeNext;
			else
				piwNoticePrevious->m_piwNoticeNext = piwNoticeNext;
			piwNotice->m_plistaParent = NULL;
			delete piwNotice;
			}
		else
			piwNoticePrevious = piwNotice;
		piwNotice = piwNoticeNext;
		} // while
	}

void
CListNotices::Notices_DeleteAllRelatedToSponsor(const void * pvSponsor)
	{
	Assert(pvSponsor != NULL);
	}

void
NoticeListRoaming_AddNotice(PA_DELETING INotice * paiwNoticeNew)
	{
	(void)g_listaNoticesRoaming.Notice_PAddUnique(PA_DELETING paiwNoticeNew, NavigationTree_PGetSelectedTreeItem());
	}

void
NoticeListRoaming_AddMessageWarning_VE(ITreeItem * pSponsor, PSZAC pszTitle, PSZAC pszFmtTemplate, ...)
	{
	WNoticeMessageSponsored * pawNotice = new WNoticeMessageSponsored(pSponsor, pszTitle, eMenuIconWarning);
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	pawNotice->NoticeMessage_SetText_VL(pszFmtTemplate, vlArgs);
	pawNotice->Notice_AddButtonIconClose();
	NoticeListRoaming_AddNotice(PA_DELETING pawNotice);
	}

void
NoticeListRoaming_TreeItemGotFocus(IN ITreeItem * piTreeItemFocus)
	{
	Assert(PGetRuntimeInterfaceOf_ITreeItem(piTreeItemFocus) == piTreeItemFocus);
	INotice * piwNoticePrevious = NULL;
	INotice * piwNotice = g_listaNoticesRoaming.m_paiwNoticeList;
	while (piwNotice != NULL)
		{
		Assert(piwNotice->m_plistaParent == &g_listaNoticesRoaming);
		INotice * piwNoticeNext = piwNotice->m_piwNoticeNext;
		Assert(piwNotice->EGetRuntimeClass() != RTI(Null));	// Make sure the vtable is always valid
		ENoticeAction eNoticeAction = piwNotice->ETreeItemGotFocus(IN piTreeItemFocus);
		if (eNoticeAction != eNoticeActionDelete)
			{
			if (eNoticeAction == eNoticeActionDisplay)
				MainWindow_DisplayNoticeWithinCurrentLayout(IN piwNotice);
			else
				piwNotice->Widget_DetachFromParent();		// Hide the notice from the current layout
			piwNoticePrevious = piwNotice;
			}
		else
			{
			MessageLog_AppendTextFormatSev(eSeverityNoise, "Deleting notice 0x$p because ETreeItemGotFocus() returned eNoticeActionDelete for Tree Item 0x$p\n", piwNotice, piTreeItemFocus);
			if (piwNoticePrevious == NULL)
				g_listaNoticesRoaming.m_paiwNoticeList = piwNoticeNext;
			else
				piwNoticePrevious->m_piwNoticeNext = piwNoticeNext;
			piwNotice->m_plistaParent = NULL;
			delete piwNotice;
			}
		piwNotice = piwNoticeNext;
		} // while
	} // NoticeListRoaming_TreeItemGotFocus()

//	Refresh the display of notices based on the current selected Tree Item
void
NoticeListRoaming_RefreshDisplay()
	{
	ITreeItem * piTreeItemSelected = NavigationTree_PGetSelectedTreeItem();
	if (piTreeItemSelected != NULL)
		NoticeListRoaming_TreeItemGotFocus(piTreeItemSelected);
	}

//	Notify the global notice list a Tree Item is being deleted, and therefore delete any notice depending on that Tree Item (in other words, delete all notices which are no longer relevant)
void
NoticeListRoaming_TreeItemDeleting(const ITreeItem * piTreeItemDeleting)
	{
	g_listaNoticesRoaming.Notices_DeleteAllRelatedToTreeItem(piTreeItemDeleting);
	}

void
NoticeListRoaming_FlushAndCauseMemoryLeak()
	{
	g_listaNoticesRoaming.m_paiwNoticeList = NULL;
	}
/*
void
NoticeListAuxiliary_AddNotice(PA_CHILD INotice * paiwNoticeNew, ITreeItem * piTreeItem)
	{
	Assert(paiwNoticeNew != NULL);
	Assert(piTreeItem != NULL);
	g_listaNoticesAuxiliary.Notice_AddUnique(PA_DELETING paiwNoticeNew, piTreeItem);
	}
*/
void
NoticeListAuxiliary_DeleteNotice(PA_DELETING INotice * paiwNoticeDelete)
	{
	g_listaNoticesAuxiliary.Notice_Delete(PA_DELETING paiwNoticeDelete);
	}

void
NoticeListAuxiliary_DeleteAllNoticesRelatedToTreeItem(const ITreeItem * piTreeItemDeleting)
	{
	g_listaNoticesAuxiliary.Notices_DeleteAllRelatedToTreeItem(piTreeItemDeleting);
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	A layout is being destroyed, so we have to make sure there are no visible roaming notices within that layout, otherwise they will be deleted along with the layout and the application will crash because g_paiwNoticeList will contain a dangling pointer.
WLayout::~WLayout()
	{
	g_listaNoticesRoaming.Notices_DetachAllFromParent(this);	// Make sure the roaming notices remain in the linked list
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	SetFocus(), virtual
//
//	Force the layout to have the focus.
//	The layout will decide which widget shall have the focus.
//
//	IMPLEMENTATION NOTES
//	Most layouts will never implement this virtual method, however some layouts behaving like a 'wizard' will implement.
//
//	SEE ALSO
//	TreeItemLayout_SetFocus()
void
WLayout::Layout_SetFocus()
	{
	}

void
WLayout::Layout_NoticeAuxiliaryAdd(PA_DELETING INotice * paiwNotice)
	{
	if (g_listaNoticesAuxiliary.Notice_PAddUnique(PA_DELETING paiwNotice) != NULL)
		Layout_NoticeDisplay(paiwNotice);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Layout_NoticeDisplay(), virtual
//
//	Virtual method giving the opportunity to a layout to customize where to display the notices.
//
//	INTERFACE NOTES
//	The parameter piwNotice is considered 'IN' despite the fact that insertWidget() is PA_CHILD because
//	the notice will not be deleted by the layout.  The destructor ~WLayout ensures any roaming notice attached
//	to the layout are detached before the layout is completely destroyed.
void
WLayout::Layout_NoticeDisplay(IN INotice * piwNotice)
	{
	Assert(piwNotice != NULL);
	Assert(piwNotice->EGetRuntimeClass() != RTI(Null));	// Make sure the vtable is still valid
	insertWidget(0, PA_CHILD piwNotice);	// By default, insert the notice at the top (at index 0)
	}


#define d_cyHeightNoticeDefault		80	// By default, set the notice height to 80 pixels which is enough to display 4 lines of text.  By having a maximum/fixed height, the QSplitter will not attemp to stretch the notice along with other widgets.
#define d_cyHeightNoticeExtraLayout	20	// Add 20 pixel for any extra layout.  This height along with the spacing between the layouts is enough to display two lines of text.

INotice::INotice() : WWidget(NULL)
	{
	m_plistaParent = NULL;
	m_piwNoticeNext = NULL;
	mu_sponsor.pvSponsor = NULL;
	m_pButtonClose = NULL; // new WButtonIconForToolbar(this, eMenuAction_Quit);
	setFixedHeight(d_cyHeightNoticeDefault);
	setContextMenuPolicy(Qt::DefaultContextMenu);
	m_pwGroupBox = NULL;
//	m_pwGroupBox = new WGroupBox(this);
//	m_pwGroupBox->setContentsMargins( 15, 10, 15, 10);
	}

INotice::~INotice()
	{
	if (m_plistaParent != NULL)
		m_plistaParent->Notice_PazDetach(this);	// Then, detach the notice from the list, so there is no dangling pointer
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	FDeleteDuplicateNotice(), virtual
//
//	This virtual method gives the opportunity to detect duplicate notices and delete them.
//
//	When a new notice is about to be added to a list, this virtual method is called to every other existing notices matching the runtime interface of the new notice.
//	For convenience, this virtual method is called only to existing notices matching the runtime class piwNoticeNew.
//	The parameter piwNoticeNew is INOUT so an existing notice may copy/steal the information from the new notice, such as appending error messages to an existing notice.
BOOL
INotice::FDeleteDuplicateNotice(INOUT INotice * piwNoticeNew)
	{
	Assert(piwNoticeNew != NULL);
	Assert(piwNoticeNew->EGetRuntimeClass() != RTI(Null));	// Make sure the vtable is valid
	return FALSE;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	ETreeItemGotFocus(), virtual
//
//	This virtual method is called every time the user select a different Tree Item.
//
//	The functionality virtual method is threefold:
//		1. Display only the notices relevant to the selected Tree Item.
//		2. Customize the notice (text) to the context, including showing/hiding widgets which are no longer relevant.
//		3. Delete the notice if no longer relevant.
//
ENoticeAction
INotice::ETreeItemGotFocus(IN ITreeItem * piTreeItemFocus)
	{
	Assert(piTreeItemFocus != NULL);
	Assert(PGetRuntimeInterfaceOf_ITreeItem(piTreeItemFocus) == piTreeItemFocus);
	return eNoticeActionDisplay;
	}

//	FDeleteNoticeBecauseDeletingTreeItem(), virtual
//
//	This virtual method is called every time an object inheriting from ITreeItem is being deleted.
//	This gives the opportunity to the notice to delete itself if having dependencies on the deleted Tree Item.
BOOL
INotice::FDeleteNoticeBecauseDeletingTreeItem(const ITreeItem * piTreeItemDeleting)
	{
	Assert(piTreeItemDeleting != NULL);
	Assert(PGetRuntimeInterfaceOf_ITreeItem(piTreeItemDeleting) == piTreeItemDeleting);
	Endorse(mu_sponsor.piTreeItem == NULL);
	return (piTreeItemDeleting == mu_sponsor.piTreeItem);
	}

#if 0
void
WNotice::paintEvent(QPaintEvent *)
	{
	QPainter p(this);
	p.setPen(d_coOrange);
	p.drawRect(2, 3, width() - 7, height() - 4);
	/*
	QStyleOptionToolButton opt;
	opt.init(this);
	opt.state |= QStyle::State_AutoRaise;
	style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);
	style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
	*/
	}

void
WNotice::resizeEvent(QResizeEvent *)
	{
	m_pButtonClose->move(width() - 20, 10);
	//m_pButtonClose->move(10, 10);
	}
#endif

void
INotice::contextMenuEvent(QContextMenuEvent *)
	{
	WMenu oMenu;
	oMenu.ActionAdd(eMenuAction_Close);
	EMenuAction eMenuAction = oMenu.EDisplayContextMenu();
	if (eMenuAction == eMenuAction_Close)
		delete this;
	}

void
INotice::ConnectButton(QAbstractButton * pwButton, PFmSlot pfmSlot)
	{
	ConnectButton_T(pwButton, pfmSlot);
	}

template<typename PFmSlot_T>
void
INotice::ConnectButton_T(QAbstractButton * pwButton, PFmSlot_T pfmSlot)
	{
	QObject::connect(pwButton, &QAbstractButton::clicked, this, pfmSlot);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
INoticeWithLayout::INoticeWithLayout()
	{
	m_poLayoutBody = new OLayoutHorizontal(PA_PARENT this);	// This layout is essential so the widget WGroupBox expands horizontally
	m_poLayoutBody->setContentsMargins(4, 4, 4, 1);			// Add a few pixels 'around' the widget WGroupBox (otherwise the WGroupBox will expand too close to the edges of its parent widget)
	m_pwGroupBox = new WGroupBox(PA_PARENT m_poLayoutBody);
	m_poLayoutNotice = new OLayoutVertical(PA_PARENT m_pwGroupBox);
	m_poLayoutNotice->setContentsMargins(5, 0, 5, 5);
	m_poLayoutBody = new OLayoutHorizontal(PA_PARENT m_poLayoutNotice);
	m_poLayoutButtons = NULL;
	//m_poLayoutNotice->addLayout(m_poLayoutBody);
	//m_poLayoutBody->setContentsMargins(5, 0, 5, 5);
	}

OLayoutHorizontal *
INoticeWithLayout::Notice_PoAddLayoutHorizontal()
	{
	m_poLayoutNotice->setSpacing(4);	// By default, the spacing is 6 pixels, however 4 makes it look better
	setMaximumHeight(maximumHeight() + d_cyHeightNoticeExtraLayout);

	OLayoutHorizontal * pLayout = new OLayoutHorizontal(PA_PARENT m_poLayoutNotice);
	/*
	//m_poLayoutNotice->setSpacing();
	QMargins m = pLayout->contentsMargins();
	MessageLog_AppendTextFormatCo(d_coBlack, "left=$i, right=$i, top=$i, bottom=$i, spacing=$i\n,", m.left(), m.right(), m.top(), m.bottom(), m_poLayoutNotice->spacing());
	*/
	// m_poLayoutNotice->addLayout(pLayout);
	return pLayout;
	}

WLabelSelectable *
INoticeWithLayout::Notice_PwAddLayoutHorizontalWithLabel()
	{
	return new WLabelSelectableWrap(PA_PARENT Notice_PoAddLayoutHorizontal());
	}

void
INoticeWithLayout::Notice_LayoutBodyMoveToBottom()
	{
	m_poLayoutNotice->removeItem(m_poLayoutBody);
	m_poLayoutNotice->addLayout(m_poLayoutBody);
	}

OLayoutVertical *
INoticeWithLayout::Notice_PoGetLayoutButtons_NZ()
	{
	if (m_poLayoutButtons == NULL)
		m_poLayoutButtons = new OLayoutVerticalAlignTop(m_poLayoutBody);
	return m_poLayoutButtons;
	}

void
INoticeWithLayout::Notice_AddButton(PA_CHILD WButtonText * pawButton, INotice::PFmSlot pfmSlotButton)
	{
	Notice_PoGetLayoutButtons_NZ()->addWidget(PA_CHILD pawButton);
	ConnectButton(pawButton, pfmSlotButton);
	}

void
INoticeWithLayout::Notice_AddButtonIconClose()
	{
	//Notice_AddButton(PA_CHILD new WButtonTextWithIcon(pszButtonText, eMenuAction_Close), SL_INoticeWithLayout(SL_ButtonClose_clicked));
	WButtonIconForToolbar * pwButton = new WButtonIconForToolbar(eMenuAction_Close, "Hide this notification message");
	Notice_PoGetLayoutButtons_NZ()->addWidget(PA_CHILD pwButton);
	ConnectButton(pwButton, SL_INoticeWithLayout(SL_ButtonClose_clicked));
	}

void
INoticeWithLayout::SL_ButtonClose_clicked()
	{
	deleteLater(PA_DELETING);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
INoticeWithIcon::INoticeWithIcon(PSZAC pszTitle, EMenuAction eMenuIcon)
	{
	m_pwGroupBox->setTitle(pszTitle);
	m_pwButtonIcon = new WButtonIconForToolbar(this, eMenuIcon);
	m_poLayoutBody->addWidget(m_pwButtonIcon);
	m_pwLabelMessage = new WLabelSelectableWrap(PA_PARENT m_poLayoutBody);
	m_pwLabelMessageExtra = NULL;
	}
/*
void
INoticeWithIcon::Notice_SetIcon(EMenuAction eMenuActionIcon)
	{
	}
*/

void
INoticeWithIcon::NoticeMessage_SetText_VE(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	NoticeMessage_SetText_VL(pszFmtTemplate, vlArgs);
	}

void
INoticeWithIcon::NoticeMessage_SetText_VL(PSZAC pszFmtTemplate, va_list vlArgs)
	{
	m_pwLabelMessage->Label_SetTextFormat_VL_Gsb(pszFmtTemplate, vlArgs);
	}

void
INoticeWithIcon::NoticeMessageExtra_SetText_VE(PSZAC pszFmtTemplate, ...)
	{
	if (m_pwLabelMessageExtra == NULL)
		m_pwLabelMessageExtra = Notice_PwAddLayoutHorizontalWithLabel();
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	m_pwLabelMessageExtra->Label_SetTextFormat_VL_Gsb(pszFmtTemplate, vlArgs);
	m_pwLabelMessageExtra->show();
	}

void
INoticeWithIcon::NoticeMessageExtra_Hide()
	{
	if (m_pwLabelMessageExtra != NULL)
		m_pwLabelMessageExtra->hide();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WNoticeMessageSponsored::WNoticeMessageSponsored(ITreeItem * pSponsor, PSZAC pszTitle, EMenuAction eMenuIcon) : INoticeWithIcon(pszTitle, eMenuIcon)
	{
	Assert(pSponsor != NULL);
	mu_sponsor.piTreeItem = pSponsor;
	}

BOOL
WNoticeMessageSponsored::FDeleteDuplicateNotice(INOUT INotice * piwNoticeNew)
	{
	Assert(piwNoticeNew->EGetRuntimeClass() == RTI(WNoticeMessageSponsored));
	if (mu_sponsor.piTreeItem == piwNoticeNew->mu_sponsor.piTreeItem)
		{
		// A notice is a duplicate if it has an identical message to another notice with the same sponsor
		return (m_pwLabelMessage->text() == ((WNoticeMessageSponsored *)piwNoticeNew)->m_pwLabelMessage->text());
		}
	return FALSE;
	}

//	Display the message only if it is relevant
ENoticeAction
WNoticeMessageSponsored::ETreeItemGotFocus(IN ITreeItem * piTreeItemFocus)
	{
	Assert(mu_sponsor.piTreeItem != NULL);
	return (ENoticeAction)(PGetRuntimeInterfaceOf_(piTreeItemFocus, mu_sponsor.piTreeItem->EGetRuntimeClass()) == mu_sponsor.piTreeItem);
	}



///////////////////////////////////////////////////////////////////////////////////////////////////
WNoticeWarning::WNoticeWarning()
	{
	m_pwLabelStatus = NULL;
	WButtonIconForToolbar * pButton = new WButtonIconForToolbar(this, eMenuIconWarning);
	/*
	QIcon oIcon = QIcon(":/ico/Warning");
	pButton->setIcon(oIcon);
	pButton->setIconSize(QSize(32, 32));
	pButton->setStyleSheet("QToolButton { border: none; padding: 1px; }");
	*/
	m_poLayoutBody->addWidget(pButton, d_zNA, Qt::AlignTop);
	m_pwLabelWarning = new WLabelSelectableWrap(PA_PARENT m_poLayoutBody);
	}

void
WNoticeWarning::NoticeWarning_SetTextFormat_VE(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	NoticeWarning_SetTextFormat_VL(pszFmtTemplate, vlArgs);
	}

void
WNoticeWarning::NoticeWarning_SetTextFormat_VL(PSZAC pszFmtTemplate, va_list vlArgs)
	{
	m_pwLabelWarning->Label_SetTextFormat_VL_Gsb(pszFmtTemplate, vlArgs);
	}

void
WNoticeWarning::NoticeStatus_SetTextFormat_VE(PSZAC pszFmtTemplate, ...)
	{
	if (m_pwLabelStatus == NULL)
		m_pwLabelStatus = Notice_PwAddLayoutHorizontalWithLabel();
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	m_pwLabelStatus->Label_SetTextFormat_VL_Gsb(pszFmtTemplate, vlArgs);
	m_pwLabelStatus->show();
	}

void
WNoticeWarning::NoticeStatus_Hide()
	{
	if (m_pwLabelStatus != NULL)
		m_pwLabelStatus->hide();
	}


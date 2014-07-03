
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
DDialog::DDialog() : QDialog(g_pwMainWindow)
	{
	}

DDialog::~DDialog()
	{
	SetCursorRestoreDefault();	// Just make sure we do not have a Wait cursor after the dialog is closed
	}

void
DDialog::Dialog_SetCaption(PSZAC pszCaption)
	{
	Widget_SetText(INOUT this, pszCaption);
	}

void
DDialog::Dialog_SetCaption(const CStr & strCaption)
	{
	Widget_SetText(INOUT this, strCaption);
	}

void
DDialog::Dialog_SetCaptionFormat_VE(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	Widget_SetTextFormat_VL_Gsb(INOUT this, pszFmtTemplate, vlArgs);
	}

void
DDialog::Dialog_SetIcon(EMenuAction eMenuIcon)
	{
	Widget_SetIcon(INOUT this, eMenuIcon);
	}

void
DDialog::Dialog_SetSizeMinimum(int cxWidthMinimum, int cyHeightMinimum)
	{
	QSize sizeMinimum = minimumSizeHint();
	if (cxWidthMinimum > d_zDEFAULT)
		sizeMinimum.setWidth(cxWidthMinimum);
	if (cyHeightMinimum > d_zDEFAULT)
		sizeMinimum.setHeight(cyHeightMinimum);
	setMinimumSize(sizeMinimum);	// Make sure the dialog is never smaller than its minimum recommended size
	}

void
DDialog::Dialog_SetSizeFixed(int cxWidth, int cyHeight)
	{
	setFixedSize(cxWidth, cyHeight);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);	// This line is important otherwise Qt will show the mouse cursor to resize the dialog despite the dialog being of fixed size
	}

BOOL
DDialog::FuExec()
	{
	return (BOOL)exec();	// The virtual method exec() returns either Accepted=1 or Rejected=0
	}

WButtonText *
DDialog::PwAllocateButton(PSZAC pszTextButton, PFmSlot pfmSlot)
	{
	return (WButtonText *)PwAllocateButton_T(pszTextButton, pfmSlot);
	}

template<typename PFmSlot_T>
WButtonText *
DDialog::PwAllocateButton_T(PSZAC pszTextButton, PFmSlot_T pfmSlot)
	{
	WButtonText * pwButton = new WButtonText(pszTextButton);
	pwButton->setParent(this);
	QObject::connect(pwButton, &QPushButton::clicked, this, pfmSlot);
	return pwButton;
	}

DDialogOkCancel::DDialogOkCancel()
	{
	m_pwButtonOK = NULL;
	m_pwButtonCancel = NULL;
	//CreateButtonsOkCancel(SL_DDialogOkCancel(SL_ButtonOK_clicked), SL_DDialogOkCancel(SL_ButtonCancel_clicked));
	/*
	m_pwButtonOK = PwAllocateButton("OK", SL_DDialogOkCancel(SL_ButtonOK_clicked));
	m_pwButtonCancel = PwAllocateButton("Cancel", SL_DDialogOkCancel(SL_ButtonCancel_clicked));
	*/
	//m_pwButtonOK = new WButtonText("OK");
	//m_pwButtonCancel = new WButtonText("Cancel");
	}

DDialogOkCancel::~DDialogOkCancel()
	{
	}

void
DDialogOkCancel::Dialog_CreateButtonsOkCancel()
	{
	Dialog_CreateButtonsOkCancel(NULL, NULL);
	}

//#define SL_DDialogOkCancel(pfmSlot)		SL_DDialog(pfmSlot, DDialogOkCancel)
#define SL_DDialogButtonOk()				SL_DDialog(SL_ButtonOK_clicked, DDialogOkCancel)
void
DDialogOkCancel::Dialog_CreateButtonsOkCancel(PFmSlot pfmSlotOk, PFmSlot pfmSlotCancel)
	{
	Assert(m_pwButtonOK == NULL && "Memory Leak!");
	Assert(m_pwButtonCancel == NULL && "Memory Leak!");
	m_pwButtonOK = PwAllocateButton("OK", (pfmSlotOk != NULL) ? (DDialog::PFmSlot)pfmSlotOk : SL_DDialogOkCancel(SL_ButtonOK_clicked));
	m_pwButtonCancel = PwAllocateButton("Cancel", (pfmSlotCancel != NULL) ? (DDialog::PFmSlot)pfmSlotCancel : SL_DDialogOkCancel(SL_ButtonCancel_clicked));
	m_pwButtonOK->setDefault(true);
	}

void
DDialogOkCancel::SL_ButtonOK_clicked()
	{
	accept();
	}

void DDialogOkCancel::SL_ButtonCancel_clicked()
	{
	reject();
	}

DDialogOkCancelWithLayouts::DDialogOkCancelWithLayouts(PSZAC pszCaption, EMenuAction eMenuIcon)
	{
	Dialog_SetCaption(pszCaption);
	Dialog_SetIcon(eMenuIcon);
	m_poLayoutDialog = new OLayoutVerticalAlignTop(PA_PARENT this);
	m_poLayoutBodyAndButtons = new OLayoutHorizontal(PA_PARENT m_poLayoutDialog);
	m_poLayoutBody = new OLayoutVerticalAlignTop(PA_PARENT m_poLayoutBodyAndButtons);
	m_poLayoutButtons = new OLayoutVerticalAlignTop(PA_PARENT m_poLayoutBodyAndButtons);
//	m_poLayoutBody->setAlignment(Qt::AlignTop);
//	m_poLayoutButtons->setAlignment(Qt::AlignTop);	// Buttons are aligned to the top (they are not stretched vertically)
	}

DDialogOkCancelWithLayouts::~DDialogOkCancelWithLayouts()
	{

	}

void
DDialogOkCancelWithLayouts::Dialog_AddButtonsOkCancel(DDialogOkCancel::PFmSlot pfmSlotOk, DDialogOkCancel::PFmSlot pfmSlotCancel)
	{
	Dialog_AddButtonsOkCancel_VEZ(pfmSlotOk, pfmSlotCancel);
	}

void
DDialogOkCancelWithLayouts::Dialog_AddButtonsOkCancel_VEZ(DDialogOkCancel::PFmSlot pfmSlotOk, DDialogOkCancel::PFmSlot pfmSlotCancel, PA_CHILDREN QWidget * pawButtonExtra, ...)
	{
	Dialog_CreateButtonsOkCancel(pfmSlotOk, pfmSlotCancel);
	va_list vlArgs;
	va_start(OUT vlArgs, pawButtonExtra);
	#if 1
	m_poLayoutButtons->addWidget(m_pwButtonOK);
	m_poLayoutButtons->addWidget(m_pwButtonCancel);
	if (pawButtonExtra != NULL)
		{
		va_list vlArgs;
		va_start(OUT vlArgs, pawButtonExtra);
		m_poLayoutButtons->Layout_AddWidgetsAndResizeWidths_VLZA(pawButtonExtra, vlArgs);
		}
	#else
	//m_poLayoutButtons->Layout_AddWidgetsAndResizeWidths_VLZA(pawButtonExtra, vlArgs);
	#endif
	m_poLayoutButtons->Layout_ResizeWidthsOfAllWidgets();
	}
//	Add the buttons OK and Cancel, however with the OK button renamed to custom text
void
DDialogOkCancelWithLayouts::Dialog_AddButtonsOkCancel_RenameButtonOk(DDialogOkCancel::PFmSlot pfmSlotOk, PSZAC pszmTextAndToolTipButtonOk, EMenuAction eMenuIconButtonOK)
	{
	Dialog_AddButtonsOkCancel(pfmSlotOk);
	Dialog_RenameButtonOk(pszmTextAndToolTipButtonOk, eMenuIconButtonOK);
	}

void
DDialogOkCancelWithLayouts::Dialog_RenameButtonOk(PSZAC pszmTextAndToolTipButtonOk, EMenuAction eMenuIconButtonOK)
	{
	m_pwButtonOK->Button_SetTextAndToolTip(pszmTextAndToolTipButtonOk);
	if (eMenuIconButtonOK != ezMenuActionNone)
		m_pwButtonOK->Button_SetIcon(eMenuIconButtonOK);
	m_poLayoutButtons->Layout_ResizeWidthsOfAllWidgets();
	}

void
DDialogOkCancelWithLayouts::Dialog_AddRowWidget_PA(PSZAC pszmLabelAndToolTip, QWidget * pawWidget)
	{
	Assert(pawWidget != NULL);
	m_poLayoutDialog->Layout_PoAddRowLabelsAndWidgets_VEZA(pszmLabelAndToolTip, pawWidget, NULL);
	}

WLabel *
DDialogOkCancelWithLayouts::DialogBody_PwAddRowLabel(PSZAC pszmLabelTextAndToolTip)
	{
	return m_poLayoutBody->Layout_PwAddRowLabel(pszmLabelTextAndToolTip);
	}

void
DDialogOkCancelWithLayouts::DialogBody_AddRowWidget_PA(QWidget * pawWidget)
	{
	m_poLayoutBody->addWidget(PA_CHILD pawWidget);
	}

void
DDialogOkCancelWithLayouts::DialogBody_AddRowWidget_PA(PSZAC pszmLabelAndToolTip, QWidget * pawWidget)
	{
	Assert(pawWidget != NULL);
	m_poLayoutBody->Layout_PoAddRowLabelsAndWidgets_VEZA(pszmLabelAndToolTip, pawWidget, NULL);
	}

void
DDialogOkCancelWithLayouts::DialogBody_AddRowWidgets_PA(PSZAC pszLabel, QWidget * pawWidget, PSZAC pszLabel2, QWidget * pawWidget2)
	{
	m_poLayoutBody->Layout_PoAddRowLabelsAndWidgets_VEZA(pszLabel, pawWidget, pszLabel2, pawWidget2, NULL);
	}

void
DDialogOkCancelWithLayouts::DialogBody_AddRowWidgets_VEZA(PSZAC pszmFirstLabelAndToolTip, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszmFirstLabelAndToolTip);
	m_poLayoutBody->Layout_PoAddRowLabelsAndWidgets_VLZA(pszmFirstLabelAndToolTip, vlArgs);
	}


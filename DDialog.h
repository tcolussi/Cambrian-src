
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#ifndef DDIALOG_H_COMPILED

#define SL_TYPEDEF(class_name)		public: typedef void (class_name::*PFmSlot)();	// Macro to define the prototype of a slot for a given class

#define SL_CAST(pfmSlot, class_name, base_class)			((base_class::PFmSlot)(class_name::PFmSlot)&class_name::pfmSlot)
#define SL_DDialog(pfmSlot, dialog_class)					SL_CAST(pfmSlot, dialog_class, DDialog)
#define SL_DDialogButtonClicked(_pfmName, dialog_class)		SL_DDialog(SL_Button##_pfmName##_clicked, dialog_class)

class DDialog : public QDialog
{
public:
	DDialog();
	virtual ~DDialog();
	void Dialog_SetCaption(PSZAC pszCaption);
	void Dialog_SetCaption(const CStr & strCaption);
	void Dialog_SetCaptionFormat_VE(PSZAC pszFmtTemplate, ...);
	void Dialog_SetIcon(EMenuIcon eMenuIcon);
	void Dialog_SetSizeMinimum(int cxWidthMinimum = d_zDEFAULT, int cyHeightMinimum = d_zDEFAULT);
	void Dialog_SetSizeFixed(int cxWidth, int cyHeight);
	BOOL FuExec();

	// Special methods to parametrize the slots
	SL_TYPEDEF(DDialog)
	WButtonText * PwAllocateButton(PSZAC pszTextButton, PFmSlot pfmSlot);
	template<typename PFmSlot_T>
	WButtonText * PwAllocateButton_T(PSZAC pszTextButton, PFmSlot_T pfmSlot);
}; // DDialog

//	Dialog having the OK and Cancel buttons
//	This dialog reduces code duplicate, and more importantly, allow the buttons to be positioned on different platforms.  For instance, on a Mac, the Ok and Cancel button are horizontally swapped compared to the Windows platform.
class DDialogOkCancel : public DDialog
{
	SL_TYPEDEF(DDialogOkCancel)
protected:
	WButtonText * m_pwButtonOK;
	WButtonText * m_pwButtonCancel;
public:
	DDialogOkCancel();
	virtual ~DDialogOkCancel();
	void Dialog_CreateButtonsOkCancel();
	void Dialog_CreateButtonsOkCancel(PFmSlot pfmSlotOk, PFmSlot pfmSlotCancel = NULL);
public slots:
	void SL_ButtonOK_clicked();
	void SL_ButtonCancel_clicked();
}; // DDialogOkCancel

#define SL_DDialogOkCancel(pfmSlot)		SL_DDialog(pfmSlot, DDialogOkCancel)
//	Typical dialog with an OK and Cancel with a layout to display the buttons on the top right of the dialog.
class DDialogOkCancelWithLayouts : public DDialogOkCancel
{
protected:
	OLayoutVertical * m_poLayoutDialog;		// Core layout allowing the dialog to display multiple widgets vertically
	OLayoutHorizontal * m_poLayoutBodyAndButtons;	// Layout creating two columns, one for the body and one for the buttons
	OLayoutVertical * m_poLayoutBody;				// Layout for the body located on the left
	OLayoutVertical * m_poLayoutButtons;			// Layout for the buttons located on the right.  This pointer is necessary in case we wish to add more buttons
public:
	DDialogOkCancelWithLayouts(PSZAC pszCaption, EMenuIcon eMenuIcon);
	virtual ~DDialogOkCancelWithLayouts();
	void Dialog_AddButtonsOkCancel(DDialogOkCancel::PFmSlot pfmSlotOk = NULL, DDialogOkCancel::PFmSlot pfmSlotCancel = NULL);
	void Dialog_AddButtonsOkCancel_VEZ(DDialogOkCancel::PFmSlot pfmSlotOk = NULL, DDialogOkCancel::PFmSlot pfmSlotCancel = NULL, PA_CHILDREN QWidget * pawButtonExtra = NULL, ...);
	void Dialog_AddButtonsOkCancel_RenameButtonOk(DDialogOkCancel::PFmSlot pfmSlotOk, PSZAC pszmTextAndToolTipButtonOk, EMenuIcon eMenuIconButtonOK = eMenuIcon_zNull);
	void Dialog_RenameButtonOk(PSZAC pszmTextAndToolTipButtonOk, EMenuIcon eMenuIconButtonOK = eMenuIcon_zNull);
	void Dialog_AddRowWidget_PA(PSZAC pszmLabelAndToolTip, QWidget * pawWidget);
	void DialogBody_AddRowWidget_PA(QWidget * pawWidget);
	void DialogBody_AddRowWidget_PA(PSZAC pszmLabelAndToolTip, QWidget * pawWidget);
	void DialogBody_AddRowWidgets_PA(PSZAC pszLabel, QWidget * pawWidget, PSZAC pszLabel2, QWidget * pawWidget2);
	void DialogBody_AddRowWidgets_VEZA(PSZAC pszLabel, ...);
	WLabel * DialogBody_PwAddRowLabel(PSZAC pszmLabelTextAndToolTip);
}; // DDialogOkCancelWithLayouts

#define DDIALOG_H_COMPILED
#endif // DDIALOG_H_COMPILED

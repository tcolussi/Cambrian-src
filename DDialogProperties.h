//	DDialogProperties.h
//
//	Core classes to create "property pages" as well as the implementation of a few of them.

#ifndef DDIALOGPROPERTIES_H
#define DDIALOGPROPERTIES_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class DDialogPropertyPage : public QListWidgetItem, public WWidget
{
public:
	DDialogPropertyPage(PSZAC pszName);
};

//	Base class to display properties of an object
//	This class uses a QListWidget to display the pages (instead of using tabs), thus allowing the possibility to have more pages and have them easily selected (because when there are many pages, the tabs tabs become very small and the text is truncated)
class DDialogProperties : public DDialogOkCancel
{
	Q_OBJECT
protected:
	QListWidget * m_pwPagesList;		// Widget displaying all the property pages
	QStackedWidget * m_pwPagesStacked;
	OLayoutHorizontal * m_poLayoutButtons;	// Layout displaying all the buttons: OK, Cancel and other buttons (if any)

public:
	DDialogProperties();
	void PageAdd(DDialogPropertyPage * pawPage);
	virtual int exec();

public slots:
	void SL_PageChanged(QListWidgetItem * pPageCurrent, QListWidgetItem * pPagePrevious);
	void SL_ButtonOK_clicked();
	void SL_ButtonCancel_clicked();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class DDialogPropertiyPageAccountGeneral : public DDialogPropertyPage
{
protected:
	TAccountXmpp * m_pAccount;
	WEditPassword * m_pwEditPassword;
public:
	DDialogPropertiyPageAccountGeneral(TAccountXmpp * pAccount);
};

class DDialogPropertiyPageAccountTest : public DDialogPropertyPage
{
public:
	DDialogPropertiyPageAccountTest(PSZAC pszName);
};

class DDialogPropertiesAccount : public DDialogProperties
{
protected:
	TAccountXmpp * m_pAccount;
public:
	DDialogPropertiesAccount(TAccountXmpp * pAccount);
};


///////////////////////////////////////////////////////////////////////////////////////////////////
class DDialogPropertiyPageContactGeneral : public DDialogPropertyPage
{
protected:
	TContact * m_pContact;
public:
	DDialogPropertiyPageContactGeneral(TContact * pContact);
};

class DDialogPropertiesContact : public DDialogProperties
{
protected:
	TContact * m_pContact;
public:
	DDialogPropertiesContact(TContact * pContact);
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class DDialogPropertiyPageGroupGeneral : public DDialogPropertyPage
{
protected:
	TGroup * m_pGroup;
public:
	DDialogPropertiyPageGroupGeneral(TGroup * pGroup);
};

class DDialogPropertiesGroup : public DDialogProperties
{
protected:
	TGroup * m_pGroup;
public:
	DDialogPropertiesGroup(TGroup * pGroup);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Widget displaying a list of contacts
class WListContacts : public QListWidget
{
public:
	QIcon m_oIconContact;

public:
	WListContacts();
	void ContactAdd(TContact * pContact);
	void ContactsAdd(IN_MOD_SORT CArrayPtrContacts & arraypContacts);
	void ContactAddTransfer(PA_DELETING QListWidgetItem * paItem);
	void ContactRemove(TContact * pContact);
	void ContactsGetAll(IOUT CArrayPtrContacts * parraypContacts) const;
};
class DDialogGroupAddContacts : public DDialogOkCancelWithLayouts
{
	Q_OBJECT
protected:
	ITreeItemChatLogEvents * m_pContactOrGroup;
	TGroup * m_pGroup;							// Group to add contacts (if this pointer is NULL, then create a new group)
	WListContacts * m_pwListContactsAvailable;
	WListContacts * m_pwListContactsInGroup;

	// Array of contacts when the dialog was initialized
	CArrayPtrContacts m_arraypContactsInGroup;
	CArrayPtrContacts m_arraypContactsAvailable;

public:
	DDialogGroupAddContacts(ITreeItemChatLogEvents * pContactOrGroup);
	~DDialogGroupAddContacts();

protected slots:
	void SL_ContactAvailableDoubleClicked(QListWidgetItem * paItem);
	void SL_ContactInGroupDoubleClicked(QListWidgetItem * paItem);
	void SL_ButtonOK();
};


//	Array of pointers of QObject
class CArrayPtrQObjects : public CArray
{
};

class CArrayPtrQWidgets : public CArrayPtrQObjects
{
public:
	inline QWidget ** PrgpGetWidgetsStop(OUT QWidget *** pppWidgetStop) const { return (QWidget **)PrgpvGetElementsStop(OUT (void ***)pppWidgetStop); }

};

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "IEventBallot.h"

class DDialogBallotSend;

class _OLayoutBallotChoice : public OLayoutHorizontal
{
public:
	WButtonIcon * m_pwButtonRemove;	// Remove the choice on the ballot
	WEdit * m_pwEditChoice;			// Widget to enter the text choice
	WEdit * m_pwEditGroup;

public:
	_OLayoutBallotChoice(DDialogBallotSend * pwDialogBallotParent);
	~_OLayoutBallotChoice();
};


class CArrayPtrLayoutsBallotChoices : public CArrayPtrQObjects
{
public:
	inline _OLayoutBallotChoice ** PrgpGetQuestionsStop(OUT _OLayoutBallotChoice *** pppLayoutStop) const { return (_OLayoutBallotChoice **)PrgpvGetElementsStop(OUT (void ***)pppLayoutStop); }
};

class DDialogBallotSend : public DDialogOkCancelWithLayouts
{
public:
	ITreeItemChatLogEvents * m_pContactOrGroup;
	CArrayPtrLayoutsBallotChoices m_arraypLayoutChoices;
	WEdit * m_pwEditTitle;
	WEditTextArea * m_pwEditDescription;
	OLayoutVertical * m_pLayoutQuestions;
	WButtonCheckbox * m_pwButtonAllowMultipleChoices;
	WButtonCheckbox * m_pwButtonAllowComments;


public:
	DDialogBallotSend(ITreeItemChatLogEvents * pContactOrGroup, CEventBallotSent * pEventBallotInit = NULL);
	~DDialogBallotSend();
	CEventBallotSent * PaAllocateBallot();

protected slots:
	void SL_buttonBallotPreview();
	void SL_ButtonBallotSend();
	void SL_ButtonAdd();
	void SL_ButtonRemove();

	SL_OBJECT(DDialogBallotSend)
}; // DDialogBallotSend


class DDialogBallotVote : public DDialogOkCancelWithLayouts
{
protected:
	BOOL m_fPreviewMode;
	CEventBallotReceived * m_pEventBallotVote;
	CArrayPtrQWidgets m_arraypwButtonsChoices;
	WEditTextArea * m_pwEditComments;
public:
	DDialogBallotVote(CEventBallotReceived * pEventBallotVote, BOOL fPreviewMode);

protected slots:
	void SL_ButtonVote();
	SL_OBJECT(DDialogBallotVote)
};

#endif // DDIALOGPROPERTIES_H

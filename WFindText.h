#ifndef WFINDTEXT_H
#define WFINDTEXT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class WFindText : public QWidget
{
	Q_OBJECT
protected:
	QTextDocument * m_poDocument;	// Document to search the text
	QWidget * m_pwFocusOnHide;		// Widget to give the focus when calling the Hide() method
	QLineEdit * m_pwLineEdit;
	QLabel * m_pwLabelInstances;	// Label to display to the user how many instances of the text was found
	int m_cInstancesFound;			// Counter of how many instances have been found.  This counter is also used to 'undo' the highlighting
public:
	WFindText(QTextDocument * poDocument, QWidget * pwFocusOnHide);
	void Show();
	void Hide();
	virtual bool event(QEvent * pEvent);

public slots:
	void SL_TextChanged(const QString & sText);
};

#endif // WFINDTEXT_H

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WFindText.h"

WFindText::WFindText(QTextDocument * poDocument, QWidget * pwFocusOnHide)
	{
	Assert(poDocument != NULL);
	m_poDocument = poDocument;
	m_pwFocusOnHide = pwFocusOnHide;
	m_cInstancesFound = 0;
	m_pwLineEdit = new QLineEdit;
	m_pwLabelInstances = new QLabel;
	m_pwLabelInstances->setFixedWidth(100);
	QHBoxLayout * pLayout = new QHBoxLayout();
	pLayout->setContentsMargins(10, 0, 10, 0);
	pLayout->addWidget(PA_CHILD new QLabel("Find:"));
	pLayout->addWidget(PA_CHILD m_pwLineEdit);
	pLayout->addWidget(PA_CHILD m_pwLabelInstances);
	setLayout(PA_CHILD pLayout);
	setFixedHeight(20);
	connect(m_pwLineEdit, SIGNAL(textChanged(QString)), this, SLOT(SL_TextChanged(QString)));
	}


void
WFindText::Show()
	{
	m_poDocument->setUndoRedoEnabled(true);		// We use the undo feature to remove highlighted text
	show();
	SL_TextChanged(m_pwLineEdit->text());		// Highlight anything already in the text edit
	//m_pwLineEdit->setFocus(Qt::ShortcutFocusReason);
	m_pwLineEdit->setFocus();
	}

void
WFindText::Hide()
	{
	m_poDocument->undo();	// Remove any previous  highlighted text
	m_poDocument->setUndoRedoEnabled(false);
	hide();
	m_cInstancesFound = 0;
	m_pwFocusOnHide->setFocus();
	}

//	WFindText::QWidget::event()
bool
WFindText::event(QEvent * pEvent)
	{
	if (pEvent->type() == QEvent::KeyPress)
		{
		QKeyEvent * pEventKey = static_cast<QKeyEvent *>(pEvent);
		if (pEventKey->key() == Qt::Key_Escape)
			{
			Hide();
			}
		}
	return QWidget::event(pEvent);
	}

void
WFindText::SL_TextChanged(const QString & sText)
	{
	MessageLog_AppendTextFormatCo(d_coBlack, "SL_TextChanged($Q)\n", &sText);

	if (m_cInstancesFound > 0)
		{
		m_cInstancesFound = 0;
		m_poDocument->undo();	// Undo any previous highlighting
		}
	CStr strTextInstancesFound;
	const int cchText = sText.length();
	if (cchText > 0)
		{
		QTextCursor oTextCursorHighlight(m_poDocument);
		QTextCursor oTextCursorDocument(m_poDocument);	// We need two QTextCursor for the operation: one for the undo of the document and another to highlight the searched text
		oTextCursorDocument.beginEditBlock();
		QTextCharFormat oFormatHighlight(oTextCursorHighlight.charFormat());
		oFormatHighlight.setForeground(Qt::red);
		oFormatHighlight.setBackground(Qt::yellow);
		while (TRUE)
			{
			oTextCursorHighlight = m_poDocument->find(sText, oTextCursorHighlight);
			if (oTextCursorHighlight.isNull())
				break;	// Unable to find text
			oTextCursorHighlight.mergeCharFormat(oFormatHighlight);
			oTextCursorHighlight.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, cchText);
			m_cInstancesFound++;
			if (oTextCursorHighlight.atEnd())
				{
				MessageLog_AppendTextFormatCo(d_coRed, "oTextCursorHighlight.atEnd()\n");
				break;	// We reached the end of the document
				}
			} // while
		oTextCursorDocument.endEditBlock();
		strTextInstancesFound.Format("$i found", m_cInstancesFound);
		} // if
	m_pwLabelInstances->setText(strTextInstancesFound);
	} // SL_TextChanged()

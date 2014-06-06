///////////////////////////////////////////////////////////////////////////////////////////////////
//	WLayoutContainer.cpp
//
//	The purpose of this widget is holding a WLayout widget when the user select an item from the navigation tree.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WLayoutContainer.h"

WLayoutContainer::WLayoutContainer() : QSplitter(g_pwMainWindow)
	{
	setObjectName("Container");
	m_pawLayoutEmpty_NZ = new WLayout;
	m_pwLayoutVisible_NZ = m_pawLayoutEmpty_NZ;
	m_pawLayoutAutoDelete = NULL;
	}

void
WLayoutContainer::SetLayout(IN WLayout * pwLayout)
	{
	Assert(m_pawLayoutAutoDelete == NULL || m_pawLayoutAutoDelete == m_pwLayoutVisible_NZ);
	if (pwLayout == NULL)
		pwLayout = m_pawLayoutEmpty_NZ;
	if (pwLayout == m_pwLayoutVisible_NZ)
		return;	// Nothing to do
	m_pwLayoutVisible_NZ->setParent(NULL);	// There is no method called removeWidget(), however setting the parent to NULL removes m_pwLayoutVisible from the widget from the splitter.
	m_pwLayoutVisible_NZ = pwLayout;
	addWidget(pwLayout);
	if (m_pawLayoutAutoDelete != NULL)
		{
		m_pawLayoutAutoDelete->deleteLater(PA_DELETING);	// This is imporant to use deleteLater() rather than the delete operator because sometimes the layout changes focus while processing a signal, and the application would crash.
		m_pawLayoutAutoDelete = NULL;
		}
	}

void
WLayoutContainer::SetLayoutAutoDelete(PA_CHILD WLayout * pawLayoutAutoDelete)
	{
	Assert(pawLayoutAutoDelete != NULL);
	SetLayout(IN pawLayoutAutoDelete);
	Assert(m_pawLayoutAutoDelete == NULL && "Memory Leak!");
	m_pawLayoutAutoDelete = pawLayoutAutoDelete;
	}

void
WLayoutContainer::DeleteLayout(PA_DELETING WLayout * pawLayoutToDelete)
	{
	Endorse(pawLayoutToDelete == NULL);
	if (pawLayoutToDelete == m_pwLayoutVisible_NZ)
		SetLayout(NULL);			// Make sure the layout we are about to delete is not selected, otherwise it will crash the application.
	delete pawLayoutToDelete;
	}

/*
void
WLayoutContainer::AddMessages(IN WNotice * pwMessageList)
	{
	Assert(m_pwLayoutVisible != NULL);
	if (m_pwLayoutVisible != NULL)
		m_pwLayoutVisible->Splitter_AddWNotices(IN pwMessageList);
	}
*/
/*
void
WLayoutContainer::paintEvent(QPaintEvent *)
	{
	QPainter p(this);
	p.setPen(QColor(255, 0, 0));
	p.drawRect(0, 0, width() - 1, height() - 1);
	}
*/

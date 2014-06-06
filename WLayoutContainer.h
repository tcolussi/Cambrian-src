///////////////////////////////////////////////////////////////////////////////////////////////////
//	WLayoutContainer.h
//
//	The purpose of this widget is holding a WLayout widget when the user select an item from the navigation tree.
//	The WLayoutContainer inherits QSplitter so the container is always resized as the main application window is resized.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WLAYOUTCONTAINER_H
#define WLAYOUTCONTAINER_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

singleton WLayoutContainer : public QSplitter
{
private:
	WLayout * m_pawLayoutEmpty_NZ;		// Empty (dummy) layout so m_pwLayoutVisible is never NULL.
	WLayout * m_pwLayoutVisible_NZ;		// Current (selected) layout being displayed to the user
	WLayout * m_pawLayoutAutoDelete;	// Layout to be deleted when another layout is set

public:
	WLayoutContainer();
	void SetLayout(IN WLayout * pwLayout);
	void SetLayoutAutoDelete(PA_CHILD WLayout * pawLayoutAutoDelete);
	void DeleteLayout(PA_DELETING WLayout * pawLayoutToDelete);
	WLayout * PwGetLayout_NZ() const { return m_pwLayoutVisible_NZ; }

//	void paintEvent(QPaintEvent * e);
};

#endif // WLAYOUTCONTAINER_H

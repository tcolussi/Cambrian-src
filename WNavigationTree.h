///////////////////////////////////////////////////////////////////////////////////////////////////
//	WNavigationTree.h
//
//	Header file for the widget displaying the navigation, such as events, accounts, servers and certificates.
//	The navigation wiget contains a tree where the user may expand and collapse nodes.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WNAVIGATIONTREE_H
#define WNAVIGATIONTREE_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif


//	Widget to provide a line edit with the search icon
class WLineEditSearch : public QLineEdit
{
	Q_OBJECT
protected:
	QToolButton * m_pwButtonSearch;		// Button to display the search icon located on the left of widget

public:
	WLineEditSearch(QWidget * pwParent);
};


singleton WNavigationTreeCaption : public QWidget
{
	Q_OBJECT
protected:
	QLineEdit * m_pwLineEdit;

public:
	WNavigationTreeCaption(QWidget *parent = 0);
#if 0
	QSize sizeHint() const { return minimumSizeHint(); }
	QSize minimumSizeHint() const;
#endif
protected:
//	void paintEvent(QPaintEvent * pEvent);
//	void mousePressEvent(QMouseEvent * pEvent);
//	void keyPressEvent(QKeyEvent * pEvent);
	bool eventFilter(QObject *obj, QEvent *event);	// From QObject
public slots:
	void SL_TextChanged(const QString & sText);
	void SL_ToggleDocking();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	singleton WNavigationTree
//
//	Main widget displaying the navigation tree (typically docked on the left of the main application window)
//
singleton WNavigationTree : public QDockWidget
{
	Q_OBJECT
public:
	WTreeWidget * m_pwTreeView;				// Object displaying the chat configuration to the user
protected:
	CTreeWidgetItem * m_pTreeWidgetItemEditing;	// Current widget being edited (if any).  This variable is important because the Qt sends a signal itemChanged() evertime an tree item is being modified, and we need a reliable mechanism to determine which tree item is actually being edited.

public:
	explicit WNavigationTree();
	virtual ~WNavigationTree();

	void NavigationTree_ExpandAllRootTreeItems();

	CTreeWidgetItem * PAllocateTreeItemWidget(ITreeItem * piTreeItem);
	void NavigationTree_SelectTreeItemWidget(CTreeWidgetItem * poTreeItem);
	void NavigationTree_RenameTreeItemWidget(CTreeWidgetItem * poTreeItem);
	void NavigationTree_DisplayTreeItemsContainingText(const QString & sSearch);

public slots:
	void SL_TreeCustomContextMenuRequested(const QPoint & ptPos);
	void SL_TreeItemSelectionChanged(QTreeWidgetItem * pItemCurrent, QTreeWidgetItem * pItemPrevious);
	void SL_TreeItemClicked(QTreeWidgetItem * pItemClicked, int iColumn);
	void SL_TreeItemEdited(QTreeWidgetItem * pItemEdited, int iColumn);
}; // WNavigationTree

extern WNavigationTree * g_pwNavigationTree;

#endif // WNAVIGATIONTREE_H

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

singleton WNavigationTreeCaption : public QWidget
{
public:
	WNavigationTreeCaption();
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
	WTreeWidget * m_pwTreeView;				// Object displaying the profile(s) to the user
protected:
	CTreeItemW * m_pTreeWidgetItemEditing;	// Current widget being edited (if any).  This variable is important because the Qt sends a signal itemChanged() evertime an tree item is being modified, and we need a reliable mechanism to determine which tree item is actually being edited.

public:
	explicit WNavigationTree();
	virtual ~WNavigationTree();
	void NavigationTree_TreeItemUnselect();

	void NavigationTree_ExpandAllRootTreeItems();

	void NavigationTree_SelectTreeItemWidget(CTreeItemW * poTreeItem);
	void NavigationTree_RenameTreeItemWidget(CTreeItemW * poTreeItem);
	void NavigationTree_DisplayTreeItemsContainingText(const QString & sSearch);

public slots:
	void SL_TreeCustomContextMenuRequested(const QPoint & ptPos);
	void SL_TreeItemSelectionChanged(QTreeWidgetItem * pItemCurrent, QTreeWidgetItem * pItemPrevious);
	void SL_TreeItemClicked(QTreeWidgetItem * pItemClicked, int iColumn);
	void SL_TreeItemEdited(QTreeWidgetItem * pItemEdited, int iColumn);
	void SL_EditSearchTextChanged(const QString & sTextFind);
	void SL_ToggleDocking();
	void SL_ContactNew();
	void SL_MenuProfilesShow();
	#ifdef COMPILE_WITH_SPLASH_SCREEN
    void SL_RolePageShow();
	#endif
	void SL_MenuProfileSelected(QAction * pAction);
}; // WNavigationTree

extern WNavigationTree * g_pwNavigationTree;


#endif // WNAVIGATIONTREE_H

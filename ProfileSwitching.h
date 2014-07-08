///////////////////////////////////////////////////////////////////////////////////////////////////
//	ProfileSwitching.h
//
//	Tree Items necessary for populating the Navigation Tree according to the selected profile.

#ifndef PROFILESWITCHING_H
#define PROFILESWITCHING_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

//	The 'Inbox' is displayed when there are multiple profiles and therefore giving a summary of the messages received.
singleton TTreeItemInbox : public ITreeItem
{
public:
	TTreeItemInbox();
	virtual void TreeItem_GotFocus();							// From ITreeItem
	RTI_IMPLEMENTATION(TTreeItemInbox)
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	This node is displayed when there are multiple profiles, or when the user wishes to create a new profile.
singleton TProfiles : public ITreeItem
{
public:
	TProfiles();
	virtual void TreeItem_GotFocus();		// From ITreeItem
	RTI_IMPLEMENTATION(TProfiles)
};

#endif // PROFILESWITCHING_H

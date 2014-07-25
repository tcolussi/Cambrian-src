///////////////////////////////////////////////////////////////////////////////////////////////////
//	ProfileSwitching.cpp
//
//	Collection of methods and objects necessary for pouplating the Navigation Tree according to the selected profile(s).
//
//	MOTIVATION
//	The motivation to have file dedicated to populate the Navigation Tree is to make the code easier to read and to maintain.
//	The file WNavigationTree.cpp is the implementation of the widget which includes the menus and search bar, while this file
//	contains the logic about the content of the Navigation Tree.
//
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "ProfileSwitching.h"
#include "WNavigationTree.h"
#include "TRecommendations.h"

const CHU c_szContacts[]	= "Contacts";	// Display name fo rthe 'Inbox' node

//	The following two Tree Items are created on-demand (and deleted) each time the Navigation Teee is re-populated.
TTreeItemInbox * g_pTreeItemInbox;
TProfiles * g_pTreeItemProfiles;


//	Core routine to populate the Navigation Tree.
//
//	IMPLEMENTATION NOTES
//	At the moment the routine is written for a single configuration.  This code will have to be revised to support multiple configurations.
void
NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(TProfile * pProfileSelected, BOOL fCreateNewProfile)
	{
	Endorse(pProfileSelected == NULL);	// Display all profiles
	Assert(g_pwNavigationTree != NULL);
	g_oConfiguration.m_pProfileSelected = pProfileSelected;

	// Flush the existing content of the Navigation Tree
	g_pwNavigationTree->NavigationTree_TreeItemUnselect();
	WTreeWidget * pwTreeView = g_pwNavigationTree->m_pwTreeView;
	QTreeWidgetItemIterator oIterator(pwTreeView);
	while (TRUE)
		{
		CTreeItemW * pTreeWidgetItem = (CTreeItemW *)*oIterator++;
		if (pTreeWidgetItem == NULL)
			break;
		ITreeItem * piTreeItem = pTreeWidgetItem->m_piTreeItem;
		Assert(piTreeItem != NULL);
		Assert(PGetRuntimeInterfaceOf_ITreeItem(piTreeItem) == piTreeItem);	// Make sure the pointer is valid
		Assert(piTreeItem->m_paTreeItemW_YZ != NULL);
		// Remember the state of each Tree Item before switching profile, so next time the profile is re-selected, then the entire arborescence is restored.
		if (piTreeItem->m_paTreeItemW_YZ->isExpanded())
			piTreeItem->m_uFlagsTreeItem |= ITreeItem::FTI_kfTreeItem_IsExpanded;
		else
			piTreeItem->m_uFlagsTreeItem &= ~ITreeItem::FTI_kfTreeItem_IsExpanded;
		piTreeItem->m_paTreeItemW_YZ = NULL;	// This would cause a memory leak in the absence of pwTreeView->clear()
		} // while
	pwTreeView->clear();
	delete g_pTreeItemInbox;
	g_pTreeItemInbox = NULL;
	delete g_pTreeItemProfiles;
	g_pTreeItemProfiles = NULL;

	int cProfiles;
	TProfile ** prgpProfiles = g_oConfiguration.m_arraypaProfiles.PrgpGetProfiles(OUT &cProfiles);
	if (cProfiles == 1 && !fCreateNewProfile)
		pProfileSelected = prgpProfiles[0];	// If there is only one profile, then pretend the user selected that profile.  There is no need to show extra nodes which may confuse the user
	if (pProfileSelected != NULL)
		{
		// Display a single profile.  This is done by displaying the contacts first, and then the profile at the bottom
		pProfileSelected->TreeItemProfile_DisplayContactsWithinNavigationTree();
		if (pProfileSelected->m_arraypaAccountsXmpp.FIsEmpty())
			pProfileSelected->TreeItemProfile_DisplayProfileInfoWithinNavigationTree();
		pProfileSelected->TreeItemProfile_DisplayApplicationsWithinNavigationTree();
		}
	else
		{
		// Since we are displaying multiple profiles, we need to create two root items for the 'Inbox' and the 'Profiles'
		g_pTreeItemInbox = new TTreeItemInbox;
		g_pTreeItemProfiles = new TProfiles;

		for (int iProfile = 0; iProfile < cProfiles; iProfile++)
			{
			TProfile * pProfile = prgpProfiles[iProfile];
			Assert(pProfile->EGetRuntimeClass() == RTI(TProfile));
			pProfile->TreeItemProfile_DisplayContactsWithinNavigationTree();
			pProfile->TreeItemProfile_DisplayProfileInfoWithinNavigationTree();
			pProfile->TreeItemProfile_DisplayApplicationsWithinNavigationTree();
			}
		}
	NavigationTree_UpdateNameOfSelectedProfile();
	if (fCreateNewProfile || cProfiles == 0)
		{
		// Set the focus to "Profiles" if creating a new profile, or if there is no profile
		Assert(g_pTreeItemProfiles != NULL);
		g_pTreeItemProfiles->TreeItemLayout_SetFocus();
		}
	else
		pwTreeView->setCurrentItem(pwTreeView->topLevelItem(0));	// Select the first item
	} // NavigationTree_PopulateTreeItemsAccordingToSelectedProfile()

#if 0
	// Create the root nodes
	g_pTreeItemCommunication = new TTreeItemInbox;
	/*
	new TTreeItemDemo(NULL, "Applications", eMenuIconComponent);
	new TTreeItemDemo(NULL, "Marketplace", eMenuIconMarketplace);
	new TTreeItemDemo(NULL, "Finance", eMenuIconSell);
	new TTreeItemDemo(NULL, "Registry", eMenuIconCorporations);
	TTreeItemDemo * pIDs = new TTreeItemDemo(NULL, "My Profiles", eMenuIconSettings);
	TTreeItemDemo * pID = new TTreeItemDemo(pIDs, "Jon Peters", eMenuIconIdentities);
		TTreeItemDemo * pApplications = new TTreeItemDemo(pID, "Applications", eMenuIconComponent);
			TTreeItemDemo * pCommunications = new TTreeItemDemo(pApplications, "Communications", eMenuIconCommunicate);
				TTreeItemDemo * pAccount = new TTreeItemDemo(pCommunications, "jon", eMenuIconXmpp);
					new TTreeItemDemo(pAccount, "Dan", eMenuAction_PresenceAccountOnline);
					new TTreeItemDemo(pAccount, "Hiro", eMenuAction_PresenceAccountOnline);
					new TTreeItemDemo(pAccount, "Light", eMenuAction_PresenceAway);
				TTreeItemDemo * pFB = new TTreeItemDemo(pCommunications, "Jon Peters", eMenuIconFacebook);
					new TTreeItemDemo(pFB, "Brittany", eMenuAction_PresenceAccountOnline);
					new TTreeItemDemo(pFB, "Mom", eMenuAction_PresenceAccountOffline);
			TTreeItemDemo * pMarketplace = new TTreeItemDemo(pApplications, "Marketplace", eMenuIconMarketplace);
				new TTreeItemDemo(pMarketplace, "Jurisdictions", eMenuIconJurisdiction);
				new TTreeItemDemo(pMarketplace, "Services", eMenuIconServices);
				new TTreeItemDemo(pMarketplace, "Smart Contracts", eMenuIconMarketplaceLawyers);
				new TTreeItemDemo(pMarketplace, "For Sale", eMenuIconSell);
				new TTreeItemDemo(pMarketplace, "Jobs", eMenuIconJobs);
				new TTreeItemDemo(pMarketplace, "Housing", eMenuIconHome);
				new TTreeItemDemo(pMarketplace, "Gigs", eMenuIconRipple);
			TTreeItemDemo * pFinance = new TTreeItemDemo(pApplications, "Finance", eMenuIconIssueDividends);
				new TTreeItemDemo(pFinance, "Exchange", eMenuIconExchange);
				new TTreeItemDemo(pFinance, "Wallets", eMenuIconBitcoin);
				new TTreeItemDemo(pFinance, "Banking", eMenuIconBank);
			TTreeItemDemo * pRegistry = new TTreeItemDemo(pApplications, "Registry", eMenuIconVote);
				new TTreeItemDemo(pRegistry, "Companies", eMenuIconCorporations);
				new TTreeItemDemo(pRegistry, "Jurisdictions", eMenuIconJurisdiction);
				new TTreeItemDemo(pRegistry, "Communities", eMenuIconCommunity);
				new TTreeItemDemo(pRegistry, "Arbitrator", eMenuIconMarketplaceArbitration);
				new TTreeItemDemo(pRegistry, "Mediator", eMenuIconMarketplaceMediation);
				new TTreeItemDemo(pRegistry, "Oracle", eMenuIconListen);
			new TTreeItemDemo(pApplications, "Calendar", eMenuIconIssueFuture);
		new TTreeItemDemo(pIDs, "Cambrian Inc.", eMenuIconIdentities);
		*/
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
TTreeItemInbox::TTreeItemInbox()
	{
	TreeItemW_DisplayWithinNavigationTreeExpand(NULL, (PSZAC)c_szContacts, eMenuIconCommunicate);
	}

//	TTreeItemInbox::ITreeItem::TreeItem_GotFocus()
void
TTreeItemInbox::TreeItem_GotFocus()
	{
	MainWindow_SetCurrentLayout(NULL);
	}

TProfiles::TProfiles()
	{
	//TreeItemW_DisplayWithinNavigationTreeExpand(NULL, d_sza_Profile"s", eMenuIconSettings);
	TreeItemW_DisplayWithinNavigationTreeExpand(NULL, "Roles", eMenuAction_DisplayProfileInfo);
	}

void
NavigationTree_DisplayProfilesToogle()
	{
	TProfile * pProfileSelected = g_oConfiguration.m_pProfileSelected;
	if (pProfileSelected != NULL)
		{
		if (pProfileSelected->m_paTreeItemW_YZ == NULL)
			{
			pProfileSelected->TreeItemProfile_DisplayProfileInfoWithinNavigationTree();
			pProfileSelected->TreeItemW_Expand();
			pProfileSelected->TreeItemLayout_SetFocus();
			return;
			}
		pProfileSelected->TreeItemW_ToggleVisibilityAndSetFocus();
		}
	else if (g_pTreeItemProfiles != NULL)
		{
		g_pTreeItemProfiles->TreeItemW_ToggleVisibilityAndSetFocus();
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
TProfile::TreeItemProfile_DisplayContactsWithinNavigationTree()
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		pAccount->TreeItemAccount_DisplayWithinNavigationTree();
		}
	}

void
TProfile::TreeItemProfile_DisplayProfileInfoWithinNavigationTree()
	{
	Endorse(g_pTreeItemProfiles == NULL); // Display the profile at the root rather than under "Profiles"
	TreeItemW_DisplayWithinNavigationTree(g_pTreeItemProfiles, eMenuIconIdentities);
	TAccountXmpp * pAccount = NULL;
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		pAccount = *ppAccount++;
		pAccount->PGetAlias_NZ()->TreeItemW_DisplayWithinNavigationTreeExpand(this, pAccount->m_strJID, eMenuIconXmpp);
		}
	}

void
TProfile::TreeItemProfile_DisplayApplicationsWithinNavigationTree()
	{
	if (!m_arraypaAccountsXmpp.FIsEmpty())
		new TMyRecommendations(this);	// Dislay the recommendations if there is at least one account
	//	new TTreeItemDemo(this, "My Reputation", eMenuIconReputation, "Display my reputation according to other organizations", "Search Reputation Feedback Comments");

	IApplication ** ppApplicationStop;
	IApplication ** ppApplication = m_arraypaApplications.PrgpGetApplicationsStop(OUT &ppApplicationStop);
	while (ppApplication != ppApplicationStop)
		{
		IApplication * pApplication = *ppApplication++;
		pApplication->TreeItemApplication_DisplayWithinNavigationTree();
		}

	TBrowser ** ppBrowserStop;
	TBrowser ** ppBrowser = m_arraypaBrowsers.PrgpGetBrowsersStop(OUT &ppBrowserStop);
	while (ppBrowser != ppBrowserStop)
		{
		TBrowser * pBrowser = *ppBrowser++;
		Assert(pBrowser->EGetRuntimeClass() == RTI(TBrowser));
		pBrowser->TreeItemBrowser_DisplayWithinNavigationTree();
		}

	TreeItemW_ExpandAccordingToSavedState();
	}

void
IApplication::TreeItemApplication_DisplayWithinNavigationTree()
	{
	TreeItemW_DisplayWithinNavigationTreeExpand((m_pProfileParent->m_paTreeItemW_YZ != NULL) ?  m_pProfileParent : NULL, PszGetClassNameApplication(), m_eMenuIcon);
	}

void
TBrowser::TreeItemBrowser_DisplayWithinNavigationTree()
	{
	TreeItemW_DisplayWithinNavigationTree((m_pProfile->m_paTreeItemW_YZ != NULL) ?  m_pProfile : NULL);
	TreeItemW_SetIcon(m_uFlags ? eMenuIconMarketplace : eMenuAction_DisplaySecureWebBrowsing);
	}

//	TAccountXmpp::ITreeItem::TreeItem_PszGetNameDisplay()
PSZUC
TAccountXmpp::TreeItem_PszGetNameDisplay() CONST_MCC
	{
	if (g_pTreeItemInbox == NULL)
		{
		if (m_pProfileParent->m_arraypaAccountsXmpp.PvGetElementUnique_YZ() != NULL)
			return c_szContacts;	// The name of the account may be substituted with the word "Contacts" if there is only one account in the profile
		}
	return TreeItemAccount_PszGetNameDisplay();
	}


void
TAccountXmpp::TreeItemAccount_DisplayWithinNavigationTree()
	{
	Endorse(g_pTreeItemInbox == NULL);	// Display the account at the root of the Navigation Tree
	TreeItemW_DisplayWithinNavigationTree(g_pTreeItemInbox);
	TContact ** ppContactStop;
	TContact ** ppContact = m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		Assert(pContact != NULL);
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		Assert(pContact->m_pAccount == this);
		pContact->TreeItemContact_DisplayWithinNavigationTree();
		} // while
	#ifdef WANT_TREE_NODE_NEW_CONTACT
	m_pTreeItemContactNew = new TContactNew(this);
	#endif

	TGroup ** ppGroupStop;
	TGroup ** ppGroup = m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
	while (ppGroup != ppGroupStop)
		{
		TGroup * pGroup = *ppGroup++;
		Assert(pGroup != NULL);
		Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
		Assert(pGroup->m_pAccount == this);
		pGroup->TreeItemGroup_DisplayWithinNavigationTree();
		}
	TreeItemW_ExpandAccordingToSavedState();
	TreeItemAccount_UpdateIcon();

	#if FIX_THIS
	// Display the alias as well
	m_paAlias->TreeItem_DisplayWithinNavigationTree(m_pProfileParent, m_strJID, eMenuIconXmpp);
	#endif
	} // TreeItemAccount_DisplayWithinNavigationTree()


extern TTreeItemDemo * g_pSecurity;
void
CChatConfiguration::NavigationTree_DisplayAllCertificates()
	{
	if (m_oTreeItemCertificates.m_paTreeItemW_YZ != NULL)
		{
		// The certificates are already on the tree, however may be hidden
		m_oTreeItemCertificates.m_paTreeItemW_YZ->setVisible(true);
		return;
		}
	m_oTreeItemCertificates.NavigationTree_DisplayCertificates((ICertificate *)g_pSecurity);
	//g_pwNavigationTree->NavigationTree_ExpandAllRootTreeItems();	// Make sure all certificates are visibles
	}

void
CChatConfiguration::NavigationTree_DisplayAllCertificatesToggle()
	{
	if (m_oTreeItemCertificates.m_paTreeItemW_YZ != NULL)
		m_oTreeItemCertificates.TreeItemW_ToggleVisibility();	// The certificates are already on the tree, however may be hidden
	else
		NavigationTree_DisplayAllCertificates();
	m_oTreeItemCertificates.TreeItemW_SelectWithinNavigationTreeExpanded();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
extern WButtonIconForToolbarWithDropDownMenu * g_pwButtonSwitchProfile;

void
NavigationTree_UpdateNameOfSelectedProfile()
	{
	Assert(g_pwButtonSwitchProfile != NULL);
	TProfile * pProfileSelected = g_oConfiguration.m_pProfileSelected;
	if (pProfileSelected == NULL)
		pProfileSelected = (TProfile *)g_oConfiguration.m_arraypaProfiles.PvGetElementUnique_YZ();
	if (pProfileSelected != NULL && g_pTreeItemProfiles == NULL)
		g_strScratchBufferStatusBar.Format(d_sza_Profile": $S", &pProfileSelected->m_strNameProfile);
	else
		g_strScratchBufferStatusBar.Format("Switch "d_sza_Profile);
	g_pwButtonSwitchProfile->setText(g_strScratchBufferStatusBar);
	}


extern QToolButton * g_pwButtonStatusOfNavigationTree;

void
NavigationTree_UpdatePresenceIcon(EMenuAction eMenuAction_Presence)
	{
	QAction * pAction = PGetMenuAction(eMenuAction_Presence);
	Assert(pAction != NULL);
	if (pAction != NULL)
		{
		g_pwButtonStatusOfNavigationTree->setIcon(pAction->icon());
		g_pwButtonStatusOfNavigationTree->setText(pAction->text());
		}
	}




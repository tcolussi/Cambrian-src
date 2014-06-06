#ifndef WLAYOUTPROFILE_H
#define WLAYOUTPROFILE_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class WLayoutMyProfiles : public WLayout, public ISocketUI
{
	Q_OBJECT
protected:
	WEdit * m_pwEditProfile;
	QCheckBox * m_pwCheckboxAutomatic;
	TAccountXmpp * m_pAccountNew;	// Pointer to the automatically created account (if any)
public:
	WLayoutMyProfiles();
	~WLayoutMyProfiles();
	virtual void Layout_SetFocus();		// From WLayout
	virtual void SocketUI_OnSuccess();	// From ISocketUI
public slots:
	void SL_ButtonCreateNewProfile();
};

class WLayoutProfile : public WLayout, public ISocketUI
{
	Q_OBJECT
protected:
	TProfile * m_pProfile;
	TAccountXmpp * m_pAccountNew;	// Pointer to the newly created account (if any)
	WGroupBox * m_pwGroupBoxAccountNew;
	OLayoutVerticalAlignTop * m_poLayoutApplications;
	WEdit * m_pwEditSearchApplications;
	QTreeWidget * m_pwTreeViewApplications;
public:
	WLayoutProfile(TProfile * pProfile);
	~WLayoutProfile();
	virtual void Layout_SetFocus();		// From WLayout
	virtual void SocketUI_OnSuccess();	// From ISocketUI

	void DisplayApplications();
	QTreeWidgetItem * PAddOption(EMenuAction eMenuIcon, PSZAC pszName, PSZAC pszDescription, QTreeWidgetItem * pParent = NULL);
public slots:
	void SL_ButtonAccountNewInstant();
	void SL_ButtonAccountNewManual();
	void SL_ButtonAccountLogin();

	void SL_SearchTextChanged(const QString & sText);
	void SL_ActionDoubleClicked(QTreeWidgetItem * pItemClicked, int iColumn);

}; // WLayoutProfile

#endif // WLAYOUTPROFILE_H

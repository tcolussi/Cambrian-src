//	DDialogChannels.cpp
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "DDialogChannels.h"

DDialogChannels::DDialogChannels(TProfile * pProfile) : DDialogOkCancelWithLayouts("Join Channel", eMenuAction_GroupChannel)
	{
	m_pProfile = pProfile;

	/*
	#if 0
	QListWidget * pwChannels = new QListWidget(this);
	m_poLayoutBody->addWidget(pwChannels);
	#else
	QSplitter * pwChannels = new WSplitterVertical;
	m_poLayoutBody->addWidget(pwChannels);
	#endif

	// Populate the dialog with all the channels available
	CArrayPtrGroups arraypChannels;
	pProfile->GetRecommendations_Channels(OUT &arraypChannels);
	TGroup ** ppChannelStop;
	TGroup ** ppChannel = arraypChannels.PrgpGetGroupsStop(OUT &ppChannelStop);
	while (ppChannel != ppChannelStop)
		{
		TGroup * pChannel = *ppChannel++;
		Assert(pChannel != NULL);
		Assert(pChannel->EGetRuntimeClass() == RTI(TGroup));
		MessageLog_AppendTextFormatCo(d_coBlack, "Adding channel '$S'\n", &pChannel->m_strNameChannel_YZ);
		#if 0
		pwChannels->addItem(pChannel->m_strNameChannel_YZ);
		#else
		pwChannels->addWidget(new QLabel(pChannel->m_strNameChannel_YZ));
		#endif
		}
	*/
	setMinimumWidth(600);
	WTreeWidget * m_pwTreeRecommendations = new WTreeWidget;
	m_poLayoutBody->addWidget(m_pwTreeRecommendations);
	m_pwTreeRecommendations->setFocus(Qt::TabFocusReason);
	m_pwTreeRecommendations->setRootIsDecorated(false);
	m_pwTreeRecommendations->setColumnCount(2);
	m_pwTreeRecommendations->setHeaderLabels((QStringList) "Name" << "Purpose" << "Popularity" << "Also Joined By");
	QHeaderView * pwHeader = m_pwTreeRecommendations->header();
	pwHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	pwHeader->setSectionResizeMode(1, QHeaderView::ResizeToContents);

	OLayoutHorizontal * poLayoutHorizontal = new OLayoutHorizontal(m_poLayoutBody);
	WButtonText * pwButtonChannelNew = new WButtonTextWithIcon("New Channel...", eMenuIconAdd);
	poLayoutHorizontal->addWidget(pwButtonChannelNew);
	WButtonText * pwButtonOK = new WButtonTextWithIcon("Join Channel ", eMenuAction_GroupChannel);
	poLayoutHorizontal->addWidget(pwButtonOK, d_zNA, Qt::AlignRight);
	connect(pwButtonOK, SIGNAL(clicked()), this, SLOT(SL_ButtonOK()));

	QIcon oIconChannel = PGetMenuAction(eMenuAction_GroupChannel)->icon();
	CChannelName ** ppChannelStop;
	CChannelName ** ppChannel = pProfile->m_arraypaChannelNames.PrgpGetChannelsStop(OUT &ppChannelStop);
	while (ppChannel != ppChannelStop)
		{
		CChannelName * pChannel = *ppChannel++;
		CTreeWidgetItem * paRecommendation = new CTreeWidgetItem;
		paRecommendation->setCheckState(0, Qt::Unchecked);
		paRecommendation->setIcon(0, oIconChannel);
		paRecommendation->setText(0, pChannel->m_strName);
	//	paRecommendation->setText(1, Timestamp_ToQDateTime(pChannel->m_tsFirstRecommended).toString());
		m_pwTreeRecommendations->addTopLevelItem(paRecommendation);
		}
	//m_pwTreeRecommendations->setFocus(Qt::ActiveWindowFocusReason); // Does not work
	}

void
DDialogChannels::SL_ButtonOK()
	{

	}

void
DisplayDialogChannelsBrowse(TProfile * pProfile)
	{
	if (pProfile == NULL)
		return;	// This happens when no profile is selected
	Assert(pProfile != NULL);
	DDialogChannels dialog(pProfile);
	dialog.FuExec();
	}

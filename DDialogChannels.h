//	DDialogChannels.h

#ifndef DDIALOGCHANNELS_H
#define DDIALOGCHANNELS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class DDialogChannels : public DDialogOkCancelWithLayouts
{
	Q_OBJECT
protected:
	TProfile * m_pProfile;
	WTreeWidget * m_pwTreeRecommendations;
public:
	DDialogChannels(TProfile * pProfile);

public slots:
	void SL_ButtonOK();
	void SL_ButtonNew();
};

class DDialogChannelNew : public DDialogOkCancelWithLayouts
{
	Q_OBJECT
protected:
	TProfile * m_pProfile;
	WEdit * m_pwEditChannelName;
	WEditTextArea * m_pwEditChannelPurpose;
public:
	DDialogChannelNew(TProfile * pProfile);

public slots:
	void SL_ButtonOK();
	void SL_ButtonCancel() { SL_ButtonCancel_clicked(); }
};

#endif // DDIALOGCHANNELS_H

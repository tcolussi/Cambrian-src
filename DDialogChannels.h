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
public:
	DDialogChannels(TProfile * pProfile);

public slots:
	void SL_ButtonOK();
};

#endif // DDIALOGCHANNELS_H

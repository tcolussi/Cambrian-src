//	TChannel.cpp

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TChannel.h"

void
CArrayPtrChannelNames::DeleteAllChannels()
	{
	CChannelName ** ppChannelStop;
	CChannelName ** ppChannel = PrgpGetChannelsStop(OUT &ppChannelStop);
	while (ppChannel != ppChannelStop)
		delete *ppChannel++;
	Empty();
	}

//	Return TRUE if a new channel was added
//	Return FALSE if the channel was already in the array
BOOL
CArrayPtrChannelNames::FNewChannelAdded(PSZUC pszChannelName)
	{
	CChannelName * pChannel;
	CChannelName ** ppChannelStop;
	CChannelName ** ppChannel = PrgpGetChannelsStop(OUT &ppChannelStop);
	while (ppChannel != ppChannelStop)
		{
		pChannel = *--ppChannelStop;	// Search from the end, as it is most likely to find a recent channel
		if (pChannel->m_strName.FCompareStringsChannelNames(pszChannelName))
			return FALSE;
		}
	//MessageLog_AppendTextFormatCo(d_coRed, "Adding new channel '$s'\n", pszChannelName);
	pChannel = new CChannelName;
	Add(PA_CHILD pChannel);
	pChannel->m_strName = pszChannelName;
	pChannel->m_tsFirstRecommended = Timestamp_GetCurrentDateTime();
	pChannel->m_uFlags = d_zNA;	// Not used yet
	return TRUE;
	}

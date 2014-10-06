//	TChannel.h
//
//	This file does not yet have a class TChannel, however contains the classes related to channel management.
//
#ifndef TCHANNEL_H
#define TCHANNEL_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Simple class to offer basic channel management.
class CChannelName
{
public:
	CStr m_strName;				// Name of the channel
	UINT m_uFlags;				// Various flags about the channel (at the moment, only ignore/archive comes to mind)
	TIMESTAMP m_tsFirstRecommended;	// Time when the channel was first recommended, and therefore giving the possibility to display new available channels first
};

//	This array is necessary because the same channel may be recommended by many contacts
//	Determining the total number of unique channels requires a lot of processing.
class CArrayPtrChannelNames : public CArray
{
public:
	inline CChannelName ** PrgpGetChannelsStop(OUT CChannelName *** pppChannelStop) const { return (CChannelName **)PrgpvGetElementsStop(OUT (void ***)pppChannelStop); }
	void DeleteAllChannels();
	BOOL FNewChannelAdded(PSZUC pszChannelName);
};

#endif // TCHANNEL_H

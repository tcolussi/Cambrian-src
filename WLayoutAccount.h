#ifndef WLAYOUTACCOUNT_H
#define WLAYOUTACCOUNT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class WLayoutAccount : public WLayout
{
public:
	WLayoutAccount(TAccountXmpp * pAccount);
};

class WLayoutAccountAlias : public WLayout
{
public:
	WLayoutAccountAlias(TAccountAlias * pAlias);
};


#endif // WLAYOUTACCOUNT_H

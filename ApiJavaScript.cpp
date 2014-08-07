//	ApiJavaScript.cpp

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "ApiJavaScript.h"


OJapiApps::OJapiApps(OJapiCambrian * poCambrian)
	{
    m_poCambrian = poCambrian;
	}

POJapiAppBallotmaster
OJapiApps::ballotmaster()
	{
	//MessageLog_AppendTextFormatCo(d_coGreen, "apps()");
	return m_poCambrian->polls();
	}

POJapiApps
OJapiCambrian::apps()
	{
	return &m_oApps;
	}

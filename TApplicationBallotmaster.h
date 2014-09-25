#ifndef TAPPLICATIONBALLOTMASTER_H
#define TAPPLICATIONBALLOTMASTER_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

struct SApplicationHtmlInfo;


typedef OJapiAppInfo * (* PFn_PaAllocateJapi)(SApplicationHtmlInfo * pInfo);
OJapiAppInfo * PaAllocateJapiGeneric(SApplicationHtmlInfo * pInfo);
//OJapiAppInfo * PaAllocateJapiBallotMaster(SApplicationHtmlInfo * pInfo);


//	The TApplicationHtml is a generic container for an HTML5 application.
//	The application has access to the root "Cambrian" JavaScript object which gives access to other applications.
//	At the moment, there is no security, so all HTML applications have full access to the JavaScript APIs.

struct SApplicationHtmlInfo
{
	PSZAC pszName;			// "Ballotmaster"
	PSZAC pszLocation;		// "Apps/Ballotmaster/index.html"
	PFn_PaAllocateJapi pfnPaAllocateJapi;
	POJapi paoJapi;			//
	// Icon
	/* NYI
	PSZAC pszUrlDocumentation;
	PSZAC pszUrlHelp;		// Where to find help on the web
	*/
};


const SApplicationHtmlInfo * PGetApplicationHtmlInfo(PSZAC pszNameApplication);

extern SApplicationHtmlInfo g_rgApplicationHtmlInfo[];
SApplicationHtmlInfo * PGetApplicationHtmlInfoBallotmaster();

class CApplicationHtmlBallotmaster
	{

	};

class CApplicationHtmlOfficeKingpin;




#endif // TAPPLICATIONBALLOTMASTER_H

//	MenuIcons.h
#ifndef MENUICONS_H
#define MENUICONS_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

enum EMenuIcon			// Must be in sync with
	{
	eMenuIcon_zNull = 0,

	eMenuIcon_Overflow,		// Icon to draw the overflow icon in the dashboard / comm panel
	eMenuIcon_OverflowHovered,

	eMenuIconMax	// Must be last
	};

const QIcon & OGetIcon(EMenuIcon eMenuIcon);

#endif // MENUICONS_H

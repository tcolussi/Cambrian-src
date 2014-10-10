//	Central repository for all icons, mostly used by menus.
//
//	Since the same icon is reused for many menu actions, the icons have their own enum.
//
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "MenuIcons.h"

//	Array mapping an eMenuIcon to its resource name
const PSZAC c_mapepszIconResources[eMenuIconMax] =
	{
	NULL,
	"MenuOverflow",		// eMenuIcon_Overflow
	"MenuOverflowH",	// eMenuIcon_OverflowHovered,
	};

//	Array of all icons.
QIcon g_rgoIcons[eMenuIconMax];

const QIcon &
OGetIcon(EMenuIcon eMenuIcon)
	{
	Assert(eMenuIcon >= 0 && eMenuIcon < eMenuIconMax);
	QIcon * poIcon = g_rgoIcons + eMenuIcon;
	if (poIcon->isNull())
		{
		*poIcon = QIcon(":/ico/" + QString(c_mapepszIconResources[eMenuIcon]));
		Assert(!poIcon->isNull());
		#ifdef DEBUG
		if (poIcon->pixmap(QSize(16,16)).isNull())
			MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "Unable to load eMenuIcon [$i] named '$s'\n", eMenuIcon, c_mapepszIconResources[eMenuIcon]);
		#endif
		}
	return *poIcon;
	}

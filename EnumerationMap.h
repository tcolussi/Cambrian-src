///////////////////////////////////////////////////////////////////////////////////////////////////
//	EnumerationMap.h
//
//	Structure to map an enumerated value to a string and vice-versa.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ENUMERATIONMAP_H
#define ENUMERATIONMAP_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

///////////////////////////////////////
//	Data structure to map an enumeration value to a hardcoded string or a description
struct SEnumerationMap
{
	ENUM eEnumeration;
	PSZAC pszValue;			// Hardcoded string of the enumeration
	PSZUC pszuDescription;	// Localized version of the string (always encoded in UTF-8)
};
#define SEnumerationMapLastEntry(pszaEnumerationMapName) { ENUM_NIL, NULL, (PSZUC)pszaEnumerationMapName }	// Use the last entry to store the name of the enumeration map (very useful for debugging)

ENUM EnumerationMap_EFindValueFromString(const SEnumerationMap prgzEnumerationMap[], PSZAC pszValue);
PSZAC EnumerationMap_PszFindValueFromEnum(const SEnumerationMap prgzEnumerationMap[], ENUM eEnumeration);
PSZUC EnumerationMap_PszuFindDescriptionFromEnum(const SEnumerationMap prgzEnumerationMap[], ENUM eEnumeration);

///////////////////////////////////////////////////////////////////////////////////////////////////
EMenuAction EnumerationMap_EFindPresence(PSZUC pszPresence);
PSZUC EnumerationMap_PszFindPresence(EMenuAction eMenuAction_Presence);

#endif // ENUMERATIONMAP_H

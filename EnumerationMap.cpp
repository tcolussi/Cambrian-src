///////////////////////////////////////////////////////////////////////////////////////////////////
//	EnumerationMap.cpp
//
//	Code mapping an enumerated value to a string and vice-versa.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

PSZUC
EnumerationMap_PszFindName(const SEnumerationMap prgzEnumerationMap[])
	{
	Assert(prgzEnumerationMap != NULL);
	if (prgzEnumerationMap != NULL)
		{
		while (TRUE)
			{
			if (prgzEnumerationMap->pszValue == NULL)
				return prgzEnumerationMap->pszuDescription;
			prgzEnumerationMap++;
			}
		}
	Assert("Enumeration Map does not have any name");
	return NULL;
	}

ENUM
EnumerationMap_EFindValueFromString(const SEnumerationMap prgzEnumerationMap[], PSZAC pszValue)
	{
	Assert(prgzEnumerationMap != NULL);
	if (pszValue == NULL)
		pszValue = c_szaEmpty;
	while (prgzEnumerationMap->pszValue != NULL)
		{
		Assert(prgzEnumerationMap->eEnumeration >= 0);
		if (FCompareStrings(pszValue, prgzEnumerationMap->pszValue))
			return prgzEnumerationMap->eEnumeration;
		prgzEnumerationMap++;
		}
	return ENUM_NIL;
	}

PSZAC
EnumerationMap_PszFindValueFromEnum(const SEnumerationMap prgzEnumerationMap[], ENUM eEnumeration)
	{
	Assert(prgzEnumerationMap != NULL);
	if (prgzEnumerationMap != NULL)
		{
		while (prgzEnumerationMap->pszValue != NULL)
			{
			Assert(prgzEnumerationMap->eEnumeration >= 0);
			if (prgzEnumerationMap->eEnumeration == eEnumeration)
				return prgzEnumerationMap->pszValue;
			prgzEnumerationMap++;
			}
		}
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Enum value $i could not be found in enumeration map '$s'\n", eEnumeration, EnumerationMap_PszFindName(prgzEnumerationMap));
	return NULL;
	}

PSZUC
EnumerationMap_PszuFindDescriptionFromEnum(const SEnumerationMap prgzEnumerationMap[], ENUM eEnumeration)
	{
	Assert(prgzEnumerationMap != NULL);
	Assert(eEnumeration >= 0);
	Assert(FALSE && "NYI");
	return NULL;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Specific enumeration maps
const char c_szaMapNamePresence[] = "presence";
SEnumerationMap c_rgzMapPresence[] =
	{
	{ eMenuAction_PresenceAccountOnline, "chat", NULL },
	{ eMenuAction_PresenceAway, "away", NULL },
	{ eMenuAction_PresenceAwayExtended, "xa", NULL },
	{ eMenuAction_PresenceBusy, "dnd", NULL },
	SEnumerationMapLastEntry(c_szaMapNamePresence)
	};

EMenuAction
EnumerationMap_EFindPresence(PSZUC pszPresence)
	{
	return (EMenuAction)EnumerationMap_EFindValueFromString(IN c_rgzMapPresence, IN (PSZAC)pszPresence);
	}

PSZUC
EnumerationMap_PszFindPresence(EMenuAction eMenuAction_Presence)
	{
	return (PSZUC)EnumerationMap_PszFindValueFromEnum(IN c_rgzMapPresence, eMenuAction_Presence);
	}

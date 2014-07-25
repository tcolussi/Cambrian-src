///////////////////////////////////////////////////////////////////////////////////////////////////
//	IRuntimeObject.cpp
//
//	Base class (root interface) for any dynamic object requiring runtime identification.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

POBJECT
PGetRuntimeInterfaceOf_(const IRuntimeObject * piObject, RTI_ENUM rti)
	{
	if (piObject != NULL)
		return piObject->PGetRuntimeInterface(rti, NULL);
	return NULL;
	}

ITreeItem *
PGetRuntimeInterfaceOf_ITreeItem(const IRuntimeObject * piObject)
	{
	return (ITreeItem *)PGetRuntimeInterfaceOf_(piObject, RTI(ITreeItem));
	}

TAccountXmpp *
PGetRuntimeInterfaceOf_TAccountXmpp(const IRuntimeObject * piObject)
	{
	return (TAccountXmpp *)PGetRuntimeInterfaceOf_(piObject, RTI(TAccountXmpp));
	}

TCertificate *
PGetRuntimeInterfaceOf_TCertificate(const IRuntimeObject * piObject)
	{
	return (TCertificate *)PGetRuntimeInterfaceOf_(piObject, RTI(TCertificate));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	PGetRuntimeInterface(), virtual
//
//	Virtual method querying the object for a supported interface.  The caller is responsible to typecast the returned pointer POBJECT of the requested interface.
//
//	Since most object have a parent, the method PGetRuntimeInterface() has a second parameter piObjectSecondary to query an additional object in case the object does not directly support the interface.
//
//	In some cases, the pointer may be a foreign object if there is a clear relationship between the object and the requested interface.
//	For instance, an object TContact may return a pointer to a TAccountXmpp if requested, because for every TContact, there is a corresponding TAccountXmpp.
//
POBJECT
IRuntimeObject::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piObjectSecondary) const
	{
	Endorse(piObjectSecondary == NULL);
	Assert(piObjectSecondary != this);
	// Use the virtual method EGetRuntimeClass() to determine if we want a pointer to our own class.  This is not the most efficient code, however the safest, as it does not require every class inheriting IRuntimeObject to implement PGetRuntimeInterface() in order to get a pointer to the interface of their own class.
	if (rti == EGetRuntimeClass())
		return (IRuntimeObject *)this;
	if (rti == RTI(IRuntimeObject))
		return (IRuntimeObject *)this;
	if (piObjectSecondary == NULL)
		return NULL;	// The interface requested is not supported and there is no secondary object to query, therefore return NULL.
	return piObjectSecondary->PGetRuntimeInterface(rti, NULL);
	}

IRuntimeObject *
CArrayPtrRuntimeObjects::PFindRuntimeObject(RTI_ENUM rti) const
	{
	IRuntimeObject ** ppRuntimeObjectStop;
	IRuntimeObject ** ppRuntimeObject = PrgpGetRuntimeObjectsStop(OUT &ppRuntimeObjectStop);
	while (ppRuntimeObject != ppRuntimeObjectStop)
		{
		IRuntimeObject * pObject = *ppRuntimeObject++;
		if (pObject->EGetRuntimeClass() == rti)
			return pObject;
		}
	return NULL;
	}

void
CArrayPtrRuntimeObjects::DeleteRuntimeObject(PA_DELETING IRuntimeObject * paRuntimeObject)
	{
	Assert(paRuntimeObject != NULL);
	(void)RemoveElementAssertI(paRuntimeObject);
	delete paRuntimeObject;
	}

void
CArrayPtrRuntimeObjects::DeleteAllRuntimeObjects()
	{
	IRuntimeObject ** ppRuntimeObjectStop;
	IRuntimeObject ** ppaRuntimeObject = PrgpGetRuntimeObjectsStop(OUT &ppRuntimeObjectStop);
	while (ppaRuntimeObject != ppRuntimeObjectStop)
		{
		delete *ppaRuntimeObject++;
		}
	RemoveAllElements();
	}

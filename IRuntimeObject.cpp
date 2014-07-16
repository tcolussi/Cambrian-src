///////////////////////////////////////////////////////////////////////////////////////////////////
//	IRuntimeObject.cpp
//
//	Base class (root interface) for any dynamic object requiring runtime identification.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

TAccountXmpp *
PGetRuntimeInterfaceOf_TAccountXmpp(IRuntimeObject * piObject)
	{
	if (piObject != NULL)
		return piObject->PGetRuntimeInterface_TAccountXmpp();
	return NULL;
	}

TCertificate *
PGetRuntimeInterfaceOf_TCertificate(IRuntimeObject * piObject)
	{
	if (piObject != NULL)
		return piObject->PGetRuntimeInterface_TCertificate();
	return NULL;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	PGetRuntimeInterface(), virtual
//
//	Virtual method returning a pointer for a supported interface.
//	In some cases, the pointer may be a foreign object if there is a clear relationship between the object and the requested interface.
//	For instance, an object TContact may retur a pointer to a TAccountXmpp if requested, because for every TContact, there is a corresponding TAccountXmpp.
//
// The caller is responsible to typecast the void pointer to the appropriate interface.
void *
IRuntimeObject::PGetRuntimeInterface(const RTI_ENUM rti) const
	{
	// Use the virtual method EGetRuntimeClass() to determine if we want a pointer to our own class.  This is not the most efficient code, however the safest, as it does not require every class inheriting IRuntimeObject to implement PGetRuntimeInterface() in order to get a pointer to the interface of their own class.
	if (rti == EGetRuntimeClass())
		return (IRuntimeObject *)this;
	if (rti == RTI(IRuntimeObject))
		return (IRuntimeObject *)this;
	return NULL;	// The interface requested is not supported, so return NULL.
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

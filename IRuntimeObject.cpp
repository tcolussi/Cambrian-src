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
PGetRuntimeInterfaceOf_(RTI_ENUM rti, const IRuntimeObject * piObject)
	{
	if (piObject != NULL)
		return piObject->PGetRuntimeInterface(rti, NULL);
	return NULL;
	}

ITreeItem *
PGetRuntimeInterfaceOf_ITreeItem(const IRuntimeObject * piObject)
	{
	return (ITreeItem *)PGetRuntimeInterfaceOf_(RTI(ITreeItem), piObject);
	}

TAccountXmpp *
PGetRuntimeInterfaceOf_TAccountXmpp(const IRuntimeObject * piObject)
	{
	return (TAccountXmpp *)PGetRuntimeInterfaceOf_(RTI(TAccountXmpp), piObject);
	}

TCertificate *
PGetRuntimeInterfaceOf_TCertificate(const IRuntimeObject * piObject)
	{
	return (TCertificate *)PGetRuntimeInterfaceOf_(RTI(TCertificate), piObject);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	PGetRuntimeInterface(), virtual
//
//	Virtual method querying the object for a supported interface.  The caller is responsible to typecast the returned pointer POBJECT of the requested interface.
//
//	Since most object have a parent, the method PGetRuntimeInterface() has a second parameter piParent to query its parent interface.
//	The way Cambrian is designed is a child object in the Navigation Tree often 'inherits' the interface of its parent.
//
//	If the object has two parents, such as a group member, then one of the parent is handled manually, and the other one passed as a parent.
//	In some cases, the pointer may be a child object if there is a one-to-one relationship between the child object and the requested interface.
//	It is imporant to never substitute a child or sibling object as a 'parent' because there is a risk of recursion (and stack overflow).
//	When querying the interface of a child or sibling object, use PGetRuntimeInterfaceOf_()
//
POBJECT
IRuntimeObject::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Endorse(piParent == NULL);
	Assert(piParent != this);
	// Use the virtual method EGetRuntimeClass() to determine if we want a pointer to our own class.  This is not the most efficient code, however the safest, as it does not require every class inheriting IRuntimeObject to implement PGetRuntimeInterface() in order to get a pointer to the interface of their own class.
	if (rti == EGetRuntimeClass())
		return (IRuntimeObject *)this;
	if (rti == RTI(IRuntimeObject))
		return (IRuntimeObject *)this;
	if (piParent == NULL)
		return NULL;	// The interface requested is not supported and there is no secondary object to query, therefore return NULL.
	return piParent->PGetRuntimeInterface(rti, NULL);
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

bool
CArrayPtrRuntimeObjects::DeleteRuntimeObjectF(PA_DELETING IRuntimeObject * paRuntimeObject)
	{
	Assert(paRuntimeObject != NULL);
	if ( RemoveElementI(paRuntimeObject) >= 0)
		{
		delete paRuntimeObject;
		return true;
		}
	return false;
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
	Empty();
	}

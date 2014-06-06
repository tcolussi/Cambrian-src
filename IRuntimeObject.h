///////////////////////////////////////////////////////////////////////////////////////////////////
//	IRuntimeObject.h
//
//	Base class (root interface) for any dynamic object requiring runtime identification.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef IRUNTIMEOBJECT_H
#define IRUNTIMEOBJECT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

TAccountXmpp * PGetRuntimeInterfaceOf_TAccountXmpp(IRuntimeObject * piObject);
TCertificate * PGetRuntimeInterfaceOf_TCertificate(IRuntimeObject * piObject);

class IRuntimeObject
{
  public:
	virtual ~IRuntimeObject() { InitToGarbage(OUT this, sizeof(*this));	} // Make sure the vtable is no longer accessible
	virtual RTI_ENUM EGetRuntimeClass() const = 0;	// Pure virtual - this method may be implemented using macro RTI_IMPLEMENTATION()
	virtual void * PGetRuntimeInterface(const RTI_ENUM rti) const;

	// Wrappers
	TAccountXmpp * PGetRuntimeInterface_TAccountXmpp() { return (TAccountXmpp *)PGetRuntimeInterface(RTI(TAccountXmpp)); }
	TCertificate * PGetRuntimeInterface_TCertificate() { return (TCertificate *)PGetRuntimeInterface(RTI(TCertificate)); }
}; // IRuntimeObject

class CArrayPtrRuntimeObjects : public CArray
{
public:
	inline IRuntimeObject ** PrgpGetRuntimeObjectsStop(OUT IRuntimeObject *** pppRuntimeObjectStop) const { return (IRuntimeObject **)PrgpvGetElementsStop(OUT (void ***)pppRuntimeObjectStop); }
	void DeleteRuntimeObject(PA_DELETING IRuntimeObject * paRuntimeObject);
	void DeleteAllRuntimeObjects();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Base interface to exchange (serialize/unserialize) XML data
class IXmlExchange : public IRuntimeObject
{
public:
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);
};

//	Base interface to serialize/unserialize an object into XML.
//	The difference between this interface and IXmlExchange is the possibility to serialize/unserialize pointers to objects.
class IXmlExchangeObjectID : public IXmlExchange
{
	friend class CArrayPtrXmlSerializableObjects;
public:
	union
		{
		UINT uSerializeObjectId;	// Identifier of the object.  Any pointer to this object will serialize this value.
		LPARAM lParam;				// Storage to store a integer or a pointer during an operation (for instance, detecting duplicates in an array)
		} mu_Cookie;

public:
	inline IXmlExchangeObjectID() { mu_Cookie.uSerializeObjectId = 0; }
	inline int UGetObjectId() const { return mu_Cookie.uSerializeObjectId; }
	virtual void * PGetRuntimeInterface(const RTI_ENUM rti) const;		// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);
}; // IXmlExchangeObjectID

class CArrayPtrXmlSerializable : public CArray
{
public:
	inline IXmlExchange ** PrgpGetObjectsStop(OUT IXmlExchange *** pppXmlObjectStop) const { return (IXmlExchange **)PrgpvGetElementsStop((void ***)pppXmlObjectStop); }
};

class CArrayPtrXmlSerializableObjects : public CArrayPtrXmlSerializable
{
public:
	inline IXmlExchangeObjectID ** PrgpGetObjectsStop(OUT IXmlExchangeObjectID *** pppXmlObjectStop) const { return (IXmlExchangeObjectID **)PrgpvGetElementsStop((void ***)pppXmlObjectStop); }
	IXmlExchangeObjectID * PFindObjectById(UINT uObjectId) const;
	IXmlExchangeObjectID * PFindObjectByIdFromXmlNodeValue(const CXmlNode * pXmlNode) const;

	UINT ForEach_UAssignObjectIds(UINT uObjectIdPrevious = 0) const;

	// Misc cookie manipulation
	void ForEach_SetCookieValue(LPARAM lParam) const;
	void ForEach_SetCookieValueToNULL() const;

	void AppendObjectsWithoutDuplicates(const CArrayPtrXmlSerializableObjects & arraypAppend);
}; // CArrayPtrXmlSerializableObjects

#endif // IRUNTIMEOBJECT_H

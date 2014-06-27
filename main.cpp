///////////////////////////////////////////////////////////////////////////////////////////////////
//	main.cpp
//
//	Main message loop.
//
//	Since this file contains little code, it will contain the conding style and Hungarian prefixes used in this project.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
DATA TYPE PREFIX
	E		Enumeration type
	H		Handle (an opaque structure referring to an object)
	S		Structure (typically containing only primitive datatypes, such as bytes, integers, pointers, and so on)
	C		Class (a class may be a structure, however may include a constructor, destructor and virtual methods)
	O		Class inheriting from QObject (this prefix is typically used to define QObject wrappers)
	W		Class inheriting from QWidget (this prefix is used when defining a new types of widget)
	D		Class inheriting from QDialog
	I		Interface (an interface is a class with virtual methods where there will be never an instance of it, but objects inheriting it)
	T		Any class inheriting the interface ITreeItem must have the prefix T.  This prefix indicates the object has the ability to appear in the Navigation Tree, interact with the user as well as serialize itself in the configuration.  The interface ITreeItem is a special interface at the core of the project and used everywhere.

SCOPE OF VARIABLES
	m_		Member variable of a class/object
	g_		Global variable
	s_		Static variable (a static variable is somewhat a global variable, however its scope if limited to a class)
	c_		Constant variable (a constant variable is also global, however the content of the variable/object does not change during the lifetime of the application)
	d_		Constant definition from #define
	u_		Union variable (multiple variables sharing the same storage space with other variables).  The union prefix is optional for a structure, as the scope of a structure is rather limited.
	mu_		Member union.  A member variable sharing the same storage space with other variables.
	u.		A union within a structure.  Having the name "u" alone is better than a nameless union. See structure SStringToNumber for an example.

FUNCTION/METHOD DECORATIONS
	_		A function/method beginning with the underscore indicates it is restricted/private and must be used with care (typically to avoid memory leaks, or other negative side effects).
	S_		Static function of a class.
	SI_		Qt SIgnal
	SL_		Qt SLot

	_VE()	The function ends with Variable arguments with Ellipsis (...)
	_VEZ()	The function ends with Variable arguments with Ellipsis (...) where the last parameter is Zero-terminated (typically the NULL pointer) indicating the end of the arguments.
	_VEZA()	Same as above with all pointers are allocated.  This suffix is a shorter version of _VEZ_PA()
	_VL()	The function ends with a Variable argument List (va_list vlArgs).
	_VLZ()	The function ends with a Variable argument List where its last parameter of vlArgs is Zero-terminated (indicating the end of the list)
	_VLZA()	Same as above with all pointers are allocated.  This suffix is a shorter version of _VLZ_PA()
	_PA()	All the parameters of the functions are allocated pointers (this suffix is a shorter version of PA_CHILDREN)
	_Gxx()	The function uses a global/static variable.  The remaining of the name identifies the global variable (example: _Gsb() - the function uses the status bar, _Gso() the global variable for the socket)
	_RG()	The function may return a pointer to a global/static variable.  This means the caller should make a copy of the returned value as soon as possible and never cache/optimize this value.

	_NZ()	The function Never returns Zero (if the function is expected to return a pointer, the _NZ() means the function will nerver return a NULL pointer).  Functions containing the word "alloc" or "add" are exempt of the _NZ() suffix, however functions having "get" or "find" must have the _NZ() suffix if they never return zero.
	_YZ()	The function maY return Zero (The use of this suffix is rare because it is used for documenting a function which logically would never return null/zero, however having the possibility of returning zero.  Example: a function allocating an object however with the possibily returning NULL may have the prefix "Paz" the suffix _YZ().
	_ZZ()	The function returns Zero if the value is empty/Zero (the suffix could be called _ZE() however the "E" could be be ambigious with the word "error", and also "E" is used for ellipsis above)
	_ZR()	The function returns Zero if there is an erroR
	_ZZR()	The function return Zero if the value is empty/Zero or if there is an erroR

	_UI()	The function modifies the user interface (UI).  For instance, a function allocating an object having the _UI() means the object is added to the user interface.
	_MB()	The function displays a Message Box to the user (typically to report an error).  Those functions are mostly used to validate user input when the user clicks on the OK button of a dialog.
	_ML()	The function display the error(s), if any, to the Message Log (which is used for debugging)
	_MN()	Message Notice

HUNGARIAN PREFIX
	o		Object (it may be just a generic class or something derived from QObject)
	w		QWidget (under some rare cases, the w prefix may used to describe a WORD (two bytes), however the context should be obvious which one it is)
	s		Dynamic string QString
	str		Dynamic string CStr
	strm	A CStr containing multiple sub-strings (similar as szm)
	strx	A CStr containing user-defined data, typically sent by another application.  This user-defined data cannot be interpreted by Cambrian, however used to communicate with another party (example: strxUserID, strxSessionID, strxStanzaID)
	p		Pointer
	pa		Pointer allocated (when this pointer goes out of scope, it must be deleted)
	paz		Pointer allocated with the possibility of being NULL.
	pw		Pointer to a QWidget
	paw		Pointer to an allocated QWidget (when this pointer goes out of scope, the widget must be deleted)
	po		Pointer to an object
	pao		Pointer to an allocated object
	pi		Pointer to an object of a given interface (this is very rare, however there may be cases where there is a need for a distinction).  The prefix "pi" could also be a pointer to an index.  The context should be obvious.
	piw		Pointer to an interface of a widget
	pai		Pointer to an allocated object of a given interface
	paiw	Pointer to an allocated object of a widget interface
	pv		Pointer to void
	pav		Pointer to an allocated block of memory and/or object
	sz		String zero-terminated
	sza		String zero-terminated of Ansi strings.  Typically Ansi strings are hardcoded strings typed by the developer.
	szu		String zero-terminated of UTF-8 characters. Typically UTF-8 strings contain data typed by the user, which may include the whole range of Unicode characters. (the prefix szu is used when there is a need to distinguish between other strings, such as sza and szw)
	szw		String zero-terminated of Wide (16-bit Unicode) characters
	szm		String zero-terminated containing multiple sub-strings.  The sub-strings are separated by a special character, such as a comma ',', tab '\t', new line '\n', or anything else, including the null-terminator '\0'.
	psz		Pointer to a string zero-terminated
	pasz	Pointer to an allocated string zero-terminated
	rg		Range (array of fixed length)
	rgz		Array zero-terminated
	prg		Pointer to an array
	ch		Character
	chu		Character UTF-8
	chw		Character Wide (16-bit Unicode character)
	pch		Pointer to a character (this is different to a pointer to a string, as the remaining of the pointer may be undefined and/or not null-terminated)
	pche	Pointer to the character representing the error (typically such pointer is returned by a function parsing text and returning the character where the error occurred).  The benefit of using pche instead of pch is the interpretation of the NULL pointer:  With pche, the NULL pointer means a 'success', while with pch, a NULL pointer typically means a failure.
	pszr	Pointer to the remaining of the string. This is the case where a function searches for a special character or a sub-string and return a pointer to what was found (if any).  This prefix indicates the pointers may be subtracted.
	rgb		Array of bytes
	rgch	Array of character
	rgzch	Array of character zero-terminated (this is essentially the same as sz)
	rgpsz	Array of pointer to strings
	rgpasz	Array of pointer to allocated strings
	rgzpasz	Array of pointer to allocated strings (the array is terminated by a NULL pointer)
	arrayb	Dinamic array inherited from QByteArray
	array	Dynamic array (inherited from CArray)
	arrayp	Dynamic array of pointers/objects
	arraypa	Dynamic array of allocated pointers/objects (when the array goes out of scope, the objects must be deleted)
	c		Count
	cb		Count of bytes
	cbl		Count of bytes using 64-bits (long long integer)
	cch		Count of characters
	cchw	Count of Unicode characters
	cx		Count of pixels in the x-direction (width)
	cy		Count of pixels in the y-direction (height)
	i		Index (typically zero-based, however it is possible to have an index having another base).
	ib		Offset in bytes
	ich		Index of a character
	u		Unsigned integer
	n		Number (which may have a negative value)
	b		Byte
	f		Flag (true or false)
	fu		Flag which may be zero or non-zero (but not necessary true=1)
	e		Enumeration value
	ef		Enumeration containing a flag (typically a single bit)
	ez		Enumeration containing the zero value
	k		Prefix to manipulate bits within a value, such as an integer.  Originaly, it was named "mask" and the name got reduced to "msk" to "mk" to just "k".
	kf		Single bit
	km		Multiple bits
	kz		Zero bits (typically the default value)
	ke		Enumeration packed within a range of bits
	kez		Enumeration containing the zero value
	kef		Enumeration containing a single bit
	ki		Index to shift the bits to transform one value to another
	h		Handle
	ha		Handle allocated
	co		Color (typically a QRgb value)
	pfn		Pointer to a function (useful to implement callbacks)
	pfm		Pointer to a function method (this is often used to parametrize slots)
	dtl		QDateTime in local time
	dtu		QDateTime in UTC
	list	QList, SList or CList
	lista	List where the objects have been allocated, and the owner must delete them when going out of scope.
	plist	Pointer to a linked list of objects (although the variable is a pointer to an object, it represents the beginning of a linked list of objects)
	palist	Allocated linked list of objects (the prefix palist indicates the pointer cannot be deleted alone, as it must iterate through the entire list)
	vl		Variable list of arguments (example: va_list vlArgs)
	vla		Variable list of arguments where the pointers are allocated, and therefore the function is ultimately responsible to delete them when no longer needed.
	tid		Timer ID.  Unique number to identify a timer returned by the method startTimer()
	tti		Timer Tick Interval.  Number of milliseconds between timer events.
	ttc		Timer Tick Count.  Number of timer events received.  This value is useful to determine when it is time to perform an action (eg: after 10 timer ticks, do 'x')
	map		Map one type to another.  At the moment the only mapping done is from byte to byte, therefore you will see only mapbb
	mv		Map Value.  Since we use only bytes, the type is mvb.

#endif

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include <QApplication>
#include <QSharedMemory>

int
main(int argc, char *argv[])
	{
	Assert(sizeof(UINT) == 4);
	QApplication oApplication(argc, argv);

	#ifdef Q_OS_WIN
	// At the moment, the object QSharedMemory only works on Windows
	QSharedMemory oSharedMemory(c_sCambrianOrg);
	if (!oSharedMemory.create(1))
		{
		EMessageBoxInformation("There is already an instance of Cambrian running.");
		return 0;
		}
	#endif

	//oApplication.setStyleSheet("QPushButton { padding: 3 10 3 10; }");
	WMainWindow wMainWindow;

	#ifdef DEBUG
	// Run the test suites just after creating the main window and before showing the application to the user
	void TEST_StringRoutines();
	TEST_StringRoutines();
	void TEST_XmlEncodingAndDecoding();
	TEST_XmlEncodingAndDecoding();
	#endif // DEBUG

	wMainWindow.ConfigurationLoadFromXml();
	wMainWindow.show();
	return oApplication.exec();
	} // main()

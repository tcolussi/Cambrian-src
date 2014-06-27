///////////////////////////////////////////////////////////////////////////////////////////////////
//	DataTypes.h
//
//	Miscellaneous data types and macros not available in standard include files.  See also main.cpp for more documentation.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DATATYPESANDMACROS_H
#define DATATYPESANDMACROS_H

#define singleton	class	// A singleton is a class having one instance.  This keyword is to document which classes have unique instances, and therefore making the project easier to understand how different classes are interacting with each other

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Common data types
//
#ifndef VOID
	#define VOID void
	typedef char CHAR;
	typedef short SHORT;
	typedef long LONG;
#endif
#ifndef PVOID
	// Define commonly used data types
	typedef void * PVOID;
	typedef unsigned char       BYTE;
	typedef unsigned short      WORD;
	typedef quint32				UINT;	// In this project, the UINT is always 32 bits.  If someone wishes to use the native 'integer', he may use 'int' or 'unsigned'
	typedef int                 BOOL;
#endif
#ifndef PCVOID
	typedef const void * PCVOID;
#endif
typedef PVOID POBJECT;	// (po) Generic pointer to an object (rahter than just a void pointer).  This datatype is useful to better document an interface where the parameter is an object, however for technical reasons, the parameter is a a generic void *.
typedef PVOID PVPARAM;	// Similar as lParam, however points to a data structure (or object).  LPARAM may contain an integer value while PVPARAM is always a pointer to some data.

typedef long long L64;				// Hungarian prefix: l
typedef unsigned long long LU64;	// Hungarian prefix: lu

typedef intptr_t	INT_P;	// Integer having the width (capacity in bits) to store a pointer.  This integer is 64 bits when the executable is 64 bits.
typedef uintptr_t	UINT_P;

#ifndef Q_OS_WIN
	typedef UINT_P	WPARAM;
	typedef INT_P	LPARAM;
	typedef INT_P	LRESULT;
#endif

typedef char	CHA;	// Ansi character
typedef BYTE	CHU;	// Unicode UTF-8 encoded character
typedef WORD	CHW;	// Wide Unicode (UTF-16) character
//typedef wchar_t	CHW;	// Wide Unicode character (the wchar_t is 4 bytes under Linux)

typedef UINT	CHS;	// Variable to store a character on the stack.  Depending on the compiler and/or the CPU architecture, it may be preferable to use a different type (such as an unsigned integer) than than the type 'char' or 'unsigned char'
typedef UINT	CHWS;	// Variable to store a wide character on the stack;

typedef CHA * PSZA;
typedef CHU * PSZU;
typedef CHW * PSZW;

//	Although a pointer to a character and a pointer to a string are the same, they may differ in the way they are interpreted.
//	For instance, a function parsing text may return a pointer to the character where the error occurred, rather to the remaining of the string.
typedef CHA * PCHA;
typedef CHU * PCHU;
typedef CHW * PCHW;

typedef const CHU * PCHC;
typedef const CHU * PCHUC;

typedef const CHA * PSZAC;
typedef const CHU * PSZUC;
typedef const CHW * PSZWC;

typedef const CHU * PCHE;
typedef const CHU * PCHERROR;	// pche (see Main.cpp for details on this type of pointer)
typedef const CHW * PCHERRORW;	// pchew

typedef PSZUC PSZREMAINING;		// pszr (return a pointer which points to the remaining of the string of one of the input parameter), or NULL.
typedef PSZUC PSZR;
typedef PSZUC PSZRU;
typedef PSZAC PSZRA;

typedef PSZUC PSZROFFSET;	// pszro (return a pointer which is guaranteed to return offset from the input parameter)
typedef PSZUC PSZRO, PCHRO;
typedef PSZUC PSZROU;
typedef PSZAC PSZROA;

typedef PSZUC PSZREMAININGNONZERO;	// pszrnz (no need for the _NZ() suffix)
typedef PSZUC PSZRNZ;
typedef PSZUC PSZRNZU;
typedef PSZWC PSZRNZW;


///////////////////////////////////////////////////////////////////////////////////////////////////
//	typedef USZU
//
//	A 32-bit value that can be casted to a string.  This is somewhat an 'extended' character having more than 256 values as it is the case of a byte.
//
//	The motivation for this rather strange datatype is the combination of performance and simplicity.  The 32-bit value may be used in a switch() statement while
//	it can be directly casted as a string.
//
//	CAPACITY
//	Up to 3 UTF-8 characters may be stored in the USZU.  Since each UTF-8 character has an effective capacity of roughly 7 bits, there is about 21 bits availables giving approximately 2 million possible combination of characters.
//
//	PLATFORM NOTES
//	The datatype UINT can be casted into a string with a little-endian processor.  For big-endian processor, the value must be adjusted.  Use the macros _USZU*() to ensure portability.
//
typedef UINT USZU;
typedef USZU USZUF;			// An USZU with flags.  To convert from USZUF to USZU, use the macro USZU_from_USZUF()

	#define _USZU1(ch1)					(ch1)
	#define _USZU2(ch1, ch2)			((ch1) | ((ch2) << 8))
	#define _USZU3(ch1, ch2, ch3)		((ch1) | ((ch2) << 8) | ((ch3) << 16))
	#define _USZUF(f)					((f) << 24)		// Add flags to the USZU
	#define USZU_from_USZUF(uszu)		((uszu) & 0x00FFFFFF)	// Strip the flags
	#define USZU1_from_USZUX(uszu)		((BYTE)(uszu))			// Strip everything and keep only the first byte

	#define PszFromUSZU(uszu)			((PSZU)&(uszu))		// Cast the 32-bit integer to a null-terminated string.
	USZU UszuFromPsz(PSZUC pszUszu);


typedef int ENUM;					// Generic enumeration type
#define ENUM_NIL		(-1)		// Invalid enumerated value
#define ENUM_NULL		  0			// Default/empty enumerated value


#define FALSE			0
#define TRUE			1


typedef LU64 TIMESTAMP;		// ts	- Unique value to identify  events in a chronological order.  The timestamp is initialized with QDateTime::toMSecsSinceEpoch().
	#define d_ts_zNULL			(TIMESTAMP)0	// Empty/null timestamp.  This #define is used when pushing a zero timestamp onto the stack when compiling 32-bit executable, otherwise the stack will not be aligned properly.
	#define d_ts_zNA			(TIMESTAMP)0	// Similar as d_ts_zNULL, however indicates the timestamp is unused, and therefore not applicable.
	#define d_ts_cSeconds		1000		// Number of seconds in a timestamp
	#define d_ts_cMinutes		60000		// Number of minutes in a timestamp
	#define d_ts_cHours			3600000		// Number of hours in a timestamp
	#define d_ts_cDays			86400000LL	// Number of days in a timestamp
	#define d_cSecondsPerHour	3600
	#define d_cSecondsPerDay	86400
	#define d_cSecondsPerWeek	604800
	#define d_cHoursPerWeek		168

typedef L64 TIMESTAMP_DELTA;	// dts - The difference between two TIMESTAMP

typedef UINT TIMESTAMP_MINUTES;	// tsm - Number of minutes since the applicatin started
//typedef UINT TIMESTAMP32;		// uts	- Timestamp in seconds since Jan 1st, 2000.  (This counter wraps every 136 years.  Perhaps in 100 years, it shoudl be extended to 64 bits).  This datatype is not used at the moment.

typedef L64 AMOUNT;				// amt - An amount is a value to display in the context of a currency such as BTC, USD or some commodity.  An amount is typically divided by its smallest unit (such as 100,000,000 Satoshis for BTC, and 100 cents for USD)

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Documenting macros (see also main.cpp for Hungarian prefixes)
//
#define IN				// Input parameter.  Typically the parameter/structure/object is marked as 'const' but there are cases where the parameter is an input parameter (not modified) depending on the state of other parameters.
#define IN_MOD_TMP		// The input parameter is temporary modified and restore to its original value before the routine terminates. This is often used for optimization to avoid duplicating the data, such as when parsing a string and inserting a temporary null-terminator within the string.
#define IN_MOD_SORT		// The input parameter will be modified by sorting the content of the data (typically an array).  Again, this is an optimization indicating the variable is slightly modified.
#define IN_MOD_INV		// The content of the input parameter will be modified by the routine.  This is not the same as an INOUT parameter because the modifications are for optimization purpose, leaving the input parameter invalid/indeterminate/garbage when the routine terminates.  If the caller wishes to keep the value, it must make a copy of the input parameter before calling such routine.
#define OUT				// Output parameter.  An output parameter does not require to be initialized since the function/method will take care of initializing the all relevant output fields.
#define OUT_F_UNCH		// Output parameter remains unchanged if function or method fails.  On the other hand, the output parameter is modified if the method succeeds.  The OUT_F_UNCH parameter may also be used at the end of a method to indicate an object remains unchanged if its method fails.
#define OUT_F_INV		// Output parameter is invalid/indeterminate/garbage if function or method fails.  This is often the case where a function starts to fill a structure to return the result, and along the way finds an error.  Instead of rolling back the data structure, the function simply return a failure code leaving the output parameter in invalid/indeterminate state.
#define OUT_F_VALID		// Output parameter is valid only the function or method fails.  This is the complement of OUT_F_UNCH and such parameter is usually used to return detailed information about the error.
#define OUT_ZZR			// The output parameter will return zero if the input parameter(s) is empty or if there is an error.  This parameter is used when parsing data from input parameter(s).
#define OUT_UNTIL_STOP	// The output paremter will return whatever value it computed until it stopped.  For instance, parsing a number and stopping at a space, or a comma separator.
#define OUT_OPTIONAL	// The oputput parameter is optional, meaning the data is typically returned by the function, or the output parameter is not necessary.
#define OUT_IGNORE		// The output parameter must be ignored.  This output parameter is used as a temporary buffer and the values inside this buffer shall not be used directly.  The routine typically returns a pointer to the temporary buffer or return a pointer to some static or member data.
#define	OUT_IGNORED		// The caller acknowledges an OUT_IGNORE parameter
#define OUT_ERROR		// This is valid only for the CError object, meaning it may contain an error code, but may be empty while there is an error.  The caller is therefore responsible of checking both the pError parameter as well as the returned error.
#define INOUT			// The parameter is both input and output.  The parameter must be initialized before calling the function/method, and the function/method will initialize/return more fields.
#define INOUT_F_UNCH	// The parameter is both input and output, however the parameter will be modified only if the function/method returns 'success'.  If the routine fails (or return FALSE), the content of the output parameter remains unchanged.
#define INOUT_F_UNCH_S	// The parameter is essentially OUT_F_UNCH, however the routine may return success without modifying the output parameter.  Of course, if the routine fails, the output parameters are unchanged.  An example of this is parsing a number; if the string is empty, then the output parameter (storage for the number) is unchanged and the routine returns success.  If the string contains invalid data, the number remains unchanged and the routine returns failure or an error code.
#define INOUT_F_VALID	// The parameter is essentially OUT_F_VALID, however the routine may return success while modifying the output parameter.  For instance, a warning message may be stored in the CErrorMessage while the routine returns success.
#define INOUT_LATER		// The parameter is IN at the moment, however will become an OUT parameter Later.  A typical case is an object added to a queue for processing.
#define IOUT			// The object is essentially an output parameter, but may have some input data.  In other words, the input data in the object won't be removed.  An example of this is appending to an array the objects matching a given criteria; the array may not be empty and it does not matter.

// All keywords beginning with PA_ are related to allocated objects, typically because an allocated object begins with "pa" (pointer allocated), however it may begin with "ha" (handle allocated), or something similar with the "a" prefix.
#define PA_CHILD		// The parameter is becoming child of the function (or object), and therefore the function will take care of deleting the allocated object when going out of scope.
#define PA_CHILDREN		// All remaining parameters of the functions are PA_CHILD.  This is a short way to document a function having multiple parameters, especially with _V(), _VL() or _VZ().  A function ending with _PA() does not need this keyword.
#define PA_PARENT		// The parameter is the parent of the object.  This keyword is typically used when allocating a child object and passing a pointer to the parent to the constructor.  This keyword is not redundant to a variable name containing the word "parent" as the variable may be a pointer without being responsible of deleting the object.
#define PA_DELETING		// The function is deleting the object (typically the parameter is deleted before the function returns, however the actual deletion may occur later).  In any regards, the caller should no longer dereference this pointer anymore.  This keyword may also be used by methods having no parameters, however deleting the object.  Example: paObject->deleteLater(PA_DELETING).
#define PA_OUT			// The function is allocating an output parameter, and therefore the caller is responsible of deleting the content of the output parameter.
#define PA_INOUT		// The parameter is INOUT and some data is allocated by the function, therefore the caller is responsible of deleting the content of the output parameter.
#define PA_TASK			// The parameter is an allocated task, which is somewhat a combination of PA_INOUT and PA_DELETING.  Once the task completes, the function/object holding the pointer to the task will automatically delete the task object.
#define PA_CAST_FROM_P	// Cast to a pointer allocated from a regular pointer.  Since the cast is not visible by the compiler, this keyword is there to explicitly show the allocation/ownership of the object is now in the pointer 'pa'.  Here is an example: PA_CAST_FROM_P pwWidget->setParent(NULL); delete pwWidget;
#define PA_CAST_TO_P	// Cast an allocated pointer to a regular pointer.  This is to show a "pa" is now a regular pointer.

#define IGNORED			// The output parameter is ignored because the caller does not care about its return value.  Sometimes a routine returns multiple values, and only a few of these parameters are of interest. Obviouslly this keyword is used by the caller with the OUT parameter.
#define PARAM_NOT_USED	// The parameter is not used. This is typically when an interface has several parameters, however given the implementation of the routine, the parameter is not used.
#define d_zNA		0	// Not Applicable.  The parameter has a value of zero because it does not apply to the context of the other parameters or variables.
#define d_zDEFAULT	0	// Use the default value of zero


///////////////////////////////////////////////////////////////////////////////////////////////////
//	CONST_TEMPORARY_MODIFIED
//
//	The object or variable is modified, however restored to its original state before the routine/method returns.
//	This keyword is a bit different than IN_MOD_TMP because it applies mostly to an object where a method may temporary modify the object.
//	The keyword IN_MOD_TMP applies mostly to parameters of a method, rather than an entire object.
//
#define CONST_TEMPORARY_MODIFIED


///////////////////////////////////////////////////////////////////////////////////////////////////
//	CONST_OBJECT
//
//	The method preserves the state of the object, however the
//	'const' keyword cannot be used because the state of the object
//	may be modified and restored by the method.
//
#define CONST_OBJECT


///////////////////////////////////////////////////////////////////////////////////////////////////
//	CONST_VIRTUAL
//
//	The method cannot be const because it calls a non-const virtual method
//	The virtual method is unlikely to modify the state of the object, but
//	for flexibility, the virtual method is non-const.
//
//	A good example of this is a method calling the virtual method CDocument::IsModified().
//	The virtual method IsModified() does not change the state of the object, however (for flexibility), is not 'const'.
//
#define CONST_VIRTUAL


///////////////////////////////////////////////////////////////////////////////////////////////////
//	CONST_MUTABLE
//
//	This keyword is complement to the 'mutable' keyword.
//	Instead of declaring all member data as mutable, it is easier to
//	declare the method (or the input parameter) as CONST_MUTABLE.
//
//	One example of this is when serializing data.  The state of the
//	object is not modified, however the virtual method XmlExchange()
//	cannot be 'const' because the same method can be used to unserialize data.
//
//	Definition of mutable: If a data member is declared mutable, then it is legal
//	to assign a value to this data member from a const member function.
//
#define CONST_MUTABLE


///////////////////////////////////////////////////////////////////////////////////////////////////
//	CONST_CODE_PATH
//
//	The parameter or the method is constant because of its code path.
//	Sometimes a parameter may be both input and output, however given other
//	parameters, the parameter may become input only.
//
//	A good example is the API ::DrawText() where the RECT parameter is CONST_CODE_PATH
//	if the DT_CALCRECT is not specified.
//
#define CONST_CODE_PATH


///////////////////////////////////////////////////////////////////////////////////////////////////
//	CONST_OUTPUT_BUFFER
//
//	The method is essentially const, however there is one output buffer that will be modified.
//	The reason this is 'const' is because calling the same method over and over again does not fundamentally change the state/values of the object.
//
#define CONST_OUTPUT_BUFFER

///////////////////////////////////////////////////////////////////////////////////////////////////
//	CONST_MODIFIED
//
//	The method must named 'const' because it modifies a mutable variable.
//	Without this keyword, the compiler would give an error of calling a non-const method.
//	In other words, CONST_MODIFIED indicates the method does modify the object, however the 'const' keyword must be present to make the compiler happy.
#define CONST_MODIFIED		const


///////////////////////////////////////////////////////////////////////////////////////////////////
//	CONST_MAY_CREATE_CACHE
//
//	The routine may create a cache to improve the performance next time the same value is requested.
//	The motivation for using a cache is reducing the memory requirements and save CPU cycles for values frequently accessed.
//	The macro is called 'const' because the object remains unchanged, as caching information does not fundamentally change the state/values of the object.
//
//	Again, the cache may not last the lifetime of the object as it may be destroyed and re-created.
//	As a result, depending on the implementation, it may be wise to not keep pointers to data inside this cache for between messages.
//
#define CONST_MAY_CREATE_CACHE
#define CONST_MCC		// Abbreviation to CONST_MAY_CREATE_CACHE

///////////////////////////////////////////////////////////////////////////////////////////////////
//	CONST_INTERFACE_ALLOW_CACHE_CREATION
//
//	The CONST_INTERFACE_ALLOW_CACHE_CREATION is complement to CONST_MAY_CREATE_CACHE to indicate
//	the virtual method does not modify the state of the object, however since it is a virtual method
//	from an interface allowing cache creation, the virtual method cannot be const.
//
#define CONST_INTERFACE_ALLOW_CACHE_CREATION


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Handy macros
//
#define LENGTH(rg)									(sizeof(rg)/sizeof(rg[0]))	// Return the length of an array
#define IS_ALIGNED_16(pv)							(((UINT_P)(pv) & 1) == 0)
#define IS_ALIGNED_32(pv)							(((UINT_P)(pv) & 3) == 0)
#define NEXT_UINT16(pv)								(((UINT_P)(pv) + 1) & ~1)
#define NEXT_UINT32(pv)								(((UINT_P)(pv) + 3) & ~3)
#define UINT16_FROM_CHARS(chA, chB)					((chB) << 8 | (chA))		// Create a WORD from two characters.  This macro is useful to append two characters in a single instruction (Example: *pwString = UINT16_FROM_CHARS('a', 'b') // Append characters 'a' and 'b' into WORD * pwString)
#define UINT32_FROM_CHARS(chA, chB, chC, chD)		((chD) << 24 | (chC) << 16 | (chB) << 8 | (chA))

/*
#pragma GCC diagnostic ignored "-Winvalid-offsetof"	// This compile directive is necessary for the use of macro OFFSET_OF_STRUCTURE()

//	Compute (at compile time) the offset of a structure.
#define OFFSET_OF_STRUCTURE(struct_name, field_name)		((UINT_P)(&((struct_name *)(0))->field_name))

//	The macro PCastPointerFromOffset() must be used with extreme care, as it cannot cast a NULL pointer.
#define PCastPointerFromOffset(base_class, field, pData)	(base_class *)(((BYTE *)pData) - OFFSET_OF_STRUCTURE(base_class, field))
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Useful macros for portability
//
#define InitToZeroes(pv, cb)			memset(pv, 0, cb)

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Macros to create RGB colors and definitions of commonly used colors

typedef QRgb QRGB;	// co

//	The macro QRGB() differs from the Win32 RGB() which stores the colors in the opposite order.
//	This way, casting QRGB to QRgb is done easily.
#define MAKE_QRGB(r,g,b)	((QRGB)(((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((UINT)(BYTE)(r))<<16)))

#define MAKE_QRGB_(x)		x		// Macro to convert an hexadecimal value to a QRGB value

#define d_coWhite			MAKE_QRGB(255, 255, 255)
#define d_coBlack			MAKE_QRGB(0, 0, 0)
#define d_coSilver			MAKE_QRGB_(0xEEEEEE)
#define d_coGrayLight		MAKE_QRGB(128, 128, 128)
#define d_coGray			MAKE_QRGB(96, 96, 96)
#define d_coGrayDark		MAKE_QRGB(64, 64, 64)
#define d_coGreen			MAKE_QRGB(0, 128, 0)
#define d_coGreenDark		MAKE_QRGB(0, 100, 0)
#define d_coGreenDarker		MAKE_QRGB(0, 64, 0)
#define d_coBlueSky			MAKE_QRGB_(0x87CEEB)
#define d_coBlue			MAKE_QRGB(0, 0, 196)
#define d_coBlueDark		MAKE_QRGB(0, 0, 128)
#define d_coYellowDirty		MAKE_QRGB(128, 128, 0)
#define d_coYellowDark		MAKE_QRGB(200, 200, 0)
#define d_coYellowPure		MAKE_QRGB(255, 255, 0)
#define d_coOrange			MAKE_QRGB(255, 96, 0)
#define d_coOrangeRed		MAKE_QRGB(255, 69, 0)
#define d_coRedDark			MAKE_QRGB(128, 0, 0)
#define d_coRed				MAKE_QRGB(200, 0, 0)
#define d_coRedHot			MAKE_QRGB(255, 0, 0)

#define d_coAqua			MAKE_QRGB_(0x00FFFF)
#define d_coChocolate 		MAKE_QRGB_(0xD2691E)
#define c_coGold			MAKE_QRGB_(0xFFD700)
#define d_coGoldenRod		MAKE_QRGB_(0xDAA520)
#define d_coGreenYellow		MAKE_QRGB_(0xADFF2F)
#define d_coThistle			MAKE_QRGB_(0xD8BFD8)		// This color is a light violet
#define d_coViolet			MAKE_QRGB_(0xEE82EE)


//	Extensions of the RGB triple
typedef QRGB QRGBX;	// cox

#define COX_mskfBold				0x01000000	// Display the color in bold.
#define COX_mskmRGB					0x00FFFFFF	// Mask for the RGB colors

#define COX_MakeBold(co)			(COX_mskfBold | (co))
#define COX_IsBold(cox)				((cox) & COX_mskfBold)
#define COX_GetQRGB(cox)			(QRgb)((cox) & COX_mskmRGB)
#define COX_GetRGB(co)				((((co) & 0xFF0000) >> 16) | (co & 0x00FF00) | (((co) & 0x0000FF) << 16))		// Macro to convert from QRGB to RGB (this is done by swapping the Red and Blue components)

#endif // DATATYPESANDMACROS_H

///////////////////////////////////////////////////////////////////////////////////////////////////
//	ErrorCodes.h
//
//	This file contains a list of error codes specific to the application.
//
//	The motivation for having all the errors codes stored within a single file is
//	the ability to chain errors and making it easier for the developer to display errors to the user.
//
///////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////
//	Enumeration of application-specific error codes.
enum EError	// err
{
	errSuccess = 0,				// No error occurred (the function/method was successful).

	errFileOpenError,			// The file could not be opened, typically because the file cannot be located on disk (file not found).
	errFilePermissionsError,	// Unable to read file because of security permissions (or maybe the file is already opened by another process).
	errFileReadError,			// An error occurred while reading from the file.
	errFileWriteError,			// An error occurred while writing to a file file.

	errParseXmlPrematureEof_i,					// Unexpected end of file at line $i
	errParseXmlPrematureEofForCDATA_i,			// Premature end of file while looking for termination of <![CDATA[ from line $i
	errParseXmlElementMismatch_ssi,				// The closing element <$s> does not match its opening element <$s> from line $i
	errParseXmlAttributeInvalidName_s,			// The attribute name '$s' is invalid
	errParseXmlAttributeMissingOpeningQuote_s,	// The attribute '$s' is missing an opening quote for its value
	errParseXmlAttributeMissingClosingQuote_s,	// The attribute '$s' is missing its closing quote for its value
	errParseXmlAttributeMissingValue_s,			// The attribute '$s' is missing a value

	errXmpp,									// Generic XMPP error
	errXmppNotAuthorized,						// The password is not valid
	errSocketWriteError,
	errTaskNeedsMoreProcessing,					// The routine executed without error, however needs more processing.  An example is task sending a large file over XMPP.  The task must send small messages of 4 kB each and return this error code to indicate it needs more processing time to complete.

	errContactDuplicate,						// The contact is already in the list

	errMax,	// Last error defined

	_mskfErrorSilent	= 0x1000				// Flag indicating the error is silent, that is, should not be reported to the user.  An error which has been displayed to the user may also become silent to avoid reporting the same error multiple times
}; // enum EError

#define EError_EMakeSilentError(err)		((EError)((err) | _mskfErrorSilent))	// Convert a normal error code into a silent error
#define EError_FuIsSilentError(err)			((BOOL)((err) & _mskfErrorSilent))			// Return non-zero if the error code is silent


///////////////////////////////////////
enum ESeverity	// Severity/importance of a string
	{
	eSeverityNull				= 0,		// The message has zero 'severity', which means to not display or log the message

	eSeverityNoise				= 1,		// Display a message considered 'noise', mostly for debugging or verbose log.
	eSeverityComment			= 2,		// Display the string as a comment (in green color)
	eSeverityInfoTextBlue		= 3,		// Display something informational to the user using the blue color
	eSeverityInfoTextBlueDark	= 4,		// Display something informational to the user using a darker blue color
	eSeverityInfoTextBlack		= 5,		// Display something informational to the user using the black color
	eSeverityWarning			= 6,		// Display a warning to user (in orange color).  A warning is a rare code path, however nothing wrong with Cambrian.
	eSeverityWarningToErrorLog	= 7,		// Display a warning to Error Log.  This warning is NOT an error, however displayed to the Error Log to draw the attention.
	eSeverityErrorWarning		= 8,		// Report a minor error error to user.  This is typically an unsual code path taken by the program worth reporting to the user as a warning, typically from invalid data.
	eSeverityErrorAssert		= 9,		// This error is the equivalent of a non-blocking Assert() which represents a flaw in the code.  Such error does not mean Cambrian will crash, however it means there is an inconsistency within the code worth reporting to the developer(s).  For isntance, if an icon fails to load, it probably means the icon is not in the resource.
	};

QRGBX CoxFromSeverity(ESeverity eSeverity);
QRgb CoFromSeverity(ESeverity eSeverity);

class CErrorMessage;


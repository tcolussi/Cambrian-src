///////////////////////////////////////////////////////////////////////////////////////////////////
//	Shared strings.
//
//	Those strings are used everywhere in the code, sometimes unrelated, yet having the same value.
//	Once a string is defined in this file, its value cannot be changed.
//
//	This file also contains prototypes of simple helper functions to find a shared string.
//
//	IMPLEMENTATION NOTES
//	Any string in this file must use the 'extern' keyword, otherwise the compiler will create a duplicate for every instance, making the code bigger.
//	Without the 'extern' keyword, some routines will no longer work (read: bug) because they compare pointers of string without using the method strcmp().
//
#ifndef STRINGSSHARED_H
#define STRINGSSHARED_H

extern const CStr c_strEmpty;
extern const CString c_sEmpty;
extern const QByteArray c_arraybEmpty;
extern const QVariant c_vEmpty;		// Empty/null variant
extern const QDateTime c_dtEmpty;	// Empty/null datetime

extern const CHU c_szXmppServerDefault[];

extern const char c_sza_xcp[];
#define d_szCambrianProtocol_xcp					"xosp"	// eXtensible Object Synchronization Protocol (this is the xml element enabling SocietyPro to send encrypted messages)
#define d_szCambrianProtocol_Attribute_hSignature	"s"		// Signature for every stanza within the element <xcp>.  Example: "<xcp s='X$l|J5N#bjN2kmLyJ){CA2ZB^'> .... </xcp>


#define d_sz0			"\0"					// An empty string, typically to create a double null-terminated string szm[].
#define c_szuEmpty		(PSZUC)c_szaEmpty

extern const char c_szaEmpty[];				// Also c_szwEmpty
extern const char c_szNewLine[];			// Line Feed
extern const char c_szCRLF[];				// Carriage Return + Line Feed
extern const char c_szaCommaSeparator[];	// Separator between multiple elements

extern const char c_szaVersion[];

///////////////////////////////////////////////////////////////////////////////////////////////////
//	XML Stanza Elements
extern const char c_sza_xml[];
extern const char c_sza_xmlns[];
extern const char c_sza_type[];
extern const char c_sza_conflict[];
extern const char c_sza_failure[];
extern const char c_sza_error[];
extern const char c_sza_success[];
extern const char c_sza_cancel[];
extern const char c_sza_text[];
extern const char c_sza_message[];
extern const char c_sza_body[];
extern const char c_sza_id[];
extern const char c_sza_jid[];
extern const char c_sza_sid[];
extern const char c_sza_iq[];
extern const char c_sza_si[];
extern const char c_sza_from[];
extern const char c_sza_to[];
extern const char c_sza_open[];
extern const char c_sza_close[];
extern const char c_sza_data[];
extern const char c_sza_query[];
extern const char c_sza_result[];
extern const char c_sza_file[];
extern const char c_sza_size[];
extern const char c_sza_state[];
extern const char c_sza_ping[];

extern const char c_szaApplicationClass_[];
extern const char c_szaApplicationClass_MayanX[];
extern const char c_szaApplicationClass_Ballotmaster[];

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Stanza XML namespaces
extern const char c_szaXmlStreamError[];
extern const char c_szaXmlStreamClose[];
extern const char c_szXmlns_jabber_x_data[];	// ^:xd
extern const char c_szXmlns_jabber_client[];	// ^:jc
extern const char c_szXmlns_urn_ietf_params_xml_ns_xmpp_sasl[];		// ^:ss
extern const char c_szXmlns_urn_ietf_params_xml_ns_xmpp_bind[];		// ^:xb
extern const char c_szXmlns_urn_ietf_params_xml_ns_xmpp_stanzas[];	// ^:xs

#define d_wXmlnsPCSZU					UINT16_FROM_CHARS('#', 's')
#define d_wXmlnsFileTransfer			UINT16_FROM_CHARS('f', 't')
#define d_wXmlnsFeatureNegociation		UINT16_FROM_CHARS('f', 'n')
#define d_wXmlnsStreamInitiation		UINT16_FROM_CHARS('s', 'i')
#define d_wXmlnsInBand					UINT16_FROM_CHARS('i', 'b')
#define d_wXmlnsJabberXData				UINT16_FROM_CHARS('x', 'd')
#define d_wXmlnsJabberClient			UINT16_FROM_CHARS('j', 'c')
#define d_wXmlnsXmppSasl				UINT16_FROM_CHARS('s', 's')
#define d_wXmlnsXmppBind				UINT16_FROM_CHARS('x', 'b')
#define d_wXmlnsXmppStanzas				UINT16_FROM_CHARS('x', 's')

PSZAC PszGetXmlnsFromWord(UINT wXmlns);

//	A direct way to store the HTML entity &nbsp; (&#160;)
#define d_ch_nbsp_A		0xC2
#define d_ch_nbsp_B		0xA0
#define d_w_nbsp		UINT16_FROM_CHARS(0xC2, 0xA0)
#define d_szu_nbsp		"\xC2\xA0" // ^_ or ^~

#endif // STRINGSSHARED_H

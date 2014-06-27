//	Routines to find shared strings

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

const CHU c_szXmppServerDefault[] = d_szXmppServerDefault;
const char c_sza_xcp[] = d_szCambrianProtocol_xcp;

const char c_szaEmpty[] = "\0";
const char c_szNewLine[]= "\n";
const char c_szCRLF[]	= "\r\n";
const char c_szaCommaSeparator[] = ", ";

const char c_szaVersion[] = "Version";

const char c_sza_xml[] = "xml";
const char c_sza_xmlns[] = "xmlns";
const char c_sza_type[] = "type";
const char c_sza_conflict[] = "conflict";
const char c_sza_failure[] = "failure";
const char c_sza_error[] = "error";
const char c_sza_success[] = "success";
const char c_sza_cancel[] = "cancel";
const char c_sza_text[] = "text";
const char c_sza_message[] = "message";
const char c_sza_body[] = "body";
const char c_sza_id[] = "id";
const char c_sza_jid[] = "jid";
const char c_sza_sid[] = "sid";
const char c_sza_iq[] = "iq";
const char c_sza_si[] = "si";
const char c_sza_from[] = "from";
const char c_sza_to[] = "to";
const char c_sza_open[] = "open";
const char c_sza_close[] = "close";
const char c_sza_data[] = "data";
const char c_sza_query[] = "query";
const char c_sza_result[] = "result";
const char c_sza_file[] = "file";
const char c_sza_size[] = "size";
const char c_sza_state[] = "state";
const char c_sza_ping[] = "ping";

const char c_szaXmlStreamError[] = "stream:error";
const char c_szaXmlStreamClose[] = "</stream:stream>";
const char c_szXmlns_jabber_x_data[] = "jabber:x:data";	// ^:xd
const char c_szXmlns_jabber_client[] = "jabber:client";	// ^:jc
const char c_szXmlns_urn_ietf_params_xml_ns_xmpp_sasl[]		= "urn:ietf:params:xml:ns:xmpp-sasl";		// ^:ss
const char c_szXmlns_urn_ietf_params_xml_ns_xmpp_bind[]		= "urn:ietf:params:xml:ns:xmpp-bind";		// ^:xb
const char c_szXmlns_urn_ietf_params_xml_ns_xmpp_stanzas[]	= "urn:ietf:params:xml:ns:xmpp-stanzas";	// ^:xs

PSZAC
PszGetXmlnsFromWord(UINT wXmlns)
	{
	switch (wXmlns)
		{
	case d_wXmlnsFileTransfer:
		return "http://jabber.org/protocol/si/profile/file-transfer";
	case d_wXmlnsFeatureNegociation:
		return "http://jabber.org/protocol/feature-neg";
	case d_wXmlnsStreamInitiation:
		return "http://jabber.org/protocol/si";
	case d_wXmlnsInBand:
		return "http://jabber.org/protocol/ibb";
	case d_wXmlnsJabberXData:
		return c_szXmlns_jabber_x_data;
	case d_wXmlnsJabberClient:
		return c_szXmlns_jabber_client;
	case d_wXmlnsXmppSasl:
		return c_szXmlns_urn_ietf_params_xml_ns_xmpp_sasl;
	case d_wXmlnsXmppBind:
		return c_szXmlns_urn_ietf_params_xml_ns_xmpp_bind;
	case d_wXmlnsXmppStanzas:
		return c_szXmlns_urn_ietf_params_xml_ns_xmpp_stanzas;
		} // switch
	Assert(FALSE && "Unknown 'xmlsn' value");
	return NULL;
	} // PszGetXmlnsFromWord()

// =================================================================================================
// Copyright 2003-2009 Adobe
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it. If you have received this file from a source other 
// than Adobe, then your use, modification, or distribution of it requires the prior written permission
// of Adobe.
//
// Adobe patent application tracking #P435, entitled 'Unique markers to simplify embedding data of
// one format in a file with a different format', inventors: Sean Parent, Greg Gilley.
// =================================================================================================

#if AdobePrivate
// =================================================================================================
// Change history
// ==============
//
// Writers:
//  AWL Alan Lillich
//  FNO Frank Nocke
//  IJS Inder Jeet Singh
//
// mm-dd-yy who Description of changes, most recent on top.
//
// 02-09-15 AJ  5.6-c038 Fixing some more warnings due to implicit typecasting
// 01-31-14 IJS 5.5-c022 Adding serialize flag kXMP_IncludeRDFHash to  include the  hash of the RDF in xmppacket.
// 09-21-12 AWL 5.5-c011 Remove Transform XMP.
//
// 05-03-12	AWL	5.4-c004 [3180908] Fix the serialization to handle namespaces where one prefix is the tail of another.
// 03-09-12	AWL	5.4-c003 Change the canonical serialization to use one outer rdf:Description element.
//
// 12-15-10 AWL 5.3-c005 Add support for canonical RDF serialization.
//
// 06-15-09 AWL 5.0-c036 Mark critical locking variables volatile, fix all namespace table usage to be clean.
// 06-11-09 AWL 5.0-c034 Finish threading revamp, implement friendly reader/writer locking.
// 05-04-09 AWL 5.0-c026 Remove compact structs and arrays from TransformXMP.
// 02-17-09 FNO 5.0-c023 [0656083] Remove moot kXMP_WriteAliasComments option.
// 04-03-09 AWL 5.0-c022 Change PlainXMP to TransformXMP.
// 03-27-09 AWL 5.0-c019 [1801772] Fix SerializeToBuffer to sanity check for outrageously large padding.
// 03-03-09 FNO 5.0-c016 [1801772] undoing failed fix attempt.
// 02-26-09 FNO 5.0-c014 [1801772] (fixing prior bug-fix attempt.)
// 02-18-09 FNO 5.0-c011 [1801772] SerializeToBuffer to sanity-check for kXMPFiles_UnknownLength.
//
// 07-01-08 AWL 4.3-c060 Hide all uses of PlainXMP.
//
// 10-09-07 AWL 4.2-c021 Add support for kXMP_OmitXMPMetaElement, no x:xmpmeta when serializing.
//
// 11-10-06 AWL 4.1-c024 [1418912] Fix serialize assert that just checks the space estimate.
// 08-16-06 AWL 4.1-c016 [1312441,1340116] Oops, tab/LF/CR are getting serialized doubly escaped.
//				Fix AppendNodeValue to output them as "&#xn;", where 'n' is '9', 'A', or 'D'.
// 08-16-06 AWL 4.1-c015 [1312441,1340116] Change the parsing code to tolerate ISO Latin-1 input. At
//				the same time, get rid of the ugly hackery that tries to deal with unescaped ASCII
//				control characters. Also get rid of the check and complaint about parsing into an
//				empty object. This is predominantly what people want, so clear the XMPMeta object.
//
// 06-07-06 AWL 4.0-c007 Improve the Unicode conversions.
// 05-16-06 AWL 4.0-c006 Add SXMPUtils::PackageForJPEG and SXMPUtils::MergeFromJPEG. Implement the
//				kXMP_OmitAllFormatting option in SerializeToBuffer.
// 05-01-06 AWL 4.0-c005 Revamp the embedded, Mac PList, and WIndows Property info.
// 03-24-06 AWL 4.0-c001 Adapt for move to ham-perforce, integrate XMPFiles, bump version to 4.
//
// 01-10-06 AWL 3.3-009 Enable Plain XMP serialization, fix Plain XMP parsing glitches.
//
// 06-07-05 AWL 3.2-112 Serialize empty packets nicely.
// 06-06-05 AWL 3.2-111 [0536565] Fix the compact serialization of empty structs.
// 06-03-05 AWL 3.2-109 [0467370] Parse failures must leave the XMP object empty, ready for another parse.
// 06-03-05 AWL 3.2-108 Output empty element in RDF for an empty array, e.g. <rdf:Seq/>.
// 06-02-05 AWL 3.2-105 [1016912] Use empty element form when rdf:Description has only attribute content.
// 06-01-05 AWL 3.2-105 [1110051] Add delete-existing option for SetProperty.
// 06-01-05 AWL 3.2-103 [0509601,1204017] Fix SetLocalizedText to find and set a matching "real"
//				language value when the specific_lang parameter is "x-default".
// 05-27-05 AWL 3.2-102 Move the setting of the alloc/free procs to the top of XMPMeta::Initialize.
// 05-16-05 AWL 3.2-100 Complete the deBIBification, integrate the internal and SDK source. Bump the
//              version to 3.3 and build to 100, well ahead of main's latest 3.3-009.
//
// 05-02-05 AWL 3.2-019 Add the Dynamic Media namespace, kXMP_NS_DM, "http://ns.adobe.com/xmp/1.0/DynamicMedia/".
// 04-14-05 AWL 3.2-018 Move the padding param, add overloads to simplify use of SerializeToBuffer.
// 04-13-05 AWL 3.2-017 Improve the documentation and behavior of Get/SetLocalizedText.
// 04-11-05 AWL 3.2-016 Add AdobePrivate conditionals where appropriate.
// 04-08-05 AWL 3.2-015 Undo unnecessary constant changes in XMP_Const.h - keep compatibility over
//				"perfection" so that SDK and internal code are easier to merge.
// 04-07-05 AWL 3.2-014 Fix SetLocalizedText to set the alt-text bit for empty alt array. This became
//				broken in the changes for 3.2-010 (SDK) and 3.3-003 (main). Don't throw an exception
//				if x:xmpmeta is missing and kXMP_RequireXMPMeta is used. Old code ignored that
//				candidate. This changed in the previous build's root node lookup changes.
// 04-06-05 AWL 3.2-013 [0509601] Normalize "single value" alt-text arrays. Improve the way the root
//				XML node is found and extract the previous toolkit version number.
// 04-05-05 AWL 3.2-012 [1172565] Move instance ID from rdf:about to xmpMM:InstanceID.
// 04-05-05 AWL 3.2-011 [0532345] Normalize xml:lang values so that compares are in effect case
//				insensitive as required by RFC 3066. Change parsing and serializing to force the
//				x-default item to be first.
// 04-01-05 AWL 3.2-010 Add leafOptions parameter to FindNode, used when creating new nodes.
// 03-17-05 AWL 3.2-006 Revise Plain XMP parsing and serialization for latest proposal.
// 02-16-05 AWL 3.2-005 Add first cut of Plain XMP parsing and serialization.
// 02-14-05 AWL 3.2-003 Add thread locks.
// 02-11-05 AWL 3.2-002 Add client reference counting.
// 01-28-05 AWL 3.2-001 Remove BIB.
//
// 02-02-05 AWL 3.1.1-110 [1145657] Fix XMPMeta::SerializeToBuffer to do an exact size UTF-16 or UTF-32
//				packet correctly. It was doing UTF-8 of the exact size, then converting.
// 01-26-05 AWL 3.1.1-107 [1141684] Add XMPMeta::UnregisterAssertNotify and XMPMeta::SendAssertNotify.
// 01-25-05 AWL 3.1.1-106 [1141007] Add XMPMeta::RegisterAssertNotify.
// 01-17-05 AWL 3.1.1-105 [1135773] Delete empty schema in addition to removing the pointer from the
//				tree node's child vector. This bug was introduced in the -101 changes.
// 01-07-05 AWL 3.1.1-104 [1129616] Fix SetObjectName to verify that the string is UTF-8.
// 12-14-04 AWL 3.1.1-101 [1085740] Delete empty schema nodes in CleanupParsedAliases.
// 12-14-04 AWL 3.1.1-100 [1022350,1075328] Add more namespaces and internal/external properties.
// 12-09-04 AWl 3.1.1-099 [1041438] Add parsing and serialization support for UTF-16 and UTF-32.
// 11-05-04 AWL 3.1.1-093 [1037508] Touch up exif:GPSTimeStamp values that have zero for the date.
// 11-05-04 AWL 3.1.1-092 [1106408] Add new IPTC namespace.
// 10-29-04 AWL 3.1.1-089 [1099634] Fix ParseFromBuffer to properly handle a partial escape followed
//				by another escape. E.g. "blah blah &#" then "xA;&#xA; blah blah".
// 10-20-04 AWL 3.1.1-085 [1084185] Fix XMP_InternalRef to not depend on BIBContainerBase layout.
// 10-12-04 AWL 3.1.1-084 [0616293] Add value checking to SetNodeValue, prefix checking to RegisterNamespace.
// 10-06-04 AWL 3.1.1-083 [1061778] Add lock tracing under TraceXMPLocking.
// 10-05-04 AWL 3.1.1-082 [1061778] Use recursive form of BIB mutex.
//
// 08-27-04 AWL 3.1-079 Remove overly aggressive option checks in SerializeAsRDF.
// 08-27-04 AWL 3.1-078 Add EXIF_Aux and CameraRaw namespaces.
// 08-23-04 AWL 3.1-076 Add kXMP_NS_PSAlbum as a predefined namespace.
// 08-04-04 AWL 3.1-073 [1050719] Fix handling of an empty default namespace declaration.
// 07-28-04 AWL 3.1-069 [1043286] Preflight parse buffers for ASCII controls.
// 07-26-04 AWL 3.1-068 [1046381,1046384] Fix XMPUtils::ConvertToXyz to throw instead of assert for
//				null or empty string. Fix XMPMeta::GetProperty_Xyz to make sure property is simple.
// 07-21-04 AWL 3.1-067 [1044036] Increase fudge factor for output reserve in SerializeToBuffer.
// 07-15-04 AWL 3.1-066 [1015904] Fix Get/SetLocalizedText to allow empty alt arrays.
// 07-14-04 AWL 3.1-062 [1026373] Strip old iX:changes elements during parsing.
// 07-14-04 AWL 3.1-059 [0658992] Fix SetQualifier to throw if property does not exist.
// 07-13-04 AWL 3.1-058 [1014255] Remove empty schema nodes.
// 07-13-04 AWL 3.1-057 [1006568] Check all XPath components for valid namespace prefixes.
// 07-09-04 AWL 3.1-053 [1037918] Use double escaping for ASCII controls other than tab, lf, and cr.
// 06-10-04 AWL 3.1-047 Fix more HPUX and AIX compilation errors and warnings.
// 06-04-04 AWL 3.1-046 Fix HPUX compilation errors and warnings.
//
// 05-25-04 AWL [1018426] Hide all use of cin/cout streams in #if DEBUG or equivalent.
// 05-14-04 AWL [1016811,1018220] Fix escaping bugs in serialization.
// 04-30-04 AWL Add new & delete operators that call BIBMemory functions. Change static objects that
//				require allocation to explicit pointers. Properly delete partial tree when parse
//				errors throw an exception. Clean up memory leaks.
// 04-19-04 AWL [1010249] Don't write a final newline after the packet trailer, the packet must end
//			    at the '>' of the trailer.
// 04-16-04 AWL Fix major bogosity in IsAttrQualifier XMPMeta.cpp.
// 04-16-04 AWL Inflate serialization size estimate to accommodate anticipated character entities,
//              e.g. &#xA; for newlines. There can be a lot in the base 64 encoding of a thumbnail.
// 03-17-04 AWL Cleanup error exceptions, make sure all have a reasonable message. Have GetProperty
//				and friends throw an exception if the schema namespace is not registered.
// 03-04-04 AWL Add hacks to handle the DC singleton denormalization of the XMP from Acrobat 5.
// 02-19-04 AWL Add the new "transient" standard namespace.
// 02-06-04 AWL Register a couple of missing standard namespaces.
// 01-29-04 AWL Add AppendArrayItem.
// 01-26-04 AWL Revamp expanded XPath handling to work better for aliases.
// 04-24-03	AWL	Initial start on the new implementation.
//
// =================================================================================================
#endif /* AdobePrivate */

#include "public/include/XMP_Environment.h"	// ! This must be the first include!

#if XMP_DebugBuild
	#include <iostream>
#endif

#include "XMPCore/source/XMPCore_Impl.hpp"

#include "XMPCore/source/XMPMeta.hpp"

#include "public/include/XMP_Version.h"
#if AdobePrivate
	#include "XMPCore/source/XMPCore_ChangeLog.hpp"
	#include "build/XMP_BuildInfo.h"
#endif

#include "source/UnicodeInlines.incl_cpp"
#include "source/UnicodeConversions.hpp"
#include "third-party/zuid/interfaces/MD5.h"

using namespace std;

#if XMP_WinBuild
	#pragma warning ( disable : 4533 )	// initialization of '...' is skipped by 'goto ...'
	#pragma warning ( disable : 4702 )	// unreachable code
	#pragma warning ( disable : 4800 )	// forcing value to bool 'true' or 'false' (performance warning)
#endif


// *** Use the XMP_PropIsXyz (Schema, Simple, Struct, Array, ...) macros
// *** Add debug codegen checks, e.g. that typical masking operations really work
// *** Change all uses of strcmp and strncmp to XMP_LitMatch and XMP_LitNMatch


// =================================================================================================
// Local Types and Constants
// =========================

static const char * kPacketHeader  = "<?xpacket begin=\"\xEF\xBB\xBF\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>";
static const char * kPacketTrailer = "<?xpacket end=\"w\"?>";	// ! The w/r is at [size-4].

#if AdobePrivate
static const char * kTXMP_PacketStart = "<txmp:XMP_Packet";
static const char * kTXMP_PacketEnd   = "</txmp:XMP_Packet>";
#endif

static const char * kTXMP_SchemaGroup = "XMP_SchemaGroup";

static const char * kRDF_XMPMetaStart = "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"";
static const char * kRDF_XMPMetaEnd   = "</x:xmpmeta>";

static const char * kRDF_RDFStart     = "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">";
static const char * kRDF_RDFEnd       = "</rdf:RDF>";

static const char * kRDF_SchemaStart  = "<rdf:Description rdf:about=";
static const char * kRDF_SchemaEnd    = "</rdf:Description>";

static const char * kRDF_StructStart  = "<rdf:Description>";
static const char * kRDF_StructEnd    = "</rdf:Description>";

static const char * kRDF_BagStart     = "<rdf:Bag>";
static const char * kRDF_BagEnd       = "</rdf:Bag>";

static const char * kRDF_SeqStart     = "<rdf:Seq>";
static const char * kRDF_SeqEnd       = "</rdf:Seq>";

static const char * kRDF_AltStart     = "<rdf:Alt>";
static const char * kRDF_AltEnd       = "</rdf:Alt>";

static const char * kRDF_ItemStart    = "<rdf:li>";
static const char * kRDF_ItemEnd      = "</rdf:li>";

static const char * kRDF_ValueStart   = "<rdf:value>";
static const char * kRDF_ValueEnd     = "</rdf:value>";


// =================================================================================================
// Static Variables
// ================


// =================================================================================================
// Local Utilities
// ===============


// -------------------------------------------------------------------------------------------------
// EstimateRDFSize
// ---------------

//  *** Pull the strlen(kXyz) calls into constants.

static size_t
EstimateRDFSize ( const XMP_Node * currNode, XMP_Index indent, size_t indentLen )
{
	size_t outputLen = 2 * (indent*indentLen + currNode->name.size() + 4);	// The property element tags.
	
	if ( ! currNode->qualifiers.empty() ) {
		// This node has qualifiers, assume it is written using rdf:value and estimate the qualifiers.

		indent += 2;	// Everything else is indented inside the rdf:Description element.
		outputLen += 2 * ((indent-1)*indentLen + strlen(kRDF_StructStart) + 2);	// The rdf:Description tags.
		outputLen += 2 * (indent*indentLen + strlen(kRDF_ValueStart) + 2);		// The rdf:value tags.

		for ( size_t qualNum = 0, qualLim = currNode->qualifiers.size(); qualNum < qualLim; ++qualNum ) {
			const XMP_Node * currQual = currNode->qualifiers[qualNum];
			outputLen += EstimateRDFSize ( currQual, indent, indentLen );
		}

	}
	
	if ( currNode->options & kXMP_PropValueIsStruct ) {
		indent += 1;
		outputLen += 2 * (indent*indentLen + strlen(kRDF_StructStart) + 2);	// The rdf:Description tags.
	} else if ( currNode->options & kXMP_PropValueIsArray ) {
		indent += 2;
		outputLen += 2 * ((indent-1)*indentLen + strlen(kRDF_BagStart) + 2);		// The rdf:Bag/Seq/Alt tags.
		outputLen += 2 * currNode->children.size() * (strlen(kRDF_ItemStart) + 2);	// The rdf:li tags, indent counted in children.
	} else if ( ! (currNode->options & kXMP_SchemaNode) ) {
		outputLen += currNode->value.size();	// This is a leaf value node.
	}

	for ( size_t childNum = 0, childLim = currNode->children.size(); childNum < childLim; ++childNum ) {
		const XMP_Node * currChild = currNode->children[childNum];
		outputLen += EstimateRDFSize ( currChild, indent+1, indentLen );
	}

	return outputLen;
	
}	// EstimateRDFSize


// -------------------------------------------------------------------------------------------------
// DeclareOneNamespace
// -------------------

static void
DeclareOneNamespace	( XMP_StringPtr   nsPrefix,
					  XMP_StringPtr   nsURI,
					  XMP_VarString	& usedNS,		// ! A catenation of the prefixes with colons.
					  XMP_VarString & outputStr,
					  XMP_StringPtr   newline,
					  XMP_StringPtr   indentStr,
					  XMP_Index       indent )
{
	XMP_VarString boundedPrefix = ":";
	boundedPrefix += nsPrefix;
	size_t nsPos = usedNS.find ( boundedPrefix );

	if ( nsPos == XMP_VarString::npos ) {
		
		outputStr += newline;
		for ( ; indent > 0; --indent ) outputStr += indentStr;
		outputStr += "xmlns:";
		outputStr += nsPrefix;
		if (outputStr[outputStr.size ( ) - 1] == ':')
			outputStr[outputStr.size ( ) - 1] = '=';	// Change the colon to =.
		else
			outputStr += '=';
		outputStr += '"';
		outputStr += nsURI;
		outputStr += '"';

		usedNS += nsPrefix;

	}

}	// DeclareOneNamespace


// -------------------------------------------------------------------------------------------------
// DeclareElemNamespace
// --------------------

static void
DeclareElemNamespace ( const XMP_VarString & elemName,
					   XMP_VarString &		 usedNS,
					   XMP_VarString &		 outputStr,
					   XMP_StringPtr		 newline,
					   XMP_StringPtr		 indentStr,
					   XMP_Index			 indent )
{
	size_t colonPos = elemName.find ( ':' );

	if ( colonPos != XMP_VarString::npos ) {
		XMP_VarString nsPrefix ( elemName.substr ( 0, colonPos+1 ) );
		XMP_StringPtr nsURI;
		bool nsFound = sRegisteredNamespaces->GetURI ( nsPrefix.c_str(), &nsURI, 0 );
		XMP_Enforce ( nsFound );
		DeclareOneNamespace ( nsPrefix.c_str(), nsURI, usedNS, outputStr, newline, indentStr, indent );
	}

}	// DeclareElemNamespace


// -------------------------------------------------------------------------------------------------
// DeclareUsedNamespaces
// ---------------------

static void
DeclareUsedNamespaces ( const XMP_Node * currNode,
						XMP_VarString &  usedNS,
						XMP_VarString &	 outputStr,
						XMP_StringPtr	 newline,
						XMP_StringPtr	 indentStr,
						XMP_Index		 indent )
{

	if ( currNode->options & kXMP_SchemaNode ) {
		// The schema node name is the URI, the value is the prefix.
		DeclareOneNamespace ( currNode->value.c_str(), currNode->name.c_str(), usedNS, outputStr, newline, indentStr, indent );
	} else if ( currNode->options & kXMP_PropValueIsStruct ) {
		for ( size_t fieldNum = 0, fieldLim = currNode->children.size(); fieldNum < fieldLim; ++fieldNum ) {
			const XMP_Node * currField = currNode->children[fieldNum];
			DeclareElemNamespace ( currField->name, usedNS, outputStr, newline, indentStr, indent );
		}
	}

	for ( size_t childNum = 0, childLim = currNode->children.size(); childNum < childLim; ++childNum ) {
		const XMP_Node * currChild = currNode->children[childNum];
		DeclareUsedNamespaces ( currChild, usedNS, outputStr, newline, indentStr, indent );
	}

	for ( size_t qualNum = 0, qualLim = currNode->qualifiers.size(); qualNum < qualLim; ++qualNum ) {
		const XMP_Node * currQual = currNode->qualifiers[qualNum];
		DeclareElemNamespace ( currQual->name, usedNS, outputStr, newline, indentStr, indent );
		DeclareUsedNamespaces ( currQual, usedNS, outputStr, newline, indentStr, indent );
	}
	
}	// DeclareUsedNamespaces

// -------------------------------------------------------------------------------------------------
// EmitRDFArrayTag
// ---------------

enum {
	kIsStartTag = true,
	kIsEndTag   = false
};

static void
EmitRDFArrayTag	( XMP_OptionBits  arrayForm,
				  XMP_VarString & outputStr,
				  XMP_StringPtr	  newline,
				  XMP_StringPtr	  indentStr,
				  XMP_Index		  indent,
				  XMP_Index		  arraySize,
				  bool			  isStartTag )
{
	if ( (! isStartTag) && (arraySize == 0) ) return;
	
	for ( XMP_Index level = indent; level > 0; --level ) outputStr += indentStr;
	if ( isStartTag ) {
		outputStr += "<rdf:";
	} else {
		outputStr += "</rdf:";
	}

	if ( arrayForm & kXMP_PropArrayIsAlternate ) {
		outputStr += "Alt";
	} else if ( arrayForm & kXMP_PropArrayIsOrdered ) {
		outputStr += "Seq";
	} else {
		outputStr += "Bag";
	}
	
	if ( isStartTag && (arraySize == 0) ) outputStr += '/';
	outputStr += '>';
	outputStr += newline;

}	// EmitRDFArrayTag


// -------------------------------------------------------------------------------------------------
// AppendNodeValue
// ---------------
//
// Append a property or qualifier value to the output with appropriate XML escaping. The escaped
// characters for elements and attributes are '&', '<', '>', and ASCII controls (tab, LF, CR). In
// addition, '"' is escaped for attributes. For efficiency, this is done in a double loop. The outer
// loop makes sure the whole value is processed. The inner loop does a contiguous unescaped run
// followed by one escaped character (if we're not at the end).
//
// We depend on parsing and SetProperty logic to make sure there are no invalid ASCII controls in
// the XMP values. The XML spec only allows tab, LF, and CR. Others are not even allowed as
// numeric escape sequences.

enum {
	kForAttribute = true,
	kForElement   = false
};

static void
AppendNodeValue ( XMP_VarString & outputStr, const XMP_VarString & value, bool forAttribute )
{

	unsigned char * runStart = (unsigned char *) value.c_str();
	unsigned char * runLimit  = runStart + value.size();
	unsigned char * runEnd;
	unsigned char   ch;
	
	while ( runStart < runLimit ) {
	
		for ( runEnd = runStart; runEnd < runLimit; ++runEnd ) {
			ch = *runEnd;
			if ( forAttribute && (ch == '"') ) break;
			if ( (ch < 0x20) || (ch == '&') || (ch == '<') || (ch == '>') ) break;
		}
		
		outputStr.append ( (char *) runStart, (runEnd - runStart) );
		
		if ( runEnd < runLimit ) {

			if ( ch < 0x20 ) {
			
				XMP_Assert ( (ch == kTab) || (ch == kLF) || (ch == kCR) );

				char hexBuf[16];
				memcpy ( hexBuf, "&#xn;", 6 );	// AUDIT: Length of "&#xn;" is 5, hexBuf size is 16.
				hexBuf[3] = kHexDigits[ch&0xF];
				outputStr.append ( hexBuf, 5 );

			} else {

				if ( ch == '"' ) {
					outputStr += "&quot;";
				} else if ( ch == '<' ) {
					outputStr += "&lt;";
				} else if ( ch == '>' ) {
					outputStr += "&gt;";
				} else {
					XMP_Assert ( ch == '&' );
					outputStr += "&amp;";
				}

			}

			++runEnd;

		}
		
		runStart = runEnd;
	
	}

}	// AppendNodeValue


// -------------------------------------------------------------------------------------------------
// CanBeRDFAttrProp
// ----------------

static bool
CanBeRDFAttrProp ( const XMP_Node * propNode )
{
	
	if ( propNode->name[0] == '[' ) return false;
	if ( ! propNode->qualifiers.empty() ) return false;
	if ( propNode->options & kXMP_PropValueIsURI ) return false;
	if ( propNode->options & kXMP_PropCompositeMask ) return false;
	
	return true;
	
}	// CanBeRDFAttrProp


// -------------------------------------------------------------------------------------------------
// IsRDFAttrQualifier
// ------------------

static XMP_StringPtr sAttrQualifiers[] = { "xml:lang", "rdf:resource", "rdf:ID", "rdf:bagID", "rdf:nodeID", "" };

static bool
IsRDFAttrQualifier ( XMP_VarString qualName )
{

	for ( size_t i = 0; *sAttrQualifiers[i] != 0; ++i ) {
		if ( qualName == sAttrQualifiers[i] ) return true;
	}

	return false;
	
}	// IsRDFAttrQualifier


// -------------------------------------------------------------------------------------------------
// StartOuterRDFDescription
// ------------------------
//
// Start the outer rdf:Description element, including all needed xmlns attributes. Leave the element
// open so that the compact form can add proprtty attributes.

static void
StartOuterRDFDescription ( const XMP_Node & xmpTree,
						   XMP_VarString &  outputStr,
						   XMP_StringPtr	newline,
						   XMP_StringPtr	indentStr,
						   XMP_Index		baseIndent )
{
	
	// Begin the outer rdf:Description start tag.
	
	for ( XMP_Index level = baseIndent+2; level > 0; --level ) outputStr += indentStr;
	outputStr += kRDF_SchemaStart;
	outputStr += '"';
	outputStr += xmpTree.name;
	outputStr += '"';
	
	// Write all necessary xmlns attributes.

	XMP_VarString usedNS;
	usedNS.reserve ( 400 );	// The predefined prefixes add up to about 320 bytes.
	usedNS = ":xml:rdf:";

	for ( size_t schema = 0, schemaLim = xmpTree.children.size(); schema != schemaLim; ++schema ) {
		const XMP_Node * currSchema = xmpTree.children[schema];
		DeclareUsedNamespaces ( currSchema, usedNS, outputStr, newline, indentStr, baseIndent+4 );
	}

}	// StartOuterRDFDescription

// -------------------------------------------------------------------------------------------------
// SerializeCanonicalRDFProperty
// -----------------------------
//
// Recursively handles the "value" for a node. It does not matter if it is a top level property, a
// field of a struct, or an item of an array.  The indent is that for the property element. An
// xml:lang qualifier is written as an attribute of the property start tag, not by itself forcing
// the qualified property form. The patterns below mostly ignore attribute qualifiers like xml:lang.
// Except for the one struct case, attribute qualifiers don't affect the output form.
//
//	<ns:UnqualifiedSimpleProperty>value</ns:UnqualifiedSimpleProperty>
//
//	<ns:UnqualifiedStructProperty>	(If no rdf:resource qualifier)
//		<rdf:Description>
//			... Fields, same forms as top level properties
//		</rdf:Description>
//	</ns:UnqualifiedStructProperty>
//
//	<ns:ResourceStructProperty rdf:resource="URI"
//		... Fields as attributes
//	>
//
//	<ns:UnqualifiedArrayProperty>
//		<rdf:Bag> or Seq or Alt
//			... Array items as rdf:li elements, same forms as top level properties
//		</rdf:Bag>
//	</ns:UnqualifiedArrayProperty>
//
//	<ns:QualifiedProperty>
//		<rdf:Description>
//			<rdf:value> ... Property "value" following the unqualified forms ... </rdf:value>
//			... Qualifiers looking like named struct fields
//		</rdf:Description>
//	</ns:QualifiedProperty>

enum { kUseCanonicalRDF = true, kUseAdobeVerboseRDF = false };
enum { kEmitAsRDFValue = true, kEmitAsNormalValue = false };

static void
SerializeCanonicalRDFProperty ( const XMP_Node * propNode,
								XMP_VarString &  outputStr,
								XMP_StringPtr	 newline,
								XMP_StringPtr	 indentStr,
								XMP_Index		 indent,
								bool			 useCanonicalRDF,
								bool			 emitAsRDFValue )
{
	XMP_Index level;
	bool emitEndTag   = true;
	bool indentEndTag = true;

	XMP_OptionBits propForm = propNode->options & kXMP_PropCompositeMask;

	// ------------------------------------------------------------------------------------------
	// Determine the XML element name. Open the start tag with the name and attribute qualifiers.
	
	XMP_StringPtr elemName = propNode->name.c_str();
	if ( emitAsRDFValue ) {
		elemName= "rdf:value";
	} else if ( *elemName == '[' ) {
		elemName = "rdf:li";
	}

	for ( level = indent; level > 0; --level ) outputStr += indentStr;
	outputStr += '<';
	outputStr += elemName;

	bool hasGeneralQualifiers = false;
	bool hasRDFResourceQual   = false;
	
	for ( size_t qualNum = 0, qualLim = propNode->qualifiers.size(); qualNum < qualLim; ++qualNum ) {
		const XMP_Node * currQual = propNode->qualifiers[qualNum];
		if ( ! IsRDFAttrQualifier ( currQual->name ) ) {
			hasGeneralQualifiers = true;
		} else {
			if ( currQual->name == "rdf:resource" ) hasRDFResourceQual = true;
			if ( ! emitAsRDFValue ) {
				outputStr += ' ';
				outputStr += currQual->name;
				outputStr += "=\"";
				AppendNodeValue ( outputStr, currQual->value, kForAttribute );
				outputStr += '"';
			}
		}
	}
	
	// --------------------------------------------------------
	// Process the property according to the standard patterns.
	
	if ( hasGeneralQualifiers && (! emitAsRDFValue) ) {
	
		// -----------------------------------------------------------------------------------------
		// This node has general, non-attribute, qualifiers. Emit using the qualified property form.
		// ! The value is output by a recursive call ON THE SAME NODE with emitAsRDFValue set.

		if ( hasRDFResourceQual ) {
			XMP_Throw ( "Can't mix rdf:resource and general qualifiers", kXMPErr_BadRDF );
		}

		if ( ! useCanonicalRDF ) {
			outputStr += " rdf:parseType=\"Resource\">";
			outputStr += newline;
		} else {
			outputStr += '>';
			outputStr += newline;
			indent += 1;
			for ( level = indent; level > 0; --level ) outputStr += indentStr;
			outputStr += "<rdf:Description>";
			outputStr += newline;
		}

		SerializeCanonicalRDFProperty ( propNode, outputStr, newline, indentStr, indent+1,
										useCanonicalRDF, kEmitAsRDFValue );

		for ( size_t qualNum = 0, qualLim = propNode->qualifiers.size(); qualNum < qualLim; ++qualNum ) {
			const XMP_Node * currQual = propNode->qualifiers[qualNum];
			if ( IsRDFAttrQualifier ( currQual->name ) ) continue;
			SerializeCanonicalRDFProperty ( currQual, outputStr, newline, indentStr, indent+1,
											useCanonicalRDF, kEmitAsNormalValue );
		}
		
		if ( useCanonicalRDF ) {
			for ( level = indent; level > 0; --level ) outputStr += indentStr;
			outputStr += "</rdf:Description>";
			outputStr += newline;
			indent -= 1;
		}
		
	} else {

		// --------------------------------------------------------------------
		// This node has no general qualifiers. Emit using an unqualified form.
		
		if ( propForm == 0 ) {
		
			// --------------------------
			// This is a simple property.
			
			if ( propNode->options & kXMP_PropValueIsURI ) {
				outputStr += " rdf:resource=\"";
				AppendNodeValue ( outputStr, propNode->value, kForAttribute );
				outputStr += "\"/>";
				outputStr += newline;
				emitEndTag = false;
			} else if ( propNode->value.empty() ) {
				outputStr += "/>";
				outputStr += newline;
				emitEndTag = false;
			} else {
				outputStr += '>';
				AppendNodeValue ( outputStr, propNode->value, kForElement );
				indentEndTag = false;
			}
			
		} else if ( propForm & kXMP_PropValueIsArray ) {
		
			// This is an array.
			outputStr += '>';
			outputStr += newline;
			EmitRDFArrayTag ( propForm, outputStr, newline, indentStr, indent+1, static_cast<XMP_Index>(propNode->children.size()), kIsStartTag );
			if ( XMP_ArrayIsAltText(propNode->options) ) NormalizeLangArray ( (XMP_Node*)propNode );
			for ( size_t childNum = 0, childLim = propNode->children.size(); childNum < childLim; ++childNum ) {
				const XMP_Node * currChild = propNode->children[childNum];
				SerializeCanonicalRDFProperty ( currChild, outputStr, newline, indentStr, indent+2,
												useCanonicalRDF, kEmitAsNormalValue );
			}
			EmitRDFArrayTag ( propForm, outputStr, newline, indentStr, indent+1, static_cast<XMP_Index>(propNode->children.size()), kIsEndTag );
		
		
		} else if ( ! hasRDFResourceQual ) {
		
			// This is a "normal" struct, use the nested field element form form.
			XMP_Assert ( propForm & kXMP_PropValueIsStruct );
			if ( propNode->children.size() == 0 ) {
				if ( ! useCanonicalRDF ) {
					outputStr += " rdf:parseType=\"Resource\"/>";
					outputStr += newline;
					emitEndTag = false;
				} else {
					outputStr += '>';
					outputStr += newline;
					for ( level = indent+1; level > 0; --level ) outputStr += indentStr;
					outputStr += "<rdf:Description/>";
					outputStr += newline;
				}
			} else {
				if ( ! useCanonicalRDF ) {
					outputStr += " rdf:parseType=\"Resource\">";
					outputStr += newline;
				} else {
					outputStr += '>';
					outputStr += newline;
					indent += 1;
					for ( level = indent; level > 0; --level ) outputStr += indentStr;
					outputStr += "<rdf:Description>";
					outputStr += newline;
				}
				for ( size_t childNum = 0, childLim = propNode->children.size(); childNum < childLim; ++childNum ) {
					const XMP_Node * currChild = propNode->children[childNum];
					SerializeCanonicalRDFProperty ( currChild, outputStr, newline, indentStr, indent+1,
													useCanonicalRDF, kEmitAsNormalValue );
				}
				if ( useCanonicalRDF ) {
					for ( level = indent; level > 0; --level ) outputStr += indentStr;
					outputStr += "</rdf:Description>";
					outputStr += newline;
					indent -= 1;
				}
			}
		
		} else {
		
			// This is a struct with an rdf:resource attribute, use the "empty property element" form.
			XMP_Assert ( propForm & kXMP_PropValueIsStruct );
			for ( size_t childNum = 0, childLim = propNode->children.size(); childNum < childLim; ++childNum ) {
				const XMP_Node * currChild = propNode->children[childNum];
				if ( ! CanBeRDFAttrProp ( currChild ) ) {
					XMP_Throw ( "Can't mix rdf:resource and complex fields", kXMPErr_BadRDF );
				}
				outputStr += newline;
				for ( level = indent+1; level > 0; --level ) outputStr += indentStr;
				outputStr += ' ';
				outputStr += currChild->name;
				outputStr += "=\"";
				outputStr += currChild->value;
				outputStr += '"';
			}
			outputStr += "/>";
			outputStr += newline;
			emitEndTag = false;
		
		}
		
	}
	
	// ----------------------------------
	// Emit the property element end tag.
	
	if ( emitEndTag ) {
		if ( indentEndTag ) for ( level = indent; level > 0; --level ) outputStr += indentStr;
		outputStr += "</";
		outputStr += elemName;
		outputStr += '>';
		outputStr += newline;
	}
	
}	// SerializeCanonicalRDFProperty


// -------------------------------------------------------------------------------------------------
// SerializeCanonicalRDFSchemas
// ----------------------------
//
// Each schema's properties are written to the single rdf:Description element. All of the necessary
// namespaces are declared in the rdf:Description element. The baseIndent is the base level for the
// entire serialization, that of the x:xmpmeta element. An xml:lang qualifier is written as an
// attribute of the property start tag, not by itself forcing the qualified property form.
//
//	<rdf:Description rdf:about="TreeName"
//			xmlns:ns="URI" ... >
//
//		... The actual properties of the schema, see SerializeCanonicalRDFProperty
//
//		<!-- ns1:Alias is aliased to ns2:Actual -->  ... If alias comments are wanted
//
//	</rdf:Description>

static void
SerializeCanonicalRDFSchemas ( const XMP_Node & xmpTree,
							   XMP_VarString &	outputStr,
							   XMP_StringPtr	newline,
							   XMP_StringPtr	indentStr,
							   XMP_Index		baseIndent,
							   bool				useCanonicalRDF )
{

	StartOuterRDFDescription ( xmpTree, outputStr, newline, indentStr, baseIndent );
	
	if ( xmpTree.children.size() > 0 ) {
		outputStr += ">";
		outputStr += newline;
	} else {
		outputStr += "/>";
		outputStr += newline;
		return;	// ! Done if there are no XMP properties.
	}

	for ( size_t schemaNum = 0, schemaLim = xmpTree.children.size(); schemaNum < schemaLim; ++schemaNum ) {
		const XMP_Node * currSchema = xmpTree.children[schemaNum];
		for ( size_t propNum = 0, propLim = currSchema->children.size(); propNum < propLim; ++propNum ) {
			const XMP_Node * currProp = currSchema->children[propNum];
			SerializeCanonicalRDFProperty ( currProp, outputStr, newline, indentStr, baseIndent+3,
											useCanonicalRDF, kEmitAsNormalValue );
		}
	}
	
	// Write the rdf:Description end tag.
	for ( XMP_Index level = baseIndent+2; level > 0; --level ) outputStr += indentStr;
	outputStr += kRDF_SchemaEnd;
	outputStr += newline;

}	// SerializeCanonicalRDFSchemas


// -------------------------------------------------------------------------------------------------
// SerializeCompactRDFAttrProps
// ----------------------------
//
// Write each of the parent's simple unqualified properties as an attribute. Returns true if all
// of the properties are written as attributes.

static bool
SerializeCompactRDFAttrProps ( const XMP_Node *	parentNode,
							   XMP_VarString &	outputStr,
							   XMP_StringPtr	newline,
							   XMP_StringPtr	indentStr,
							   XMP_Index		indent )
{
	size_t prop, propLim;
	bool allAreAttrs = true;

	for ( prop = 0, propLim = parentNode->children.size(); prop != propLim; ++prop ) {

		const XMP_Node * currProp = parentNode->children[prop];
		if ( ! CanBeRDFAttrProp ( currProp ) ) {
			allAreAttrs = false;
			continue;
		}
		
		outputStr += newline;
		for ( XMP_Index level = indent; level > 0; --level ) outputStr += indentStr;
		outputStr += currProp->name;
		outputStr += "=\"";
		AppendNodeValue ( outputStr, currProp->value, kForAttribute );
		outputStr += '"';

	}
	
	return allAreAttrs;

}	// SerializeCompactRDFAttrProps


// -------------------------------------------------------------------------------------------------
// SerializeCompactRDFElemProps
// ----------------------------
//
// Recursively handles the "value" for a node that must be written as an RDF property element. It
// does not matter if it is a top level property, a field of a struct, or an item of an array.  The
// indent is that for the property element. The patterns bwlow ignore attribute qualifiers such as
// xml:lang, they don't affect the output form.
//
//	<ns:UnqualifiedStructProperty-1
//		... The fields as attributes, if all are simple and unqualified
//	/>
//
//	<ns:UnqualifiedStructProperty-2 rdf:parseType="Resource">
//		... The fields as elements, if none are simple and unqualified
//	</ns:UnqualifiedStructProperty-2>
//
//	<ns:UnqualifiedStructProperty-3>
//		<rdf:Description
//			... The simple and unqualified fields as attributes
//		>
//			... The compound or qualified fields as elements
//		</rdf:Description>
//	</ns:UnqualifiedStructProperty-3>
//
//	<ns:UnqualifiedArrayProperty>
//		<rdf:Bag> or Seq or Alt
//			... Array items as rdf:li elements, same forms as top level properties
//		</rdf:Bag>
//	</ns:UnqualifiedArrayProperty>
//
//	<ns:QualifiedProperty rdf:parseType="Resource">
//		<rdf:value> ... Property "value" following the unqualified forms ... </rdf:value>
//		... Qualifiers looking like named struct fields
//	</ns:QualifiedProperty>

// *** Consider numbered array items, but has compatibility problems.
// *** Consider qualified form with rdf:Description and attributes.

static void
SerializeCompactRDFElemProps ( const XMP_Node *	parentNode,
							   XMP_VarString &	outputStr,
							   XMP_StringPtr	newline,
							   XMP_StringPtr	indentStr,
							   XMP_Index		indent )
{
	XMP_Index level;

	for ( size_t prop = 0, propLim = parentNode->children.size(); prop != propLim; ++prop ) {

		const XMP_Node * propNode = parentNode->children[prop];
		if ( CanBeRDFAttrProp ( propNode ) ) continue;

		bool emitEndTag = true;
		bool indentEndTag = true;

		XMP_OptionBits propForm = propNode->options & kXMP_PropCompositeMask;

		// -----------------------------------------------------------------------------------
		// Determine the XML element name, write the name part of the start tag. Look over the
		// qualifiers to decide on "normal" versus "rdf:value" form. Emit the attribute
		// qualifiers at the same time.
		
		XMP_StringPtr elemName = propNode->name.c_str();
		if ( *elemName == '[' ) elemName = "rdf:li";

		for ( level = indent; level > 0; --level ) outputStr += indentStr;
		outputStr += '<';
		outputStr += elemName;
	
		bool hasGeneralQualifiers = false;
		bool hasRDFResourceQual   = false;

		for ( size_t qualNum = 0, qualLim = propNode->qualifiers.size(); qualNum < qualLim; ++qualNum ) {
			const XMP_Node * currQual = propNode->qualifiers[qualNum];
			if ( ! IsRDFAttrQualifier ( currQual->name ) ) {
				hasGeneralQualifiers = true;
			} else {
				if ( currQual->name == "rdf:resource" ) hasRDFResourceQual = true;
				outputStr += ' ';
				outputStr += currQual->name;
				outputStr += "=\"";
				AppendNodeValue ( outputStr, currQual->value, kForAttribute );
				outputStr += '"';
			}
		}
		
		// --------------------------------------------------------
		// Process the property according to the standard patterns.
	
		if ( hasGeneralQualifiers ) {
		
			// -------------------------------------------------------------------------------------
			// The node has general qualifiers, ones that can't be attributes on a property element.
			// Emit using the qualified property pseudo-struct form. The value is output by a call
			// to SerializeCanonicalRDFProperty with emitAsRDFValue set.
			
			// *** We're losing compactness in the calls to SerializeCanonicalRDFProperty.
			// *** Should refactor to have SerializeCompactRDFProperty that does one node.

			outputStr += " rdf:parseType=\"Resource\">";
			outputStr += newline;

			SerializeCanonicalRDFProperty ( propNode, outputStr, newline, indentStr, indent+1,
											kUseAdobeVerboseRDF, kEmitAsRDFValue );

			size_t qualNum = 0;
			size_t qualLim = propNode->qualifiers.size();
			if ( propNode->options & kXMP_PropHasLang ) ++qualNum;
			
			for ( ; qualNum < qualLim; ++qualNum ) {
				const XMP_Node * currQual = propNode->qualifiers[qualNum];
				SerializeCanonicalRDFProperty ( currQual, outputStr, newline, indentStr, indent+1,
												kUseAdobeVerboseRDF, kEmitAsNormalValue );
			}
			
		} else {

			// --------------------------------------------------------------------
			// This node has only attribute qualifiers. Emit as a property element.
			
			if ( propForm == 0 ) {
			
				// --------------------------
				// This is a simple property.
				
				if ( propNode->options & kXMP_PropValueIsURI ) {
					outputStr += " rdf:resource=\"";
					AppendNodeValue ( outputStr, propNode->value, kForAttribute );
					outputStr += "\"/>";
					outputStr += newline;
					emitEndTag = false;
				} else if ( propNode->value.empty() ) {
					outputStr += "/>";
					outputStr += newline;
					emitEndTag = false;
				} else {
					outputStr += '>';
					AppendNodeValue ( outputStr, propNode->value, kForElement );
					indentEndTag = false;
				}
				
			} else if ( propForm & kXMP_PropValueIsArray ) {

				// -----------------
				// This is an array.
				
				outputStr += '>';
				outputStr += newline;
				EmitRDFArrayTag ( propForm, outputStr, newline, indentStr, indent+1, static_cast<XMP_Index>(propNode->children.size()), kIsStartTag );
			
				if ( XMP_ArrayIsAltText(propNode->options) ) NormalizeLangArray ( (XMP_Node*)propNode );
				SerializeCompactRDFElemProps ( propNode, outputStr, newline, indentStr, indent+2 );

				EmitRDFArrayTag ( propForm, outputStr, newline, indentStr, indent+1, static_cast<XMP_Index>(propNode->children.size()), kIsEndTag );

			} else {

				// ----------------------
				// This must be a struct.
				
				XMP_Assert ( propForm & kXMP_PropValueIsStruct );

				bool hasAttrFields = false;
				bool hasElemFields = false;
			
				size_t field, fieldLim;
				for ( field = 0, fieldLim = propNode->children.size(); field != fieldLim; ++field ) {
					XMP_Node * currField = propNode->children[field];
					if ( CanBeRDFAttrProp ( currField ) ) {
						hasAttrFields = true;
						if ( hasElemFields ) break;	// No sense looking further.
					} else {
						hasElemFields = true;
						if ( hasAttrFields ) break;	// No sense looking further.
					}
				}
				
				if ( hasRDFResourceQual && hasElemFields ) {
					XMP_Throw ( "Can't mix rdf:resource qualifier and element fields", kXMPErr_BadRDF );
				}
				
				if ( propNode->children.size() == 0 ) {
				
					// Catch an empty struct as a special case. The case below would emit an empty
					// XML element, which gets reparsed as a simple property with an empty value.
					outputStr += " rdf:parseType=\"Resource\"/>";
					outputStr += newline;
					emitEndTag = false;
				
				} else if ( ! hasElemFields ) {

					// All fields can be attributes, use the emptyPropertyElt form.
					SerializeCompactRDFAttrProps ( propNode, outputStr, newline, indentStr, indent+1 );
					outputStr += "/>";
					outputStr += newline;
					emitEndTag = false;

				} else if ( ! hasAttrFields ) {
				
					// All fields must be elements, use the parseTypeResourcePropertyElt form.
					outputStr += " rdf:parseType=\"Resource\">";
					outputStr += newline;
					SerializeCompactRDFElemProps ( propNode, outputStr, newline, indentStr, indent+1 );
				
				} else {
				
					// Have a mix of attributes and elements, use an inner rdf:Description.
					outputStr += '>';
					outputStr += newline;
					for ( level = indent+1; level > 0; --level ) outputStr += indentStr;
					outputStr += "<rdf:Description";
					SerializeCompactRDFAttrProps ( propNode, outputStr, newline, indentStr, indent+2 );
					outputStr += ">";
					outputStr += newline;
					SerializeCompactRDFElemProps ( propNode, outputStr, newline, indentStr, indent+1 );
					for ( level = indent+1; level > 0; --level ) outputStr += indentStr;
					outputStr += kRDF_StructEnd;
					outputStr += newline;

				}

			}

		}
		
		// ----------------------------------
		// Emit the property element end tag.
		
		if ( emitEndTag ) {
			if ( indentEndTag ) for ( level = indent; level > 0; --level ) outputStr += indentStr;
			outputStr += "</";
			outputStr += elemName;
			outputStr += '>';
			outputStr += newline;
		}

	}
	
}	// SerializeCompactRDFElemProps


// -------------------------------------------------------------------------------------------------
// SerializeCompactRDFSchemas
// --------------------------
//
// All properties from all schema are written in a single rdf:Description element, as are all of the
// necessary namespace declarations. The baseIndent is the base level for the entire serialization,
// that of the x:xmpmeta element. The x:xmpmeta and rdf:RDF elements have already been written.
//
// Top level simple unqualified properties are written as attributes of the (only) rdf:Description
// element. Structs, arrays, and qualified properties are written by SerializeCompactRDFElemProp. An
// xml:lang qualifier on a simple property prevents the attribute form.
//
//	<rdf:Description rdf:about="TreeName"
//			xmlns:ns="URI" ...
//		ns:UnqualifiedSimpleProperty="value" ... >
//		... The remaining properties of the schema, see SerializeCompactRDFElemProps
//	</rdf:Description>

static void
SerializeCompactRDFSchemas ( const XMP_Node & xmpTree,
							 XMP_VarString &  outputStr,
							 XMP_StringPtr	  newline,
							 XMP_StringPtr	  indentStr,
							 XMP_Index		  baseIndent )
{
	XMP_Index level;
	size_t schema, schemaLim;
	
	StartOuterRDFDescription ( xmpTree, outputStr, newline, indentStr, baseIndent );
	
	// Write the top level "attrProps" and close the rdf:Description start tag.
	bool allAreAttrs = true;
	for ( schema = 0, schemaLim = xmpTree.children.size(); schema != schemaLim; ++schema ) {
		const XMP_Node * currSchema = xmpTree.children[schema];
		allAreAttrs &= SerializeCompactRDFAttrProps ( currSchema, outputStr, newline, indentStr, baseIndent+3 );
	}
	if ( ! allAreAttrs ) {
		outputStr += ">";
		outputStr += newline;
	} else {
		outputStr += "/>";
		outputStr += newline;
		return;	// ! Done if all properties in all schema are written as attributes.
	}

	// Write the remaining properties for each schema.
	for ( schema = 0, schemaLim = xmpTree.children.size(); schema != schemaLim; ++schema ) {
		const XMP_Node * currSchema = xmpTree.children[schema];
		SerializeCompactRDFElemProps ( currSchema, outputStr, newline, indentStr, baseIndent+3 );
	}
	
	// Write the rdf:Description end tag.
	for ( level = baseIndent+2; level > 0; --level ) outputStr += indentStr;
	outputStr += kRDF_SchemaEnd;
	outputStr += newline;

}	// SerializeCompactRDFSchemas

// -------------------------------------------------------------------------------------------------
// SerializeAsRDF
// --------------
//
//	<?xpacket begin... ?>
//	<x:xmpmeta xmlns:x=... >
//		<rdf:RDF xmlns:rdf=... >
//
//			... The properties, see SerializeCanonicalRDFSchema or SerializeCompactRDFSchemas
//
//		</rdf:RDF>
//	</x:xmpmeta>
//	<?xpacket end... ?>

// *** Need to strip empty arrays?
// *** Option to strip/keep empty structs?
// *** Need to verify handling of rdf:type qualifiers in canonical and compact.
// *** Need to verify round tripping of rdf:ID and similar qualifiers, see RDF 7.2.21.
// *** Check cases of rdf:resource plus explicit attr qualifiers (like xml:lang).

static void
SerializeAsRDF ( const XMPMeta & xmpObj,
				 XMP_VarString & headStr,	// Everything up to the padding.
				 XMP_VarString & tailStr,	// Everything after the padding.
				 XMP_OptionBits	 options,
				 XMP_StringPtr	 newline,
				 XMP_StringPtr	 indentStr,
				 XMP_Index		 baseIndent )
{
	const size_t treeNameLen = xmpObj.tree.name.size();
	const size_t indentLen   = strlen ( indentStr );

	// First estimate the worst case space and reserve room in the output string. This optimization
	// avoids reallocating and copying the output as it grows. The initial count does not look at
	// the values of properties, so it does not account for character entities, e.g. &#xA; for newline.
	// Since there can be a lot of these in things like the base 64 encoding of a large thumbnail,
	// inflate the count by 1/4 (easy to do) to accommodate.
	
	// *** Need to include estimate for alias comments.
	
	size_t outputLen = 2 * (strlen(kPacketHeader) + strlen(kRDF_XMPMetaStart) + strlen(kRDF_RDFStart) + 3*baseIndent*indentLen);

	for ( size_t schemaNum = 0, schemaLim = xmpObj.tree.children.size(); schemaNum < schemaLim; ++schemaNum ) {
		const XMP_Node * currSchema = xmpObj.tree.children[schemaNum];
		outputLen += 2*(baseIndent+2)*indentLen + strlen(kRDF_SchemaStart) + treeNameLen + strlen(kRDF_SchemaEnd) + 2;
		outputLen += EstimateRDFSize ( currSchema, baseIndent+2, indentLen );
	}
	
	outputLen += (outputLen >> 2);	// Inflate by 1/4, an empirical fudge factor.
	
	// Now generate the RDF into the head string as UTF-8.
	
	XMP_Index level;
	
	std::string rdfstring;
	headStr.erase();
	rdfstring.reserve ( outputLen );

	// Write the rdf:RDF start tag.
	rdfstring += kRDF_RDFStart;
	rdfstring += newline;
	
	// Write all of the properties.
	if ( options & kXMP_UseCompactFormat ) {
		SerializeCompactRDFSchemas ( xmpObj.tree, rdfstring, newline, indentStr, baseIndent );
	} else {
		bool useCanonicalRDF = XMP_OptionIsSet ( options, kXMP_UseCanonicalFormat );
		SerializeCanonicalRDFSchemas ( xmpObj.tree, rdfstring, newline, indentStr, baseIndent, useCanonicalRDF );
	}

	// Write the rdf:RDF end tag.
	for ( level = baseIndent+1; level > 0; --level ) rdfstring += indentStr;
	rdfstring += kRDF_RDFEnd;
	// Write the packet header PI.
	if ( ! (options & kXMP_OmitPacketWrapper) ) {
		for ( level = baseIndent; level > 0; --level ) headStr += indentStr;
		headStr += kPacketHeader;
		headStr += newline;
	}

	// Write the xmpmeta element's start tag.
	if ( ! (options & kXMP_OmitXMPMetaElement) ) {
		for ( level = baseIndent; level > 0; --level ) headStr += indentStr;
		headStr += kRDF_XMPMetaStart;
		headStr += kXMPCore_VersionMessage  "\"";
		std::string digestStr;
		unsigned char digestBin [16];
		if (options & kXMP_IncludeRDFHash)
		{
			std::string hashrdf;	
			MD5_CTX    context;
			MD5Init ( &context );
			MD5Update ( &context, (XMP_Uns8*)rdfstring.c_str(), (unsigned int)rdfstring.size() );
			MD5Final ( digestBin, &context );
			char buffer [40];
			for ( int in = 0, out = 0; in < 16; in += 1, out += 2 ) {
				XMP_Uns8 byte = digestBin[in];
				buffer[out]   = kHexDigits [ byte >> 4 ];
				buffer[out+1] = kHexDigits [ byte & 0xF ];
			}
			buffer[32] = 0;
			digestStr.append ( buffer );
			headStr += " rdfhash=\"";
			headStr += digestStr + "\"";
			headStr += " merged=\"0\"";
		}
		headStr += ">";
		headStr += newline;
	}

	for ( level = baseIndent+1; level > 0; --level ) headStr += indentStr;
	headStr+= rdfstring ;
	headStr += newline;

	// Write the xmpmeta end tag.
	if ( ! (options & kXMP_OmitXMPMetaElement) ) {
		for ( level = baseIndent; level > 0; --level ) headStr += indentStr;
		headStr += kRDF_XMPMetaEnd;
		headStr += newline;
	}
	
	// Write the packet trailer PI into the tail string as UTF-8.
	tailStr.erase();
	if ( ! (options & kXMP_OmitPacketWrapper) ) {
		tailStr.reserve ( strlen(kPacketTrailer) + (strlen(indentStr) * baseIndent) );
		for ( level = baseIndent; level > 0; --level ) tailStr += indentStr;
		tailStr += kPacketTrailer;
		if ( options & kXMP_ReadOnlyPacket ) tailStr[tailStr.size()-4] = 'r';
	}
	
}	// SerializeAsRDF


// -------------------------------------------------------------------------------------------------
// SerializeToBuffer
// -----------------

void
XMPMeta::SerializeToBuffer ( XMP_VarString * rdfString,
							 XMP_OptionBits	 options,
							 XMP_StringLen	 padding,
							 XMP_StringPtr	 newline,
							 XMP_StringPtr	 indentStr,
							 XMP_Index		 baseIndent ) const
{
	XMP_Enforce( rdfString != 0 );
	XMP_Assert ( (newline != 0) && (indentStr != 0) );
	rdfString->erase();
	
	// Fix up some default parameters.
	
	enum { kDefaultPad = 2048 };
	size_t unicodeUnitSize = 1;
	XMP_OptionBits charEncoding = options & kXMP_EncodingMask;

	if ( charEncoding != kXMP_EncodeUTF8 ) {
		if ( options & _XMP_UTF16_Bit ) {
			if ( options & _XMP_UTF32_Bit ) XMP_Throw ( "Can't use both _XMP_UTF16_Bit and _XMP_UTF32_Bit", kXMPErr_BadOptions );
			unicodeUnitSize = 2;
		} else if ( options & _XMP_UTF32_Bit ) {
			unicodeUnitSize = 4;
		} else {
			XMP_Throw ( "Can't use _XMP_LittleEndian_Bit by itself", kXMPErr_BadOptions );
		}
	}
	
	if ( options & kXMP_OmitAllFormatting ) {
		newline = " ";	// ! Yes, a space for "newline". This ensures token separation.
		indentStr = "";
	} else {
		if ( *newline == 0 ) newline = "\xA";	// Linefeed
		if ( *indentStr == 0 ) {
			indentStr = " ";
			if ( ! (options & kXMP_UseCompactFormat) ) indentStr  = "   ";
		}
	}
	
	if ( options & kXMP_ExactPacketLength ) {
		if ( options & (kXMP_OmitPacketWrapper | kXMP_IncludeThumbnailPad) ) {
			XMP_Throw ( "Inconsistent options for exact size serialize", kXMPErr_BadOptions );
		}
		if ( (padding & (unicodeUnitSize-1)) != 0 ) {
			XMP_Throw ( "Exact size must be a multiple of the Unicode element", kXMPErr_BadOptions );
		}
	} else if ( options & kXMP_ReadOnlyPacket ) {
		if ( options & (kXMP_OmitPacketWrapper | kXMP_IncludeThumbnailPad) ) {
			XMP_Throw ( "Inconsistent options for read-only packet", kXMPErr_BadOptions );
		}
		padding = 0;
	} else if ( options & kXMP_OmitPacketWrapper ) {
		if ( options & kXMP_IncludeThumbnailPad ) {
			XMP_Throw ( "Inconsistent options for non-packet serialize", kXMPErr_BadOptions );
		}
		padding = 0;
	} else if ( options & kXMP_OmitXMPMetaElement ) {
		if ( options & kXMP_IncludeRDFHash ) {
			XMP_Throw ( "Inconsistent options for x:xmpmeta serialize", kXMPErr_BadOptions );
		}
		padding = 0;
	} else {
		if ( padding == 0 ) {
			padding = static_cast<XMP_StringLen>(kDefaultPad * unicodeUnitSize);
		} else if ( (padding >> 28) != 0 ) {
			XMP_Throw ( "Outrageously large padding size", kXMPErr_BadOptions );	// Bigger than 256 MB.
		}
		if ( options & kXMP_IncludeThumbnailPad ) {
			if ( ! this->DoesPropertyExist ( kXMP_NS_XMP, "Thumbnails" ) ) padding += (10000 * unicodeUnitSize);	// *** Need a better estimate.
		}
	}

	// Serialize as UTF-8, then convert to UTF-16 or UTF-32 if necessary, and assemble with the padding and tail.
	
	std::string tailStr;

	SerializeAsRDF ( *this, *rdfString, tailStr, options, newline, indentStr, baseIndent );

	if ( charEncoding == kXMP_EncodeUTF8 ) {

		if ( options & kXMP_ExactPacketLength ) {
			size_t minSize = rdfString->size() + tailStr.size();
			if ( minSize > padding ) XMP_Throw ( "Can't fit into specified packet size", kXMPErr_BadSerialize );
			padding -= minSize;	// Now the actual amount of padding to add.
		}
		
		size_t newlineLen = strlen ( newline );
	
		if ( padding < newlineLen ) {
			rdfString->append ( padding, ' ' );
		} else {
			padding -= newlineLen;	// Write this newline last.
			while ( padding >= (100 + newlineLen) ) {
				rdfString->append ( 100, ' ' );
				*rdfString += newline;
				padding -= (100 + newlineLen);
			}
			rdfString->append ( padding, ' ' );
			*rdfString += newline;
		}

		*rdfString += tailStr;
	
	} else {
	
		// Need to convert the encoding. Swap the UTF-8 into a local string and convert back. Assemble everything.
		
		XMP_VarString utf8Str, newlineStr;
		bool bigEndian = ((charEncoding & _XMP_LittleEndian_Bit) == 0);
		
		if ( charEncoding & _XMP_UTF16_Bit ) {

			std::string padStr ( "  " );  padStr[0] = 0;	// Assume big endian.
			
			utf8Str.swap ( *rdfString );
			ToUTF16 ( (UTF8Unit*)utf8Str.c_str(), utf8Str.size(), rdfString, bigEndian );
			utf8Str.swap ( tailStr );
			ToUTF16 ( (UTF8Unit*)utf8Str.c_str(), utf8Str.size(), &tailStr, bigEndian );

			if ( options & kXMP_ExactPacketLength ) {
				size_t minSize = rdfString->size() + tailStr.size();
				if ( minSize > padding ) XMP_Throw ( "Can't fit into specified packet size", kXMPErr_BadSerialize );
				padding -= minSize;	// Now the actual amount of padding to add (in bytes).
			}

			utf8Str.assign ( newline );
			ToUTF16 ( (UTF8Unit*)utf8Str.c_str(), utf8Str.size(), &newlineStr, bigEndian );
			size_t newlineLen = newlineStr.size();
	
			if ( padding < newlineLen ) {
				for ( int i = padding/2; i > 0; --i ) *rdfString += padStr;
			} else {
				padding -= newlineLen;	// Write this newline last.
				while ( padding >= (200 + newlineLen) ) {
					for ( int i = 100; i > 0; --i ) *rdfString += padStr;
					*rdfString += newlineStr;
					padding -= (200 + newlineLen);
				}
				for ( int i = padding/2; i > 0; --i ) *rdfString += padStr;
				*rdfString += newlineStr;
			}

			*rdfString += tailStr;

		} else {

			std::string padStr ( "    " );  padStr[0] = padStr[1] = padStr[2] = 0;	// Assume big endian.
			UTF8_to_UTF32_Proc Converter = UTF8_to_UTF32BE;

			if ( charEncoding & _XMP_LittleEndian_Bit ) {
				padStr[0] = ' '; padStr[1] = padStr[2] = padStr[3] = 0;
				Converter = UTF8_to_UTF32LE;
			}
			
			utf8Str.swap ( *rdfString );
			ToUTF32 ( (UTF8Unit*)utf8Str.c_str(), utf8Str.size(), rdfString, bigEndian );
			utf8Str.swap ( tailStr );
			ToUTF32 ( (UTF8Unit*)utf8Str.c_str(), utf8Str.size(), &tailStr, bigEndian );

			if ( options & kXMP_ExactPacketLength ) {
				size_t minSize = rdfString->size() + tailStr.size();
				if ( minSize > padding ) XMP_Throw ( "Can't fit into specified packet size", kXMPErr_BadSerialize );
				padding -= minSize;	// Now the actual amount of padding to add (in bytes).
			}

			utf8Str.assign ( newline );
			ToUTF32 ( (UTF8Unit*)utf8Str.c_str(), utf8Str.size(), &newlineStr, bigEndian );
			size_t newlineLen = newlineStr.size();
	
			if ( padding < newlineLen ) {
				for ( int i = padding/4; i > 0; --i ) *rdfString += padStr;
			} else {
				padding -= newlineLen;	// Write this newline last.
				while ( padding >= (400 + newlineLen) ) {
					for ( int i = 100; i > 0; --i ) *rdfString += padStr;
					*rdfString += newlineStr;
					padding -= (400 + newlineLen);
				}
				for ( int i = padding/4; i > 0; --i ) *rdfString += padStr;
				*rdfString += newlineStr;
			}

			*rdfString += tailStr;

		}
	
	}

}	// SerializeToBuffer

// =================================================================================================

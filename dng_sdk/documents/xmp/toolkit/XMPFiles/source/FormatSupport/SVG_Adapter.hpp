#ifndef __SVG_Adapter_hpp__
#define __SVG_Adapter_hpp__

// =================================================================================================
// Copyright 2015 Adobe
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it. If you have received this file from a source other 
// than Adobe, then your use, modification, or distribution of it requires the prior written permission
// of Adobe.
//
// This file includes implementation of SVG metadata, according to Scalable Vector Graphics (SVG) 1.1 Specification. 
// "https://www.w3.org/TR/2003/REC-SVG11-20030114/"
// Copyright � 1994-2002 World Wide Web Consortium, (Massachusetts Institute of Technology, 
// Institut National de Recherche en Informatique et en Automatique, Keio University). 
// All Rights Reserved . http://www.w3.org/Consortium/Legal
//
// =================================================================================================

#if AdobePrivate
// =================================================================================================
// Change history
// ==============
//
// Writers:
//  AB  Amit Bhatti
//
// mm-dd-yy who Description of changes, most recent on top.
//
// 03-13-15 AB	5.6-f140 Making SVG support only for UTF-8 SVG files.
// 03-12-15 AB  5.6-f139 Moving SVG_Adapter from XMPCore to XMPFiles.
// 03-04-15 AB	5.6-c043 Initial creation of SVG Adapter for SVG handler.
//
// =================================================================================================
#endif // AdobePrivate

#include "public/include/XMP_Environment.h"	// ! Must be the first #include!
#include "source/XMLParserAdapter.hpp"

// =================================================================================================
// Derived SVG parser adapter for Expat.
// =================================================================================================

#ifndef BanAllEntityUsage
#define BanAllEntityUsage	0
#endif

struct XML_ParserStruct;	// ! Hack to avoid exposing expat.h to all clients.
typedef struct XML_ParserStruct *XML_Parser;
typedef std::map<std::string, XMP_Int64>::iterator IteratorStringXMP_Int64;

#define kURI_SVG			   "http://www.w3.org/2000/svg"

struct OffsetStruct
{
	XMP_Int64 startOffset;
	XMP_Int64 nextOffset;
	XMP_Int64 endOffset;
	std::string parent;
	OffsetStruct() : startOffset( -1 ), nextOffset( -1 ), endOffset( -1 ) {}
	OffsetStruct( std::string reqParent ) : startOffset( -1 ), nextOffset( -1 ), endOffset( -1 ), parent( reqParent ) {}
};

typedef std::map<std::string, OffsetStruct>::iterator IteratorStringOffsetStruct;

class SVG_Adapter : public XMLParserAdapter {
public:

	XML_Parser parser;
	XMP_NamespaceTable * registeredNamespaces;

#if BanAllEntityUsage
	bool isAborted;
#endif

	SVG_Adapter( );
	virtual ~SVG_Adapter();

	virtual void ParseBuffer( const void * buffer, size_t length, bool last = true );
	virtual XMP_Bool ParseBufferNoThrow( const void * buffer, size_t length, bool last = true );

	virtual OffsetStruct GetElementOffsets( std::string elementName );
	virtual void RegisterElement( std::string elementName, std::string reqParent );

	virtual XMP_Int64 GetPIOffset( std::string PIName, XMP_Uns32 requiredIndex = 1 );
	virtual void RegisterPI( std::string PIName );
	virtual XMP_Bool IsParsingRequire();

	std::multimap<std::string, XMP_Int64> mPIWithOffsetMap;
	std::map<std::string, OffsetStruct> mOffsetsMap;
	XMP_Int64 firstSVGElementOffset;

	std::string mPrevRequiredElement;
	XMP_Uns32 depth;	
};

// =================================================================================================

#endif	// __SVG_Adapter_hpp__

// =================================================================================================
// Copyright Adobe
// Copyright 2008 Adobe
// All Rights Reserved
//
// NOTICE: Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it. If you have received this file from a source other 
// than Adobe, then your use, modification, or distribution of it requires the prior written permission
// of Adobe.
// =================================================================================================

#if AdobePrivate
// =================================================================================================
// Change history
// ==============
//
// Writers:
//	FNO Frank Nocke
//
// mm-dd-yy who Description of changes, most recent on top
//
// 07-05-12 AWL 5.5-f015 [2999627] Improve ID3 genre handling.
//
// 09-20-11 AWL 5.4-f011 Fix bugs left in ID3 v2.2 support.
// 09-14-11 AWL 5.4-f010 Finish adding support for ID3 v2.2.
// 09-06-11 AWL 5.4-f009 Add support for ID3 v2.2 to ID3_Support.hpp.
//
// 08-19-10 AWL 5.3-f004 Move the seek mode constants to XMP_Const.
// 08-19-10 AWL 5.3-f003 Remove all use of the LFA_* names.
// 08-17-10 AWL 5.3-f001 Integrate I/O revamp to main.
//
// 10-29-09 SAM 5.0-f095 [2430362] MP3: fix import/export of date and time.
// 10-14-09 SAM 5.0-f089 [2338569] MP3: fix truncation of file length.
// 09-16-09 FNO 5.0-f080 [2430362] xmp:CreateDate import from ID3v2.
// 09-10-09 FNO 5.0-f079 Add ID3v1 and ID3v1.1 support to MP3 handler.
// 05-27-09 FNO 5.0-f042 [2352188] MP3: map ID3v2:TCOP to dc:rights['x-default'].
// 11-17-08 FNO 5.0-f006 Move MP3 handler CS5 rewrite from NewHandlers to regular handlers.
// 10-08-08 FNO 4.4.0 Thorough rewrite of MP3 handler.
//
// ================================================================================================
#endif // AdobePrivate

#include "public/include/XMP_Environment.h"	// ! XMP_Environment.h must be the first included header.

#include <sstream>

#include "public/include/XMP_Const.h"
#include "public/include/XMP_IO.hpp"

#include "XMPFiles/source/XMPFiles_Impl.hpp"
#include "XMPFiles/source/FormatSupport/Reconcile_Impl.hpp"
#include "source/UnicodeConversions.hpp"
#include "source/XMPFiles_IO.hpp"
#include "source/XIO.hpp"

#include "XMPFiles/source/FileHandlers/MP3_Handler.hpp"

// =================================================================================================
/// \file MP3_Handler.cpp
/// \brief MP3 handler class.
// =================================================================================================

// =================================================================================================
// Helper structs and private routines
// ====================
struct ReconProps {
	const char* mainID;	// The stored v2.3 and v2.4 ID, also used as the main logical ID.
	const char* v22ID;	// The stored v2.2 ID.
	const char* ns;
	const char* prop;
};

const static XMP_Uns32 XMP_V23_ID = 0x50524956;	// PRIV
const static XMP_Uns32 XMP_V22_ID = 0x50525600;	// PRV

const static ReconProps reconProps[] = {
	{ "TPE1", "TP1", kXMP_NS_DM,	"artist" },
	{ "TALB", "TAL", kXMP_NS_DM,	"album"  },
	{ "TRCK", "TRK", kXMP_NS_DM,	"trackNumber"  },
	// exceptions that need attention:
	{ "TCON", "TCO", kXMP_NS_DM,	"genre"  }, // genres might be numeric
	{ "TIT2", "TT2", kXMP_NS_DC,	"title"  }, // ["x-default"] language alternatives
	{ "COMM", "COM", kXMP_NS_DM,	"logComment"  }, // two distinct strings, language alternative

	{ "TYER", "TYE", kXMP_NS_XMP,	"CreateDate"  }, // Year (YYYY) Deprecated in 2.4
	{ "TDAT", "TDA", kXMP_NS_XMP,	"CreateDate"  }, // Date (DDMM) Deprecated in 2.4
	{ "TIME", "TIM", kXMP_NS_XMP,	"CreateDate"  }, // Time (HHMM) Deprecated in 2.4
	{ "TDRC", "", kXMP_NS_XMP,	"CreateDate"  }, // assembled date/time v2.4

	// new reconciliations introduced in Version 5
	{ "TCMP", "TCP", kXMP_NS_DM,	"partOfCompilation"  },	// presence/absence of TCMP frame dedides
	{ "USLT", "ULT", kXMP_NS_DM,	"lyrics"  },
	{ "TCOM", "TCM", kXMP_NS_DM,	"composer"  },
	{ "TPOS", "TPA", kXMP_NS_DM,	"discNumber"  },		// * a text field! might contain "/<total>"
	{ "TCOP", "TCR", kXMP_NS_DC,	"rights"  },  // ["x-default"] language alternatives
	{ "TPE4", "TP4", kXMP_NS_DM,	"engineer"  },
	{ "WCOP", "WCP", kXMP_NS_XMP_Rights,	"WebStatement"  },

	{ 0, 0, 0, 0 }	// must be last as a sentinel
};

// =================================================================================================
// MP3_MetaHandlerCTor
// ====================

XMPFileHandler * MP3_MetaHandlerCTor ( XMPFiles * parent )
{
	return new MP3_MetaHandler ( parent );
}

// =================================================================================================
// MP3_CheckFormat
// ===============

// For MP3 we check parts .... See the MP3 spec for offset info.

bool MP3_CheckFormat ( XMP_FileFormat format,
					  XMP_StringPtr  filePath,
					  XMP_IO*    file,
					  XMPFiles *     parent )
{
	IgnoreParam(filePath); IgnoreParam(parent);	//supress warnings
	XMP_Assert ( format == kXMP_MP3File );		//standard assert

	if ( file->Length() < 10 ) return false;
	file ->Rewind();

	XMP_Uns8 header[3];
	file->ReadAll ( header, 3 );
	if ( ! CheckBytes( &header[0], "ID3", 3 ) ) return (parent->format == kXMP_MP3File);

	XMP_Uns8 major = XIO::ReadUns8( file );
	XMP_Uns8 minor = XIO::ReadUns8( file );

	if ( (major < 2) || (major > 4) || (minor == 0xFF) ) return false;

	XMP_Uns8 flags = XIO::ReadUns8 ( file );

	//TODO
	if ( flags & 0x10 ) XMP_Throw ( "no support for MP3 with footer", kXMPErr_Unimplemented );
	if ( flags & 0x80 ) return false; //no support for unsynchronized MP3 (as before, also see [1219125])
	if ( flags & 0x0F ) XMP_Throw ( "illegal header lower bits", kXMPErr_Unimplemented );

	XMP_Uns32 size = XIO::ReadUns32_BE ( file );
	if ( (size & 0x80808080) != 0 ) return false; //if any bit survives -> not a valid synchsafe 32 bit integer

	return true;
	
}	// MP3_CheckFormat


// =================================================================================================
// MP3_MetaHandler::MP3_MetaHandler
// ================================

MP3_MetaHandler::MP3_MetaHandler ( XMPFiles * _parent )
{
	this->oldTagSize = 0;
	this->oldPadding = 0;
	this->oldFramesSize = 0;
	this->newTagSize = 0;
	this->newPadding = 0;
	this->newFramesSize = 0;
	this->tagIsDirty = false;
	this->mustShift = false;
#if AdobePrivate
	this->majorVersion = 0;
	this->minorVersion = 0;
#else
	this->majorVersion = 2;
	this->minorVersion = 2;
#endif
	this->hasID3Tag = false;
	this->hasFooter = false;
	this->extHeaderSize = 0;
	this->hasExtHeader = false;
	this->parent = _parent;
	this->handlerFlags = kMP3_HandlerFlags;
	this->stdCharForm  = kXMP_Char8Bit;
}

// =================================================================================================
// MP3_MetaHandler::~MP3_MetaHandler
// =================================

MP3_MetaHandler::~MP3_MetaHandler()
{
	// free frames
	ID3v2Frame* curFrame;
	while ( !this->framesVector.empty() ) {
		curFrame = this->framesVector.back();
		delete curFrame;
		framesVector.pop_back();
	}
#if AdobePrivate
	// If require, we need to free memory allocated for album art
	if ( this->needsArtUpdate )
	{
		std::vector<AlbumArt>::iterator iter;
		for ( iter = this->albumArtVector.begin(); iter != this->albumArtVector.end(); ++iter )
		{
			if ( iter->description != NULL )
			{
				delete[] iter->description;
				iter->description = NULL;
			}
			if ( iter->imageData != NULL )
			{
				delete[] iter->imageData;
				iter->imageData = NULL;
			}
		}
	}
#endif
}

// =================================================================================================
// MP3_MetaHandler::CacheFileData
// ==============================

void MP3_MetaHandler::CacheFileData()
{

	//*** abort procedures
	this->containsXMP = false;		//assume no XMP for now

	XMP_IO* file = this->parent->ioRef;
	XMP_PacketInfo &packetInfo = this->packetInfo;

	file->Rewind();

	this->hasID3Tag = this->id3Header.read( file );
	this->majorVersion = this->id3Header.fields[ID3Header::o_vMajor];
	this->minorVersion = this->id3Header.fields[ID3Header::o_vMinor];
	this->hasExtHeader = (0 != ( 0x40 & this->id3Header.fields[ID3Header::o_flags])); //'naturally' false if no ID3Tag
	this->hasFooter = ( 0 != ( 0x10 & this->id3Header.fields[ID3Header::o_flags])); //'naturally' false if no ID3Tag

	// stored size is w/o initial header (thus adding 10)
	// + but extended header (if existing)
	// + padding + frames after unsynchronisation (?)
	// (if no ID3 tag existing, constructed default correctly sets size to 10.)
	this->oldTagSize = ID3Header::kID3_TagHeaderSize + synchToInt32(GetUns32BE( &id3Header.fields[ID3Header::o_size] ));

	if ( ! hasExtHeader ) {

		this->extHeaderSize = 0; // := there is no such header.

	} else {

		this->extHeaderSize = synchToInt32( XIO::ReadInt32_BE( file));
		XMP_Uns8 extHeaderNumFlagBytes = XIO::ReadUns8( file );

		// v2.3 doesn't include the size, while v2.4 does
		if ( this->majorVersion < 4 ) this->extHeaderSize += 4;
		XMP_Validate( this->extHeaderSize >= 6, "extHeader size too small", kXMPErr_BadFileFormat );

		file->Seek ( this->extHeaderSize - 6, kXMP_SeekFromCurrent );

	}

	this->framesVector.clear(); //mac precaution
#if AdobePrivate
	this->apicFramesIndices.clear();	// Clearing the vector for fresh start
#endif
	ID3v2Frame* curFrame = 0; // reusable

	////////////////////////////////////////////////////
	// read frames
	
	XMP_Uns32 xmpID = XMP_V23_ID;
	XMP_Uns16 frameHeaderSize = ID3v2Frame::kV23_FrameHeaderSize;
#if AdobePrivate
	XMP_Uns32 apicID = apicLogicalID;		// APIC
#endif
	if ( this->majorVersion == 2 )
	{
		xmpID = XMP_V22_ID;
		frameHeaderSize = ID3v2Frame::kV22_FrameHeaderSize;
#if AdobePrivate
		apicID = 0x50494300;			// PIC
#endif
	}

	while ( file->Offset() < this->oldTagSize ) {

		curFrame = new ID3v2Frame();

		try {
			XMP_Int64 frameSize = curFrame->read ( file, this->majorVersion );
			if ( frameSize == 0 ) {
				delete curFrame; // ..since not becoming part of vector for latter delete.
				break;			 // not a throw. There's nothing wrong with padding.
			}
#if AdobePrivate
			// Now, We are just storing index into a vector,
			// Later, GetAlbumArt will do further processing
			if ( curFrame->id == apicID )
				this->apicFramesIndices.push_back( this->framesVector.size() );
#endif
			this->containsXMP = true;
		} catch ( ... ) {
			delete curFrame;
			throw;
		}

		// these are both pointer assignments, no (copy) construction
		// (MemLeak Note: for all things pushed, memory cleanup is taken care of in destructor.)
		this->framesVector.push_back ( curFrame );

		//remember XMP-Frame, if it occurs:
		if ( (curFrame->id ==xmpID) &&
			 (curFrame->contentSize > 8) && CheckBytes ( &curFrame->content[0], "XMP\0", 4 ) ) {

			// be sure that this is the first packet (all else would be illegal format)
			XMP_Validate ( this->framesMap[xmpID] == 0, "two XMP packets in one file", kXMPErr_BadFileFormat );
			//add this to map, needed on reconciliation
			this->framesMap[xmpID] = curFrame;
	
			this->packetInfo.length = curFrame->contentSize - 4; // content minus "XMP\0"
			this->packetInfo.offset = ( file->Offset() - this->packetInfo.length );
	
			this->xmpPacket.erase(); //safety
			this->xmpPacket.assign( &curFrame->content[4], curFrame->contentSize - 4 );
			this->containsXMP = true; // do this last, after all possible failure
		
		}

		// No space for another frame? => assume into ID3v2.4 padding.
		XMP_Int64 newPos = file->Offset();
		XMP_Int64 spaceLeft = this->oldTagSize - newPos;	// Depends on first check below!
		if ( (newPos > this->oldTagSize) || (spaceLeft < frameHeaderSize ) ) break;

	}

	////////////////////////////////////////////////////
	// padding

	this->oldPadding = this->oldTagSize - file->Offset();
	this->oldFramesSize = this->oldTagSize - ID3Header::kID3_TagHeaderSize - this->oldPadding;

	XMP_Validate ( (this->oldPadding >= 0), "illegal oldTagSize or padding value", kXMPErr_BadFileFormat );

	for ( XMP_Int64 i = this->oldPadding; i > 0; ) {
		if ( i >= 8 ) {
			if ( XIO::ReadInt64_BE(file) != 0 ) XMP_Throw ( "padding not nulled out", kXMPErr_BadFileFormat );
			i -= 8;
			continue;
		}
		if ( XIO::ReadUns8(file) != 0) XMP_Throw ( "padding(2) not nulled out", kXMPErr_BadFileFormat );
		i--;
	}

	//// read ID3v1 tag
	if ( ! this->containsXMP ) this->containsXMP = id3v1Tag.read ( file, &this->xmpObj );

}	// MP3_MetaHandler::CacheFileData


#if AdobePrivate
static XMP_Uns8 GetUTF8String( const XMP_Uns8 & encodingType, const char * contentPtr, std::string & utf8String, AlbumArt & albumArt, XMP_Uns32 maxLimit )
{
	XMP_Uns8 length = 0;
	XMP_Uns8 offset = 0;

	switch ( encodingType )
	{
	case 0: //ISO-8859-1, 0-terminated
	{
		for ( ; length < maxLimit && contentPtr[ length ] != 0; ++length );
		if ( contentPtr[ length ] != 0 )
			XMP_Throw( "Invalid IDIT data block", kXMPErr_BadFileFormat );

		ReconcileUtils::Latin1ToUTF8( contentPtr, length, &utf8String );
		++length;		// For 0x00
		albumArt.encodingType = kXMPFiles_AlbumArt_Enc_Latin1;
		break;
	}

	case 1: // Unicode, v2.4: UTF-16 (undetermined Endianess), with BOM, terminated 0x00 00
	case 2: // UTF-16BE without BOM, terminated 0x00 00
	{
		
		bool bigEndian = true;	// assume for now (if no BOM follows)
		
		if ( GetUns16BE( contentPtr ) == 0xFEFF ) {
			offset = 2;
			bigEndian = true;
			albumArt.encodingType = kXMPFiles_AlbumArt_Enc_UTF16Big;
		}
		else if ( GetUns16BE( contentPtr ) == 0xFFFE ) {
			offset = 2;
			bigEndian = false;
			albumArt.encodingType = kXMPFiles_AlbumArt_Enc_UTF16Little;
		}

		for ( ; length < ( 2 * maxLimit ) && GetUns16BE( contentPtr + length + offset ) != 0x0000; length +=2 );
		if ( contentPtr[ length + offset ] != 0 )
			XMP_Throw( "Invalid IDIT data block", kXMPErr_BadFileFormat );

		FromUTF16( ( UTF16Unit* ) ( contentPtr + offset ), ( length / 2 ), &utf8String, bigEndian );
		length += 2;		// For 0x0000

		break;
	}

	case 3: // UTF-8 unicode, terminated \0
	{
		if ( ( GetUns32BE( contentPtr ) & 0xFFFFFF00 ) == 0xEFBBBF00 ){
			offset = 3;	// swallow any BOM, just in case
		}

		for ( ; length < maxLimit && contentPtr[ length + offset ] != 0; ++length );
		if ( contentPtr[ length + offset ] != 0 )
			XMP_Throw( "Invalid IDIT data block", kXMPErr_BadFileFormat );
		
		utf8String.assign( contentPtr + offset, length - offset );
		++length;		// For 0x00
		albumArt.encodingType = kXMPFiles_AlbumArt_Enc_UTF8;

		break;
	}

	default:
		albumArt.encodingType = kXMPFiles_AlbumArt_Enc_Unknown;
		XMP_Throw( "unknown text encoding", kXMPErr_BadFileFormat ); //COULDDO assume latin-1 or utf-8 as best-effort
		break;
	}

	return length + offset;
}
#endif
// =================================================================================================
// MP3_MetaHandler::ProcessXMP
// ===========================
//
// Process the raw XMP and legacy metadata that was previously cached.

void MP3_MetaHandler::ProcessXMP()
{

	// Process the XMP packet.
	if ( ! this->xmpPacket.empty() ) {
		XMP_Assert ( this->containsXMP );
		XMP_StringPtr packetStr = this->xmpPacket.c_str();
		XMP_StringLen packetLen = (XMP_StringLen) this->xmpPacket.size();
		this->xmpObj.ParseFromBuffer ( packetStr, packetLen );
		this->processedXMP = true;
	}

	///////////////////////////////////////////////////////////////////
	// assumptions on presence-absence "flag tags"
	//   ( unless no xmp whatsoever present )
	if ( ! this->xmpPacket.empty() ) this->xmpObj.SetProperty ( kXMP_NS_DM, "partOfCompilation", "false" );

	////////////////////////////////////////////////////////////////////
	// import of legacy properties
	ID3v2Frame* curFrame;
	XMP_Bool hasTDRC = false;
	XMP_DateTime newDateTime;

	if ( this->hasID3Tag ) {	// otherwise pretty pointless...

		for ( int r = 0; reconProps[r].mainID != 0; ++r ) {

			//get the frame ID to look for
			XMP_Uns32 logicalID = GetUns32BE ( reconProps[r].mainID );
			XMP_Uns32 storedID = logicalID;
			if ( this->majorVersion == 2 && reconProps[r].v22ID != NULL  && *(reconProps[r].v22ID) != '\0') //handling "" value of v22ID
				storedID = GetUns32BE ( reconProps[r].v22ID );

			// deal with each such frame in the frameVector
			// (since there might be several, some of them not applicable, i.e. COMM)

			vector<ID3_Support::ID3v2Frame*>::iterator it;
			for ( it = this->framesVector.begin(); it != this->framesVector.end(); ++it ) {

				curFrame = *it;
				if ( storedID != curFrame->id ) continue;

				// go deal with it!
				// get the property
				std::string id3Text, xmpText;
				bool result = curFrame->getFrameValue ( this->majorVersion, logicalID, &id3Text );
				if ( ! result ) continue; //ignore but preserve this frame (i.e. not applicable COMM frame)

				//////////////////////////////////////////////////////////////////////////////////
				// if we come as far as here, it's proven that there's a relevant XMP property

				this->containsXMP = true;

				ID3_Support::ID3v2Frame* t = this->framesMap [ storedID ];
				if ( t != 0 )  t->active = false;

				// add this to map (needed on reconciliation)
				// note: above code reaches, that COMM/USLT frames
				// only then reach this map, if they are 'eng'(lish)
				// multiple occurences indeed leads to last one survives
				// ( in this map, all survive in the file )
				this->framesMap [ storedID ] = curFrame;

				// now write away as needed;
				// merely based on existence, relevant even if empty:
				if ( logicalID == 0x54434D50) {	// TCMP if exists: part of compilation

					this->xmpObj.SetProperty ( kXMP_NS_DM, "partOfCompilation", "true" );

				} else if ( ! id3Text.empty() ) {

					switch ( logicalID ) {

						case 0x54495432: // TIT2 -> title["x-default"]
						case 0x54434F50: // TCOP -> rights["x-default"]
							this->xmpObj.SetLocalizedText ( reconProps[r].ns, reconProps[r].prop,"", "x-default", id3Text );
							break;

						case 0x54434F4E: // TCON -> genre
							ID3_Support::GenreUtils::ConvertGenreToXMP ( id3Text.c_str(), &xmpText );
							if ( ! xmpText.empty() ) {
								this->xmpObj.SetProperty ( reconProps[r].ns, reconProps[r].prop, xmpText );
							}
							break;

						case 0x54594552: // TYER -> xmp:CreateDate.year
							{
								try {	// Don't let wrong dates in id3 stop import.
									if ( ! hasTDRC ) {
										newDateTime.year = (XMP_Int32)SXMPUtils::ConvertToInt ( id3Text );
										newDateTime.hasDate = true;
									}
								} catch ( ... ) {
										// Do nothing, let other imports proceed.
								}
								break;
							}

						case 0x54444154: //TDAT	-> xmp:CreateDate.month and day
							{
								try {	// Don't let wrong dates in id3 stop import.
									// only if no TDRC has been found before
									//&& must have the format DDMM
									if ( (! hasTDRC) && (id3Text.length() == 4) ) {
										newDateTime.day = (XMP_Int32)SXMPUtils::ConvertToInt (id3Text.substr(0,2));
										newDateTime.month = (XMP_Int32)SXMPUtils::ConvertToInt ( id3Text.substr(2,2));
										newDateTime.hasDate = true;
									}
								} catch ( ... ) {
									// Do nothing, let other imports proceed.
								}
								break;
							}

						case 0x54494D45: //TIME	-> xmp:CreateDate.hours and minutes
							{
								try {	// Don't let wrong dates in id3 stop import.
									// only if no TDRC has been found before
									// && must have the format HHMM
									if ( (! hasTDRC) && (id3Text.length() == 4) ) {
										newDateTime.hour = (XMP_Int32)SXMPUtils::ConvertToInt (id3Text.substr(0,2));
										newDateTime.minute = (XMP_Int32)SXMPUtils::ConvertToInt ( id3Text.substr(2,2));
										newDateTime.hasTime = true;
									}
								} catch ( ... ) {
									// Do nothing, let other imports proceed.
								}
								break;
							}

						case 0x54445243: // TDRC -> xmp:CreateDate //id3 v2.4
							{
								try {	// Don't let wrong dates in id3 stop import.
									hasTDRC = true;
									// This always wins over TYER, TDAT and TIME
									SXMPUtils::ConvertToDate ( id3Text, &newDateTime );
								} catch ( ... ) {
									// Do nothing, let other imports proceed.
								}
								break;
							}

						default:
							// NB: COMM/USLT need no special fork regarding language alternatives/multiple occurence.
							//		relevant code forks are in ID3_Support::getFrameValue()
							this->xmpObj.SetProperty ( reconProps[r].ns, reconProps[r].prop, id3Text );
							break;

					}//switch
					
				}

			} //for iterator

		}//for reconProps

		// import DateTime
        XMP_DateTime oldDateTime;
        bool haveNewDateTime  = newDateTime.year != 0 ;
        if ( xmpObj.GetProperty_Date ( kXMP_NS_XMP, "CreateDate", &oldDateTime, 0 ) )
        {

            haveNewDateTime = haveNewDateTime && ( (newDateTime.year != oldDateTime.year) || ( (newDateTime.month != 0 )
							  && ( (newDateTime.day != oldDateTime.day) || (newDateTime.month != oldDateTime.month) ) )
							  || ( newDateTime.hasTime && ( (newDateTime.hour != oldDateTime.hour) || (newDateTime.minute != oldDateTime.minute) ) ) );
        }
            // NOTE: no further validation nessesary the function "SetProperty_Date" will care about validating date and time
            // any exception will be caught and block import
        try {
            if ( haveNewDateTime ) {
                    this->xmpObj.SetProperty_Date ( kXMP_NS_XMP, "CreateDate", newDateTime );
            }
        } catch ( ... ) {
            // Dont import invalid dates from ID3
        }

	}

	// very important to avoid multiple runs! (in which case I'd need to clean certain
	// fields (i.e. regarding ->active setting)
	this->processedXMP = true;

}	// MP3_MetaHandler::ProcessXMP


// =================================================================================================
// MP3_MetaHandler::UpdateFile
// ===========================
void MP3_MetaHandler::UpdateFile ( bool doSafeUpdate )
{
	if ( doSafeUpdate ) XMP_Throw ( "MP3_MetaHandler::UpdateFile: Safe update not supported", kXMPErr_Unavailable );

	XMP_IO* file = this->parent->ioRef;

	// leave 2.3 resp. 2.4 header, since we want to let alone
	// and don't know enough about the encoding of unrelated frames...
	XMP_Assert( this->containsXMP );

#if AdobePrivate
	// Updating the album art related processing
	this->UpdateAPICFrames();
#endif
	tagIsDirty = false;
	mustShift = false;

	// write out native properties:
	// * update existing ones
	// * create new frames as needed
	// * delete frames if property is gone!
	// see what there is to do for us:

	// RECON LOOP START
	for (int r = 0; reconProps[r].mainID != 0; r++ ) {

		std::string value;
		bool needDescriptor = false;
		bool needEncodingByte = true;

		XMP_Uns32 logicalID = GetUns32BE ( reconProps[r].mainID );
		XMP_Uns32 storedID = logicalID;
		if ( this->majorVersion == 2 && reconProps[r].v22ID != NULL  && *(reconProps[r].v22ID) != '\0') //handling "" value of v22ID)
			storedID = GetUns32BE ( reconProps[r].v22ID );

		ID3v2Frame* frame = framesMap[ storedID ];	// the actual frame (if already existing)

		// get XMP property
		//	* honour specific exceptions
		//  * leave value empty() if it doesn't exist ==> frame must be delete/not created
		switch ( logicalID ) {

			case 0x54434D50: // TCMP if exists: part of compilation
				if ( xmpObj.GetProperty( kXMP_NS_DM, "partOfCompilation", &value, 0 ) && ( 0 == stricmp( value.c_str(), "true" ) )) {
					value = "1"; // set a TCMP frame of value 1
				} else {
					value.erase(); // delete/prevent creation of frame
				}
				break;
	
			case 0x54495432: // TIT2 -> title["x-default"]
			case 0x54434F50: // TCOP -> rights["x-default"]
				if (! xmpObj.GetLocalizedText( reconProps[r].ns, reconProps[r].prop, "", "x-default", 0, &value, 0 )) value.erase(); // if not, erase string.
				break;
	
			case 0x54434F4E: // TCON -> genre
				{
					bool found = xmpObj.GetProperty ( reconProps[r].ns, reconProps[r].prop, &value, 0 );
					if ( found ) {
						std::string xmpValue = value;
						ID3_Support::GenreUtils::ConvertGenreToID3 ( xmpValue.c_str(), &value );
					}
				}
				break;

			case 0x434F4D4D: // COMM
			case 0x55534C54: // USLT, both need descriptor.
				needDescriptor = true;
				if (! xmpObj.GetProperty( reconProps[r].ns, reconProps[r].prop, &value, 0 )) value.erase();
				break;
	
			case 0x54594552: //TYER
			case 0x54444154: //TDAT
			case 0x54494D45: //TIME
				{
					if ( majorVersion <= 3 ) {	// TYER, TIME and TDAT depricated since v. 2.4 -> else use TDRC

						XMP_DateTime dateTime;
						if (! xmpObj.GetProperty_Date( reconProps[r].ns, reconProps[r].prop, &dateTime, 0 )) {	// nothing found? -> Erase string. (Leads to Unset below)
							value.erase();
							break;
						}
	
						// TYER
						if ( logicalID == 0x54594552 ) {
							XMP_Validate( dateTime.year <= 9999 && dateTime.year > 0, "Year is out of range", kXMPErr_BadParam);
							// get only Year!
							SXMPUtils::ConvertFromInt( dateTime.year, "", &value );
							break;
						} else if ( logicalID == 0x54444154 && dateTime.hasDate ) {
							std::string day, month;
							SXMPUtils::ConvertFromInt( dateTime.day, "", &day );
							SXMPUtils::ConvertFromInt( dateTime.month, "", &month );
							if ( dateTime.day < 10 )
								value = "0";
							value += day;
							if ( dateTime.month < 10 )
								value += "0";
							value += month;
							break;
						} else if ( logicalID == 0x54494D45 && dateTime.hasTime ) {
							std::string hour, minute;
							SXMPUtils::ConvertFromInt( dateTime.hour, "", &hour );
							SXMPUtils::ConvertFromInt( dateTime.minute, "", &minute );
							if ( dateTime.hour < 10 )
								value = "0";
							value += hour;
							if ( dateTime.minute < 10 )
								value += "0";
							value += minute;
							break;
						} else {
							value.erase();
							break;
						}
					} else {
						value.erase();
						break;
					}
				}
				break;
	
			case 0x54445243: //TDRC (only v2.4)
				{
					// only export for id3 > v2.4
					if ( majorVersion > 3 )  {
						if (! xmpObj.GetProperty( reconProps[r].ns, reconProps[r].prop, &value, 0 )) value.erase();
					}
					break;
				}
				break;
	
			case 0x57434F50: //WCOP
				needEncodingByte = false;
				if (! xmpObj.GetProperty( reconProps[r].ns, reconProps[r].prop, &value, 0 )) value.erase(); // if not, erase string
				break;
	
			case 0x5452434B: // TRCK
			case 0x54504F53: // TPOS
				// no break, go on:
	
			default:
				if (! xmpObj.GetProperty( reconProps[r].ns, reconProps[r].prop, &value, 0 )) value.erase(); // if not, erase string
				break;

		}

		// [XMP exist] x [frame exist] => four cases:
		// 1/4) nothing before, nothing now
		if ( value.empty() && (frame==0)) continue; // nothing to do

		// all else means there will be rewrite work to do:
		tagIsDirty = true;

		// 2/4) value before, now gone:
		if ( value.empty() && (frame!=0)) {
			frame->active = false; //mark for non-use
			continue;
		}

		// 3/4) no old value, create new value
		bool needUTF16 = false;
		if ( needEncodingByte ) needUTF16 = (! ReconcileUtils::IsASCII ( value.c_str(), value.size() ) );
		if ( frame != 0 ) {
			frame->setFrameValue( value, needDescriptor, needUTF16, false, needEncodingByte );
		} else {
			ID3v2Frame* newFrame=new ID3v2Frame( storedID );
			newFrame->setFrameValue( value, needDescriptor,  needUTF16, false, needEncodingByte ); //always write as utf16-le incl. BOM
			framesVector.push_back( newFrame );
			framesMap[ storedID ] = newFrame;
			continue;
		}

	} 	// RECON LOOP END

	/////////////////////////////////////////////////////////////////////////////////
	// (Re)Build XMP frame:

	XMP_Uns32 xmpID = XMP_V23_ID;
	if ( this->majorVersion == 2 ) xmpID = XMP_V22_ID;
	
	ID3v2Frame* frame = framesMap[ xmpID ];
	if ( frame != 0 ) {
		frame->setFrameValue( this->xmpPacket, false, false, true );
	} else {
		ID3v2Frame* newFrame=new ID3v2Frame( xmpID );
		newFrame->setFrameValue ( this->xmpPacket, false, false, true );
		framesVector.push_back ( newFrame );
		framesMap[ xmpID ] = newFrame;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Decision making

	XMP_Int32 frameHeaderSize = ID3v2Frame::kV23_FrameHeaderSize;
	if ( this->majorVersion == 2 ) frameHeaderSize = ID3v2Frame::kV22_FrameHeaderSize;
	
	newFramesSize = 0;
	for ( XMP_Uns32 i = 0; i < framesVector.size(); i++ ) {
		if ( framesVector[i]->active ) newFramesSize += (frameHeaderSize + framesVector[i]->contentSize);
	}

	mustShift = (newFramesSize > (XMP_Int64)(oldTagSize - ID3Header::kID3_TagHeaderSize)) ||
	//optimization: If more than 8K can be saved by rewriting the MP3, go do it:
				((newFramesSize + 8*1024) < oldTagSize );

	if ( ! mustShift )	{	// fill what we got
		newTagSize = oldTagSize;
	} else { // if need to shift anyway, get some nice 2K padding
		newTagSize = newFramesSize + 2048 + ID3Header::kID3_TagHeaderSize;
	}
	newPadding = newTagSize - ID3Header::kID3_TagHeaderSize - newFramesSize;

	// shifting needed? -> shift
	if ( mustShift ) {
		XMP_Int64 filesize = file ->Length();
		if ( this->hasID3Tag ) {
			XIO::Move ( file, oldTagSize, file, newTagSize, filesize - oldTagSize ); //fix [2338569]
		} else {
			XIO::Move ( file, 0, file, newTagSize, filesize ); // move entire file up.
		}
	}

	// correct size stuff, write out header
	file ->Rewind();
	id3Header.write ( file, newTagSize );

	// write out tags
	for ( XMP_Uns32 i = 0; i < framesVector.size(); i++ ) {
		if ( framesVector[i]->active ) framesVector[i]->write ( file, majorVersion );
	}

	// write out padding:
	for ( XMP_Int64 i = newPadding; i > 0; ) {
		const XMP_Uns64 zero = 0;
		if ( i >= 8 ) {
			file->Write ( &zero, 8  );
			i -= 8;
			continue;
		}
		file->Write ( &zero, 1  );
		i--;
	}

	// check end of file for ID3v1 tag
	XMP_Int64 possibleTruncationPoint = file->Seek ( -128, kXMP_SeekFromEnd );
	bool alreadyHasID3v1 = (XIO::ReadInt32_BE( file ) & 0xFFFFFF00) == 0x54414700; // "TAG"
	if ( ! alreadyHasID3v1 ) file->Seek ( 128, kXMP_SeekFromEnd );	// Seek will extend the file.
	id3v1Tag.write( file, &this->xmpObj );

	this->needsUpdate = false; //do last for safety reasons

}	// MP3_MetaHandler::UpdateFile

// =================================================================================================
// MP3_MetaHandler::WriteTempFile
// ==============================

void MP3_MetaHandler::WriteTempFile ( XMP_IO* tempRef )
{
	IgnoreParam(tempRef);
	XMP_Throw ( "MP3_MetaHandler::WriteTempFile: Not supported", kXMPErr_Unimplemented );
}	// MP3_MetaHandler::WriteTempFile

#if AdobePrivate
// =================================================================================================
// MP3_MetaHandler::ConvertAPICToAlbumArt
// ==============================

#define curAPICMaxLimit (apicFramePtr->contentSize - curPos)

void MP3_MetaHandler::ConvertAPICToAlbumArt( ID3_Support::ID3v2Frame * apicFramePtr, AlbumArt & albumArt )
{
	// Parsing and storing information in 'APIC' tag
	char * contentPtr = apicFramePtr->content;
	XMP_Uns32 curPos = 0;
	XMP_Uns8 encodingByte = contentPtr[ curPos++ ];
	XMP_Uns32 mimeLength = 0;
	while ( true )
	{
		if ( mimeLength == curAPICMaxLimit )
			XMP_Throw ( "Invalid IDIT data block" , kXMPErr_BadFileFormat );
		if ( contentPtr[curPos + mimeLength] == 0 )
			break;
		++mimeLength;
	}

	std::string mimeType;
	mimeType.reserve( mimeLength );
	mimeType.assign( contentPtr + curPos, mimeLength );
	curPos += ( mimeLength + 1 );			// 1 extra byte for NULL

	albumArt.usageType = contentPtr[ curPos++ ];

	std::string description;
	curPos += GetUTF8String( encodingByte, contentPtr + curPos, description, albumArt, curAPICMaxLimit );

	if ( !description.empty() )
	{
		albumArt.description = new char[ description.length() + 1 ];
		albumArt.descLength = static_cast< XMP_Uns32 >( description.length() );
		memset( ( void * ) albumArt.description, 0, description.length() + 1 );
		memcpy( ( void * ) albumArt.description, description.c_str(), description.length() );
	}

	std::string picData;
	albumArt.imageDataLength = curAPICMaxLimit;
	if ( albumArt.imageDataLength != 0 )
		albumArt.imageData = new XMP_Uns8[albumArt.imageDataLength];
	else
		albumArt.imageData = NULL;
	memcpy( albumArt.imageData, contentPtr + curPos, albumArt.imageDataLength );
	
	// If we get supported mime type then we will write value for format,
	// but if we get unsupported mime type or "image/" then we will not write format information
	// If we get "-->" mimetype then ignore whole tag as we don't support linked image
	XMP_Uns8 formatType = kXMPFiles_AlbumArt_NoFormat;
	if ( mimeType == "image/jpeg" )
		formatType = kXMPFiles_AlbumArt_JPEG;
	else if ( mimeType == "image/png" )
		formatType = kXMPFiles_AlbumArt_PNG;
	else if ( mimeType == "image/tiff" || mimeType == "image/tiff-fx" )
		formatType = kXMPFiles_AlbumArt_TIFF;
	else if ( mimeType == "image/gif" )
		formatType = kXMPFiles_AlbumArt_GIF;
	else if ( mimeType == "-->" )
		formatType = kXMPFiles_AlbumArt_URLImage;
	else
		formatType = kXMPFiles_AlbumArt_UnSupportedFormat;

	albumArt.formatType = formatType;

}	//	MP3_MetaHandler::ConvertAPICToAlbumArt

// =================================================================================================
// MP3_MetaHandler::GetAlbumArtworks
// ==============================

bool MP3_MetaHandler::GetAlbumArtworks( )
{

	// If no ID3 then no need to parse for 'APIC' tag
	if ( this->hasID3Tag )
	{
		std::vector<size_t>::iterator iter;
		XMP_Uns32 apicID = 0x41504943;		// APIC
		if ( this->majorVersion == 2 )
			apicID = 0x50494300;			// PIC

		for ( iter = this->apicFramesIndices.begin(); iter != this->apicFramesIndices.end(); iter++ )
		{
			XMP_Assert( (*iter) < this->framesVector.size() );
			ID3_Support::ID3v2Frame * apicFramePtr = this->framesVector.at(*iter);
			XMP_Assert( apicFramePtr->id == apicID );
			XMP_Assert( ( apicFramePtr->content != 0 ) && ( apicFramePtr->contentSize >= 0 ) && ( apicFramePtr->contentSize < 20 * 1024 * 1024 ) );

			if ( apicFramePtr->contentSize < 2 )
				continue;

			AlbumArt albumArt;
			this->ConvertAPICToAlbumArt( apicFramePtr, albumArt );
			this->albumArtVector.push_back( albumArt );
		}
	}
	if ( this->albumArtVector.size() != 0 )
		return true;
	return false;

}	// MP3_MetaHandler::GetAlbumArtworks

// =================================================================================================
// MP3_MetaHandler::ConvertUTF8ToEncoding
// ==============================

XMP_Uns8 MP3_MetaHandler::ConvertUTF8ToEncoding( const XMP_Uns8 & encodingType, XMP_StringPtr utf8Desc, XMP_StringLen utf8DescLen, std::string & encodedDesc )
{

	encodedDesc.clear();
	XMP_Uns8 localEnc = 0;
	if ( utf8DescLen > 64 )
		utf8DescLen = 64;
	switch ( encodingType )
	{
	case kXMPFiles_AlbumArt_Enc_UTF16Big:
		localEnc = 1;
		ToUTF16( ( XMP_Uns8* ) utf8Desc, utf8DescLen, &encodedDesc, true );
		encodedDesc.append( "\x00\x00", 2 );
		break;

	case kXMPFiles_AlbumArt_Enc_UTF16Little:
		localEnc = 1;
		ToUTF16( ( XMP_Uns8* ) utf8Desc, utf8DescLen, &encodedDesc, false );
		encodedDesc.append( "\x00\x00", 2 );
		return 1;
		
	default:
		localEnc = 0;
		ReconcileUtils::UTF8ToLatin1( utf8Desc, utf8DescLen, &encodedDesc );
		encodedDesc.append( "\x00", 1 );
		break;
	}

	return localEnc;

}	// MP3_MetaHandler::ConvertUTF8ToEncoding

// =================================================================================================
// MP3_MetaHandler::UpdateAPICFrames
// ==============================

void MP3_MetaHandler::UpdateAPICFrames()
{
	/* Updating album arts if required
	There are four cases exits. T = APIC exists, F = APIC not exits
	Old File  |  New File	|	Operation
	T		 |		T		|	Complex ( 2 options )
	T		 |		F		|	Delete existing frame
	F		 |		T		|	Create new Frame
	F		 |		F		|	No Need to do anything
	1) is complex.
	If user has separetly called SetAlbumArt then we need to update this case by deleting existing APIC frames and creating new frames.
	We will check for in-place update only when old file had just 1 APIC frame,
	For files, having more than one frame, we will delete existing APIC frames and create new APIC frames
	In future, we may extend in-place update support for files having more than 1 APIC frame.
	If user has not called SetAlbumArt then we no need to anything. Just ignore this case.
	*/
	if ( this->needsArtUpdate )
	{
		// 3 possibilities, TF, FT, TT
		bool needsDeleteExisting = true;
		bool needsNewFrame = true;
		if ( this->apicFramesIndices.size() == 1 && this->albumArtVector.size() == 1 )
		{
			// Compare existing album art
			AlbumArt oldAlbumArt;
			AlbumArt newAlbumArt = this->albumArtVector[ 0 ];
			ConvertAPICToAlbumArt( this->framesVector[ this->apicFramesIndices[ 0 ] ], oldAlbumArt );
			if ( ( oldAlbumArt.imageDataLength == newAlbumArt.imageDataLength ) &&
				( oldAlbumArt.usageType == newAlbumArt.usageType ) &&
				( oldAlbumArt.formatType == newAlbumArt.formatType ) &&
				( oldAlbumArt.encodingType == newAlbumArt.encodingType ) &&
				( oldAlbumArt.descLength == newAlbumArt.descLength ) )
			{
				XMP_Uns32 index = 0;
				for ( ; index < oldAlbumArt.descLength; ++index )
				{
					if ( oldAlbumArt.description[ index ] != newAlbumArt.description[ index ] )
						break;
				}
				if ( ( index == oldAlbumArt.descLength ) &&
					( memcmp( oldAlbumArt.imageData, newAlbumArt.imageData, oldAlbumArt.imageDataLength ) == 0 ) )
					needsDeleteExisting = needsNewFrame = false;
			}
			// Deleting the variables filled by ConvertAPICToAlbumArt()
			if ( oldAlbumArt.description != NULL )
			{
				delete[] oldAlbumArt.description;
				oldAlbumArt.description = 0;
			}
			if ( oldAlbumArt.imageData != NULL )
			{
				delete[] oldAlbumArt.imageData;
				oldAlbumArt.imageData = 0;
			}
		}

		// Deleting existing APIC frames from file
		if ( needsDeleteExisting )
		{
			std::vector<size_t>::iterator iter;
			for ( iter = this->apicFramesIndices.begin(); iter != this->apicFramesIndices.end(); ++iter )
				( this->framesVector[ ( *iter ) ] )->active = false;
		}

		// Creating new APIC frames if required
		if ( needsNewFrame )
		{
			std::vector<AlbumArt>::iterator iter;
			for ( iter = this->albumArtVector.begin(); iter != this->albumArtVector.end(); ++iter )
			{

				ID3v2Frame * newFramePtr = new ID3v2Frame( apicLogicalID );
				std::stringstream apicstream;
				std::string encodedDesc;

				// Convert description according to encoding type
				XMP_Uns8 localEnc = ConvertUTF8ToEncoding( iter->encodingType, iter->description, iter->descLength,encodedDesc );
				apicstream.put( localEnc );
				switch ( iter->formatType )				// Mime type
				{
				case kXMPFiles_AlbumArt_JPEG:
					apicstream << "image/jpeg";
					break;
				case kXMPFiles_AlbumArt_TIFF:
					apicstream << "image/tiff";
					break;
				case kXMPFiles_AlbumArt_PNG:
					apicstream << "image/png";
					break;
				case kXMPFiles_AlbumArt_GIF:
					apicstream << "image/gif";
					break;
				case kXMPFiles_AlbumArt_URLImage:
					apicstream << "-->";
					break;
				default:
					apicstream << "image/";
					break;
				}
				apicstream.put('\0');						// Inserting NULL character
				// XMP is making 0xFF as constant for representing usage as None
				apicstream << iter->usageType;
				if ( iter->encodingType == kXMPFiles_AlbumArt_Enc_UTF16Big )
					apicstream << "\xFE\xFF";
				else if ( iter->encodingType == kXMPFiles_AlbumArt_Enc_UTF16Little )
					apicstream << "\xFF\xFE";
				apicstream << encodedDesc;
				apicstream.write( (const char *)iter->imageData, iter->imageDataLength );
				newFramePtr->setFrameValue( apicstream.str(), false, false, false, false, true );
				newFramePtr->active = true;
				this->framesVector.push_back( newFramePtr );

			}
		}
	}
}	// UpdateAPICFrames
#endif

#ifndef __Basic_Handler_hpp__
#define __Basic_Handler_hpp__	1

// =================================================================================================
// Copyright Adobe
// Copyright 2004 Adobe
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
//	AWL Alan Lillich
//
// mm-dd-yy who Description of changes, most recent on top
//
// 08-17-10 AWL 5.3-f001 Integrate I/O revamp to main.
//
// 01-21-08 AWL 4.2-f058 Global tweak to have the handler's packetInfo always refer to the file.
//
// 04-07-06 AWL 4.0-f002 Add XMPFiles::GetThumbnail. Change XMPFiles::OpenFile to close the disk file
//				early for read-only access. Change XMPFileHandler::ExtractXMP to CacheFileData.
// 03-24-06 AWL 4.0-f001 Adapt for move to ham-perforce, integrate XMPFiles, bump version to 4.
//
// 09-01-05 AWL 1.3-003 [1225858] Don't implicitly set the SafeUpdate flag for non-owning handlers.
//
// 01-06-05 AWL 1.0-027 [1014854] Cleanup handling for UTF-16 and UTF-32. This involves some general
//				changes to the separation of work between the common code and handlers, making the
//				storage of the SXMPMeta object and packet string more visible.
// 10-07-04 AWL 1.0-009 Add first cut of support for crash safe file update.
// 08-13-04 AWL First draft.
//
// =================================================================================================
#endif // AdobePrivate

#include "public/include/XMP_Environment.h"	// ! XMP_Environment.h must be the first included header.

#include "public/include/XMP_Const.h"
#include "public/include/XMP_IO.hpp"

#include "XMPFiles/source/XMPFiles_Impl.hpp"

// =================================================================================================
/// \file Basic_Handler.hpp
///
/// \brief Base class for handlers that support a simple file model allowing insertion and expansion
/// of XMP, but probably not reconciliation with other forms of metadata. Reconciliation would have
/// to be done within the I/O model presented here.
///
/// \note Any specific derived handler might not be able to do insertion, but all must support
/// expansion. If a handler can't do either it should be derived from Trivial_Handler. Common code
/// must check the actual canInject flag where appropriate.
///
/// The model for a basic handler divides the file into 6 portions:
///
/// \li The front of the file. This portion can be arbitrarily large. Files over 4GB are supported.
/// Adding or expanding the XMP must not require expanding this portion of the file. The XMP offset
/// or length might be written into reserved space in this section though.
///
/// \li A prefix for the XMP section. The prefix and suffix for the XMP "section" are the format
/// specific portions that surround the raw XMP packet. They must be generated on the fly, even when
/// updating existing XMP with or without expansion. Their length must not depend on the XMP packet.
///
/// \li The XMP packet, as created by SXMPMeta::SerializeToBuffer. The size must be less than 2GB.
///
/// \li A suffix for the XMP section.
///
/// \li Trailing file content. This portion can be arbitarily large. It must be possible to remove
/// the XMP, move this portion of the file forward, then reinsert the XMP after this portion. This
/// is actually how the XMP is expanded. There must not be any embedded file offsets in this part,
/// this content must not change if the XMP changes size.
///
/// \li The back of the file. This portion must have modest size, and/or be generated on the fly.
/// When inserting XMP, part of this may be buffered in RAM (hence the modest size requirement), the
/// XMP section is written, then this portion is rewritten. There must not be any embedded file
/// offsets in this part, this content must not change if the XMP changes size.
///
/// \note There is no general promise here about crash-safe I/O. An update to an existing file might
/// have invalid partial state, for example while moving the trailing content portion forward if the
/// XMP increases in size or even rewriting existing XMP in-place. Crash-safe updates are managed at
/// a higher level of XMPFiles, using a temporary file and final swap of file content.
///
// =================================================================================================

#if AdobePrivate
// *** The Basic_Handler handler model ought to be revised to not do the shuffle-forward of the file
// *** tail. Instead use a rolling buffer-ahead/write-behind approach to insert fresh XMP.
#endif

static const XMP_OptionBits kBasic_HandlerFlags = (kXMPFiles_CanInjectXMP |
                                                   kXMPFiles_CanExpand |
                                                   kXMPFiles_CanRewrite |
                                                   kXMPFiles_PrefersInPlace |
                                                   kXMPFiles_AllowsOnlyXMP |
                                                   kXMPFiles_ReturnsRawPacket |
                                                   kXMPFiles_AllowsSafeUpdate);

class Basic_MetaHandler : public XMPFileHandler
{
public:

	Basic_MetaHandler() :
		xmpFileOffset(0), xmpFileSize(0), xmpPrefixSize(0), xmpSuffixSize(0), trailingContentSize(0) {};
	~Basic_MetaHandler();

	virtual void CacheFileData() = 0;	// Sets offset for insertion if no XMP yet.

	void UpdateFile ( bool doSafeUpdate );
    void WriteTempFile ( XMP_IO* tempRef );

protected:

	// Write a cached or fixed prefix or suffix for the XMP. The file is passed because it could be
	// either the original file or a safe-update temp file.
	virtual void WriteXMPPrefix ( XMP_IO* fileRef ) = 0;	// ! Must have override in actual handlers!
	virtual void WriteXMPSuffix ( XMP_IO* fileRef ) = 0;	// ! Must have override in actual handlers!

	// Note that the XMP is being removed or inserted. The file is passed because it could be either
	// the original file or a safe-update temp file.
	virtual void NoteXMPRemoval ( XMP_IO* fileRef ) = 0;	// ! Must have override in actual handlers!
	virtual void NoteXMPInsertion ( XMP_IO* fileRef ) = 0;	// ! Must have override in actual handlers!

	// Capture or restore the tail portion of the file. The file is passed because it could be either
	// the original file or a safe-update temp file.
	virtual void CaptureFileEnding ( XMP_IO* fileRef ) = 0;	// ! Must have override in actual handlers!
	virtual void RestoreFileEnding ( XMP_IO* fileRef ) = 0;	// ! Must have override in actual handlers!

	// Move the trailing content portion forward. Excludes "back" of the file. The file is passed
	// because it could be either the original file or a safe-update temp file.
	void ShuffleTrailingContent ( XMP_IO* fileRef );	// Has a common implementation.

	XMP_Uns64 xmpFileOffset;	// The offset of the XMP in the file.
	XMP_Uns32 xmpFileSize;	// The size of the XMP in the file.
		// ! The packetInfo offset and length are updated by PutXMP, before the file is updated!

	XMP_Uns32 xmpPrefixSize;	// The size of the existing header for the XMP section.
	XMP_Uns32 xmpSuffixSize;	// The size of the existing trailer for the XMP section.

	XMP_Uns64 trailingContentSize;	// The size of the existing trailing content. Excludes "back" of the file.

};	// Basic_MetaHandler

// =================================================================================================

#endif /* __Basic_Handler_hpp__ */

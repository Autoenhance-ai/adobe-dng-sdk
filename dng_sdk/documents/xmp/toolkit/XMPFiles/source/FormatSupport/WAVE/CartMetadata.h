// =================================================================================================
// Copyright Adobe
// Copyright 2011 Adobe
// All Rights Reserved
//
// NOTICE: Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it. If you have received this file from a source other 
// than Adobe, then your use, modification, or distribution of it requires the prior written permission
// of Adobe.
// =================================================================================================

#ifndef _CartMetadata_h_
#define _CartMetadata_h_

#if AdobePrivate
// =================================================================================================
// Change history
// ==============
//
// Writers:
//	PW  Patrick Wollek
//  SAM Samy Makki
//	ADC Amandeep Chawla
//
// mm-dd-yy who Description of changes, most recent on top
//
// 03-23-15 ADC 5.6-f142 Adding support for parsing and serializing TRACK_LIST tag in iXML block.
//
// 10-19-11 SAM 5.4-f025 [2984517] Adding Mapping for Cart Post Timer Array for WAVE
// 08-02-11 AWL 5.4-f003 Adding mapping from/to Cart PostTimer Array for WAVE
//
// =================================================================================================
#endif // AdobePrivate

#include "public/include/XMP_Environment.h"	// ! XMP_Environment.h must be the first included header.

#include "public/include/XMP_Const.h"
#include "public/include/XMP_IO.hpp"

#include "XMPFiles/source/NativeMetadataSupport/IMetadata.h"

namespace IFF_RIFF {

// =================================================================================================

class CartMetadata : public IMetadata {

public:

	enum {

		// IDs for the mapped cart chunk fields.
		kVersion,				// Local text, max 4 bytes
		kTitle,					// Local text, max 64 bytes
		kArtist,				// Local text, max 64 bytes
		kCutID,					// Local text, max 64 bytes
		kClientID,				// Local text, max 64 bytes
		kCategory,				// Local text, max 64 bytes
		kClassification,		// Local text, max 64 bytes
		kOutCue,				// Local text, max 64 bytes
		kStartDate,				// Local text, max 10 bytes
		kStartTime,				// Local text, max 8 bytes
		kEndDate,				// Local text, max 10 bytes
		kEndTime,				// Local text, max 8 bytes
		kProducerAppID,			// Local text, max 64 bytes
		kProducerAppVersion,	// Local text, max 64 bytes
		kUserDef,				// Local text, max 64 bytes
		kURL,					// Local text, max 1024 bytes
		kTagText,				// Local text, no limit
		kLevelReference,		// Little endian unsigned 32
		kPostTimer,				// array[8] of usage(Uns32), value(Uns32)
		kReserved,				// 276 reserved bytes

		// Constants for the range of fixed length text fields.
		kFirstFixedTextField = kVersion,
		kLastFixedTextField  = kURL

	};

	struct StoredCartTimer {
		XMP_Uns32 usage;
		XMP_Uns32 value;

		bool operator == (const StoredCartTimer & other ) const {
			return usage == other.usage && value == other.value;
		}

		bool operator != ( const StoredCartTimer & other ) const {
			return usage != other.usage || value != other.value;
		}
	};

	enum { kPostTimerLength = 8 };

	CartMetadata();
	virtual ~CartMetadata();

	void parse ( const XMP_Uns8* chunkData, XMP_Uns64 chunkSize );
	void parse ( XMP_IO* input ) { IMetadata::parse ( input ); }
	
	XMP_Uns64 serialize ( XMP_Uns8** buffer );

protected:

	virtual	bool isEmptyValue ( XMP_Uns32 id, ValueObject& valueObj );

private:

	// Operators hidden on purpose.
	CartMetadata ( const CartMetadata& ) {};
	CartMetadata& operator= ( const CartMetadata& ) { return *this; };

};	// CartMetadata

}	// namespace IFF_RIFF

#endif

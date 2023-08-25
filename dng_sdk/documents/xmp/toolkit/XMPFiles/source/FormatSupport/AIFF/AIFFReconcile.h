// =================================================================================================
// Copyright Adobe
// Copyright 2010 Adobe
// All Rights Reserved
//
// NOTICE: Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it. If you have received this file from a source other 
// than Adobe, then your use, modification, or distribution of it requires the prior written permission
// of Adobe.
// =================================================================================================

#ifndef _AIFFReconcile_h_
#define _AIFFReconcile_h_

#if AdobePrivate
// =================================================================================================
// Change history
// ==============
//
// Writers:
//	JEH Joerg Ehrlich
//
// mm-dd-yy who Description of changes, most recent on top
//
// 10-26-10 PW  5.3-f015 Integration from winwood
//							New design for Reconciliation code
// 07-28-10	JEH	5.2-f001 Initial creation of new AIFF handler.

//
// =================================================================================================
#endif // AdobePrivate


#include "XMPFiles/source/NativeMetadataSupport/IReconcile.h"

namespace IFF_RIFF
{

class AIFFReconcile : public IReconcile
{
public:
	~AIFFReconcile() {};

	/** 
	* @see IReconcile::importToXMP
	* Legacy values are always imported.
	* If the values are not UTF-8 they will be converted to UTF-8 except in ServerMode
	*/
	XMP_Bool importToXMP( SXMPMeta& outXMP, const MetadataSet& inMetaData );

	/** 
	* @see IReconcile::exportFromXMP
	* XMP values are always exported to Legacy as UTF-8 encoded
	*/
	XMP_Bool exportFromXMP( MetadataSet& outMetaData, SXMPMeta& inXMP );

};

}

#endif

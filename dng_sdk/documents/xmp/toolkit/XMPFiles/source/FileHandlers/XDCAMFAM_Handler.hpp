#ifndef __XDCAMFAM_Handler_hpp__
#define __XDCAMFAM_Handler_hpp__	1

// =================================================================================================
// Copyright Adobe
// Copyright 2015 Adobe
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
//	AB  Amit Bhatti
//
// mm-dd-yy who Description of changes, most recent on top
//
// 12-12-14 AB 5.6-f123 Redesign XDCAM Handler by dividing it into 2 child handlers XDCAMFAM_Handler and XDCAMSAM_Handler and making it more consistent as per latest spec.
//						More history could be found at XDCAM_Handler.hpp.
//
// =================================================================================================
#endif // AdobePrivate

#include "public/include/XMP_Environment.h"	// ! This must be the first include.
#include "XMPFiles/source/FileHandlers/XDCAM_Handler.hpp"


extern XMPFileHandler * XDCAMFAM_MetaHandlerCTor ( XMPFiles * parent );

extern bool XDCAMFAM_CheckFormat ( XMP_FileFormat format,
								const std::string & rootPath,
								const std::string & gpName,
								const std::string & parentName,
								const std::string & leafName,
								XMPFiles * parent );

static const XMP_OptionBits kXDCAMFAM_HandlerFlags = (kXMPFiles_CanInjectXMP |
												   kXMPFiles_CanExpand |
												   kXMPFiles_CanRewrite |
												   kXMPFiles_PrefersInPlace |
												   kXMPFiles_CanReconcile |
												   kXMPFiles_AllowsOnlyXMP |
												   kXMPFiles_ReturnsRawPacket |
												   kXMPFiles_HandlerOwnsFile |
												   kXMPFiles_AllowsSafeUpdate |
												   kXMPFiles_FolderBasedFormat);


class XDCAMFAM_MetaHandler : public XDCAM_MetaHandler
{

public:

	void FillAssociatedResources ( std::vector<std::string> * resourceList );
	XDCAMFAM_MetaHandler ( XMPFiles * _parent );
	virtual ~XDCAMFAM_MetaHandler()	{ };

private:

	bool isXDStyle;

	bool MakeClipFilePath ( std::string * path, XMP_StringPtr suffix, bool checkFile = false );
	void SetPathVariables ( const std::string & clientPath );
	bool GetMediaProMetadata ( SXMPMeta * xmpObjPtr, const std::string& clipUMID, bool digestFound );
	bool GetInfoFiles ( std::vector<std::string> &infoList, std::string pathToFolder) ;
	bool GetPlanningFiles ( std::vector<std::string> &planInfoList, std::string pathToFolder) ;
	bool IsClipsPlanning ( std::string clipUmid , XMP_StringPtr planPath ) ;
	bool GetClipUmid ( std::string &clipUmid );
	bool MakeLocalFilePath ( std::string * path, XMP_Uns8 fileType, bool checkFile = false );

	XDCAMFAM_MetaHandler() : XDCAM_MetaHandler() {};	// Hidden on purpose.

	enum
	{
		k_LocalPPNFile,
		k_LocalClipInfoFile
	};

};	// XDCAMFAM_MetaHandler

// =================================================================================================
#endif /* __XDCAMFAM_Handler_hpp__ */

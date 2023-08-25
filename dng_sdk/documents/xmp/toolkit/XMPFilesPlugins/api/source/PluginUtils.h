#ifndef __PLUGIN_UTILS_H__
#define __PLUGIN_UTILS_H__ 1

// =================================================================================================
// Copyright Adobe
// Copyright 2014 Adobe
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
// 01-05-15	AB	5.6-f122 Provide more functionalities to Plugin( Existing XMP packet, PacketInfo, OpenFlags, Error Callback and progress notification),
//						 more standard handler access API getFileModDate,IsMetadataWritable,putXMP,getAssociatedResources.
//						 New plugin handler for MPEG4 with Exif support.
//
// =================================================================================================
#endif // AdobePrivate

#include "XMP_Const.h"
#include "source/XMP_LibUtils.hpp"

namespace XMP_PLUGIN
{
	class ErrorCallbackInfo : public GenericErrorCallback {
	public:
		XMPFiles_ErrorCallbackWrapper	wrapperProc;
		XMPFiles_ErrorCallbackProc		clientProc;
		void *							context;
		std::string						filePath;

		ErrorCallbackInfo(): wrapperProc(0), clientProc(0), context(0) {};

		void Clear() {
			this->wrapperProc = 0; this->clientProc = 0; this->context = 0;
			GenericErrorCallback::Clear(); 
		};

		bool CanNotify() const;

		bool ClientCallbackWrapper(
			XMP_StringPtr filePath,
			XMP_ErrorSeverity severity,
			XMP_Int32 cause,
			XMP_StringPtr messsage) const;
	};

	void ErrorNotifyClient( GenericErrorCallback * errCBptr, XMP_ErrorSeverity severity, XMP_Error & error);
}

#endif	// __PLUGIN_UTILS_H__

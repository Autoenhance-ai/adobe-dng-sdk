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

#if AdobePrivate
// =================================================================================================
// Change history
// ==============
//
// Writers:
//	ADC Amandeep Chawla
//  IJS Inder Jeet Singh
//	AB  Amit Bhatti
//
// mm-dd-yy who Description of changes, most recent on top
//
// 02-09-15 AJ  5.6-f132 Fixing some more warnings due to implicit typecasting
// 01-05-15	AB	5.6-f122 Provide more functionalities to Plugin( Existing XMP packet, PacketInfo, OpenFlags, Error Callback and progress notification),
//						 more standard handler access API getFileModDate,IsMetadataWritable,putXMP,getAssociatedResources.
//						 New plugin handler for MPEG4 with Exif support.
// 05-06-13 IJS 5.6-f058 Refactoring the GetXMPStandard code as per review comments
// 04-26-13 IJS 5.6-f056 Make API to access overridden file handler available for public SDK
// 04-26-13 ADC 5.6-f054 [3526891] Read-only checks in IsMetadataWritable() API try creating temporary files if no metadata exists, instead of checking for file permissions.
//						 [3525961] IsMetaDataWritable checks for Plugins and embedded handlers are different.
// 01-11-13 IJS 5.6-f020 Reworked GetFileModDate, GetAssociatedResources and IsMetadataWritable APIs.
//						 Added IsMetadataWritable Plugin API in the Plugin Architechture.Bumped the Plugin API to version 3.
// 12-21-12 IJS 5.6-f009 Added FillAssociatedResources support in plugins 
// 12-19-12 IJS 5.6-f007 Removing #if  DISABLE_SERIALIZED_IMPORT_EXPORT 
// 12-19-12 IJS 5.6-f005 Making the importToXMP and exportFromXMP as public Plug-in methods
//
// 12-10-12 IJS 5.5-f059 Adding Serialized versions for importToXMP and exportFromXMP Plug-in APIs.
// 11-05-12 ADC 5.5-f056 XMP Plug-ins support for LocateMetadataFiles API.
//
// =================================================================================================
#endif // AdobePrivate

#include "PluginBase.h"
#include "PluginUtils.h"
#include "source/Host_IO.hpp"
#include "XMP.incl_cpp"

#if XMP_WinBuild
#include <Windows.h>
#endif

#if ( XMP_MacBuild || XMP_UNIXBuild )
#include <sys/stat.h>
#endif

namespace XMP_PLUGIN
{
static bool GetModifyDate ( const char* filePath, XMP_DateTime* modifyDate );
	
void PluginBase::cacheFileData( XMP_IORef fileRef, XMP_StringPtr* xmpStr )
{
	std::string buffer;
	IOAdapter file( fileRef );
	
	this->cacheFileData( file, buffer );

	if( !buffer.empty() )
	{
		XMP_Uns32 length = (XMP_Uns32)buffer.size() + 1 ;
		StringPtr bufferPtr = HostStringCreateBuffer( length );
		memcpy( bufferPtr, buffer.c_str(), length );
		*xmpStr = bufferPtr; // callee function should free the memory.
	}
}

// ============================================================================

void PluginBase::updateFile( XMP_IORef fileRef, bool doSafeUpdate, XMP_StringPtr xmpStr )
{
	IOAdapter file( fileRef );
	std::string buffer( xmpStr );

	this->updateFile( file, doSafeUpdate, buffer );
}

// ============================================================================

void PluginBase::writeTempFile( XMP_IORef srcFileRef, XMP_IORef tmpFileRef, XMP_StringPtr xmpStr )
{
	IOAdapter srcFile( srcFileRef );
	IOAdapter tmpFile( tmpFileRef );
	std::string buffer( xmpStr );

	this->writeTempFile( srcFile, tmpFile, buffer );
}

void PluginBase::FillMetadataFiles( StringVectorRef metadataFiles, SetStringVectorProc SetStringVector )
{
	if ( metadataFiles == 0 ) 
		XMP_Throw ( "A result file list vector must be provided", kXMPErr_BadParam );

	std::vector<std::string> fileList;	// Pass a local vector, not the client's.
	(*SetStringVector)( metadataFiles, 0, 0 );	// Clear the client's result vector.
	FillMetadataFiles( &fileList );

	// since we are dealing with STL ojbects across different DLL boundaries,
	// we are extracting const char* to actual path strings, constructing a vector
	// using these pointers and then passing the address of the underlying data
	// to the clients procedure which will repopulate its own vector of strings
	// using this information.
	if ( ! fileList.empty() ) {
		const size_t fileCount = fileList.size();
		std::vector<XMP_StringPtr> ptrArray;
		ptrArray.reserve ( fileCount );
		for ( size_t i = 0; i < fileCount; ++i ) {
			ptrArray.push_back ( fileList[i].c_str() );
		}
		(*SetStringVector) ( metadataFiles, ptrArray.data(), (XMP_Uns32)fileCount );
	}
}

void PluginBase::FillMetadataFiles( std::vector<std::string> * metadataFiles )
{
	XMP_OptionBits flags = this->mHandlerFlags;
	if ( (flags & kXMPFiles_UsesSidecarXMP) ||
		(flags & kXMPFiles_FolderBasedFormat) ) {
			XMP_Throw ( "Base implementation of FillMetadataFiles only for embedding handlers", kXMPErr_PluginFillMetadataFiles );
	}

	metadataFiles->push_back ( this->getPath() );
}


void PluginBase::FillAssociatedResources( StringVectorRef resourceList, SetStringVectorProc SetStringVector )
{
	if ( resourceList == 0 ) 
		XMP_Throw ( "A result file list vector must be provided", kXMPErr_BadParam );

	std::vector<std::string> resList;	// Pass a local vector, not the client's.
	(*SetStringVector)( resourceList, 0, 0 );	// Clear the client's result vector.
	FillAssociatedResources( &resList );

	if ( ! resList.empty() ) {
		const size_t fileCount = resList.size();
		std::vector<XMP_StringPtr> ptrArray;
		ptrArray.reserve ( fileCount );
		for ( size_t i = 0; i < fileCount; ++i ) {
			ptrArray.push_back ( resList[i].c_str() );
		}
		(*SetStringVector) ( resourceList, ptrArray.data(), (XMP_Uns32)fileCount );
	}
}


void PluginBase::FillAssociatedResources( std::vector<std::string> * resourceList ) 
{	
	XMP_OptionBits flags = this->mHandlerFlags;
	if ( (flags & kXMPFiles_HandlerOwnsFile) ||
		 (flags & kXMPFiles_UsesSidecarXMP) ||
		 (flags & kXMPFiles_FolderBasedFormat) ) {
			XMP_Throw ( "GetAssociatedResources is not implemented for this file format", kXMPErr_PluginFillAssociatedResources );
	}
	resourceList->push_back ( this->getPath() );
}

// ============================================================================
#if AdobePrivate

void PluginBase::importToXMP( XMPMetaRef xmp ) 
{
	SXMPMeta meta( xmp );
	this->importToXMP( meta );
}

#endif 
// ============================================================================
#if AdobePrivate

void PluginBase::exportFromXMP( XMPMetaRef xmp ) 
{
	SXMPMeta meta( xmp );
	this->exportFromXMP( meta );
}

#endif
// ============================================================================

void PluginBase::importToXMP( XMP_StringPtr* xmpStr, XMP_StringPtr* packetPtr , XMP_PacketInfo * packetInfo ) 
{
	this->importToXMP( xmpStr );
}

// ============================================================================

void PluginBase::importToXMP( XMP_StringPtr* xmpStr ) 
{
#if AdobePrivate
	std::string xmp( *xmpStr );
	SXMPMeta meta( xmp.c_str(),static_cast<XMP_StringLen>(xmp.length()) );
	this->importToXMP( meta );
	xmp.resize(0);
	meta.SerializeToBuffer(&xmp, kXMP_NoOptions, 0);
	XMP_Uns32 length = (XMP_Uns32)xmp.size() + 1 ;
	StringPtr bufferPtr = HostStringCreateBuffer( length );
	memcpy( bufferPtr, xmp.c_str(), length );
	*xmpStr = bufferPtr; // callee function should free the memory.
#else
	// To be implemented by the Plug-In Developer
	// Generally a Plugin developer should follow the following steps
	// when implementing this function
	// a) Create a XMP object from serialized XMP packet.
	// b) Import the data from Non-XMP content to XMP object
	// c) Serialize the XMP object to a dynamic buffer.
	//    Dynamic buffer is allocated using HostAPI HostStringCreateBuffer
	// d) Copy the dynamic buffer address to xmpStr
#endif 
}

// ============================================================================

void PluginBase::exportFromXMP( XMP_StringPtr xmpStr ) 
{
#if AdobePrivate
	std::string buffer(xmpStr);
	SXMPMeta meta( buffer.c_str(),static_cast<XMP_StringLen>(buffer.length()) );
	this->exportFromXMP( meta );
#else
	// To be implemented by the Plug-In Developer
	// Generally a Plugin developer should follow the following steps
	// when implementing this function
	// a) Create a XMP object from serialized XMP packet.
	// b) Export the data from XMP object to non-XMP Content.
#endif 
}


// ============================================================================

bool PluginBase::getFileModDate ( XMP_DateTime * modDate )
{
	XMP_OptionBits flags = this->getHandlerFlags();
	const std::string & filePath = this->getPath();
	
	if ( (flags & kXMPFiles_HandlerOwnsFile)   ||
		 (flags & kXMPFiles_UsesSidecarXMP)    ||
		 (flags & kXMPFiles_FolderBasedFormat) ||
		 filePath.empty() )
	{
		return false;
	}

	return GetModifyDate ( filePath.c_str(), modDate );
}

bool PluginBase::IsMetadataWritable ( )
{
	XMP_OptionBits flags = this->getHandlerFlags();
	const std::string & filePath = this->getPath();
	
	if ( (flags & kXMPFiles_HandlerOwnsFile)   ||
		 (flags & kXMPFiles_UsesSidecarXMP)    ||
		 (flags & kXMPFiles_FolderBasedFormat) ||
		 filePath.empty() )
	{
		XMP_Throw ( "IsMetadataWritable is not implemented for this file format", kXMPErr_PluginIsMetadataWritable );
	}

	try {
		return Host_IO::Writable( this->getPath().c_str() );
	} catch ( ... ) {

	}
	return false;
}

// ============================================================================

bool PluginBase::checkAbort( bool doAbort /*= false*/)
{
	bool abort = CheckAbort( this );

	if( abort && doAbort )
	{
		throw XMP_Error( kXMPErr_UserAbort, "User abort" );
	}

	return abort;
}

// ============================================================================


bool PluginBase::checkFormatStandard( const std::string* path /*= NULL*/ )
{
	const StringPtr _path	= (const StringPtr)( path == NULL ? this->getPath().c_str() : path->c_str() );

	return CheckFormatStandard( this, this->getFormat(), _path );
}

// ============================================================================

#if AdobePrivate

bool PluginBase::getXMPStandard( XMPMetaRef xmp, const std::string* path /*= NULL*/, bool* containsXMP /*= NULL*/ )
{
	const StringPtr _path	= (const StringPtr)( path == NULL ? this->getPath().c_str() : path->c_str() );

	std::string xmpStr;
	bool ret = GetXMPStandard( this, this->getFormat(), _path, xmpStr, containsXMP );

	SXMPMeta meta(xmp);
	meta.ParseFromBuffer( xmpStr.c_str(), static_cast<XMP_StringLen>(xmpStr.length()) );

	return ret;
}

// ============================================================================
#endif

bool PluginBase::getXMPStandard( std::string& xmpStr, const std::string* path /*= NULL*/, bool* containsXMP /*= NULL*/ )
{
	const StringPtr _path	= (const StringPtr)( path == NULL ? this->getPath().c_str() : path->c_str() );

	bool ret = GetXMPStandard( this, this->getFormat(), _path, xmpStr, containsXMP );

	return ret;
}

// ============================================================================

bool PluginBase::getXMPStandard( std::string& xmpStr, XMP_OptionBits flags, const std::string* path /*= NULL*/, bool* containsXMP /*= NULL*/, std::string *packet /*= NULL*/, XMP_PacketInfo *packetInfo /*= NULL*/, ErrorCallbackInfo * errorCallback /*= NULL*/, XMP_ProgressTracker::CallbackInfo * progCBInfoPtr /*= NULL*/ )
{
	const StringPtr _path	= (const StringPtr)( path == NULL ? this->getPath().c_str() : path->c_str() );

	bool ret = GetXMPStandard( this, this->getFormat(), _path, xmpStr, containsXMP, flags, packet, packetInfo, errorCallback, progCBInfoPtr );

	return ret;
}

// ============================================================================

bool PluginBase::putXMPStandard( const XMP_StringPtr xmpStr, XMP_OptionBits flags /*= NULL */, const std::string* path /*= NULL*/, ErrorCallbackInfo * errorCallback /*= NULL*/ , XMP_ProgressTracker::CallbackInfo * progCBInfoPtr /*= NULL*/ )
{
	const StringPtr _path	= (const StringPtr)( path == NULL ? this->getPath().c_str() : path->c_str() );

	bool ret = PutXMPStandard( this, this->getFormat(), _path, xmpStr, flags, errorCallback, progCBInfoPtr );

	return ret;
}

// ============================================================================

bool PluginBase::getFileModDateStandardHandler( XMP_DateTime * modDate, XMP_Bool * isSuccess, XMP_OptionBits flags /*= NULL */, const std::string* path /*= NULL*/ )
{
	const StringPtr _path	= (const StringPtr)( path == NULL ? this->getPath().c_str() : path->c_str() );

	bool ret = GetFileModDateStandardHandler( this, this->getFormat(), _path, modDate, isSuccess, flags );

	return ret;
}

// ============================================================================

bool PluginBase::getAssociatedResourcesStandardHandler( std::vector<std::string> * resourceList, XMP_OptionBits flags /*= NULL */, const std::string* path /*= NULL*/ )
{
	const StringPtr _path	= (const StringPtr)( path == NULL ? this->getPath().c_str() : path->c_str() );

	bool ret = GetAssociatedResourcesStandardHandler( this, this->getFormat(), _path, resourceList, flags );

	return ret;
}

// ============================================================================

bool PluginBase::isMetadataWritableStandardHandler( XMP_Bool * isWritable, XMP_OptionBits flags /*= NULL */, const std::string* path /*= NULL*/ )
{
	const StringPtr _path	= (const StringPtr)( path == NULL ? this->getPath().c_str() : path->c_str() );

	bool ret = IsMetadataWritableStandardHandler( this, this->getFormat(), _path, isWritable, flags );

	return ret;
}

// ============================================================================

void PluginBase::SetErrorCallback ( XMPFiles_ErrorCallbackWrapper wrapperProc,
		XMPFiles_ErrorCallbackProc clientProc,
		void * context,
		XMP_Uns32 limit )
{
	this->mErrorCallback.Clear();
	this->mErrorCallback.wrapperProc = wrapperProc;
	this->mErrorCallback.clientProc = clientProc;
	this->mErrorCallback.context = context;
	this->mErrorCallback.limit = limit;
	this->mErrorCallback.filePath = mPath;
}

// ============================================================================

#if XMP_WinBuild
	
static bool GetModifyDate ( const char* filePath, XMP_DateTime* modifyDate )
{
	bool ret = false;
	
	//
	// open file
	//
	HANDLE fileHandle = INVALID_HANDLE_VALUE;
	DWORD access = GENERIC_READ;	// read only
	DWORD share  = FILE_SHARE_READ;

	std::string wideName;
	const size_t utf8Len = strlen(filePath);
	const size_t maxLen = 2 * (utf8Len+1);
	
	wideName.reserve ( maxLen );
	wideName.assign ( maxLen, ' ' );
	
	if( MultiByteToWideChar ( CP_UTF8, 0, filePath, -1, (LPWSTR)wideName.data(), (int)maxLen ) != 0 )
	{
		if ( GetFileAttributes ( (LPCWSTR)wideName.data() ) != INVALID_FILE_ATTRIBUTES )
		{
			fileHandle = CreateFileW ( (LPCWSTR)wideName.data(), access, share, 0, OPEN_EXISTING,
											  (FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS), 0 );
		}
	}

	//
	// get the file modification time
	//
	if( fileHandle != INVALID_HANDLE_VALUE )
	{
		FILETIME binTime;
		
		if ( GetFileTime ( fileHandle, 0, 0, &binTime ) )
		{
			SYSTEMTIME utcTime;
			
			if ( FileTimeToSystemTime ( &binTime, &utcTime ) )
			{
				if ( modifyDate != NULL )
				{
					// Ignore the fractional seconds for consistency with UNIX and to avoid false newness even on
					// Windows. Some other sources of time only resolve to seconds, we don't want 25.3 looking
					// newer than 25.
					
					modifyDate->year		= utcTime.wYear;
					modifyDate->month		= utcTime.wMonth;
					modifyDate->day			= utcTime.wDay;
					modifyDate->hasDate		= true;
					
					modifyDate->hour		= utcTime.wHour;
					modifyDate->minute		= utcTime.wMinute;
					modifyDate->second		= utcTime.wSecond;
					modifyDate->nanoSecond	= 0;	// See note above; winTime.wMilliseconds * 1000*1000;
					modifyDate->hasTime		= true;
					
					modifyDate->tzSign		= kXMP_TimeIsUTC;
					modifyDate->tzHour		= 0;
					modifyDate->tzMinute	= 0;
					modifyDate->hasTimeZone	= true;
				}
				
				ret = true;
			}
		}

		CloseHandle ( fileHandle );
	}
	
	return ret;
}

#endif
	
#if ( XMP_MacBuild || XMP_UNIXBuild )

static bool GetModifyDate ( const char* filePath, XMP_DateTime* modifyDate )
{
	bool ret = false;
	struct stat info;
	
	if ( stat ( filePath, &info ) == 0 )
	{
		if ( S_ISREG(info.st_mode) || S_ISDIR(info.st_mode) )
		{
			ret = true;
			
			if( modifyDate != NULL )
			{
				struct tm posixUTC;
				gmtime_r ( &(info.st_mtime), &posixUTC );
				
				modifyDate->year		= posixUTC.tm_year + 1900;
				modifyDate->month		= posixUTC.tm_mon + 1;
				modifyDate->day			= posixUTC.tm_mday;
				modifyDate->hasDate		= true;
				
				modifyDate->hour		= posixUTC.tm_hour;
				modifyDate->minute		= posixUTC.tm_min;
				modifyDate->second		= posixUTC.tm_sec;
				modifyDate->nanoSecond	= 0;	// The time_t resolution is only to seconds.
				modifyDate->hasTime		= true;
				
				modifyDate->tzSign		= kXMP_TimeIsUTC;
				modifyDate->tzHour		= 0;
				modifyDate->tzMinute	= 0;
				modifyDate->hasTimeZone = true;
			}
		}
	}
	
	return ret;
}

#endif	

} //namespace XMP_PLUGIN

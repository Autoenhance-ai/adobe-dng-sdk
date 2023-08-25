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
//	PKG Praveen Kumar Goyal
//	ADC Amandeep Chawla
//
// mm-dd-yy who Description of changes, most recent on top
//
// 02-27-13 ADC 5.6-f043 Adding support for preloading plugins based on handler flags. PDF Handler will support preloading.
// 01-02-13 ADC 5.6-f011 Allowing multiple different plugins to be loaded simultaneously.
//
// 06-27-11 PKG 5.4-f001 Initial checkin of plugin architecture
//
// =================================================================================================
#endif // AdobePrivate

#ifndef MODULE_H
#define MODULE_H
#include "ModuleUtils.h"
#include "PluginManager.h"

namespace XMP_PLUGIN
{

/** @class Module
 *  @brief Manages module's loading and unloading.
 */
class Module
{
public:	
	Module( std::string & path ):
	  mPath( path ), mHandle( NULL ), mPluginAPIs( NULL ), mLoaded( kModuleNotLoaded ) { }
	
	~Module() { unload(); }

	inline OS_ModuleRef getHandle() const { return mHandle; }
	inline const std::string & getPath() const { return mPath; }
	
	/**
	 *  Returns pluginAPI. It loads the module if not already loaded.
	 *  @return pluginAPI.
	 */
	PluginAPIRef getPluginAPIs();

	/**
	 *  It loads the module if not already loaded.
	 *  @return true if module is loaded successfully otherwise returns false.
	 */
	bool load();

	/**
	 *  Unloads the module if it is loaded.
	 *  @return Void.
	 */
	void unload();

private:

	/************************************************************************/
	/* Loads the module without acquiring the lock                        */
	/************************************************************************/
	bool loadInternal();

	/************************************************************************/
	/* Unloads the module without acquiring the lock                        */
	/************************************************************************/
	void unloadInternal();

	typedef enum
	{
		kModuleNotLoaded = 0,
		kModuleLoaded,
		kModuleErrorOnLoad
	} LoadStatus;
	
	std::string			mPath;
	OS_ModuleRef		mHandle;
	PluginAPIRef		mPluginAPIs;
	LoadStatus			mLoaded;
	XMP_ReadWriteLock	mLoadingLock;
};

} //namespace XMP_PLUGIN
#endif //MODULE_H

// =================================================================================================
// Copyright 2006 Adobe
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
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
//	ADC Amandeep Chawla
//
// mm-dd-yy who Description of changes, most recent on top.
//
// 03-21-13 ADC 5.6-f047 [3526905] In MAC and Linux, progress notifications are not returned to the registered callbacks as per the set time-interval.
// 03-13-13 ADC 5.6-f046 Remove deprecated OSX APIs from XMP toolkit code.
//
// 06-22-12 AWL 5.5-f019 Add file update progress tracking to the MPEG-4 handler.
//
// 09-21-06 AWL Initial creation.
//
// =================================================================================================
#endif

#include "public/include/XMP_Environment.h"
#include "public/include/XMP_Const.h"

#include "source/PerfUtils.hpp"

#include <stdio.h>

#if XMP_WinBuild
	#pragma warning ( disable : 4996 )	// '...' was declared deprecated
#endif

// =================================================================================================
// =================================================================================================

#if XMP_WinBuild

#pragma warning ( disable : 4800 )	// forcing bool to 0/1

const char * PerfUtils::GetTimerInfo()
{
	LARGE_INTEGER freq;
	static char msgBuffer[1000];

	bool ok = (bool) QueryPerformanceFrequency ( &freq );
	if ( ! ok ) throw XMP_Error ( kXMPErr_ExternalFailure, "Failure from QueryPerformanceFrequency" );
	
	if ( freq.HighPart != 0 ) {
		return "Windows PerfUtils measures finer than nanoseconds, using the QueryPerformanceCounter() timer";
	}
	
	double rate = 1.0 / (double)freq.LowPart;
	_snprintf ( msgBuffer, sizeof(msgBuffer),
			   "Windows PerfUtils measures %e second, using the QueryPerformanceCounter() timer", rate );
	return msgBuffer;
	
}	// PerfUtils::GetTimerInfo

// ------------------------------------------------------------------------------------------------

PerfUtils::MomentValue PerfUtils::NoteThisMoment()
{
	LARGE_INTEGER now;

	bool ok = (bool) QueryPerformanceCounter ( &now );
	if ( ! ok ) throw XMP_Error ( kXMPErr_ExternalFailure, "Failure from QueryPerformanceCounter" );
	return now.QuadPart;

}	// PerfUtils::NoteThisMoment

// ------------------------------------------------------------------------------------------------

double PerfUtils::GetElapsedSeconds ( PerfUtils::MomentValue start, PerfUtils::MomentValue finish )
{
	LARGE_INTEGER freq;

	bool ok = (bool) QueryPerformanceFrequency ( &freq );
	if ( ! ok ) throw XMP_Error ( kXMPErr_ExternalFailure, "Failure from QueryPerformanceFrequency" );

	const double scale = (double)freq.QuadPart;
	
	const double elapsed = (double)(finish - start) / scale;
	return elapsed;

}	// PerfUtils::GetElapsedSeconds

#endif

// =================================================================================================

#if XMP_UNIXBuild | XMP_AndroidBuild

const char * PerfUtils::GetTimerInfo()
{
	return "UNIX PerfUtils measures nano seconds, using the POSIX clock_gettime() timer";
}	// PerfUtils::GetTimerInfo

// ------------------------------------------------------------------------------------------------

PerfUtils::MomentValue PerfUtils::NoteThisMoment()
{
	MomentValue moment = kZeroMoment;
	if ( clock_gettime( CLOCK_MONOTONIC, &moment ) != 0 )
		throw XMP_Error( kXMPErr_ExternalFailure, "Failure from clock_gettime" );
	return moment;
}	// PerfUtils::NoteThisMoment

// ------------------------------------------------------------------------------------------------

double PerfUtils::GetElapsedSeconds ( PerfUtils::MomentValue start, PerfUtils::MomentValue finish )
{
	double startInSeconds = start.tv_sec + start.tv_nsec / 1000000000.0;
	double finishInSeconds = finish.tv_sec + finish.tv_nsec / 1000000000.0;
	return finishInSeconds - startInSeconds;
}	// PerfUtils::GetElapsedSeconds

#endif

// =================================================================================================

#if XMP_MacBuild | XMP_iOSBuild

#include <mach/mach.h>
#include <mach/mach_time.h>

const char * PerfUtils::GetTimerInfo()
{
	return "Mac PerfUtils measures nano seconds, using the mach_absolute_time() timer";
}	// PerfUtils::GetTimerInfo

// ------------------------------------------------------------------------------------------------

PerfUtils::MomentValue PerfUtils::NoteThisMoment()
{
	return mach_absolute_time();
}	// PerfUtils::NoteThisMoment

// ------------------------------------------------------------------------------------------------

double PerfUtils::GetElapsedSeconds ( PerfUtils::MomentValue start, PerfUtils::MomentValue finish )
{
	static mach_timebase_info_data_t sTimebaseInfo;
	static double sConversionFactor = 0.0;
	// If this is the first time we've run, get the timebase.
	// We can use denom == 0 to indicate that sTimebaseInfo is 
	// uninitialized because it makes no sense to have a zero 
	// denominator is a fraction.

	if ( sTimebaseInfo.denom == 0 ) {
		mach_timebase_info(&sTimebaseInfo);
		sConversionFactor = ((double)sTimebaseInfo.denom / (double)sTimebaseInfo.numer);
		sConversionFactor /= 1000000000.0;
	}

	return ( finish - start ) * sConversionFactor;
}	// PerfUtils::GetElapsedSeconds

#endif



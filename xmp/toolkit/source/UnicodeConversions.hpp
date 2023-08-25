#ifndef __UnicodeConversions_h__
#define __UnicodeConversions_h__

// =================================================================================================
// Copyright 2004 Adobe
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
//  AWL Alan Lillich
//
// mm-dd-yy who Description of changes, most recent on top.
//
// 06-07-06 AWL 4.0-c007-f010 Improve the Unicode conversions.
// 03-24-06 AWL 4.0-c001 Adapt for move to ham-perforce, integrate XMPFiles, bump version to 4.
//
// 01-28-05 AWL 3.2-001 Remove BIB.
// 02-02-05 AWL 3.1.1-110 [1145657] Add UTF8_to_UTF16 and UTF8_to_UTF16 to convert std::string values.
// 12-09-04 AWl 3.1.1-099 [1041438] Add parsing and serialization support for UTF-16 and UTF-32.
// 12-06-04 AWL Cleanup for toolkit builds.
// 11-30-04 AWL Initial creation.
//
// =================================================================================================
#endif // AdobePrivate

#include <string>

// =================================================================================================

typedef XMP_Uns8  UTF8Unit;
typedef XMP_Uns16 UTF16Unit;
typedef XMP_Uns32 UTF32Unit;

// -------------------------------------------------------------------------------------------------

// ! The UTF16 and UTF32 counts are in storage units, not bytes! CodePoint values are always native.

// *** MIght be better to return a status than throw an exception for errors?

typedef void (*CodePoint_to_UTF16_Proc) ( const UTF32Unit cpIn, UTF16Unit * utf16Out, const size_t utf16Len, size_t * utf16Written );

typedef void (*CodePoint_from_UTF16_Proc) ( const UTF16Unit * utf16In, const size_t utf16Len, UTF32Unit * cpOut, size_t * utf16Read );

typedef void (*UTF8_to_UTF16_Proc)  ( const UTF8Unit *  utf8In,    const size_t utf8Len,
				                      UTF16Unit *       utf16Out,  const size_t utf16Len,
				                      size_t *          utf8Read,  size_t *     utf16Written );

typedef void (*UTF8_to_UTF32_Proc)  ( const UTF8Unit *  utf8In,    const size_t utf8Len,
				                      UTF32Unit *       utf32Out,  const size_t utf32Len,
				                      size_t *          utf8Read,  size_t *     utf32Written );

typedef void (*UTF16_to_UTF8_Proc)  ( const UTF16Unit * utf16In,   const size_t utf16Len,
				                      UTF8Unit *        utf8Out,   const size_t utf8Len,
				                      size_t *          utf16Read, size_t *     utf8Written );

typedef void (*UTF32_to_UTF8_Proc)  ( const UTF32Unit * utf32In,   const size_t utf32Len,
				                      UTF8Unit *        utf8Out,   const size_t utf8Len,
				                      size_t *          utf32Read, size_t *     utf8Written );

typedef void (*UTF16_to_UTF32_Proc) ( const UTF16Unit * utf16In,   const size_t utf16Len,
				                      UTF32Unit *       utf32Out,  const size_t utf32Len,
				                      size_t *          utf16Read, size_t *     utf32Written );

typedef void (*UTF32_to_UTF16_Proc) ( const UTF32Unit * utf32In,   const size_t utf32Len,
				                      UTF16Unit *       utf16Out,  const size_t utf16Len,
				                      size_t *          utf32Read, size_t *     utf16Written );

// -------------------------------------------------------------------------------------------------

extern void CodePoint_to_UTF8 ( const UTF32Unit cpIn, UTF8Unit *  utf8Out,  const size_t utf8Len,  size_t * utf8Written );

extern void CodePoint_from_UTF8 ( const UTF8Unit *  utf8In,  const size_t utf8Len,  UTF32Unit * cpOut, size_t * utf8Read );

extern CodePoint_to_UTF16_Proc CodePoint_to_UTF16BE;
extern CodePoint_to_UTF16_Proc CodePoint_to_UTF16LE;

extern CodePoint_from_UTF16_Proc CodePoint_from_UTF16BE;
extern CodePoint_from_UTF16_Proc CodePoint_from_UTF16LE;

extern UTF8_to_UTF16_Proc  UTF8_to_UTF16BE;
extern UTF8_to_UTF16_Proc  UTF8_to_UTF16LE;

extern UTF8_to_UTF32_Proc  UTF8_to_UTF32BE;
extern UTF8_to_UTF32_Proc  UTF8_to_UTF32LE;

extern UTF16_to_UTF8_Proc  UTF16BE_to_UTF8;
extern UTF16_to_UTF8_Proc  UTF16LE_to_UTF8;

extern UTF32_to_UTF8_Proc  UTF32BE_to_UTF8;
extern UTF32_to_UTF8_Proc  UTF32LE_to_UTF8;

extern UTF8_to_UTF16_Proc  UTF8_to_UTF16Native;
extern UTF8_to_UTF32_Proc  UTF8_to_UTF32Native;

extern UTF16_to_UTF8_Proc  UTF16Native_to_UTF8;
extern UTF32_to_UTF8_Proc  UTF32Native_to_UTF8;

extern UTF16_to_UTF32_Proc UTF16BE_to_UTF32BE;
extern UTF16_to_UTF32_Proc UTF16BE_to_UTF32LE;

extern UTF16_to_UTF32_Proc UTF16LE_to_UTF32BE;
extern UTF16_to_UTF32_Proc UTF16LE_to_UTF32LE;

extern UTF32_to_UTF16_Proc UTF32BE_to_UTF16BE;
extern UTF32_to_UTF16_Proc UTF32BE_to_UTF16LE;

extern UTF32_to_UTF16_Proc UTF32LE_to_UTF16BE;
extern UTF32_to_UTF16_Proc UTF32LE_to_UTF16LE;

extern void SwapUTF16 ( const UTF16Unit * utf16In, UTF16Unit * utf16Out, const size_t utf16Len );
extern void SwapUTF32 ( const UTF32Unit * utf32In, UTF32Unit * utf32Out, const size_t utf32Len );

extern void ToUTF16 ( const UTF8Unit * utf8In, size_t utf8Len, std::string * utf16Str, bool bigEndian );
extern void ToUTF32 ( const UTF8Unit * utf8In, size_t utf8Len, std::string * utf32Str, bool bigEndian );

extern void FromUTF16 ( const UTF16Unit * utf16In, size_t utf16Len, std::string * utf8Str, bool bigEndian );
extern void FromUTF32 ( const UTF32Unit * utf32In, size_t utf32Len, std::string * utf8Str, bool bigEndian );

extern void ToUTF16Native ( const UTF8Unit * utf8In, size_t utf8Len, std::string * utf16Str );
extern void ToUTF32Native ( const UTF8Unit * utf8In, size_t utf8Len, std::string * utf32Str );

extern void FromUTF16Native ( const UTF16Unit * utf16In, size_t utf16Len, std::string * utf8Str );
extern void FromUTF32Native ( const UTF32Unit * utf32In, size_t utf32Len, std::string * utf8Str );

extern void InitializeUnicodeConversions();

// =================================================================================================

#endif	// __UnicodeConversions_h__

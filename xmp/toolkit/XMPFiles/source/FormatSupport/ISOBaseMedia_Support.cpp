// =================================================================================================
// Copyright Adobe
// Copyright 2007 Adobe
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
//  IJS Inder Jeet Singh
//	AB  Amit Bhatti
//
// mm-dd-yy who Description of changes, most recent on top
//
// 09-03-14 AB  5.6-f119 Adding support of UUID atom in MPEG4 Handler.
// 09-05-13 IJS 5.6-f077 [3611478] MP4/QT files- Differentiate incomplete box data from Garbage and on repair, truncate only if it is Garbage.
//
// 08-19-10 AWL 5.3-f004 Move the seek mode constants to XMP_Const.
// 08-19-10 AWL 5.3-f003 Remove all use of the LFA_* names.
// 08-18-10 AWL 5.3-f002 Don't include XIO.hpp in any headers, only .cpp files.
// 08-17-10 AWL 5.3-f001 Integrate I/O revamp to main.
//
// 03-10-09 AWL 5.0-f027 Initial integration of QuickTime support into MPEG-4 handler.
//
// 11-13-07 AWL 4.2-f035 Add MPEG-4 skeleton handler. Rename MPEG handler to MPEG-2.
//
// =================================================================================================
#endif // AdobePrivate

#include "public/include/XMP_Environment.h"	// ! XMP_Environment.h must be the first included header.
#include <sstream>

#include "public/include/XMP_Const.h"

#include "XMPFiles/source/FormatSupport/ISOBaseMedia_Support.hpp"
#include "XMPFiles/source/XMPFiles_Impl.hpp"
#include "source/XIO.hpp"

// =================================================================================================
/// \file ISOBaseMedia_Support.cpp
/// \brief Manager for parsing and serializing ISO Base Media files ( MPEG-4 and JPEG-2000).
///
// =================================================================================================

namespace ISOMedia {

typedef  std::set<XMP_Uns32> KnownBoxList;
static KnownBoxList boxList;
#define ISOboxType(x,y) boxList.insert(y)
#define SEPARATOR ;
	bool IsKnownBoxType(XMP_Uns32 boxType) {
		if (boxList.empty()){
			ISOBoxList ISOBoxPrivateList ;
		}
		if (boxList.find(boxType)!=boxList.end()){
			return true;
		}
		return false;
	}
	void TerminateGlobals()
	{
		boxList.clear();
	}
#undef ISOboxType
#undef SEPARATOR
#if AdobePrivate
#undef ISOBoxPrivateList
#endif

static BoxInfo voidInfo;

// =================================================================================================
// GetBoxInfo - from memory
// ========================

const XMP_Uns8 * GetBoxInfo ( const XMP_Uns8 * boxPtr, const XMP_Uns8 * boxLimit,
							  BoxInfo * info, bool throwErrors /* = false */ )
{
	XMP_Uns32 u32Size;
	
	if ( info == 0 ) info = &voidInfo;
	info->boxType = info->headerSize = 0;
	info->contentSize = 0;
	memset( info->idUUID, 0, 16 );

	if ( boxPtr >= boxLimit ) XMP_Throw ( "Bad offset to GetBoxInfo", kXMPErr_InternalFailure );
	
	if ( (boxLimit - boxPtr) < 8 ) {	// Is there enough space for a standard box header?
		if ( throwErrors ) XMP_Throw ( "No space for ISO box header", kXMPErr_BadFileFormat );
		info->headerSize = (XMP_Uns32) (boxLimit - boxPtr);
		return boxLimit;
	}
	
	u32Size = GetUns32BE ( boxPtr );
	info->boxType = GetUns32BE ( boxPtr+4 );

	if ( u32Size >= 8 ) {
		if( info->boxType == ISOMedia::k_uuid )
		{
			if ( (boxLimit - boxPtr) < 24 ) 
			{	// Is there enough space for a uuid box header?
				if ( throwErrors ) XMP_Throw ( "No space for UUID box header", kXMPErr_BadFileFormat );
				info->headerSize = (XMP_Uns32) (boxLimit - boxPtr);
				return boxLimit;
			}
			info->headerSize  = 8 + 16;	//  16  for ID in UUID  
			memcpy( info->idUUID, boxPtr + 8, 16 );
		}
		else
		{
			info->headerSize  = 8;	// Normal explicit size case.
		}
		info->contentSize = u32Size - info->headerSize;
	} else if ( u32Size == 0 ) {
		info->headerSize  = 8;	// The box goes to EoF - treat it as "to limit".
		info->contentSize = (boxLimit - boxPtr) - 8;
	} else if ( u32Size != 1 ) {
		if ( throwErrors ) XMP_Throw ( "Bad ISO box size, 2..7", kXMPErr_BadFileFormat );
		info->headerSize  = 8;	// Bad total size in the range of 2..7, treat as 8.
		info->contentSize = 0;
	} else {
		if ( (boxLimit - boxPtr) < 16 ) {	// Is there enough space for an extended box header?
			if ( throwErrors ) XMP_Throw ( "No space for ISO extended header", kXMPErr_BadFileFormat );
			info->headerSize = (XMP_Uns32) (boxLimit - boxPtr);
			return boxLimit;
		}
		XMP_Uns64 u64Size = GetUns64BE ( boxPtr+8 );
		if ( u64Size < 16 ) {
			if ( throwErrors ) XMP_Throw ( "Bad ISO extended box size, < 16", kXMPErr_BadFileFormat );
			u64Size = 16;	// Treat bad total size as 16.
		}
		info->headerSize  = 16;
		info->contentSize = u64Size - 16;
	}
	
	XMP_Assert ( (XMP_Uns64)(boxLimit - boxPtr) >= (XMP_Uns64)info->headerSize );
	if ( info->contentSize > (XMP_Uns64)((boxLimit - boxPtr) - info->headerSize) ) {
		if ( throwErrors ) XMP_Throw ( "Bad ISO box content size", kXMPErr_BadFileFormat );
		info->contentSize = ((boxLimit - boxPtr) - info->headerSize);	// Trim a bad content size to the limit.
	}
	
	return (boxPtr + info->headerSize + info->contentSize);

}	// GetBoxInfo

// =================================================================================================
// GetBoxInfo - from a file
// ========================

XMP_Uns64 GetBoxInfo ( XMP_IO* fileRef, const XMP_Uns64 boxOffset, const XMP_Uns64 boxLimit,
					   BoxInfo * info, bool doSeek /* = true */, bool throwErrors /* = false */ )
{
	XMP_Uns8  buffer [8];
	XMP_Uns32 u32Size;
	
	if ( info == 0 ) info = &voidInfo;
	info->boxType = info->headerSize = 0;
	info->contentSize = 0;
	memset( info->idUUID, 0, 16 );
	
	if ( boxOffset >= boxLimit ) XMP_Throw ( "Bad offset to GetBoxInfo", kXMPErr_InternalFailure );
	
	if ( (boxLimit - boxOffset) < 8 ) {	// Is there enough space for a standard box header?
		if ( throwErrors ) XMP_Throw ( "No space for ISO box header", kXMPErr_BadFileFormat );
		info->headerSize = (XMP_Uns32) (boxLimit - boxOffset);
		return boxLimit;
	}
	
	if ( doSeek ) fileRef->Seek ( boxOffset, kXMP_SeekFromStart );
	(void) fileRef->ReadAll ( buffer, 8 );

	u32Size = GetUns32BE ( &buffer[0] );
	info->boxType = GetUns32BE ( &buffer[4] );

	if ( u32Size >= 8 ) {
		if( info->boxType == ISOMedia::k_uuid )
		{
			if ( (boxLimit - boxOffset) < 24 ) 
			{	// Is there enough space for a uuid box header?
				if ( throwErrors ) XMP_Throw ( "No space for UUID box header", kXMPErr_BadFileFormat );
				info->headerSize = (XMP_Uns32) (boxLimit - boxOffset);
				return boxLimit;
			}
			info->headerSize  = 8 + 16;	//  16  for ID in UUID  
			(void) fileRef->ReadAll ( info->idUUID, 16 );
		}
		else
		{
			info->headerSize  = 8;	// Normal explicit size case.
		}
		info->contentSize = u32Size - info->headerSize;
	} else if ( u32Size == 0 ) {
		info->headerSize  = 8;	// The box goes to EoF.
		info->contentSize = fileRef->Length() - (boxOffset + 8);
	} else if ( u32Size != 1 ) {
		if ( throwErrors ) XMP_Throw ( "Bad ISO box size, 2..7", kXMPErr_BadFileFormat );
		info->headerSize  = 8;	// Bad total size in the range of 2..7, treat as 8.
		info->contentSize = 0;
	} else {
		if ( (boxLimit - boxOffset) < 16 ) {	// Is there enough space for an extended box header?
			if ( throwErrors ) XMP_Throw ( "No space for ISO extended header", kXMPErr_BadFileFormat );
			info->headerSize = (XMP_Uns32) (boxLimit - boxOffset);
			return boxLimit;
		}
		(void) fileRef->ReadAll ( buffer, 8 );
		XMP_Uns64 u64Size = GetUns64BE ( &buffer[0] );
		if ( u64Size < 16 ) {
			if ( throwErrors ) XMP_Throw ( "Bad ISO extended box size, < 16", kXMPErr_BadFileFormat );
			u64Size = 16;	// Treat bad total size as 16.
		}
		info->headerSize  = 16;
		info->contentSize = u64Size - 16;
	}
	
	XMP_Assert ( (boxLimit - boxOffset) >= info->headerSize );
	if ( info->contentSize > (boxLimit - boxOffset - info->headerSize) ) {
		if ( throwErrors ) XMP_Throw ( "Bad ISO box content size", kXMPErr_BadFileFormat );
		info->contentSize = (boxLimit - boxOffset - info->headerSize);	// Trim a bad content size to the limit.
	}
	
	return (boxOffset + info->headerSize + info->contentSize);

}	// GetBoxInfo

#if AdobePrivate
bool SemiProXMLMeta::ValidateXML(const void* inXmlContent, XMP_Uns32 inSize)
{
	if (inSize == 0)
		return false;

	xmlAdapter = XMP_NewExpatAdapter(ExpatAdapter::kUseLocalNamespaces);
	xmlAdapter->ParseBuffer(inXmlContent, inSize, true);

	XML_NodePtr rootNode = 0;
	for (size_t index = 0, limit = xmlAdapter->tree.content.size(); index < limit; ++index)
		if (xmlAdapter->tree.content[index]->kind == kElemNode)
			rootNode = xmlAdapter->tree.content[index];

	if (rootNode == 0)
		return false;
	XMP_StringPtr rootLocalName = rootNode->name.c_str() + rootNode->nsPrefixLen;
	if (!XMP_LitMatch(rootLocalName, "ClipMain"))
		return false;

	XML_NodePtr contentNode = 0;
	XMP_StringPtr nameSpace = rootNode->ns.c_str();

	if (!XMP_LitMatch(nameSpace, "urn:schemas-Professional-Plug-in:Semi-Pro:ClipMetadata:v1.0"))
		return false;

	contentNode = rootNode->GetNamedElement(nameSpace, "ClipContent");
	if (contentNode == 0)
		return false;

	isValid = true;
	return isValid;
}

bool SemiProXMLMeta::IsXMLValid()
{
	return isValid;
}

bool SemiProXMLMeta::ImportXMLProperties(SXMPMeta * xmp)
{
	bool haveImports = false;
	std::string value;
	XML_NodePtr node = 0, contentNode = 0, rootNode = 0, tempNode = 0;

	for (size_t index = 0, limit = xmlAdapter->tree.content.size(); index < limit; ++index)
		if (xmlAdapter->tree.content[index]->kind == kElemNode)
			rootNode = xmlAdapter->tree.content[index];

	XMP_StringPtr nameSpace = rootNode->ns.c_str();
	contentNode = rootNode->GetNamedElement(nameSpace, "ClipContent");

	node = contentNode->GetNamedElement(nameSpace, "ClipMetadata");
	if (node != 0)
	{
		tempNode = node->GetNamedElement(nameSpace, "Device");
		if (tempNode != 0)
		{
			GetValue(tempNode->GetNamedElement(nameSpace, "Manufacturer"), value);
			if (!value.empty())
			{
				xmp->SetProperty(kXMP_NS_TIFF, "Make", value, kXMP_DeleteExisting);
				haveImports = true;
			}

			GetValue(tempNode->GetNamedElement(nameSpace, "ModelName"), value);
			if (!value.empty())
			{
				xmp->SetProperty(kXMP_NS_TIFF, "Model", value, kXMP_DeleteExisting);
				haveImports = true;
			}
		}
	}

	node = contentNode->GetNamedElement(nameSpace, "EssenceList");
	if (node != 0)
	{
		std::string codec;
		tempNode = node->GetNamedElement(nameSpace, "Video");
		if (tempNode != 0)
		{
			XML_NodePtr codecPtr = tempNode->GetNamedElement(nameSpace, "Codec");
			GetValue(codecPtr, codec);
			if (!codec.empty())
			{
				if (codecPtr->attrs.size() == 1)
				{
					GetAttributeValue(codecPtr->attrs[0], value);
					if (!value.empty())
					{
						codec = codec + " " + value;
					}
				}
				xmp->SetProperty(kXMP_NS_DM, "videoCompressor", codec, kXMP_DeleteExisting);
				haveImports = true;
			}
		}
	}

	XML_NodePtr userNode = rootNode->GetNamedElement(nameSpace, "UserArea");
	if (userNode == 0)
		return haveImports;

	XML_NodePtr acquisitionMetadata = 0;
	for (size_t index = 0, limit = userNode->content.size(); index < limit; ++index)
		if (userNode->content[index]->kind == kElemNode)
			acquisitionMetadata = userNode->content[index];
	if (acquisitionMetadata == 0)
		return haveImports;

	XMP_StringPtr userNameSpace = acquisitionMetadata->ns.c_str();
	if (!XMP_LitMatch(userNameSpace, "urn:schemas-Professional-Plug-in:P2:CameraMetadata:v1.2") &&
		!XMP_LitMatch(userNameSpace, "urn:schemas-Professional-Plug-in:P2:CameraMetadata:v1.0"))
		return haveImports;

	XML_NodePtr cameraData = 0;
	cameraData = acquisitionMetadata->GetNamedElement(userNameSpace, "CameraUnitMetadata");
	if (cameraData != 0)
	{
		tempNode = cameraData->GetNamedElement(userNameSpace, "Gamma");
		if (tempNode != 0)
		{
			GetValue(tempNode->GetNamedElement(userNameSpace, "CaptureGamma"), value);
			if (!value.empty())
			{
				xmp->SetProperty(kXMP_NS_DM, "gammaCurve", value, kXMP_DeleteExisting);
				haveImports = true;
			}
		}

		tempNode = cameraData->GetNamedElement(userNameSpace, "Gamut");
		if (tempNode != 0)
		{
			GetValue(tempNode->GetNamedElement(userNameSpace, "CaptureGamut"), value);
			if (!value.empty())
			{
				xmp->SetProperty(kXMP_NS_DM, "videoColorSpace", value, kXMP_DeleteExisting);
				haveImports = true;
			}
		}

		GetValue(cameraData->GetNamedElement(userNameSpace, "WhiteBalanceColorTemperature"), value);
		if (!value.empty())
		{
			if (value.back() == 'K')
				value.pop_back();

			std::istringstream stream(value);
			XMP_Uns32 temperature;
			stream >> temperature;

			xmp->SetProperty_Int(kXMP_NS_DM, "whiteBalanceColorTemperature", temperature, kXMP_DeleteExisting);
			haveImports = true;
		}

		GetValue(cameraData->GetNamedElement(userNameSpace, "ISOSensitivity"), value);
		if (!value.empty())
		{
			std::istringstream stream(value);
			XMP_Uns32 sensitivity;
			stream >> sensitivity;

			xmp->SetProperty_Int(kXMP_NS_ExifEX, "SensitivityType", 3, kXMP_DeleteExisting);
			if (sensitivity < 65535)
			{
				xmp->SetProperty_Int(kXMP_NS_ExifEX, "PhotographicSensitivity", sensitivity, kXMP_DeleteExisting);
				if (xmp->DoesPropertyExist(kXMP_NS_ExifEX, "ISOSpeed"))
				{
					xmp->SetProperty_Int(kXMP_NS_ExifEX, "ISOSpeed", sensitivity, kXMP_DeleteExisting);
				}
			}
			else
			{
				xmp->SetProperty_Int(kXMP_NS_ExifEX, "PhotographicSensitivity", 65535, kXMP_DeleteExisting);
				xmp->SetProperty_Int64(kXMP_NS_ExifEX, "ISOSpeed", sensitivity, kXMP_DeleteExisting);
			}
			haveImports = true;
		}

	}
	return haveImports;
}



void SemiProXMLMeta::GetAttributeValue(XML_NodePtr node, std::string& value)
{
	value.assign("");
	if (node != 0)
	{
		value = node->value;
	}
}

void SemiProXMLMeta::GetValue(XML_NodePtr node, std::string &value)
{
	value.assign("");
	if (node != 0 && node->content.size() != 0)
		value = node->content[0]->value;
}

SemiProXMLMeta::~SemiProXMLMeta()
{
	delete xmlAdapter;
	xmlAdapter = 0;
}
#endif

}	// namespace ISO_Media

// =================================================================================================
// ISOBaseMedia_Manager::GetBoxInfo
// ===========================
void ISOBaseMedia_Manager::GetBoxInfo(const BoxRef ref, BoxInfo * info) const
{

	this->FillBoxInfo(*((BoxNode*)ref), info);

}	// MOOV_Manager::GetBoxInfo

// =================================================================================================
// ISOBaseMedia_Manager::FillBoxInfo
// ===========================
void ISOBaseMedia_Manager::FillBoxInfo(const BoxNode & node, BoxInfo * info) const
{
	if (info == 0) return;

	info->boxType = node.boxType;
	info->childCount = (XMP_Uns32)node.children.size();
	info->contentSize = node.contentSize;
	info->content = PickContentPtr(node);
	if (node.boxType == ISOMedia::k_uuid)
		memcpy(info->idUUID, node.idUUID, 16);

}	// ISOBaseMedia_Manager::FillBoxInfo
// =================================================================================================

// =================================================================================================
// ISOBaseMedia_Manager::PickContentPtr
// ===========================
XMP_Uns8 * ISOBaseMedia_Manager::PickContentPtr(const BoxNode & node) const
{
	if (node.contentSize == 0) {
		return 0;
	}
	else if (node.changed && node.changedContent.size()>0) {

		return (XMP_Uns8*)&node.changedContent[0];
	}
	else {
		return (XMP_Uns8*) &this->fullSubtree[0] + node.offset + node.headerSize;
	}
}	// ISOBaseMedia_Manager::PickContentPtr

// =================================================================================================
// ISOBaseMedia_Manager::GetNthChild
// ===========================
ISOBaseMedia_Manager::BoxRef ISOBaseMedia_Manager::GetNthChild(BoxRef parentRef, size_t childIndex, BoxInfo * info) const
{
	XMP_Assert(parentRef != 0);
	const BoxNode & parent = *((BoxNode*)parentRef);
	if (info != 0) memset(info, 0, sizeof(BoxInfo));

	if (childIndex >= parent.children.size()) return 0;

	const BoxNode & currNode = parent.children[childIndex];

	this->FillBoxInfo(currNode, info);
	return &currNode;

}	// ISOBaseMedia_Manager::GetNthChild

// =================================================================================================
// ISOBaseMedia_Manager::GetTypeChild
// ===========================
ISOBaseMedia_Manager::BoxRef ISOBaseMedia_Manager::GetTypeChild(BoxRef parentRef, XMP_Uns32 childType, BoxInfo * info) const
{
	XMP_Assert(parentRef != 0);
	const BoxNode & parent = *((BoxNode*)parentRef);
	if (info != 0) memset(info, 0, sizeof(BoxInfo));
	if (parent.children.empty()) return 0;

	size_t i = 0, limit = parent.children.size();
	for (; i < limit; ++i) {
		const BoxNode & currNode = parent.children[i];
		if (currNode.boxType == childType) {
			this->FillBoxInfo(currNode, info);
			return &currNode;
		}
	}

	return 0;

}	// ISOBaseMedia_Manager::GetTypeChild

// =================================================================================================
// ISOBaseMedia_Manager::GetParsedOffset
// ===========================
XMP_Uns32 ISOBaseMedia_Manager::GetParsedOffset(BoxRef ref) const
{
	XMP_Assert(ref != 0);
	const BoxNode & node = *((BoxNode*)ref);

	if (node.changed) return 0;
	return node.offset;

}	// ISOBaseMedia_Manager::GetParsedOffset

// =================================================================================================
// ISOBaseMedia_Manager::GetHeaderSize
// ===========================

XMP_Uns32 ISOBaseMedia_Manager::GetHeaderSize(BoxRef ref) const
{
	XMP_Assert(ref != 0);
	const BoxNode & node = *((BoxNode*)ref);

	if (node.changed) return 0;
	return node.headerSize;

}	// ISOBaseMedia_Manager::GetHeaderSize

// =================================================================================================
// ISOBaseMedia_Manager::NoteChange
// ========================

void ISOBaseMedia_Manager::NoteChange()
{

	this->subtreeRootNode.changed = true;

} // ISOBaseMedia_Manager::NoteChange

// =================================================================================================
// ISOBaseMedia_Manager::SetBox
// ====================
//
// Save the new data, set this box's changed flag, and set the top changed flag.

void ISOBaseMedia_Manager::SetBox(BoxRef theBox, const void* dataPtr, XMP_Uns32 size, const XMP_Uns8 * idUUID)
{
	XMP_Enforce(size < TopBoxSizeLimit);
	BoxNode * node = (BoxNode*)theBox;

	if (node->contentSize == size) {
		if (node->boxType == ISOMedia::k_uuid && idUUID != 0)
		{
			memcpy(node->idUUID, idUUID, 16);
			this->subtreeRootNode.changed = true;
		}
		XMP_Uns8 * oldContent = PickContentPtr(*node);
		if (memcmp(oldContent, dataPtr, size) == 0) return;	// No change.
		memcpy(oldContent, dataPtr, size);	// Update the old content in-place
		this->subtreeRootNode.changed = true;

#if TraceUpdateMoovTree
		XMP_Uns32 be32 = MakeUns32BE(node->boxType);
		fprintf(stderr, "Updated '%.4s', parse offset 0x%X, same size\n", &be32, node->offset);
#endif

	}
	else {

		node->changedContent.assign(size, 0);	// Fill with 0's first to get the storage.
		memcpy(&node->changedContent[0], dataPtr, size);
		node->contentSize = size;
		node->changed = true;
		if (node->boxType == ISOMedia::k_uuid && idUUID != 0)
			memcpy(node->idUUID, idUUID, 16);
		this->subtreeRootNode.changed = true;

#if TraceUpdateMoovTree
		XMP_Uns32 be32 = MakeUns32BE(node->boxType);
		XMP_Uns32 addr32 = (XMP_Uns32) this->PickContentPtr(*node);
		fprintf(stderr, "Updated '%.4s', parse offset 0x%X, new size %d, new content @ 0x%X\n",
			&be32, node->offset, node->contentSize, addr32);
#endif

	}

}	// MOOV_Manager::SetBox

// =================================================================================================
// ISOBaseMedia_Manager::DeleteNthChild
// ============================

bool ISOBaseMedia_Manager::DeleteNthChild ( BoxRef parentRef, size_t childIndex )
{
	BoxNode * parent = (BoxNode*)parentRef;
	
	if ( childIndex >= parent->children.size() ) return false;

	parent->children.erase ( parent->children.begin() + childIndex );
	return true;
	
}	// ISOBaseMedia_Manager::DeleteNthChild

// =================================================================================================
// ISOBaseMedia_Manager::DeleteTypeChild
// =============================

bool ISOBaseMedia_Manager::DeleteTypeChild ( BoxRef parentRef, XMP_Uns32 childType )
{
	BoxNode * parent = (BoxNode*)parentRef;
	
	BoxListPos child = parent->children.begin();
	BoxListPos limit = parent->children.end();
	
	for ( ; child != limit; ++child ) {
		if ( child->boxType == childType ) {
			parent->children.erase ( child );
			this->subtreeRootNode.changed = true;
			return true;
		}
	}
	
	return false;
	
}	// ISOBaseMedia_Manager::DeleteTypeChild

void ISOBaseMedia_Manager::WriteBoxHeader(XMP_IO* fileRef, XMP_Uns32 boxType, XMP_Uns64 boxSize)
{
	XMP_Uns32 u32;
	XMP_Uns64 u64;
	XMP_Enforce(boxSize >= 8);	// The size must be the full size, not just the content.

	if (boxSize <= 0xFFFFFFFF) {

		u32 = MakeUns32BE((XMP_Uns32)boxSize);
		fileRef->Write(&u32, 4);
		u32 = MakeUns32BE(boxType);
		fileRef->Write(&u32, 4);

	}
	else {

		u32 = MakeUns32BE(1);
		fileRef->Write(&u32, 4);
		u32 = MakeUns32BE(boxType);
		fileRef->Write(&u32, 4);
		u64 = MakeUns64BE(boxSize);
		fileRef->Write(&u64, 8);

	}

}	// WriteBoxHeader

// =================================================================================================
// WipeBoxFree
// ===========
//
// Change the box's type to 'free' (or create a 'free' box) and zero the content.

static XMP_Uns8 kZeroes[64 * 1024];	// C semantics guarantee zero initialization.

void ISOBaseMedia_Manager::WipeBoxFree(XMP_IO* fileRef, XMP_Uns64 boxOffset, XMP_Uns32 boxSize)
{
	if (boxSize == 0) return;
	XMP_Enforce(boxSize >= 8);

	if (boxOffset > Max_XMP_Int64) 
		XMP_Throw( "Offset out of Int64 bound", kXMPErr_EnforceFailure);
	
	fileRef->Seek(boxOffset, kXMP_SeekFromStart);
	XMP_Uns32 u32;
	u32 = MakeUns32BE(boxSize);	// ! The actual size should not change, but might have had a long header.
	fileRef->Write(&u32, 4);
	u32 = MakeUns32BE(ISOMedia::k_free);
	fileRef->Write(&u32, 4);

	XMP_Uns32 ioCount = sizeof(kZeroes);
	for (boxSize -= 8; boxSize > 0; boxSize -= ioCount) {
		if (ioCount > boxSize) ioCount = boxSize;
		fileRef->Write(&kZeroes[0], ioCount);
	}

}	// WipeBoxFree

// =================================================================================================
// ISOBaseMedia_Manager::CreateFreeSpaceList
// ===================
void ISOBaseMedia_Manager::CreateFreeSpaceList(XMP_IO* fileRef, XMP_Uns64 fileSize,
	XMP_Uns64 oldOffset, XMP_Uns32 oldSize, SpaceList * spaceList)
{
	XMP_Uns64 boxPos = 0, boxNext = 0, adjacentFree = 0;
	ISOMedia::BoxInfo currBox;

	fileRef->Rewind();
	spaceList->clear();

	for (boxPos = 0; boxPos < fileSize; boxPos = boxNext) {

		boxNext = ISOMedia::GetBoxInfo(fileRef, boxPos, fileSize, &currBox, true /* throw errors */);
		XMP_Uns64 currSize = currBox.headerSize + currBox.contentSize;

		if ((currBox.boxType == ISOMedia::k_free) ||
			(currBox.boxType == ISOMedia::k_skip) ||
			((boxPos == oldOffset) && (currSize == oldSize))) {

			if (spaceList->empty() || (boxPos != adjacentFree)) {
				spaceList->push_back(SpaceInfo(boxPos, currSize));
				adjacentFree = boxPos + currSize;
			}
			else {
				SpaceInfo * lastSpace = &spaceList->back();
				lastSpace->size += currSize;
			}

		}

	}

}	// ISOBaseMedia_Manager::CreateFreeSpaceList

// =================================================================================================
// ISOBaseMedia_Manager::AddChildBox
// =========================
ISOBaseMedia_Manager::BoxRef ISOBaseMedia_Manager::AddChildBox(BoxRef parentRef, XMP_Uns32 childType, const void* dataPtr, XMP_Uns32 size, const XMP_Uns8 * idUUID)
{
	BoxNode * parent = (BoxNode*)parentRef;
	XMP_Assert(parent != 0);

	if (childType == ISOMedia::k_uuid && idUUID != 0)
		parent->children.push_back(BoxNode(0, childType, 0, idUUID, 0));
	else
		parent->children.push_back(BoxNode(0, childType, 0, 0));
	BoxNode * newNode = &parent->children.back();
	this->SetBox(newNode, dataPtr, size);

	return newNode;

}	// ISOBaseMedia_Manager::AddChildBox



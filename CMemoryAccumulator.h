///////////////////////////////////////////////////////////////////////////////////////////////////
//	CMemoryAccumulator.cpp
//
//	Class specialized for allocating small blocks of memory and accumulating them.
//	The benefits of this class is an incredibly small overhead since there is
//	no housekeeping of allocated blocks.  Instead the allocated blocks are appended
//	in the allocator.
//	Another benefit is there is no need to free a memory block allocated by the allocator,
//	since the destructor of CMemoryAccumulator deletes all memory blocks at once.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CMEMORYACCUMULATOR_H
#define CMEMORYACCUMULATOR_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif


//	Flags for PrgbzAllocateSortKey*()
#define ASK_mskmAllocateExtraBytesMask		0x000000FF	// Extra bytes to allocate before allocating the sort key
#define ASK_mskzKeepPunctuation				0x00000000	// Create a sort key to compare string with spaces, accents and punctuation (normal sorting)
#define ASK_mskfIgnorePunctuation			0x00010000	// Ignore the spaces and punctuation when sorting
#define ASK_mskmReserved					0x0000FF00	// To make sure we don't pass extreme values.

class CMemoryAccumulator
{
protected:
	// The allocator contains a linked list of memory blocks
	struct SHeaderWithData;
	struct SHeaderOnly
		{
		SHeaderWithData * pNext;	// Pointer to the next memory block (if any)
		int cbAlloc;		// Number of bytes allocated after the header
		int cbData;			// Number of bytes in the buffer after the header
		};
	struct SHeaderWithData : public SHeaderOnly
		{
		BYTE rgbData[d_nLengthArrayDebug];		// The data follows the header and the number of bytes in rgbData[] is  cbData
		};
	SHeaderWithData * m_paData;

public:
	inline CMemoryAccumulator() { m_paData = NULL; }
	~CMemoryAccumulator();
	VOID SetAllocSize(int cbAlloc);
	void RecycleBuffer();
	BOOL FIsEmpty();
	void AppendData(const void * pvData, int cbData);
	void AppendTextU(PSZUC pszString);
	void AppendStringFormat_Gsb(PSZAC pszFmtTemplate, ...);
	EError EAppendToFileAndEmpty(const QString & sFileName);

	void AlignToUInt32();
	void * PvGetBuffer(int cbBufferMin);
	void CommitBuffer(int cbBufferUsed);
	void * PvAllocateData(int cbAlloc);
	void * PvAllocateDataCopy(int cbAlloc, PCVOID pvDataSrcCopy);
	VOID * PvAllocateDataCopyAndAlignToDWORD(int cbAlloc, PCVOID pvDataSrcCopy);
	VOID * PvAllocateDataAndStringW(int cbAlloc, PSZWC pszwString);

	PSZU PszuAllocateEncodeStringW(PSZWC pszwString);
	PSZU PszuAllocateEncodeStringQ(const QString & sString);
	PSZU PszuAllocateCopyCBinString(IN_MOD_TMP CBin & binString);
	PSZU PszuAllocateCopyStrU(const CStr & strString);
	PSZU PszuAllocateCopyStringU(PSZUC pszuString);
	PSZU PszuAllocateCopyStringNullU(PSZUC pszuString);
	PSZU PszuAllocateStringFromSingleCharacter(CHS chString);

	PSZU PszuAllocateStringNullUandAlignToDWORD(PSZUC pszuString);
	PSZU PszuAllocateStringW(PSZWC pszwString);
	PSZW PszwAllocateStringW(PSZWC pszwString);
	PSZWC PszwAllocateStringWfromU(PSZUC pszuString);
	PSZUC PrgbzAllocateSortKeyW(PSZWC pszwString, UINT uFlagsASK = ASK_mskzKeepPunctuation);
	/*
	SORTKEY * PSortKeyAllocateForStringA(PSZAC pszaString);
	SORTKEYINTEGER * PSortKeyAllocateForInteger(int nValue);
	*/

protected:
	void _AllocateMemoryBlock(int cbAlloc);

}; // CMemoryAccumulator

#endif // CMEMORYACCUMULATOR_H

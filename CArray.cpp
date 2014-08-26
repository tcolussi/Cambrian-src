/////////////////////////////////////////////////////////////////////
//	CArray.cpp
//
//	Storage efficient high-perforformance dynamic array.
//
//	Also contains methods for commonly used array operations.
//
//	IMPLEMENTATION NOTES
//	The code has been written to enhance execution speed at the expense
//	of simplicity and ease to read.
/////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif


/////////////////////////////////////////////////////////////////////
//	S_PaAllocateElements(), static
//
//	Allocate memory to hold a number of elements
//
CArray::SHeaderWithData *
CArray::S_PaAllocateElements(int cElementsAlloc)
	{
	Assert(cElementsAlloc > 0);
	SHeaderWithData * paArrayHdr = (SHeaderWithData *)new BYTE[sizeof(SHeaderOnly) + cElementsAlloc * sizeof(void *)];
	Assert(paArrayHdr!= NULL);
	InitToGarbage(OUT paArrayHdr, sizeof(SHeaderOnly) + cElementsAlloc * sizeof(void *));
	paArrayHdr->cElementsAlloc = cElementsAlloc;
	//ValidateHeapPointerDescr(paArrayHdr, "CArray::S_PaAllocateElements()");
	return paArrayHdr;
	}

/////////////////////////////////////////////////////////////////////
//	PrgpvGetElementsStop()
//
//	Return the pointer to the first element in the array, and the
//	pointer to the end of the array.
//
//	This method is ideal to create a loop for all the elements in the array.
//
//	EXAMPLE
//	void ** ppObjectStop;
//	void ** ppObject = PrgpvGetElementsStop(OUT &ppObjectStop);
//	while (ppObject != ppObjectStop)
//		{
//		... do some stuff here
//		}
//
void **
CArray::PrgpvGetElementsStop(OUT void *** pppvDataStop) const
	{
	Assert(pppvDataStop != NULL);
	if (m_paArrayHdr != NULL)
		{
		//ValidateHeapPointerDescr(m_paArrayHdr, "CArray::PpvGetElementsStop()");
		Assert(m_paArrayHdr->cElements <= m_paArrayHdr->cElementsAlloc);
		*pppvDataStop = (void **)m_paArrayHdr->rgpvData + m_paArrayHdr->cElements;
		return (void **)m_paArrayHdr->rgpvData;
		}
	*pppvDataStop = NULL;
	return NULL;
	}

/////////////////////////////////////////////////////////////////////
//	PrgpvGetElements()
//
//	Return a pointer to beginning of array to allow direct access
//	to elements in the array.  May return NULL.
//
void **
CArray::PrgpvGetElements() const
	{
	if (m_paArrayHdr != NULL)
		return (void **)&m_paArrayHdr->rgpvData;
	return NULL;
	}

//	Similar as above except return the total number of elements instea
void **
CArray::PrgpvGetElements(OUT int * pcElements) const
	{
	if (m_paArrayHdr != NULL)
		{
		*pcElements = m_paArrayHdr->cElements;
		return (void **)&m_paArrayHdr->rgpvData;
		}
	*pcElements = 0;
	return NULL;
	}

/*
/////////////////////////////////////////////////////////////////////
void ** 
CArray::PrgpvGetElements()
	{
	if (m_paArrayHdr != NULL)
		return (void **)&m_paArrayHdr->rgpvData;
	return NULL;
	}
*/

//	Prevent the array from having more than cElementsMax
VOID
CArray::KeepMaximumElements(int cElementsMax)
	{
	if (m_paArrayHdr != NULL)
		{
		if (m_paArrayHdr->cElements > cElementsMax)
			m_paArrayHdr->cElements = cElementsMax;
		}
	}

/////////////////////////////////////////////////////////////////////
//	SetSizeGrow()
//
//	Sets the number of elements to be contained in this array.
//
//	If the desired size is smaller than the current number of element,
//	the routine will simply truncate the array to the desired size.
//
//	If the desired size is larger than the current number of elements,
//	the routine will allocate memory to accommodate the number of elements.
//	The content of array will be preserved and allocated elements will be
//	initialized with zeroes.
//
//	PERFORMANCE NOTES
//	Shrinking the array does not free unused memory.  Use FreeExtra()
//	to free unused memory.
//
void
CArray::SetSizeGrow(int cElements, BOOL fInitializeWithZeroes)
	{
	Assert(cElements >= 0);
	if (cElements <= 0)
		{
		if (m_paArrayHdr != NULL)
			m_paArrayHdr->cElements = 0;
		return;
		}
	int cElementsPrev;		// Number of elements in array before re-allocating
	if (m_paArrayHdr == NULL)
		{
		cElementsPrev = 0;
		m_paArrayHdr = S_PaAllocateElements(cElements);
		}
	else
		{
		if (cElements <= m_paArrayHdr->cElements)
			{
			m_paArrayHdr->cElements = cElements;	// Truncate the array
			return;
			}
		cElementsPrev = m_paArrayHdr->cElements;
		if (cElements > m_paArrayHdr->cElementsAlloc)
			{
			// Need to re-allocate memory
			SHeaderWithData * paArrayHdr = S_PaAllocateElements(cElements + m_paArrayHdr->cElementsAlloc);		// Allocate at least double the memory previously allocated
			Assert(paArrayHdr!= NULL);
			if (cElementsPrev > 0)
				{
				// Copy the previous array into the new array
				memcpy(OUT paArrayHdr->rgpvData, IN m_paArrayHdr->rgpvData, cElementsPrev * sizeof(void *));
				}
			ValidateHeapPointer(m_paArrayHdr);
			delete m_paArrayHdr;	// Delete previous array
			m_paArrayHdr = paArrayHdr;
			}
		}
	m_paArrayHdr->cElements = cElements;
	Assert(cElements >= cElementsPrev);
	if (fInitializeWithZeroes)
		{
		// Initialize the end of the array with zeroes
		InitToZeroes(OUT &m_paArrayHdr->rgpvData[cElementsPrev], (cElements - cElementsPrev) * sizeof(void *));
		}
	} // SetSizeGrow()


/////////////////////////////////////////////////////////////////////
//	Allocate elements and initialize all the pointers to NULL.
//
void
CArray::ZeroInitialize(int cElements)
	{
	if (m_paArrayHdr != NULL)
		m_paArrayHdr->cElements = 0;	// This will force the entire array to be re-initialized with zeroes
	SetSizeGrow(cElements, TRUE);
	#ifdef DEBUG
	// Verify all the elements are zeroes
	if (m_paArrayHdr != NULL)
		{
		int cElements = m_paArrayHdr->cElements;
		for (int iElement = 0; iElement < cElements; iElement++)
			{
			PCVOID pvElement = m_paArrayHdr->rgpvData[iElement];
			Assert(pvElement == 0);
			}
		}
	#endif
	} // ZeroInitialize()

/////////////////////////////////////////////////////////////////////
//	PvGetElementAtSafe_YZ()
//
//	Return the element at a given index if within bounds, otherwise return NULL.
//
void *
CArray::PvGetElementAtSafe_YZ(int iElement) const
	{
	if (m_paArrayHdr == NULL)
		return NULL;
	Assert(m_paArrayHdr->cElements <= m_paArrayHdr->cElementsAlloc);
	if (iElement >= 0 && iElement < m_paArrayHdr->cElements)
		return (void *)m_paArrayHdr->rgpvData[iElement];
	return NULL;
	}

void *
CArray::PvGetElementAtSafeModuloSize_YZ(UINT iElement) const
	{
	if (m_paArrayHdr != NULL)
		{
		UINT cElements = m_paArrayHdr->cElements;
		if (cElements > 0)
			return (void *)m_paArrayHdr->rgpvData[iElement % cElements];
		}
	return NULL;
	}

void *
CArray::PvGetElementAtSafeRandom_YZ() const
	{
	return PvGetElementAtSafeModuloSize_YZ(qrand());
	}

//	Get the first element in the array.
//	If the array is empty, return NULL.
void *
CArray::PvGetElementFirst_YZ() const
	{
	if (m_paArrayHdr != NULL && m_paArrayHdr->cElements > 0)
		return (void *)m_paArrayHdr->rgpvData[0];	// Return the first element
	return NULL;
	}

//	Get the last element in the array.
//	If the array is empty, return NULL.
void *
CArray::PvGetElementLast_YZ() const
	{
	if (m_paArrayHdr != NULL && m_paArrayHdr->cElements > 0)
		return (void *)m_paArrayHdr->rgpvData[m_paArrayHdr->cElements - 1];	// Return the last element
	return NULL;
	}

//	Return NULL if the the array does not contain exactly 1 element,
//	otherwise return the element
void *
CArray::PvGetElementUnique_YZ() const
	{
	if (m_paArrayHdr != NULL && m_paArrayHdr->cElements == 1)
		return (void *)m_paArrayHdr->rgpvData[0];	// Return the first element
	return NULL;
	}

//	Initialize he array to hold cElements
void **
CArray::PrgpvAllocateElementsSetSize(int cElements)
	{
	Assert(cElements >= 0);
	void ** ppvElementFirst = PrgpvAllocateElementsEmpty(cElements);
	if (ppvElementFirst != NULL)
		m_paArrayHdr->cElements = cElements;
	return ppvElementFirst;
	}

/////////////////////////////////////////////////////////////////////
//	PrgpvAllocateElementsEmpty()
//
//	Fast method to allocate memory to hold a number of elements.
//	
//	USAGE
//	Use this method to allocate elements before using the array.
//	Pre-allocating the size of the array will avoid the need to
//	reallocate and copy the array when elements are added.
//
//	INTERFACE NOTES
//	This method destroys the content of the array.
//	After calling this method, the size returned by GetSize() is always zero.
//	To change the size of the array without loosing its content,
//	use the SetSizeGrow() or SetAtGrow() methods.
//
//	Return pointer to the beginning of the array as GetData() would.
void **
CArray::PrgpvAllocateElementsEmpty(int cElementsAlloc)
	{
	if (m_paArrayHdr == NULL)
		{
		if (cElementsAlloc <= 0)
			return NULL;
		m_paArrayHdr = S_PaAllocateElements(cElementsAlloc);
		}
	else if (cElementsAlloc > m_paArrayHdr->cElementsAlloc)
		{
		SHeaderWithData * paArrayHdr = S_PaAllocateElements(cElementsAlloc);
		Assert(paArrayHdr!= NULL);
		ValidateHeapPointer(m_paArrayHdr);
		delete m_paArrayHdr;	// Delete previous array
		m_paArrayHdr = paArrayHdr;
		}
	m_paArrayHdr->cElements = 0;	// Empty the array
	return (void **)&m_paArrayHdr->rgpvData;
	}

void **
CArray::PrgpvAllocateElementsEmpty(const CArray * parray)
	{
	Assert(parray != NULL);
	return PrgpvAllocateElementsEmpty(parray->GetSize());
	}

/////////////////////////////////////////////////////////////////////
//	Grow the array to include more elements.
//
//	Return pointer where the data should be added.
void **
CArray::PrgpvAllocateElementsAppend(int cElementsAllocGrowBy)
	{
	Assert(cElementsAllocGrowBy >= 0);
	if (cElementsAllocGrowBy > 0)
		{
		if (m_paArrayHdr != NULL)
			{
			int cElementsOld = m_paArrayHdr->cElements;
			int cElementsNew = cElementsOld + cElementsAllocGrowBy;
			if (cElementsNew >= m_paArrayHdr->cElementsAlloc)
				{
				if (cElementsAllocGrowBy < cElementsOld)
					cElementsAllocGrowBy = cElementsOld;
				SHeaderWithData * paArrayHdr = S_PaAllocateElements(cElementsNew + cElementsAllocGrowBy);
				Assert(paArrayHdr!= NULL);
				// Copy the previous array into the new array
				memcpy(OUT paArrayHdr->rgpvData, IN m_paArrayHdr->rgpvData, cElementsOld * sizeof(void *));
				ValidateHeapPointer(m_paArrayHdr);
				delete m_paArrayHdr;	// Delete previous array
				m_paArrayHdr = paArrayHdr;
				}
			m_paArrayHdr->cElements = cElementsNew;
			return (void **)&m_paArrayHdr->rgpvData + cElementsOld;
			}
		else
			{
			m_paArrayHdr = S_PaAllocateElements(cElementsAllocGrowBy);
			m_paArrayHdr->cElements = cElementsAllocGrowBy;
			}
		}
	if (m_paArrayHdr != NULL)
		return (void **)&m_paArrayHdr->rgpvData;
	return NULL;
	} // PrgpvAllocateElementsAppend()


/////////////////////////////////////////////////////////////////////
//	RemoveAll()
//
//	Empty the array.
//
//	IMPLEMENTATION NOTES
//	For performance the memory is not freed.  To free the memory use FreeExtra().
//
void
CArray::RemoveAllElements()
	{
	if (m_paArrayHdr != NULL)
		m_paArrayHdr->cElements = 0;
	}


/////////////////////////////////////////////////////////////////////
//	FreeExtra()
//
//	Frees all unused memory above the current upper bound.
//
//	IMPLEMENTATION NOTES
//	Memory is freed only if array is empty
//
void
CArray::FreeExtra()
	{
	if (m_paArrayHdr == NULL)
		return;	// Array already empty
	if (m_paArrayHdr->cElements == 0)
		{
		ValidateHeapPointer(m_paArrayHdr);
		delete m_paArrayHdr;
		m_paArrayHdr = NULL;
		return;
		}
	Assert(m_paArrayHdr->cElementsAlloc >= m_paArrayHdr->cElements);
	if (m_paArrayHdr->cElementsAlloc > m_paArrayHdr->cElements)
		{
		int cElements = m_paArrayHdr->cElements;
		SHeaderWithData * paArrayHdr = S_PaAllocateElements(cElements);
		Assert(paArrayHdr!= NULL);
		// Copy the previous array into the new array
		memcpy(OUT paArrayHdr->rgpvData, IN m_paArrayHdr->rgpvData, cElements * sizeof(void *));
		paArrayHdr->cElements = cElements;
		ValidateHeapPointer(m_paArrayHdr);
		delete m_paArrayHdr;	// Delete previous array
		m_paArrayHdr = paArrayHdr;
		}
	} // FreeExtra()


/////////////////////////////////////////////////////////////////////
//	Add()
//
//	Add an element at the end of the array.
//
//	Return index at which the element was added.
//
int
CArray::Add(const void * pvElement)
	{
	Endorse(pvElement == NULL);
	if (m_paArrayHdr != NULL)
		{
		int iElementAdded = m_paArrayHdr->cElements;	// TODO: Optimize this
		if (m_paArrayHdr->cElements < m_paArrayHdr->cElementsAlloc)
			{
			m_paArrayHdr->rgpvData[m_paArrayHdr->cElements++] = pvElement;
			}
		else
			{
			// We need to allocate more memory
			Assert(m_paArrayHdr->cElementsAlloc >= 0);
			Assert(m_paArrayHdr->cElements == m_paArrayHdr->cElementsAlloc);
			SHeaderWithData * paArrayHdr = S_PaAllocateElements((m_paArrayHdr->cElementsAlloc + 2) * 2);
			Assert(paArrayHdr!= NULL);
			// Copy the previous array into the new array
			memcpy(OUT paArrayHdr->rgpvData, IN m_paArrayHdr->rgpvData, m_paArrayHdr->cElements * sizeof(void *));
			paArrayHdr->cElements = m_paArrayHdr->cElements;
			paArrayHdr->rgpvData[paArrayHdr->cElements++] = pvElement;
			ValidateHeapPointer(m_paArrayHdr);
			delete m_paArrayHdr;	// Delete previous array
			m_paArrayHdr = paArrayHdr;
			}
		return iElementAdded;
		}
	m_paArrayHdr = (SHeaderWithData *)new BYTE[sizeof(SHeaderOnly) + (sizeof(void *) * 4)];	// Allocate 4 elements (to improve speed)
	m_paArrayHdr->cElementsAlloc = 4;
	m_paArrayHdr->cElements = 1;
	m_paArrayHdr->rgpvData[0] = pvElement;
	ValidateHeapPointer(m_paArrayHdr);
	return 0;
	} // Add()

//	Add an element in the array by replacing the first NULL pointer by pvElement.
//	If there are no NULL pointers, then pvElement is added to the end of the array.
//	Return the index at which the element was inserted/added.
//
//	USAGE
//	This method is used when adding a new object to an array which may contains NULL pointers, where
//	the NULL pointers were the result of previously deleted elements.
//	
int
CArray::AddReplaceNULL(const void * pvElement)
	{
	Endorse(pvElement == NULL);
	if (m_paArrayHdr != NULL)
		{
		// Search for any NULL pointer
		const void ** ppvStart = m_paArrayHdr->rgpvData;
		const void ** ppvEnd = ppvStart + m_paArrayHdr->cElements;
		const void ** ppv = ppvStart;
		while (ppv != ppvEnd)
			{
			if (*ppv == NULL)
				{
				Assert(PvGetElementAt(ppv - ppvStart) == NULL);
				*ppv = pvElement;
				return (ppv - ppvStart);
				}
			ppv++;
			} // while
		} // if
	return Add(pvElement);
	} // AddReplaceNULL()

//	Add an element at the beginning of the array (list).
//	If the number of elements (length of the array) exceeds cElementsMax, then
//	the end of the array is truncated to contain a maximum of cElementsMax elements.
//	If the element was already in the array, then the element is moved at the beginning of the array.
void
CArray::AddMRU(const void * pvElement, int cElementsMax)
	{
	Assert(cElementsMax > 1);
	if (m_paArrayHdr != NULL && m_paArrayHdr->cElements > 0)
		{
		if (m_paArrayHdr->rgpvData[0] == pvElement)
			return;	// The element is already at the beginning of the array
		}
	(void)RemoveElementI(pvElement);	// Remove the element (if present)
	InsertElementAtHead(pvElement);
	Assert(m_paArrayHdr != NULL);
	if (m_paArrayHdr->cElements > cElementsMax)
		m_paArrayHdr->cElements = cElementsMax;
	}

/////////////////////////////////////////////////////////////////////
//	Add the element at the end of the array only if the element was not found in the array.
//
//	Return TRUE if the element was added to the array.
//	Return FALSE if the element was already in the array.
//
//	TODO: Search from the end to the start, since a duplicate is more likely to be found at the end
//
//	SEE ALSO
//	AppendUnique() to append an array of elements
BOOL
CArray::AddUniqueF(const void * pvElement)
	{
	if (m_paArrayHdr != NULL)
		{
		// Search if the element is not already there
		const void ** ppvStart = m_paArrayHdr->rgpvData;
		const void ** ppvEnd = ppvStart + m_paArrayHdr->cElements;
		while (ppvStart != ppvEnd)
			{
			if (*ppvStart == pvElement)
				{
				Assert(PvGetElementAt(ppvStart - m_paArrayHdr->rgpvData) == pvElement);
				return FALSE;
				}
			ppvStart++;
			} // while
		}
	(void)Add(pvElement);
	return TRUE;
	} // AddUniqueF()

//	Empty the array and add a single element.  After calling this method, the size of the array is always 1 element.
//
//	This method is the combination of RemoveAll() and Add().
VOID
CArray::RemoveAllAndAdd(const void * pvElement)
	{
	if (m_paArrayHdr != NULL && m_paArrayHdr->cElementsAlloc > 0)
		{
		m_paArrayHdr->rgpvData[0] = pvElement;
		m_paArrayHdr->cElements = 1;
		}
	else
		{
		// The array was empty
		Assert(GetSize() == 0);
		Add(pvElement);
		}
	Assert(GetSize() == 1);
	}

/////////////////////////////////////////////////////////////////////
//	InsertElementAtHeadFast()
//
//	Insert the element at the head of the array without
//	preserving the order.  In other words, the first and last
//	elements are swapped.
//
//	USAGE
//	Adding the element at the head of the array can reduce the time
//	to access thee element in the future.  Use this routine if
//	the elements in the array do not need to be sorted and the
//	last inserted element is likely to be accessed in the near future.
//
void
CArray::InsertElementAtHeadFast(const void * pvElement)
	{
	if (m_paArrayHdr == NULL || m_paArrayHdr->cElements == 0)
		{
		Add(pvElement);
		}
	else
		{
		// Swap the first and last elements
		Add(m_paArrayHdr->rgpvData[0]);	// Move the first element to the end
		m_paArrayHdr->rgpvData[0] = pvElement;
		}
	} // InsertElementAtHeadFast()

/////////////////////////////////////////////////////////////////////
//	InsertElementAt()
//
//	Insert an element at a given position
//
//	If the position is -1, then the element is appended at the end of the array.
//
void
CArray::InsertElementAt(int iElement, const void * pvElement)
	{
	if (m_paArrayHdr == NULL)
		{
		Assert(iElement == 0 || iElement == -1);
		m_paArrayHdr = (SHeaderWithData *)new BYTE[sizeof(SHeaderOnly) + sizeof(void *)];
		m_paArrayHdr->cElements = m_paArrayHdr->cElementsAlloc = 1;
		m_paArrayHdr->rgpvData[0] = pvElement;
		ValidateHeapPointer(m_paArrayHdr);
		return;
		}
	if (m_paArrayHdr->cElements >= m_paArrayHdr->cElementsAlloc)
		{
		Assert(m_paArrayHdr->cElementsAlloc > 0);
		Assert(m_paArrayHdr->cElements == m_paArrayHdr->cElementsAlloc);
		// We need to reallocate memory
		SHeaderWithData * paArrayHdr = S_PaAllocateElements((m_paArrayHdr->cElementsAlloc + 1) * 2);
		paArrayHdr->cElements = m_paArrayHdr->cElements;
		memcpy(OUT paArrayHdr->rgpvData, IN m_paArrayHdr->rgpvData, m_paArrayHdr->cElements * sizeof(void *));
		ValidateHeapPointer(m_paArrayHdr);
		delete m_paArrayHdr;
		m_paArrayHdr = paArrayHdr;
		}
	Assert(m_paArrayHdr != NULL);
	if (iElement < 0)
		{
		Assert(iElement == -1);	// Append element to end of list
		iElement = m_paArrayHdr->cElements;
		}
	Assert(iElement >= 0);
	if (iElement < m_paArrayHdr->cElements)
		{
		// We want to insert the element in the middle, so we need to move the rest of the array
		int cElementsMove = m_paArrayHdr->cElements - iElement;		// Number of elements to move
		Assert(cElementsMove > 0);
		const void ** ppv = &m_paArrayHdr->rgpvData[iElement];
		memmove(OUT ppv + 1, IN ppv, cElementsMove * sizeof(void *));
		}
	// Store the element at its desired location
	Assert(iElement <= m_paArrayHdr->cElements);
	Assert(m_paArrayHdr->cElements < m_paArrayHdr->cElementsAlloc);
	m_paArrayHdr->rgpvData[iElement] = pvElement;
	m_paArrayHdr->cElements++;
	} // InsertElementAt()

void
CArray::InsertElementAtHead(const void * pvElement)
	{
	InsertElementAt(0, pvElement);
	}

/////////////////////////////////////////////////////////////////////
//	InsertElementsAtHead()
//
//	Insert elements at the head of the array.
//	Each inserted element is initialized with zeroes.
//
void
CArray::InsertElementsAtHead(int cElementsInsert)
	{
	Assert(cElementsInsert >= 0);
	if (m_paArrayHdr == NULL)
		{
		SetSizeGrow(cElementsInsert, TRUE);
		return;
		}
	const int cElements = m_paArrayHdr->cElements;
	int cElementsAlloc = cElements + cElementsInsert;
	if (cElementsAlloc > m_paArrayHdr->cElementsAlloc)
		{
		SHeaderWithData * paArrayHdr = S_PaAllocateElements((cElementsAlloc + 1) * 2);
		memcpy(OUT paArrayHdr->rgpvData + cElementsInsert, IN m_paArrayHdr->rgpvData, cElements * sizeof(void *));
		ValidateHeapPointer(m_paArrayHdr);
		delete m_paArrayHdr;
		m_paArrayHdr = paArrayHdr;
		}
	else
		{
		memmove(OUT m_paArrayHdr->rgpvData + cElementsInsert, IN m_paArrayHdr->rgpvData, cElements * sizeof(void *));
		}
	m_paArrayHdr->cElements = cElementsAlloc;
	InitToZeroes(OUT m_paArrayHdr->rgpvData, cElementsInsert * sizeof(void *));
	} // InsertElementsAtHead()


//	Very similar to SetAtGrow(), but don't grow if pvElement is NULL.
void
CArray::SetAtGrowOptional(int iElement, const void * pvElement)
	{
	if (pvElement == NULL)
		{
		if (m_paArrayHdr == NULL || iElement >= m_paArrayHdr->cElements)
			return;
		}
	SetAtGrow(iElement, pvElement);
	}

/////////////////////////////////////////////////////////////////////
//	SetAtGrow()
//
//	Sets the element for a given index; grows the array if necessary.
//
//	Initialize elements to zeroes when allocating new elements.
//
//	PERFORMANCE NOTES
//	The array may grow well above the index.
//	Using GetSize() may not return the value of the largest
//	index but how many elements the array can hold.
//	It is the caller's responsibility to keep track of the upper
//	bound of the array to determine the correct number of elements
//	in the array.
//
void
CArray::SetAtGrow(int iElement, const void * pvElement)
	{
	Endorse(pvElement == NULL);
	if (m_paArrayHdr == NULL || iElement >= m_paArrayHdr->cElements)
		SetSizeGrow(iElement + 1, TRUE);
	Assert(m_paArrayHdr != NULL);
	Assert(iElement >= 0 && iElement < m_paArrayHdr->cElements);
	m_paArrayHdr->rgpvData[iElement] = pvElement;
	} // SetAtGrow()

/////////////////////////////////////////////////////////////////////
//	RemoveElementAt()
//
//	Remove an element at a given index.
//
void
CArray::RemoveElementAt(int iElement)
	{
	Assert(m_paArrayHdr != NULL);
	Assert(iElement >= 0 && iElement < m_paArrayHdr->cElements);
	Assert(m_paArrayHdr->cElements> 0);
	m_paArrayHdr->cElements--;
	int cElementsMove = m_paArrayHdr->cElements - iElement;		// Number of elements to move
	if (cElementsMove > 0)
		{
		const void ** ppv = &m_paArrayHdr->rgpvData[iElement];
		memmove(OUT ppv, IN ppv + 1, cElementsMove * sizeof(void *));
		}
	}

void
CArray::RemoveElementAtPpv(const void ** ppvElement)
	{
	RemoveElementAt(ppvElement - (const void **)m_paArrayHdr->rgpvData);
	}

/////////////////////////////////////////////////////////////////////
//	Remove an element at a given index.
//
//	The element to be removed is substituted by the element at the end of the array.
//
void
CArray::RemoveElementAtFast(int iElement)
	{
	Assert(m_paArrayHdr != NULL);
	Assert(iElement >= 0 && iElement < m_paArrayHdr->cElements);
	Assert(m_paArrayHdr->cElements > 0);
	int cElements = m_paArrayHdr->cElements - 1;
	if (iElement < cElements)
		m_paArrayHdr->rgpvData[iElement] = m_paArrayHdr->rgpvData[cElements];
	m_paArrayHdr->cElements = cElements;	// Shrink the array by one
	}

void
CArray::RemoveElementAtPpvFast(const void ** ppvElement)
	{
	RemoveElementAtFast(ppvElement - (const void **)m_paArrayHdr->rgpvData);
	}

PVOID
CArray::RemoveElementFirstPv()
	{
	Assert(m_paArrayHdr != NULL);
	Assert(m_paArrayHdr->cElements > 0);
	void * pvElementRemoved = const_cast<void *>(m_paArrayHdr->rgpvData[0]);
	RemoveElementAt(0);	// Remove the first element
	return pvElementRemoved;
	}

//	Transfer an element from one array to another.
//	If the element is already into the destination array, then do nothing
void
CArray::ElementTransferFrom(const void * pvElementTransfer, INOUT CArray * pArraySrc)
	{
	int iElementRemoved = pArraySrc->RemoveElementI(pvElementTransfer);	// Try to remove the element from the array
	if (iElementRemoved >= 0)
		Add(pvElementTransfer);	// Add the element to the array only if it was removed from the other array
	Assert(FindElementF(pvElementTransfer));
	}

//	Method very similar to Append() however the content of the array is
//	inserted at the beginning of the array rather than appended to the end of the array.
VOID
CArray::InsertAtHead(const CArray * pArrayInsert)
	{
	Assert(pArrayInsert != NULL);
	Assert(pArrayInsert != this);
	int cElementsInsert = pArrayInsert->GetSize();
	if (cElementsInsert <= 0)
		return;	// Nothing to insert
	int cElements = GetSize();
	if (cElements == 0)
		{
		Copy(IN pArrayInsert);
		return;
		}
	int cElementsAlloc = cElementsInsert + cElements;
	if (cElementsAlloc > m_paArrayHdr->cElementsAlloc)
		{
		SHeaderWithData * paArrayHdr = S_PaAllocateElements(cElementsAlloc * 2);
		memcpy(OUT paArrayHdr->rgpvData + cElementsInsert, IN m_paArrayHdr->rgpvData, cElements * sizeof(void *));
		ValidateHeapPointer(m_paArrayHdr);
		delete m_paArrayHdr;
		m_paArrayHdr = paArrayHdr;
		}
	else
		{
		// Make some room at the head of the array for the new elements to inset
		memmove(OUT m_paArrayHdr->rgpvData + cElementsInsert, IN m_paArrayHdr->rgpvData, cElements * sizeof(void *));
		}
	m_paArrayHdr->cElements = cElementsAlloc;
	memcpy(OUT m_paArrayHdr->rgpvData, pArrayInsert->m_paArrayHdr->rgpvData, cElementsInsert * sizeof(void *));
	} // InsertAtHead()

/////////////////////////////////////////////////////////////////////
//	Append()
//
//	Add the contents of another array to the end of the given array.
//
//	If necessary, Append() may allocate extra memory to accommodate
//	the elements appended to the array.
//
void
CArray::Append(const CArray * pArrayAppend)
	{
	Assert(pArrayAppend != NULL);
	Assert(pArrayAppend != this);
	int cElementsAppend = pArrayAppend->GetSize();
	if (cElementsAppend <= 0)
		return;	// Nothing to append
	Assert(cElementsAppend <= pArrayAppend->m_paArrayHdr->cElementsAlloc);
	int cElements = GetSize();
	if (cElements == 0)
		{
		Copy(IN pArrayAppend);
		return;
		}
	int cElementsAlloc = cElements + cElementsAppend;
	if (cElementsAlloc > m_paArrayHdr->cElementsAlloc)
		cElementsAlloc *= 2;
	SetSizeGrow(cElementsAlloc, FALSE);
	Assert(GetSize() == cElementsAlloc);
	Assert(m_paArrayHdr != NULL);
	memcpy(OUT &m_paArrayHdr->rgpvData[cElements], &pArrayAppend->m_paArrayHdr->rgpvData[0], cElementsAppend * sizeof(void *));
	m_paArrayHdr->cElements = cElements + cElementsAppend;
	Assert(m_paArrayHdr->cElements <= m_paArrayHdr->cElementsAlloc);
	} // Append()

//	Append only the unique elements from pArraySrc.
//	If an element in pArraySrc is already in the current array, just skip it.
//	This method is not optimized and can be very slow.  Use carefully.
void
CArray::AppendUnique(const CArray * pArrayAppend)
	{
	Assert(pArrayAppend != NULL);
	int cElementsAppend = pArrayAppend->GetSize();
	for (int iElementAppend = 0; iElementAppend < cElementsAppend; iElementAppend++)
		(void)AddUniqueF(pArrayAppend->m_paArrayHdr->rgpvData[iElementAppend]);
	}

/////////////////////////////////////////////////////////////////////
//	Copy()
//
//	Deep copies another array to the array; grows the array if necessary.
//
//	If necessary, Copy() may allocate extra memory to accommodate the elements copied to the array.
//
void
CArray::Copy(const CArray * pArraySrc)
	{
	Assert(pArraySrc != NULL);
	Assert(pArraySrc != this);
	ValidateHeapPointer(m_paArrayHdr);
	ValidateHeapPointer(pArraySrc->m_paArrayHdr);

	int cElementsSrc = pArraySrc->GetSize();
	if (cElementsSrc > 0)
		{
		PrgpvAllocateElementsEmpty(cElementsSrc);
		Assert(GetSize() == 0);
		Assert(m_paArrayHdr->rgpvData != pArraySrc->m_paArrayHdr->rgpvData);	
		memcpy(OUT m_paArrayHdr->rgpvData, pArraySrc->m_paArrayHdr->rgpvData, cElementsSrc * sizeof(void *));
		m_paArrayHdr->cElements = cElementsSrc;
		}
	else
		{
		if (m_paArrayHdr != NULL)
			m_paArrayHdr->cElements = 0;
		}
	Assert(GetSize() == pArraySrc->GetSize());
	ValidateHeapPointer(m_paArrayHdr);
	} // Copy()

/////////////////////////////////////////////////////////////////////
//	CopyStealFromSource()
//
//	Get the content of another array by stealing it.
//	This method has the illusion of copying the array but
//	empty the source array.
//
//	USAGE
//	Utilize this method if the source array is a temporary object
//	to be saved.
//
void
CArray::CopyStealFromSource(INOUT CArray * pArraySrc)
	{
	Assert(pArraySrc != NULL);
	Assert(pArraySrc != this);
	ValidateHeapPointer(m_paArrayHdr);
	if (m_paArrayHdr != NULL)
		delete m_paArrayHdr;
	m_paArrayHdr = pArraySrc->m_paArrayHdr;
	pArraySrc->m_paArrayHdr = NULL;
	}

void
CArray::AppendStealFromSource(INOUT CArray * pArraySrc)
	{
	Assert(pArraySrc != NULL);
	Assert(pArraySrc != this);
	ValidateHeapPointer(m_paArrayHdr);
	if (m_paArrayHdr == NULL)
		{
		// Our array is empty, so we can just steal the entire array from the source
		CopyStealFromSource(INOUT pArraySrc);
		return;
		}
	SHeaderWithData * pArrayHdrSource = pArraySrc->m_paArrayHdr;
	if (pArrayHdrSource != NULL)
		{
		Append(pArraySrc);	// Copy each pointer
		pArrayHdrSource->cElements = 0;
		}
	}

//	Swap the content of 'this' and pArraySrc
VOID
CArray::Swap(INOUT CArray * pArraySrc)
	{
	Assert(pArraySrc != NULL);
	Assert(pArraySrc != this);
	SHeaderWithData * paArrayHdrTemp = m_paArrayHdr;
	m_paArrayHdr = pArraySrc->m_paArrayHdr;
	pArraySrc->m_paArrayHdr = paArrayHdrTemp;
	}


VOID
CArray::SetDataRaw(PCVOID prgpvData[], int cElements)
	{
	Endorse(prgpvData == NULL);
	Assert(cElements >= 0);

	if (cElements > 0)
		{
		Assert(m_paArrayHdr != NULL);	// Just in case, since the array should have not been flushed
		Assert(m_paArrayHdr->cElementsAlloc >= cElements);

		if (m_paArrayHdr == NULL || m_paArrayHdr->cElementsAlloc < cElements)
			PrgpvAllocateElementsEmpty(cElements);
		Assert(m_paArrayHdr != NULL);
		m_paArrayHdr->cElements = cElements;
		if (prgpvData != NULL)
			memcpy(OUT m_paArrayHdr->rgpvData, IN prgpvData, cElements * sizeof(PVOID));
		}
	else
		{
		if (m_paArrayHdr != NULL)
			m_paArrayHdr->cElements = 0;
		}
	} // SetDataRaw()

/////////////////////////////////////////////////////////////////////
//	Assignment operator.
//
//	INTERFACE NOTES
//	This operator is implemented to assert when copying an array using the assignment operator.
//	The motivation of this method is preventing an accidental copy the array
//	rather than assigning a reference to it.
//	In the future, we may remove this assertion and use the assignment operator for convenience.
//
void
CArray::operator = (const CArray & arraySrc)
	{
	Assert(&arraySrc != this);
	Assert(FALSE && "Use the Copy() method instead");
	Copy(IN &arraySrc);
	}

/////////////////////////////////////////////////////////////////////
//	Pop one element from the stack.
//	If the stack is empty, return NULL.
void *
CArray::PvStackPopElement()
	{
	if (m_paArrayHdr == NULL || m_paArrayHdr->cElements <= 0)
		return NULL;	// Stack is already empty
	return (void *)m_paArrayHdr->rgpvData[--m_paArrayHdr->cElements];
	}

/*
/////////////////////////////////////////////////////////////////////
//	Return the top element of the stack.
//	
//	An optional parameter may specify which element from the top
//	of the stack to return.
//
//	If the stack is empty or the value is out of range, return NULL.
//
void *
CArray::PvStackGetTopElement(int iElement) const
	{
	Assert(iElement >= 0);
	return PvGetElementAtSafe_YZ(GetSize() - 1 - iElement);
	}
*/

//	Return NULL if there is no next element, or if the array has zero or one element.
//	If the array contains multiple elements and pvElement is the last element, then return the first element.
void *
CArray::PFindNextElementFromCircularRing(const void * pvElement) const
	{
	if (m_paArrayHdr != NULL)
		{
		void ** ppvStart = const_cast<void **>(m_paArrayHdr->rgpvData);
		void ** ppvEnd = ppvStart + m_paArrayHdr->cElements;
		void ** ppv = ppvStart;
		while (ppv != ppvEnd)
			{
			if (*ppv++ == pvElement)
				{
				if (ppv != ppvEnd)
					return *ppv;
				void * pvFirst = *ppvStart;
				if (pvElement != pvFirst)
					return pvFirst;	// We have an array with more than two elements
				break;	// Only one element, so return NULL
				}
			} // while
		} // if
	return NULL;
	}

//	Return NULL if there is no previous element
void *
CArray::PFindPrevElementFromCircularRing(const void * pvElement) const
	{
	if (m_paArrayHdr != NULL)
		{
		void ** ppvStart = const_cast<void **>(m_paArrayHdr->rgpvData);
		void ** ppvEnd = ppvStart + m_paArrayHdr->cElements;
		void ** ppv = ppvStart;
		while (ppv != ppvEnd)
			{
			void * pvTemp = *ppv;
			if (pvTemp == pvElement)
				{
				void * pvFirst = *ppvStart;
				if (pvTemp == pvFirst)
					{
					pvTemp = *(ppvEnd - 1);	// We got the first element, so return the last one
					if (pvTemp != pvFirst)
						return pvTemp;
					break;	// Only one element, so return NULL
					}
				return *--ppv;	// Return the previous element
				}
			ppv++;
			} // while
		} // if
	return NULL;
	}

/////////////////////////////////////////////////////////////////////
//	FindElementI()
//
//	Find the first matching element in the array.
//
int
CArray::FindElementI(const void * pvElement) const
	{
	Endorse(pvElement == NULL);
	if (m_paArrayHdr == NULL)
		return -1;
	const void ** ppvStart = m_paArrayHdr->rgpvData;
	const void ** ppvEnd = ppvStart + m_paArrayHdr->cElements;
	while (ppvStart != ppvEnd)
		{
		if (*ppvStart == pvElement)
			{
			Assert(PvGetElementAt(ppvStart - m_paArrayHdr->rgpvData) == pvElement);
			return (ppvStart - m_paArrayHdr->rgpvData);
			}
		ppvStart++;
		}
	return -1;
	}

//	Search from the end of the array to compute the number of remaining elements
UINT
CArray::UFindRemainingElements(const void * pvElement) const
	{
	Endorse(pvElement == NULL);	// This is a rare case, however it is allowed
	if (m_paArrayHdr != NULL)
		{
		const void ** ppvFirst = m_paArrayHdr->rgpvData;
		const void ** ppvLast = ppvFirst + m_paArrayHdr->cElements - 1;
		const void ** ppv = ppvLast;
		while (ppv >= ppvFirst)
			{
			Assert(ppvLast - ppv >= 0);
			if (*ppv == pvElement)
				return ppvLast - ppv;
			ppv--;
			} // while
		}
	Assert(FALSE && "Element not found in array");
	return 0;
	}

/////////////////////////////////////////////////////////////////////
//	Replace an element by another one.
//
//	Return the index where the element was replaced, otherwise return -1.
//
int
CArray::ReplaceElementI(const void * pvElementOld, const void * pvElementNew)
	{
	int iElement = FindElementI(pvElementOld);
	if (iElement >= 0)
		SetAt(iElement, pvElementNew);
	return iElement;
	}

/////////////////////////////////////////////////////////////////////
//	RemoveElementI()
//
//	Remove the first element from the array without deleting
//	the actual object.
//
//	Return the index where the element has been removed.
//	Return -1 if no element was removed.
//
int
CArray::RemoveElementI(const void * pvElement)
	{
	Endorse(pvElement == NULL);
	if (m_paArrayHdr != NULL)
		{
		const void ** ppvStart = m_paArrayHdr->rgpvData;
		const void ** ppvEnd = ppvStart + m_paArrayHdr->cElements;
		while (ppvStart != ppvEnd)
			{
			if (*ppvStart == pvElement)
				{
				int iElementRemove = (ppvStart - m_paArrayHdr->rgpvData);
				Assert(PvGetElementAt(iElementRemove) == pvElement);
				RemoveElementAt(iElementRemove);
				return iElementRemove;
				}
			ppvStart++;
			}
		}
	return -1;
	} // RemoveElementI()

//	Similar as RemoveElementI(), except the removed element is substituted by the element at the end of the array.
//	Return TRUE if the element was removed, otherwise return FALSE.
BOOL
CArray::RemoveElementFastF(const void * pvElement)
	{
	Endorse(pvElement == NULL);
	if (m_paArrayHdr != NULL)
		{
		const void ** ppvStart = m_paArrayHdr->rgpvData;
		const void ** ppvEnd = ppvStart + m_paArrayHdr->cElements;
		while (ppvStart != ppvEnd)
			{
			if (*ppvStart == pvElement)
				{
				Assert(PvGetElementAt(ppvStart - m_paArrayHdr->rgpvData) == pvElement);
				//RemoveElementAtFast(ppvStart - m_paArrayHdr->rgpvData);
				RemoveElementAtPpvFast(ppvStart);
				return TRUE;
				}
			ppvStart++;
			}
		}
	return FALSE;
	} // RemoveElementFastF()


/////////////////////////////////////////////////////////////////////
//	RemoveElementAllInst()
//
//	Remove all instances of the element from the array.
//
//	Return the number of instances removed.
//
//	TODO: Optimize speed by removing counters and use pStop
int
CArray::RemoveElementAllInst(const void * pvElement)
	{
	Endorse(pvElement == NULL);
	int cInstancesRemoved = 0;
	int cElementCount = GetSize();
	void ** ppvDataDst = PrgpvGetElements();
	void ** ppvDataSrc = ppvDataDst;
	for (int i = 0; i < cElementCount; i++)
		{
		void * pvDataSrc = *ppvDataSrc++;
		if (pvDataSrc != pvElement)
			{
			*ppvDataDst++ = pvDataSrc;
			}
		else
			{
			cInstancesRemoved++;
			}
		} // for
	if (cInstancesRemoved > 0)
		{
		m_paArrayHdr->cElements = cElementCount - cInstancesRemoved;
		}
	return cInstancesRemoved;
	} // RemoveElementAllInst()

//	Remove any NULL value at the end of the array.
//	Return the number of NULLs removed.
UINT
CArray::RemoveTailingNULLsU()
	{
	if (m_paArrayHdr != NULL)
		{
		const int cElements = m_paArrayHdr->cElements;
		int iElement = cElements - 1;
		while (iElement >= 0)
			{
			if (m_paArrayHdr->rgpvData[iElement] != NULL)
				break;
			iElement--;
			}
		iElement++;
		m_paArrayHdr->cElements = iElement;
		return cElements - iElement;
		}
	return 0;
	}

//	Return TRUE if all the elements in the array are NULL, or if the array is empty.
//
//	USAGE
//	This method is used to determine if there is something in the array.
//
BOOL
CArray::FAllElementsNULL() const
	{
	if (m_paArrayHdr != NULL)
		{
		int cElements = m_paArrayHdr->cElements;
		while (cElements-- > 0)
			{
			if (m_paArrayHdr->rgpvData[cElements] != NULL)
				return FALSE;
			}
		}
	return TRUE;
	} // FAllElementsNULL()

//	Return TRUE if there is a non-NULL pointer from the list of indices.
//
//	USAGE
//	This method is useful to determine if there is some specific data in the array.
//
//	EXAMPLE
//	BOOL fDivorce = m_arrayp.FHasNonNullPointer(istrDivorcePlace, istrDivorceSource, -1);
//
BOOL
CArray::FHasNonNullPointer(int iElement, ...) const
	{
	Assert(iElement >= 0);
	if (m_paArrayHdr != NULL)
		{
		const int cElements = m_paArrayHdr->cElements;
		PCVOID * prgpvData = m_paArrayHdr->rgpvData;
		if (iElement < cElements && prgpvData[iElement] != NULL)
			return TRUE;
		va_list vlArgs;
		va_start(OUT vlArgs, iElement);
		while (TRUE)
			{
			iElement = va_arg(vlArgs, int);
			if (iElement < 0)
				break;
			if (iElement < cElements && prgpvData[iElement] != NULL)
				return TRUE;
			} // while
		} // if
	return FALSE;
	} // FHasNonNullPointer()

//	Return the number of non-NULL pointers in the array
UINT
CArray::CountNonNullPointers() const
	{
	UINT cNonNullPointers = 0;
	if (m_paArrayHdr != NULL)
		{
		const void ** ppvStart = m_paArrayHdr->rgpvData;
		const void ** ppvEnd = ppvStart + m_paArrayHdr->cElements;
		while (ppvStart != ppvEnd)
			{
			if (*ppvStart != NULL)
				cNonNullPointers++;
			ppvStart++;
			}
		}
	return cNonNullPointers;
	} // CountNonNullPointers()

/////////////////////////////////////////////////////////////////////
//	FIsContentIdentical()
//
//	Compare if two arrays have same content.
//	The function returns TRUE ONLY IF both array have identical content.
//	
BOOL
CArray::FIsContentIdentical(const CArray * pArrayCompare) const
	{
	Assert(pArrayCompare != NULL);
	Endorse(pArrayCompare == this);		// Will always return TRUE
	int cElements = GetSize();			// Number of elements in array
	int cElementsCompare = pArrayCompare->GetSize();
	if (cElements == 0 && cElementsCompare == 0)
		return TRUE;		// Two empty arrays are considered identical
	if (cElementsCompare != cElements)
		return FALSE;
	/*
	const void ** rgpvElements = PrgpvGetElements();		// Array of pointers to elements
	Assert(rgpvElements != NULL);
	const void ** rgpvElementsCompare = pArrayCompare->PrgpvGetElements();
	Assert(rgpvElementsCompare != NULL);
	return (0 == memcmp(IN rgpvElements, IN rgpvElementsCompare, cElements * sizeof(void *)));
	*/
	return (0 == memcmp(IN m_paArrayHdr->rgpvData, IN pArrayCompare->m_paArrayHdr->rgpvData, cElements * sizeof(void *)));
	}

/////////////////////////////////////////////////////////////////////
//	Reverse()
//
//	Reverse the content of the array where the first element becomes the last
//	and vice versa.
//
void
CArray::Reverse(int iElementFirst, int cElementsReverse)
	{
	Assert(iElementFirst >= 0);
	Assert(iElementFirst == 0);
	Assert(cElementsReverse >= -1 && cElementsReverse <= GetSize());
	if (m_paArrayHdr == NULL)
		return;	// Nothing to do
	if (cElementsReverse == -1)
		cElementsReverse = m_paArrayHdr->cElements;
	if (cElementsReverse <= 1)
		return;	// Array too small
	const void ** ppvFirst = m_paArrayHdr->rgpvData;
	const void ** ppvLast = ppvFirst + cElementsReverse;
	cElementsReverse /= 2;
	while (cElementsReverse > 0)
		{
		const void * pvTemp = *ppvFirst;
		*ppvFirst++ = *--ppvLast;
		*ppvLast = pvTemp;
		cElementsReverse--;
		}
	}


/////////////////////////////////////////////////////////////////////
//	ShiftElementBy()
//
//	Move iElement by diShift.
//
//	If diShift < 0, then shift iElement towards the beginning of the array.
//	If diShift > 0, then shift iElement towards the end of the array.
//
//	Return -1 if no shifting occurred (out of range, or no-op),
//	otherwise the index of the shifted iElement.
//
//	REMARKS
//	If diShift causes the new position to be out of range, the method will
//	adjust the position to the boundary.
int
CArray::ShiftElementBy(int iElement, int diShift)
	{
	Assert(diShift != 0);	// This can be an Endorse()
	if (m_paArrayHdr == NULL)
		return -1;	// No element to shift
	int cElements = m_paArrayHdr->cElements;
	if (iElement < 0 || iElement >= cElements)
		return -1;	// The element is out of range, and cannot be shifted
	Assert(cElements > 0);
	int iElementShifted = iElement + diShift;
	// Adjust to the boundaries
	if (iElementShifted >= cElements)
		iElementShifted = cElements - 1;
	if (iElementShifted < 0)
		iElementShifted = 0;
	if (iElementShifted != iElement)
		{
		PCVOID * prgpvData = m_paArrayHdr->rgpvData;
		PCVOID * ppvElement = prgpvData + iElement;
		PCVOID * ppvElementShifted = prgpvData + iElementShifted;
		PCVOID pvElement = *ppvElement;
		if (iElementShifted < iElement)
			{
			// Shift the element towards the beginning of the array,
			// therefore shift the content of the array to the end by one position
			while (ppvElement != ppvElementShifted)
				{
				ppvElement[0] = ppvElement[-1];
				ppvElement--;
				}
			}
		else
			{
			// Shift towards the end of the array
			while (ppvElement != ppvElementShifted)
				{
				ppvElement[0] = ppvElement[+1];
				ppvElement++;
				}
			}
		*ppvElement = pvElement;
		return iElementShifted;
		}
	return -1;
	} // ShiftElementBy()


/////////////////////////////////////////////////////////////////////
//	Generic routine to determine if an array is sorted ascending, descending or both.
ESortOrder
CArray::EGetSortedOrder(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare) const
	{
	if (m_paArrayHdr == NULL)
		return eSortOrder_kmBoth;	// An empty array is always sorted both ways
	return ::Sort_EGetSortedOrder(IN m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, pfnCompareSort, lParamCompare);
	}

/////////////////////////////////////////////////////////////////////
//	Return TRUE if the array is sorted in ascending order
//
BOOL
CArray::FIsSortedAscending(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare) const
	{
	if (m_paArrayHdr == NULL)
		return TRUE;	// An empty array is always sorted
	return ::Sort_FIsSortedAscending(IN m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, pfnCompareSort, lParamCompare);
	}


/////////////////////////////////////////////////////////////////////
//	Return TRUE if the array is sorted in descending order
//
BOOL
CArray::FIsSortedDescending(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare) const
	{
	if (m_paArrayHdr == NULL)
		return TRUE;	// An empty array is always sorted
	return ::Sort_FIsSortedDescending(IN m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, pfnCompareSort, lParamCompare);
	}


/////////////////////////////////////////////////////////////////////
//	DoSortAscending()
//
//	Sort the array according to the comparison routine.
//
void
CArray::DoSortAscending(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare)
	{
	if (m_paArrayHdr == NULL)
		return;	// Nothing to do
	::Sort_DoSorting(INOUT m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, TRUE, pfnCompareSort, lParamCompare);
	}


/////////////////////////////////////////////////////////////////////
//	DoSortDescending()
//
void
CArray::DoSortDescending(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare)
	{
	if (m_paArrayHdr == NULL)
		return;	// Nothing to do
	::Sort_DoSorting(INOUT m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, FALSE, pfnCompareSort, lParamCompare);
	}


/////////////////////////////////////////////////////////////////////
//	Routine to sort either ascending or descending.
void
CArray::DoSort(BOOL fSortAscending, PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare)
	{
	if (m_paArrayHdr == NULL)
		return;	// Nothing to do
	::Sort_DoSorting(INOUT m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, fSortAscending, pfnCompareSort, lParamCompare);
	}

/////////////////////////////////////////////////////////////////////
//	Routine to sort either ascending or descending.
//	If the array is already sorted, then no sorting is done.
void
CArray::Sort(BOOL fSortAscending, PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare)
	{
	if (m_paArrayHdr == NULL)
		return;	// Nothing to do
	// Determine how the array is sorted
	ESortOrder eSortOrder = ::Sort_EGetSortedOrder(IN m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, pfnCompareSort, lParamCompare);
	if (fSortAscending)
		{
		if (eSortOrder & eSortOrder_kfAscending)
			return;	// Array is already sorted
		}
	else
		{
		if (eSortOrder & eSortOrder_kfDescending)
			return;	// Array is already sorted
		}
	if (eSortOrder == eSortOrder_kzNone)
		{
		// We need to sort the whole array
		::Sort_DoSorting(INOUT m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, fSortAscending, pfnCompareSort, lParamCompare);
		}
	else
		{
		// Array is already sorted, but in the opposite order
		::Sort_DoReverseIdenticalElements(m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, pfnCompareSort, lParamCompare);
		Reverse(); // Reverse the order of each element
		}
	#ifdef DEBUG
	if (fSortAscending)
		Assert(FIsSortedAscending(pfnCompareSort, lParamCompare));
	else
		Assert(FIsSortedDescending(pfnCompareSort, lParamCompare));
	#endif
	} // Sort()

/////////////////////////////////////////////////////////////////////
//	Sort the array in the ascending order
void
CArray::Sort(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare)
	{
	if (m_paArrayHdr == NULL)
		return;	// Nothing to do
	// Determine how the array is sorted
	ESortOrder eSortOrder = ::Sort_EGetSortedOrder(IN m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, pfnCompareSort, lParamCompare);
	if (eSortOrder & eSortOrder_kfAscending)
		return;	// Array is already sorted
	if (eSortOrder == eSortOrder_kzNone)
		{
		// We need to sort the whole array
		::Sort_DoSorting(INOUT m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, TRUE, pfnCompareSort, lParamCompare);
		}
	else
		{
		// Array is already sorted, but in the opposite order
		::Sort_DoReverseIdenticalElements(m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, pfnCompareSort, lParamCompare);
		Reverse(); // Reverse the order of each element
		Assert(FIsSortedAscending(pfnCompareSort, lParamCompare));
		}
	}

//	Similar to Sort() but never call Reverse() if the array is sorted in opposite order
//	This method is used to preserve the order of the objects having identical keys.
void
CArray::SortNeverReverse(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare)
	{
	if (m_paArrayHdr == NULL)
		return;	// Nothing to do
	// Determine how the array is already sorted
	ESortOrder eSortOrder = ::Sort_EGetSortedOrder(IN m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, pfnCompareSort, lParamCompare);
	if (eSortOrder & eSortOrder_kfAscending)
		return;	// Array is already sorted
	// We need to sort the whole array
	::Sort_DoSorting(INOUT m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, TRUE, pfnCompareSort, lParamCompare);
	Assert(FIsSortedAscending(pfnCompareSort, lParamCompare));
	}

/////////////////////////////////////////////////////////////////////
//	Toggle the sorting of the array.
//
//	- If the array is sorted ascending, then sort descending.
//	- If the array is sorted descending, then sort ascending.
//	- If the array is not sorted, then sort ascending.
//
void
CArray::ToggleSort(PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare)
	{
	if (m_paArrayHdr == NULL)
		return;	// Nothing to do
	// Determine how the array is sorted
	ESortOrder eSortOrder = ::Sort_EGetSortedOrder(IN m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, pfnCompareSort, lParamCompare);
	if (eSortOrder == eSortOrder_kzNone)
		{
		// Sort the array in ascending order
		::Sort_DoSorting(INOUT m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, TRUE, pfnCompareSort, lParamCompare);
		}
	else
		{
		::Sort_DoReverseIdenticalElements(m_paArrayHdr->rgpvData, m_paArrayHdr->cElements, pfnCompareSort, lParamCompare);
		Reverse(); // Reverse the order of each element
		}
	}

/////////////////////////////////////////////////////////////////////
//	Merge a single element to the array by using the comparison
//	routine to determine its position.
//
void
CArray::MergeAppend(const void * pvElement, PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare)
	{
	Endorse(pvElement == NULL);
	Assert(pfnCompareSort != NULL);
	if (m_paArrayHdr != NULL)
		{
		int cElementCount = m_paArrayHdr->cElements;
		PCVOID * ppvSrc = m_paArrayHdr->rgpvData;
		for (int i = 0; i < cElementCount; i++, ppvSrc++)
			{
			//	Insert at the end of the elements
			if (pfnCompareSort(pvElement, *ppvSrc, lParamCompare) > 0)
				{
				InsertElementAt(i, pvElement);
				return;
				}
			}
		}
	Add(pvElement);
	Assert(FIsSortedAscending(pfnCompareSort, lParamCompare));
	}

/////////////////////////////////////////////////////////////////////
//	MergeAppend()
//
//	Merge two sorted arrays. The method uses the comparison routine to do the merging.
//
//	INTERFANCE NOTES
//	-  Both arrays must be sorted before the merge.
//	-  The appended element will always be appended to the end of the array if the comparison routine considers them equals.
//
void
CArray::MergeAppend(const CArray * pArrayAppend, PFn_NCompareSortElements pfnCompareSort, LPARAM lParamCompare)
	{
	Assert(pArrayAppend != NULL);
	Assert(pfnCompareSort != NULL);
	Assert(pArrayAppend->FIsSortedAscending(pfnCompareSort, lParamCompare));
	Assert(FIsSortedAscending(pfnCompareSort, lParamCompare));

	int cElementsAppend = pArrayAppend->GetSize();
	if (cElementsAppend == 0)
		return;	// Nothing to do
	int cElements = GetSize();
	if (cElements == 0)
		{
		Copy(IN pArrayAppend);
		return;
		}
	int cElementsDst = cElements + cElementsAppend;	// Number of destination elements
	int cElementsAlloc = cElementsDst;
	if (cElementsAlloc > m_paArrayHdr->cElementsAlloc)
		cElementsAlloc *= 2;

	// Allocate a new array to hold the merged result
	SHeaderWithData * paArrayHdrMerged = S_PaAllocateElements(cElementsAlloc);
	Assert(paArrayHdrMerged!= NULL);
	paArrayHdrMerged->cElements = cElementsDst;

	// Merge the arrays
	PCVOID * prgpvSrc1 = m_paArrayHdr->rgpvData;
	PCVOID * prgpvSrc2 = pArrayAppend->m_paArrayHdr->rgpvData;
	PCVOID * prgpvDst = paArrayHdrMerged->rgpvData;
	while (TRUE)
		{
		if (cElements <= 0)
			{
			// Copy the remaining array 2 to destination
			memcpy(OUT prgpvDst, IN prgpvSrc2, cElementsAppend * sizeof(PCVOID));
			break;
			}
		if (cElementsAppend <= 0)
			{
			// Copy the remaining array 1 to destination
			memcpy(OUT prgpvDst, IN prgpvSrc1, cElements * sizeof(PCVOID));
			break;
			}
		// Compare two elements
		if (pfnCompareSort(*prgpvSrc1, *prgpvSrc2, lParamCompare) <= 0)
			{
			*prgpvDst++ = *prgpvSrc1++;
			cElements--;
			}
		else
			{
			*prgpvDst++ = *prgpvSrc2++;
			cElementsAppend--;
			}
		} // while

	ValidateHeapPointer(m_paArrayHdr);
	delete m_paArrayHdr;	// Delete previous array
	m_paArrayHdr = paArrayHdrMerged;
	Assert(FIsSortedAscending(pfnCompareSort, lParamCompare));
	} // MergeAppend()


/////////////////////////////////////////////////////////////////////
void
CArrayPsz::GetStrings(OUT CStr * pstrStrings, PSZAC pszaSeparator) const
	{
	pstrStrings->Empty();
	PSZUC * ppszStringStop;
	PSZUC * ppszString = PrgpszGetStringsStop(OUT &ppszStringStop);
	while (ppszString != ppszStringStop)
		{
		pstrStrings->AppendSeparatorAndTextU(pszaSeparator, *ppszString++);
		}
	}

int
CArrayPsz::FindStringI(PSZUC pszStringSearch) const
	{
	PSZUC * ppszStringStop;
	PSZUC * ppszStringFirst = PrgpszGetStringsStop(OUT &ppszStringStop);
	PSZUC * ppszString = ppszStringFirst;
	while (ppszString != ppszStringStop)
		{
		if (FCompareStrings_YZ(*ppszString, pszStringSearch))
			return ppszString - ppszStringFirst;
		ppszString++;
		}
	return -1;
	}

/////////////////////////////////////////////////////////////////////
CArrayPasz::~CArrayPasz()
	{
	DeleteAllStrings();
	}

int
CArrayPasz::AddStringAllocate(PSZUC pszString)
	{
	Endorse(pszString == NULL);		// A NULL or empty string will be stored as a NULL pointer
	return Add(PA_CHILD PaszAllocStringU(pszString));
	}
/*
int
CArrayPasz::AddStringAllocate(const QString & sString)
	{
	return Add(PA_CHILD PaszAllocStringQ(sString));
	}
*/
int
CArrayPasz::AddStringAllocateUnique(const QString & sString)
	{
	PSZU paszString = PaszAllocStringQ(sString);
	if (FindStringI(paszString) < 0)
		return Add(PA_CHILD paszString);
	delete paszString;
	return -1;
	}

void
CArrayPasz::AddStringsUniqueStealFromSource(INOUT CArrayPasz * parraypaszSource)
	{
	AppendStealFromSource(INOUT parraypaszSource);	// This code needs to be fixed
	}

void
CArrayPasz::DeleteAllStrings()
	{
	PSZUC * ppaszStringStop;
	PSZUC * ppaszString = PrgpszGetStringsStop(OUT &ppaszStringStop);
	while (ppaszString != ppaszStringStop)
		{
		delete *ppaszString++;
		}
	RemoveAllElements();
	}


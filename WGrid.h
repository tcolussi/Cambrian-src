#ifndef WGRID_H
#define WGRID_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class OPainterGridCell : public CPainter
{
public:
	QRect m_rcCell;
	CString m_sText;	// Text of the cell
};

#define GCF_kmWidth						0x000001FF	// Bits designating the width of the column
#define GCF_kfWidthBestFit				0x00000400
#define GCF_kmStretchFactor				0x00003000	// Allow the column to automatically stretch by x1, x2 or x3 relative to other columns.  By default, columns do not automatically stretch, unless the user does it
#define GCF_kmStretchFactor_x1			0x00001000
#define GCF_kmStretchFactor_x2			0x00002000
#define GCF_kmStretchFactor_x3			0x00003000
#define GCF_kiStretchFactor				12			// Shift by twelve bits to get the stretch factor value
#define GCF_kfWidthFixed				0x00004000	// Bit indicating the user is not allowed to change the width of the column

#define GCF_Width(cxWidth)				(cxWidth)
#define GCF_WidthStretch(cxWidth)		((cxWidth) | GCF_kmStretchFactor_x1)


struct SGridColumn
{
	PSZAC pszName;
	UINT uGridColumnFlags;	// GCF_*
};

class CGridColumn;
class CGridRow;
class CGridCell
{
	CGridColumn * m_pColumn;
	CGridRow * m_pRow;
};

typedef void * PVDATA;
typedef PVDATA PVDATAROW;

class CGridSortParameters	// Extra parameters to sort the data within a column, such as the column index/identifier and other cached data
{

};

enum EGridCellText	// EGridCellTextPurpose_, EGridGetTextPurpose
	{
	eGridCellTextPaint,		// Get the text to paint
	eGridCellTextDisplay,	// Get the text to display the data on the screen
	eGridCellTextSort,		// Get the text to sort the data (this is rarely used, as PFn_GridCellGetSortKey() is much better)
	eGridCellTextSearch,	// Get the text to search the data
	eGridCellTextEdit,		// Get the text to edit the data
	eGridCellTextToolTip,	// Get the text to display as a tooltip related to the data
	eGridCellTextStatusBar
	};

typedef CString (* PFn_GridCellGetText)(PVDATAROW pvDataRow, const CGridColumn * pColumn, EGridCellText eGridCellText);
typedef void (* PFn_GridCellDrawBackground)(CGridRow * pRow, const CGridColumn * pColumn, OPainterGridCell * pPainter);
typedef void (* PFn_GridCellDrawText)(CGridRow * pRow, const CGridColumn * pColumn, OPainterGridCell * pPainter);

typedef PVDATA (* PFn_GridCellGetSortKey)(PVDATAROW pvDataRow, const CGridColumn * pColumn, IOUT CMemoryAccumulator * pAccumulator);
typedef int (* PFn_GridSortCompareRows)(PVDATAROW pvDataRowA, PVDATAROW pvDataRowB, const CGridSortParameters * pSortParameters);	// PFn_GridCompareSortRows

//	Hold the data for a single column within the grid
class CGridColumn
{
public:
	int m_iColumn;
	int m_idColumn;
	PSZAC m_pszColumnId;
	int m_cxWidth;					// Width in pixels of the column
	UINT m_uGridColumnWidth;		// Original width
	UINT m_uGridColumnFlags;
	CStr m_strName;
	CStr m_strDescription;

	PFn_GridCellGetText m_pfnGetText;
	PFn_GridCellDrawText m_pfnDraw;
	PFn_GridSortCompareRows m_pfnSortCompareRows;

public:
	CGridColumn();
	~CGridColumn();
}; // CGridColumn

class CArrayPtrGridColumns : public CArray
{
public:
	inline CGridColumn ** PrgpGetColumnsStop(OUT CGridColumn *** pppColumnStop) const { return (CGridColumn **)PrgpvGetElementsStop(OUT (void ***)pppColumnStop); }
	void DeleteAllColumns();
};

class CGridRow
{
public:
	PVDATAROW m_pvDataRow;	// Data associated with the row (typically a pointer to an object within the application)
	PVDATA m_pvCache;		// Cached data for a given row (such as a sort key during a sort operation)
	UINT m_uGridRowFlags;	// Flags associated with the row
};

class CArrayPtrGridRows : public CArray
{
public:
	inline CGridRow ** PrgpGetRowsStop(OUT CGridRow *** pppRowStop) const { return (CGridRow **)PrgpvGetElementsStop(OUT (void ***)pppRowStop); }
	void DeleteAllRows();
};


//	Widget to display a grid (table) of items.
//	This class has been optimized to handle a large number of rows without taking too much memory or processing time.

class WGrid : public QAbstractScrollArea
{
private:
	CArrayPtrGridColumns m_arraypaColumnsData;		// Columns of the grid as added/inserted by the developer
	CArrayPtrGridColumns m_arraypColumnsDisplay;	// Columns of the grid as displayed in the GUI (the user may reorder the columns)
	CArrayPtrGridRows m_arraypaRowsData;			// Rows for the grid
	CArrayPtrGridRows m_arraypRowsDisplay;			// Rows to display in the grid.  This array is a subset of m_arraypaRowsData.

	int m_cyHeightFont;								// Height of the font
	int m_cyHeightRow;								// Height of a row
	int m_cyHeightHeaderTop;
	int m_cyHeightHeaderBottom;

	int m_cxDisplay;
	int m_cyDisplay;

	int m_cxWidthColumns;							// Total width of the columns to display
	int m_xScrollStart;								// Starting pixel to do the horizontal scrolling
	int m_iRowScrollStart;							// Display index of the row to start scrolling

	enum
		{
		FI_kfScrollBarHorizontalVisible	= 0x00000010,	// The horizontal scroll bar is visible
		FI_kfScrollBarHorizontalNeeded	= 0x00000020,	// The horizontal scroll bar is needed
		FI_kfScrollBarVerticalVisible	= 0x00000040,	// The vertical scroll bar is visible
		FI_kfScrollBarVerticalNeeded	= 0x00000080	// The vertical scroll bar is needed
		};
	UINT m_uFlagsInternal;

public:
	WGrid(QWidget * pwParent);
	~WGrid();

	void _ColumnsRecalculateWidths();
	void ColumnsAdd(const SGridColumn * prgzColumns, PFn_GridCellGetText pfnGetText);

	CGridRow * _RowAllocateP(PVDATA pvData);
	void RowsAdd(const CArray & arraypData);

	virtual void paintEvent(QPaintEvent * pEvent);
	virtual void resizeEvent(QResizeEvent *);

	virtual QSize viewportSizeHint() const;
};


#endif // WGRID_H

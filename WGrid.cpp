#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WGrid.h"

//	Must have the same interface as PFn_GridCellGetText()
CString
GridCellGetText(PVDATAROW pvDataRow, const CGridColumn * pColumn, EGridCellText)
	{
	Assert(pvDataRow != NULL);
	Assert(pColumn != NULL);
	return c_sEmpty;
	}

CGridColumn::CGridColumn()
	{
	InitToZeroes(OUT this, sizeof(*this));
	}

CGridColumn::~CGridColumn()
	{
	}

void
CArrayPtrGridColumns::DeleteAllColumns()
	{
	CGridColumn ** ppColumnStop;
	CGridColumn ** ppColumn = PrgpGetColumnsStop(OUT &ppColumnStop);
	while (ppColumn != ppColumnStop)
		delete *ppColumn++;
	RemoveAllElements();
	}

void
CArrayPtrGridRows::DeleteAllRows()
	{
	CGridRow ** ppRowStop;
	CGridRow ** ppRow = PrgpGetRowsStop(OUT &ppRowStop);
	while (ppRow != ppRowStop)
		delete *ppRow++;
	RemoveAllElements();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WGrid::WGrid(QWidget * pwParent) : QAbstractScrollArea(pwParent)
	{
	m_cyHeightFont = fontMetrics().height();
	m_cyHeightRow = m_cyHeightFont + 4;
	m_cyHeightHeaderTop = m_cyHeightFont + 3;
	m_cyHeightHeaderBottom = 0;	// No bottom header

	m_cxDisplay = width();
	m_cyDisplay = height();

	m_cxWidthColumns = 0;
	m_xScrollStart = 0;
	m_iRowScrollStart = 0;

	m_uFlagsInternal = 0;
	}

WGrid::~WGrid()
	{
	m_arraypaColumnsData.DeleteAllColumns();
	m_arraypaRowsData.DeleteAllRows();
	}

void
WGrid::_ColumnsRecalculateWidths()
	{
	Assert(m_cxWidthColumns == 0);
	CGridColumn ** ppColumnStop;
	CGridColumn ** ppColumn = m_arraypColumnsDisplay.PrgpGetColumnsStop(OUT &ppColumnStop);
	while (ppColumn != ppColumnStop)
		{
		CGridColumn * pColumn = *ppColumn++;
		m_cxWidthColumns += pColumn->m_cxWidth;
		}
	}

void
WGrid::ColumnsAdd(const SGridColumn * prgzColumns, PFn_GridCellGetText pfnGetText)
	{
	while (TRUE)
		{
		PSZUC pszName = (PSZUC)prgzColumns->pszName;
		if (pszName == NULL)
			return;
		CGridColumn * paColumn = new CGridColumn;
		paColumn->m_iColumn = m_arraypaColumnsData.Add(PA_CHILD paColumn);
		m_arraypColumnsDisplay.Add(paColumn);
		paColumn->m_strName = pszName;
		paColumn->m_uGridColumnFlags = prgzColumns->uGridColumnFlags;
		paColumn->m_cxWidth = (prgzColumns->uGridColumnFlags & GCF_kmWidth);
		paColumn->m_pfnGetText = pfnGetText;
		prgzColumns++;
		} // while
	} // ColumnsAdd()

/*
CGridRow *
WGrid::_RowAllocateP(PVDATA pvData)
	{
	}
*/


//	IMPLEMENTATION NOTES
//	This method has been optimized for performance, and consequently its code may be a bit difficult to understand.
void
WGrid::RowsAdd(const CArray & arraypData)
	{
	int cRowsAdd = arraypData.GetSize();
	CGridRow ** prgpaRowsData = (CGridRow **)m_arraypaRowsData.PrgpvAllocateElementsAppend(cRowsAdd);
	CGridRow ** prgpRowsDisplay = (CGridRow **)m_arraypRowsDisplay.PrgpvAllocateElementsAppend(cRowsAdd);

	PVDATAROW * prgpvDataRowsAdd = arraypData.PrgpvGetElements();
	while (cRowsAdd-- > 0)
		{
		CGridRow * paGridRow = new CGridRow;
		prgpaRowsData[cRowsAdd] = prgpRowsDisplay[cRowsAdd] = paGridRow;
		paGridRow->m_pvDataRow = prgpvDataRowsAdd[cRowsAdd];
		paGridRow->m_uGridRowFlags = 0;
		}
	} // RowsAdd()

void
OPainter::DrawLineHorizontal(int xLeft, int xRight, int yPos)
	{
	drawLine(xLeft, yPos, xRight, yPos);
	}

void
OPainter::DrawLineVertical(int xPos, int yTop, int yBottom)
	{
	drawLine(xPos, yTop, xPos, yBottom);
	}

void
WGrid::paintEvent(QPaintEvent * pEvent)
	{
	QRect rcPaint = pEvent->rect();
	const int xPaintLeft = rcPaint.left();
	const int xPaintRight = rcPaint.right();
	const int yPaintBottom = rcPaint.bottom();
	Assert(xPaintLeft >= 0 && xPaintLeft <= m_cxDisplay);
	Assert(xPaintRight >= 0 && xPaintRight <= m_cxDisplay);
	Assert(yPaintBottom >= 0 && yPaintBottom <= m_cyDisplay);

	OPainter oPainter(viewport());

	if (m_cxWidthColumns == 0)
		_ColumnsRecalculateWidths();

	// Find the column to start drawing
	int cxSkip = m_xScrollStart + xPaintLeft;
	CGridColumn ** ppColumnStop;
	CGridColumn ** prgpColumns = m_arraypColumnsDisplay.PrgpGetColumnsStop(OUT &ppColumnStop);
	CGridColumn ** ppColumnStart = prgpColumns;
	while (ppColumnStart != ppColumnStop)
		{
		cxSkip -= ppColumnStart[0]->m_cxWidth;
		if (cxSkip <= 0)
			break;
		ppColumnStart++;
		} // while

	CString sText;
	if (m_cyHeightHeaderTop > 0)
		{
		//QRect rcHeader(0, 0, rcClient.width(), m_cy)
		oPainter.setPen(d_coGrayDark);
		oPainter.DrawLineHorizontal(xPaintLeft, xPaintRight, 0);
		oPainter.DrawLineHorizontal(xPaintLeft, xPaintRight, m_cyHeightHeaderTop);
		oPainter.DrawLineVertical(0, 0, m_cyHeightHeaderTop);

		// Draw the columns of the header
		CGridColumn ** ppColumn = ppColumnStart;
		int xLeft = 0;
		while (ppColumn != ppColumnStop)
			{
			CGridColumn * pColumn = *ppColumn++;
			sText.InitFromStringU(pColumn->m_strName);
			const int cxWidth = pColumn->m_cxWidth;
			oPainter.drawText(xLeft, 0, cxWidth, m_cyHeightHeaderTop, Qt::AlignHCenter | Qt::AlignVCenter, sText);

			// Display all the data for a column
			oPainter.setPen(d_coBlack);
			int yTop = m_cyHeightHeaderTop;
			PFn_GridCellGetText pfnGetText = pColumn->m_pfnGetText;
			if (pfnGetText != NULL)
				{
				CGridRow ** ppRowStop;
				CGridRow ** ppRow = m_arraypRowsDisplay.PrgpGetRowsStop(OUT &ppRowStop);
				while (ppRow != ppRowStop)
					{
					CGridRow * pRow = *ppRow++;
					sText = pfnGetText(pRow->m_pvDataRow, pColumn, eGridCellTextDisplay);
					if (!sText.isEmpty())
						{
						oPainter.drawText(xLeft + 2, yTop, cxWidth, m_cyHeightHeaderTop, Qt::AlignLeft | Qt::AlignVCenter, sText);
						yTop += m_cyHeightRow;
						if (yTop >= yPaintBottom)
							break;	// Don't draw the rows outside of the drawing area
						}
					} // while
				} // if

			xLeft += cxWidth;
			if (xLeft > xPaintRight)
				break;	// No need to draw anything outside of the drawing area
			oPainter.DrawLineVertical(xLeft, 0, m_cyHeightHeaderTop);
			} // while
		} // if

	// Draw the gridlines
	oPainter.setPen(d_coGrayDark);
	int yTop = m_cyHeightHeaderTop;
	while (yTop < yPaintBottom)
		{
		yTop += m_cyHeightRow;
		oPainter.DrawLineHorizontal(xPaintLeft, xPaintRight, yTop);
		}
	/*
	oPainter.setPen(d_coOrange);
	oPainter.drawText(rcClient, Qt::AlignHCenter | Qt::AlignVCenter, "Under Development");
	*/
	} // paintEvent()

void
WGrid::resizeEvent(QResizeEvent *)
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "WGrid::resizeEvent()\n");
	m_cxWidthColumns = 0;	// Force the width of the columns to be recalculated (in case there is a stretchable column)
	m_cxDisplay = width();
	m_cyDisplay = height();
	int cxRows = m_arraypRowsDisplay.GetSize() * m_cyHeightRow;
	if (cxRows > m_cyDisplay)
		{
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		verticalScrollBar()->setMaximum(cxRows);
		}
	}

//	WGrid::QAbstractScrollArea::viewportSizeHint()
QSize
WGrid::viewportSizeHint() const
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "WGrid::viewportSizeHint()\n");
	return QSize(m_cxWidthColumns, m_arraypRowsDisplay.GetSize() * m_cyHeightRow);
	}


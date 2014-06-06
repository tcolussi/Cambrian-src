#ifndef CMARKETPLACE_H
#define CMARKETPLACE_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WMarketplace.h"

class TMarketplace : public ITreeItem
{
	RTI_IMPLEMENTATION(TMarketplace)
protected:
	CStr m_strServer;
	WMarketplace * m_pawMarketplace;

public:
	TMarketplace();
	~TMarketplace();
	virtual void TreeItem_GotFocus();		// From ITreeItem

}; // TMarketplace

class CMarketplaceArbitrators : public ITreeItem
{
	RTI_IMPLEMENTATION(CMarketplaceArbitrators)
protected:
	WMarketplaceArbitrators * m_pawMarketplace;

public:
	CMarketplaceArbitrators();
	~CMarketplaceArbitrators();
	virtual void TreeItem_GotFocus();		// From ITreeItem

}; // CMarketplaceArbitrators

#endif // CMARKETPLACE_H

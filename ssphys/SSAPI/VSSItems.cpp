// VSSItems.cpp : Implementation of CVSSItems
#include "stdafx.h"
#include "SSAPI.h"
#include "VSSItems.h"
#include "VSSItem.h"

/////////////////////////////////////////////////////////////////////////////
// CVSSItems

STDMETHODIMP CVSSItems::get_Count(LONG * piCount)
{
	if (piCount == NULL)
		return E_POINTER;
	
  ATLASSERT (m_pSSItems);
  *piCount = m_pSSItems->GetCount ();
  
  return NO_ERROR;
}
STDMETHODIMP CVSSItems::get_Item(VARIANT sItem, IVSSItem * * ppIItem)
{
	if (ppIItem == NULL)
		return E_POINTER;
		
  ATLASSERT (m_pSSItems);
  // starts counting with 1
  long count = _variant_t(sItem);
  SSItem* pItem = m_pSSItems->GetItem (count - 1);
  
  CComObject<CVSSItem>* pVssItem;
  HRESULT hRes = CComObject<CVSSItem>::CreateInstance(&pVssItem);
	if (FAILED(hRes))
    return hRes;

	pVssItem->AddRef();
  pVssItem->Init (pItem);
   
  *ppIItem = pVssItem;

	return NO_ERROR;
}
STDMETHODIMP CVSSItems::_NewEnum(IUnknown * * ppIEnum)
{
	if (ppIEnum == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}

// VSSItem.cpp : Implementation of CVSSItem
#include "stdafx.h"
#include "SSAPI.h"
#include "VSSItem.h"
#include "VSSItems.h"
#include "VSSVersions.h"

/////////////////////////////////////////////////////////////////////////////
// CVSSItem

STDMETHODIMP CVSSItem::get_Spec (BSTR * pSpec)
{
	if (pSpec == NULL)
		return E_POINTER;
	
  ATLASSERT (m_pSSItem);
  *pSpec = _bstr_t (m_pSSItem->GetSpec().c_str ()).copy ();
  
	return NO_ERROR;
}
STDMETHODIMP CVSSItem::get_Binary (VARIANT_BOOL * pbBinary)
{
	if (pbBinary == NULL)
		return E_POINTER;
		
  ATLASSERT (m_pSSItem);
  *pbBinary = _variant_t (m_pSSItem->GetBinary());
  
	return NO_ERROR;
}
STDMETHODIMP CVSSItem::put_Binary (VARIANT_BOOL pbBinary)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::get_Deleted (VARIANT_BOOL * pbDeleted)
{
	if (pbDeleted == NULL)
		return E_POINTER;
		
  ATLASSERT (m_pSSItem);
  *pbDeleted = _variant_t (m_pSSItem->GetDeleted());
  
	return NO_ERROR;
}
STDMETHODIMP CVSSItem::put_Deleted (VARIANT_BOOL pbDeleted)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::get_Type (INT * piType)
{
	if (piType == NULL)
		return E_POINTER;
		
  ATLASSERT (m_pSSItem);
  *piType = m_pSSItem->GetType() == SSITEM_FILE ? VSSITEM_FILE : VSSITEM_PROJECT;
  
	return NO_ERROR;
}
STDMETHODIMP CVSSItem::get_LocalSpec (BSTR * pLocal)
{
	if (pLocal == NULL)
		return E_POINTER;
		
  ATLASSERT (m_pSSItem);
  *pLocal = _bstr_t (m_pSSItem->GetLocalSpec().c_str()).copy ();
  
	return NO_ERROR;
}
STDMETHODIMP CVSSItem::put_LocalSpec (BSTR pLocal)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::get_Name (BSTR * pName)
{
	if (pName == NULL)
		return E_POINTER;
		
  ATLASSERT (m_pSSItem);
  *pName = _bstr_t (m_pSSItem->GetName().c_str()).copy ();
  
	return NO_ERROR;
}
STDMETHODIMP CVSSItem::put_Name (BSTR pName)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::get_Parent (IVSSItem * * ppIParent)
{
	if (ppIParent == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::get_VersionNumber (LONG * piVersion)
{
	if (piVersion == NULL)
		return E_POINTER;
		
  ATLASSERT (m_pSSItem);
  *piVersion = _variant_t (m_pSSItem->GetVersionNumber());
  
	return NO_ERROR;
}
STDMETHODIMP CVSSItem::get_Items (VARIANT_BOOL IncludeDeleted, IVSSItems * * ppIItems)
{
	if (ppIItems == NULL)
		return E_POINTER;
		
  ATLASSERT (m_pSSItem);
  SSItems* pItems = m_pSSItem->GetItems (IncludeDeleted);

  CComObject<CVSSItems>* pVssItems;
  HRESULT hRes = CComObject<CVSSItems>::CreateInstance(&pVssItems);
	if (FAILED(hRes))
    return hRes;
 
	pVssItems->AddRef();
  pVssItems->Init (pItems);
   
  *ppIItems = pVssItems;

  return NO_ERROR;
}
STDMETHODIMP CVSSItem::Get (BSTR * Local, LONG iFlags)
{
	if (Local == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::Checkout (BSTR Comment, BSTR Local, LONG iFlags)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::Checkin (BSTR Comment, BSTR Local, LONG iFlags)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::UndoCheckout (BSTR Local, LONG iFlags)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::get_IsCheckedOut (LONG * piStatus)
{
	if (piStatus == NULL)
		return E_POINTER;
		
  ATLASSERT (m_pSSItem);
  *piStatus = _variant_t (m_pSSItem->GetIsCheckedOut());
  
	return NO_ERROR;
}
STDMETHODIMP CVSSItem::get_Checkouts (IVSSCheckouts * * ppICheckouts)
{
	if (ppICheckouts == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::get_IsDifferent (BSTR Local, VARIANT_BOOL * pbDifferent)
{
	if (pbDifferent == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::Add (BSTR Local, BSTR Comment, LONG iFlags, IVSSItem * * ppIItem)
{
	if (ppIItem == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::NewSubproject (BSTR Name, BSTR Comment, IVSSItem * * ppIItem)
{
	if (ppIItem == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::Share (IVSSItem * pIItem, BSTR Comment, LONG iFlags)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::Destroy ()
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::Move (IVSSItem * pINewParent)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::Label (BSTR Label, BSTR Comment)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::get_Versions (LONG iFlags, IVSSVersions * * pIVersions)
{
	if (pIVersions == NULL)
		return E_POINTER;

  CComObject<CVSSVersions>* pVssVersions;
  HRESULT hRes = CComObject<CVSSVersions>::CreateInstance(&pVssVersions);
	if (FAILED(hRes))
    return hRes;

	pVssVersions->AddRef();
   
  *pIVersions = pVssVersions;
		
	return NO_ERROR;
}
STDMETHODIMP CVSSItem::get_Version (VARIANT Version, IVSSItem * * ppIItem)
{
	if (ppIItem == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
// IVSSItem
STDMETHODIMP CVSSItem::get_Links (IVSSItems * * ppIItems)
{
	if (ppIItems == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
STDMETHODIMP CVSSItem::Branch (BSTR Comment, LONG iFlags, IVSSItem * * ppIItem)
{
	if (ppIItem == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}

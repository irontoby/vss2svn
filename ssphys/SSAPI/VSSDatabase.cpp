// VSSDatabase.cpp : Implementation of CVSSDatabase
#include "stdafx.h"
#include "SSAPI.h"
#include "VSSDatabase.h"
#include "VSSItem.h"
#include "atlconv.h"
#include "ssauterr.h"

/////////////////////////////////////////////////////////////////////////////
// CVSSDatabase

STDMETHODIMP CVSSDatabase::Open(BSTR SrcSafeIni, BSTR Username, BSTR Password)
{
	ATLASSERT (SrcSafeIni);
	ATLASSERT (Username);
	ATLASSERT (Password);
  _bstr_t _SrcSafeIni (SrcSafeIni);
  _bstr_t _Username (Username);
  _bstr_t _Password (Password);
  ATLTRACE (_T("CVSSDatabase::Open(BSTR %s, BSTR %s, BSTR %s"), (LPCTSTR)_SrcSafeIni, (LPCTSTR)_Username, (LPCTSTR)_Password);
  
  if (!m_Database.Open ((LPCTSTR)_SrcSafeIni, (LPCTSTR)_Username, (LPCTSTR)_Password))
    return E_FAIL;

  return NO_ERROR;
}

STDMETHODIMP CVSSDatabase::get_SrcSafeIni (BSTR * pSrcSafeIni)
{
	if (pSrcSafeIni == NULL)
		return E_POINTER;
		
  _bstr_t srcSafeIni = m_Database.GetSrcSafeIni ().c_str();
  *pSrcSafeIni = srcSafeIni.copy();
  
  return NO_ERROR;
}
STDMETHODIMP CVSSDatabase::get_DatabaseName (BSTR * pDatabaseName)
{
	if (pDatabaseName == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
STDMETHODIMP CVSSDatabase::get_Username (BSTR * pUsername)
{
	if (pUsername == NULL)
		return E_POINTER;
		
  _bstr_t srcSafeIni = m_Database.GetUsername ().c_str();
  *pUsername = srcSafeIni.copy();

  return NO_ERROR;
}
STDMETHODIMP CVSSDatabase::get_CurrentProject (BSTR * pPrj)
{
	if (pPrj == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
STDMETHODIMP CVSSDatabase::put_CurrentProject (BSTR pPrj)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSDatabase::get_VSSItem (BSTR Spec, VARIANT_BOOL Deleted, IVSSItem * * ppIVSSItem)
{
	if (ppIVSSItem == NULL)
		return E_POINTER;
		
  if (!m_Database.IsOpen ())
    return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 11251);

  USES_CONVERSION;
  SSItem* pItem = m_Database.GetSSItem (OLE2CA(Spec), Deleted);
  if (!pItem)
    return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, ESS_VS_NOT_FOUND);
  
  CComObject<CVSSItem>* pVssItem;
  HRESULT hRes = CComObject<CVSSItem>::CreateInstance(&pVssItem);
	if (FAILED(hRes))
    return hRes;

	pVssItem->AddRef();
  pVssItem->Init (pItem);
   
  *ppIVSSItem = pVssItem;

  return NO_ERROR;
}


STDMETHODIMP CVSSDatabase::get_ProjectRightsEnabled (VARIANT_BOOL * pEnabled)
{
	if (pEnabled == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
STDMETHODIMP CVSSDatabase::put_ProjectRightsEnabled (VARIANT_BOOL pEnabled)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSDatabase::get_DefaultProjectRights (LONG * pRights)
{
	if (pRights == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
STDMETHODIMP CVSSDatabase::put_DefaultProjectRights (LONG pRights)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVSSDatabase::AddUser (BSTR User, BSTR Password, VARIANT_BOOL ReadOnly, IVSSUser * * ppIUser)
{
	if (ppIUser == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
STDMETHODIMP CVSSDatabase::get_User (BSTR Name, IVSSUser * * ppIUser)
{
	if (ppIUser == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}
STDMETHODIMP CVSSDatabase::get_Users (IVSSUsers * * ppIUsers)
{
	if (ppIUsers == NULL)
		return E_POINTER;
		
	return E_NOTIMPL;
}

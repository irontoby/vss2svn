// VSSVersions.cpp : Implementation of CVSSVersions
#include "stdafx.h"
#include "SSAPI.h"
#include "VSSVersions.h"

/////////////////////////////////////////////////////////////////////////////
// CVSSVersions

STDMETHODIMP CVSSVersions::_NewEnum (IUnknown * * ppIEnum)
{
	if (ppIEnum == NULL)
		return E_POINTER;
		
//  CComQIPtr<IEnumVARIANT> spEnum (this);
//  HRESULT hRes = spEnum.CoCreateInstance(IID_IEnumVARIANT);
//	if (FAILED(hRes))
//    return hRes;

	*ppIEnum = this->_GetRawUnknown();
  (*ppIEnum)->AddRef();
   
	return NO_ERROR;
}

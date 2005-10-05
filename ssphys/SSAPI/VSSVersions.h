// VSSVersions.h : Declaration of the CVSSVersions

#ifndef __VSSVERSIONS_H_
#define __VSSVERSIONS_H_

#include "resource.h"       // main symbols
//#import "C:\WINDOWS\system32\Stdole2.tlb" raw_interfaces_only, raw_native_types, no_namespace, named_guids 

/////////////////////////////////////////////////////////////////////////////
// CVSSVersions
class ATL_NO_VTABLE CVSSVersions : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CVSSVersions, &CLSID_VSSVersions>,
	public IDispatchImpl<IVSSVersions, &IID_IVSSVersions, &LIBID_SourceSafeTypeLib>,
	public IEnumVARIANT
{
public:
	CVSSVersions()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_VSSVERSIONS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CVSSVersions)
	COM_INTERFACE_ENTRY(IVSSVersions)
	COM_INTERFACE_ENTRY(IEnumVARIANT)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
// IVSSVersions
	STDMETHOD(_NewEnum)(IUnknown * * ppIEnum);

// IEnumVARIANT
	STDMETHOD(Next)(ULONG celt, VARIANT * rgvar, ULONG * pceltFetched)
	{
		if (pceltFetched == NULL)
			return E_POINTER;
			
		return E_NOTIMPL;
	}
	STDMETHOD(Skip)(ULONG celt)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Reset)()
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Clone)(IEnumVARIANT * * ppenum)
	{
		if (ppenum == NULL)
			return E_POINTER;
			
		return E_NOTIMPL;
	}
};

#endif //__VSSVERSIONS_H_

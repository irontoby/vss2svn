// VSSApp.h : Declaration of the CVSSApp

#ifndef __VSSAPP_H_
#define __VSSAPP_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CVSSApp
class ATL_NO_VTABLE CVSSApp : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CVSSApp, &CLSID_VSSApp>,
	public IDispatchImpl<IVSS, &IID_IVSS, &LIBID_SourceSafeTypeLib>
{
public:
	CVSSApp()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_VSSAPP)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CVSSApp)
	COM_INTERFACE_ENTRY(IVSS)
//DEL 	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY2(IDispatch, IVSS)
END_COM_MAP()

// IVSS
public:
	STDMETHOD(get_VSSDatabase)(/*[out, retval]*/ IVSSDatabase* *pVal);
};

#endif //__VSSAPP_H_

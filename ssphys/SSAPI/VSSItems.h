// VSSItems.h : Declaration of the CVSSItems

#ifndef __VSSITEMS_H_
#define __VSSITEMS_H_

#include "resource.h"       // main symbols
#include <SSPhysLib\SSItem.h>

/////////////////////////////////////////////////////////////////////////////
// CVSSItems
class ATL_NO_VTABLE CVSSItems : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CVSSItems, &CLSID_VSSItems>,
	public IDispatchImpl<IVSSItems, &IID_IVSSItems, &LIBID_SourceSafeTypeLib>
{
public:
	CVSSItems()
    : m_pSSItems (NULL)
	{
	}
  
  void Init (SSItems* pItems)
  {
    m_pSSItems = pItems;
  }
private:
  SSItems* m_pSSItems;

public:
DECLARE_REGISTRY_RESOURCEID(IDR_VSSITEMS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CVSSItems)
	COM_INTERFACE_ENTRY(IVSSItems)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
// IVSSItems
	STDMETHOD(get_Count)(LONG * piCount);
	STDMETHOD(get_Item)(VARIANT sItem, IVSSItem * * ppIItem);
	STDMETHOD(_NewEnum)(IUnknown * * ppIEnum);
};

#endif //__VSSITEMS_H_

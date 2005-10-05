// VSSItem.h : Declaration of the CVSSItem

#ifndef __VSSITEM_H_
#define __VSSITEM_H_

#include "resource.h"       // main symbols
#include <SSPhysLib\SSItem.h>

/////////////////////////////////////////////////////////////////////////////
// CVSSItem
class ATL_NO_VTABLE CVSSItem : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CVSSItem, &CLSID_VSSItem>,
	public IDispatchImpl<IVSSItem, &IID_IVSSItem, &LIBID_SourceSafeTypeLib>
{
public:
	CVSSItem()
    : m_pSSItem (NULL)
	{
	}

  void Init (SSItem* pItem)
  {
    m_pSSItem = pItem;
  }

private:
  SSItem* m_pSSItem;

public:
DECLARE_REGISTRY_RESOURCEID(IDR_VSSITEM)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CVSSItem)
//DEL 	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY2(IDispatch, IVSSItem)
	COM_INTERFACE_ENTRY(IVSSItem)
END_COM_MAP()

// IVSSItem
public:
// IVSSItemOld
	STDMETHOD(get_Spec)(BSTR * pSpec);
	STDMETHOD(get_Binary)(VARIANT_BOOL * pbBinary);
	STDMETHOD(put_Binary)(VARIANT_BOOL pbBinary);
	STDMETHOD(get_Deleted)(VARIANT_BOOL * pbDeleted);
	STDMETHOD(put_Deleted)(VARIANT_BOOL pbDeleted);
	STDMETHOD(get_Type)(INT * piType);
	STDMETHOD(get_LocalSpec)(BSTR * pLocal);
	STDMETHOD(put_LocalSpec)(BSTR pLocal);
	STDMETHOD(get_Name)(BSTR * pName);
	STDMETHOD(put_Name)(BSTR pName);
	STDMETHOD(get_Parent)(IVSSItem * * ppIParent);
	STDMETHOD(get_VersionNumber)(LONG * piVersion);
	STDMETHOD(get_Items)(VARIANT_BOOL IncludeDeleted, IVSSItems * * ppIItems);
	STDMETHOD(Get)(BSTR * Local, LONG iFlags);
	STDMETHOD(Checkout)(BSTR Comment, BSTR Local, LONG iFlags);
	STDMETHOD(Checkin)(BSTR Comment, BSTR Local, LONG iFlags);
	STDMETHOD(UndoCheckout)(BSTR Local, LONG iFlags);
	STDMETHOD(get_IsCheckedOut)(LONG * piStatus);
	STDMETHOD(get_Checkouts)(IVSSCheckouts * * ppICheckouts);
	STDMETHOD(get_IsDifferent)(BSTR Local, VARIANT_BOOL * pbDifferent);
	STDMETHOD(Add)(BSTR Local, BSTR Comment, LONG iFlags, IVSSItem * * ppIItem);
	STDMETHOD(NewSubproject)(BSTR Name, BSTR Comment, IVSSItem * * ppIItem);
	STDMETHOD(Share)(IVSSItem * pIItem, BSTR Comment, LONG iFlags);
	STDMETHOD(Destroy)();
	STDMETHOD(Move)(IVSSItem * pINewParent);
	STDMETHOD(Label)(BSTR Label, BSTR Comment);
	STDMETHOD(get_Versions)(LONG iFlags, IVSSVersions * * pIVersions);
	STDMETHOD(get_Version)(VARIANT Version, IVSSItem * * ppIItem);

// IVSSItem
	STDMETHOD(get_Links)(IVSSItems * * ppIItems);
	STDMETHOD(Branch)(BSTR Comment, LONG iFlags, IVSSItem * * ppIItem);

};

#endif //__VSSITEM_H_

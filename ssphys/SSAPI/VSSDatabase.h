// VSSDatabase.h : Declaration of the CVSSDatabase

#ifndef __VSSDATABASE_H_
#define __VSSDATABASE_H_

#include "resource.h"       // main symbols
#include <SSPhysLib\SSDatabase.h>

/////////////////////////////////////////////////////////////////////////////
// CVSSDatabase
class ATL_NO_VTABLE CVSSDatabase : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CVSSDatabase, &CLSID_VSSDatabase>,
	public IDispatchImpl<IVSSDatabase, &IID_IVSSDatabase, &LIBID_SourceSafeTypeLib>
{
public:
	CVSSDatabase()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_VSSDATABASE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CVSSDatabase)
//DEL 	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY2(IDispatch, IVSSDatabase)
  COM_INTERFACE_ENTRY(IVSSDatabase)
END_COM_MAP()

public:
// IVSSDatabase
	STDMETHOD(get_ProjectRightsEnabled)(VARIANT_BOOL * pEnabled);
	STDMETHOD(put_ProjectRightsEnabled)(VARIANT_BOOL pEnabled);
	STDMETHOD(get_DefaultProjectRights)(LONG * pRights);
	STDMETHOD(put_DefaultProjectRights)(LONG pRights);
	STDMETHOD(AddUser)(BSTR User, BSTR Password, VARIANT_BOOL ReadOnly, IVSSUser * * ppIUser);
	STDMETHOD(get_User)(BSTR Name, IVSSUser * * ppIUser);
	STDMETHOD(get_Users)(IVSSUsers * * ppIUsers);

// IVSSDatabaseOld
	STDMETHOD(Open)(BSTR SrcSafeIni, BSTR Username, BSTR Password);
  STDMETHOD(get_SrcSafeIni)(BSTR * pSrcSafeIni);
	STDMETHOD(get_DatabaseName)(BSTR * pDatabaseName);
	STDMETHOD(get_Username)(BSTR * pUsername);
	STDMETHOD(get_CurrentProject)(BSTR * pPrj);
	STDMETHOD(put_CurrentProject)(BSTR pPrj);
	STDMETHOD(get_VSSItem)(BSTR Spec, VARIANT_BOOL Deleted, IVSSItem * * ppIVSSItem);

private:
  SSDatabase m_Database;
};

#endif //__VSSDATABASE_H_

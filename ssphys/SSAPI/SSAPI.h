/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Fri Mar 11 00:07:28 2005
 */
/* Compiler settings for F:\src\vss2svn\ssphys\SSAPI\SSAPI.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __SSAPI_h__
#define __SSAPI_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IVSSItemOld_FWD_DEFINED__
#define __IVSSItemOld_FWD_DEFINED__
typedef interface IVSSItemOld IVSSItemOld;
#endif 	/* __IVSSItemOld_FWD_DEFINED__ */


#ifndef __IVSSItem_FWD_DEFINED__
#define __IVSSItem_FWD_DEFINED__
typedef interface IVSSItem IVSSItem;
#endif 	/* __IVSSItem_FWD_DEFINED__ */


#ifndef __IVSSVersions_FWD_DEFINED__
#define __IVSSVersions_FWD_DEFINED__
typedef interface IVSSVersions IVSSVersions;
#endif 	/* __IVSSVersions_FWD_DEFINED__ */


#ifndef __IVSSVersionOld_FWD_DEFINED__
#define __IVSSVersionOld_FWD_DEFINED__
typedef interface IVSSVersionOld IVSSVersionOld;
#endif 	/* __IVSSVersionOld_FWD_DEFINED__ */


#ifndef __IVSSVersion_FWD_DEFINED__
#define __IVSSVersion_FWD_DEFINED__
typedef interface IVSSVersion IVSSVersion;
#endif 	/* __IVSSVersion_FWD_DEFINED__ */


#ifndef __IVSSItems_FWD_DEFINED__
#define __IVSSItems_FWD_DEFINED__
typedef interface IVSSItems IVSSItems;
#endif 	/* __IVSSItems_FWD_DEFINED__ */


#ifndef __IVSSCheckouts_FWD_DEFINED__
#define __IVSSCheckouts_FWD_DEFINED__
typedef interface IVSSCheckouts IVSSCheckouts;
#endif 	/* __IVSSCheckouts_FWD_DEFINED__ */


#ifndef __IVSSCheckout_FWD_DEFINED__
#define __IVSSCheckout_FWD_DEFINED__
typedef interface IVSSCheckout IVSSCheckout;
#endif 	/* __IVSSCheckout_FWD_DEFINED__ */


#ifndef __IVSSDatabaseOld_FWD_DEFINED__
#define __IVSSDatabaseOld_FWD_DEFINED__
typedef interface IVSSDatabaseOld IVSSDatabaseOld;
#endif 	/* __IVSSDatabaseOld_FWD_DEFINED__ */


#ifndef __IVSSDatabase_FWD_DEFINED__
#define __IVSSDatabase_FWD_DEFINED__
typedef interface IVSSDatabase IVSSDatabase;
#endif 	/* __IVSSDatabase_FWD_DEFINED__ */


#ifndef __IVSSUser_FWD_DEFINED__
#define __IVSSUser_FWD_DEFINED__
typedef interface IVSSUser IVSSUser;
#endif 	/* __IVSSUser_FWD_DEFINED__ */


#ifndef __IVSSUsers_FWD_DEFINED__
#define __IVSSUsers_FWD_DEFINED__
typedef interface IVSSUsers IVSSUsers;
#endif 	/* __IVSSUsers_FWD_DEFINED__ */


#ifndef __IVSSEventsOld_FWD_DEFINED__
#define __IVSSEventsOld_FWD_DEFINED__
typedef interface IVSSEventsOld IVSSEventsOld;
#endif 	/* __IVSSEventsOld_FWD_DEFINED__ */


#ifndef __IVSSEvents_FWD_DEFINED__
#define __IVSSEvents_FWD_DEFINED__
typedef interface IVSSEvents IVSSEvents;
#endif 	/* __IVSSEvents_FWD_DEFINED__ */


#ifndef __IVSS_FWD_DEFINED__
#define __IVSS_FWD_DEFINED__
typedef interface IVSS IVSS;
#endif 	/* __IVSS_FWD_DEFINED__ */


#ifndef __IVSSEventHandler_FWD_DEFINED__
#define __IVSSEventHandler_FWD_DEFINED__
typedef interface IVSSEventHandler IVSSEventHandler;
#endif 	/* __IVSSEventHandler_FWD_DEFINED__ */


#ifndef __VSSItem_FWD_DEFINED__
#define __VSSItem_FWD_DEFINED__

#ifdef __cplusplus
typedef class VSSItem VSSItem;
#else
typedef struct VSSItem VSSItem;
#endif /* __cplusplus */

#endif 	/* __VSSItem_FWD_DEFINED__ */


#ifndef __VSSItems_FWD_DEFINED__
#define __VSSItems_FWD_DEFINED__

#ifdef __cplusplus
typedef class VSSItems VSSItems;
#else
typedef struct VSSItems VSSItems;
#endif /* __cplusplus */

#endif 	/* __VSSItems_FWD_DEFINED__ */


#ifndef __VSSVersions_FWD_DEFINED__
#define __VSSVersions_FWD_DEFINED__

#ifdef __cplusplus
typedef class VSSVersions VSSVersions;
#else
typedef struct VSSVersions VSSVersions;
#endif /* __cplusplus */

#endif 	/* __VSSVersions_FWD_DEFINED__ */


#ifndef __VSSVersion_FWD_DEFINED__
#define __VSSVersion_FWD_DEFINED__

#ifdef __cplusplus
typedef class VSSVersion VSSVersion;
#else
typedef struct VSSVersion VSSVersion;
#endif /* __cplusplus */

#endif 	/* __VSSVersion_FWD_DEFINED__ */


#ifndef __VSSCheckout_FWD_DEFINED__
#define __VSSCheckout_FWD_DEFINED__

#ifdef __cplusplus
typedef class VSSCheckout VSSCheckout;
#else
typedef struct VSSCheckout VSSCheckout;
#endif /* __cplusplus */

#endif 	/* __VSSCheckout_FWD_DEFINED__ */


#ifndef __VSSDatabase_FWD_DEFINED__
#define __VSSDatabase_FWD_DEFINED__

#ifdef __cplusplus
typedef class VSSDatabase VSSDatabase;
#else
typedef struct VSSDatabase VSSDatabase;
#endif /* __cplusplus */

#endif 	/* __VSSDatabase_FWD_DEFINED__ */


#ifndef __VSSUser_FWD_DEFINED__
#define __VSSUser_FWD_DEFINED__

#ifdef __cplusplus
typedef class VSSUser VSSUser;
#else
typedef struct VSSUser VSSUser;
#endif /* __cplusplus */

#endif 	/* __VSSUser_FWD_DEFINED__ */


#ifndef __VSSApp_FWD_DEFINED__
#define __VSSApp_FWD_DEFINED__

#ifdef __cplusplus
typedef class VSSApp VSSApp;
#else
typedef struct VSSApp VSSApp;
#endif /* __cplusplus */

#endif 	/* __VSSApp_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __SourceSafeTypeLib_LIBRARY_DEFINED__
#define __SourceSafeTypeLib_LIBRARY_DEFINED__

/* library SourceSafeTypeLib */
/* [helpcontext][helpfile][helpstring][version][uuid] */ 

















typedef /* [public][helpstring][uuid] */ 
enum __MIDL___MIDL_itf_SSAPI_0000_0001
    {	VSSFLAG_USERRONO	= 1,
	VSSFLAG_USERROYES	= 2,
	VSSFLAG_TIMENOW	= 4,
	VSSFLAG_TIMEMOD	= 8,
	VSSFLAG_TIMEUPD	= 12,
	VSSFLAG_EOLCR	= 16,
	VSSFLAG_EOLLF	= 32,
	VSSFLAG_EOLCRLF	= 48,
	VSSFLAG_REPASK	= 64,
	VSSFLAG_REPREPLACE	= 128,
	VSSFLAG_REPSKIP	= 192,
	VSSFLAG_REPMERGE	= 256,
	VSSFLAG_CMPFULL	= 512,
	VSSFLAG_CMPTIME	= 1024,
	VSSFLAG_CMPCHKSUM	= 1536,
	VSSFLAG_CMPFAIL	= 2048,
	VSSFLAG_RECURSNO	= 4096,
	VSSFLAG_RECURSYES	= 8192,
	VSSFLAG_FORCEDIRNO	= 16384,
	VSSFLAG_FORCEDIRYES	= 32768,
	VSSFLAG_KEEPNO	= 65536,
	VSSFLAG_KEEPYES	= 131072,
	VSSFLAG_DELNO	= 262144,
	VSSFLAG_DELYES	= 524288,
	VSSFLAG_DELNOREPLACE	= 786432,
	VSSFLAG_BINTEST	= 1048576,
	VSSFLAG_BINBINARY	= 2097152,
	VSSFLAG_BINTEXT	= 3145728,
	VSSFLAG_DELTAYES	= 4194304,
	VSSFLAG_DELTANO	= 8388608,
	VSSFLAG_UPDASK	= 16777216,
	VSSFLAG_UPDUPDATE	= 33554432,
	VSSFLAG_UPDUNCH	= 50331648,
	VSSFLAG_GETYES	= 67108864,
	VSSFLAG_GETNO	= 134217728,
	VSSFLAG_CHKEXCLUSIVEYES	= 268435456,
	VSSFLAG_CHKEXCLUSIVENO	= 536870912,
	VSSFLAG_HISTIGNOREFILES	= 1073741824
    }	VSSFlags;

typedef /* [public][helpstring][uuid] */ 
enum __MIDL___MIDL_itf_SSAPI_0000_0002
    {	VSSFILE_NOTCHECKEDOUT	= 0,
	VSSFILE_CHECKEDOUT	= 1,
	VSSFILE_CHECKEDOUT_ME	= 2
    }	VSSFileStatus;

typedef /* [public][helpstring][uuid] */ 
enum __MIDL___MIDL_itf_SSAPI_0000_0003
    {	VSSITEM_PROJECT	= 0,
	VSSITEM_FILE	= 1
    }	VSSItemType;

typedef /* [public][helpstring][uuid] */ 
enum __MIDL___MIDL_itf_SSAPI_0000_0004
    {	VSSRIGHTS_READ	= 1,
	VSSRIGHTS_CHKUPD	= 2,
	VSSRIGHTS_ADDRENREM	= 4,
	VSSRIGHTS_DESTROY	= 8,
	VSSRIGHTS_ALL	= 15,
	VSSRIGHTS_INHERITED	= 16
    }	VSSRights;


EXTERN_C const IID LIBID_SourceSafeTypeLib;

#ifndef __IVSSItemOld_INTERFACE_DEFINED__
#define __IVSSItemOld_INTERFACE_DEFINED__

/* interface IVSSItemOld */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSItemOld;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("783CD4E1-9D54-11CF-B8EE-00608CC9A71F")
    IVSSItemOld : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Spec( 
            /* [retval][out] */ BSTR __RPC_FAR *pSpec) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Binary( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbBinary) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Binary( 
            /* [in] */ VARIANT_BOOL pbBinary) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Deleted( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDeleted) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Deleted( 
            /* [in] */ VARIANT_BOOL pbDeleted) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Type( 
            /* [retval][out] */ int __RPC_FAR *piType) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_LocalSpec( 
            /* [retval][out] */ BSTR __RPC_FAR *pLocal) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_LocalSpec( 
            /* [in] */ BSTR pLocal) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR __RPC_FAR *pName) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pName) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIParent) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_VersionNumber( 
            /* [retval][out] */ long __RPC_FAR *piVersion) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Items( 
            /* [defaultvalue][optional][in] */ VARIANT_BOOL IncludeDeleted,
            /* [retval][out] */ IVSSItems __RPC_FAR *__RPC_FAR *ppIItems) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Get( 
            /* [optional][out][in] */ BSTR __RPC_FAR *Local,
            /* [defaultvalue][optional][in] */ long iFlags = 0) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Checkout( 
            /* [defaultvalue][optional][in] */ BSTR Comment = L"",
            /* [defaultvalue][optional][in] */ BSTR Local = L"",
            /* [defaultvalue][optional][in] */ long iFlags = 0) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Checkin( 
            /* [defaultvalue][optional][in] */ BSTR Comment = L"",
            /* [defaultvalue][optional][in] */ BSTR Local = L"",
            /* [defaultvalue][optional][in] */ long iFlags = 0) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE UndoCheckout( 
            /* [defaultvalue][optional][in] */ BSTR Local = L"",
            /* [defaultvalue][optional][in] */ long iFlags = 0) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_IsCheckedOut( 
            /* [retval][out] */ long __RPC_FAR *piStatus) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Checkouts( 
            /* [retval][out] */ IVSSCheckouts __RPC_FAR *__RPC_FAR *ppICheckouts) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_IsDifferent( 
            /* [defaultvalue][optional][in] */ BSTR Local,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDifferent) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [in] */ BSTR Local,
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE NewSubproject( 
            /* [in] */ BSTR Name,
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Share( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [defaultvalue][optional][in] */ BSTR Comment = L"",
            /* [defaultvalue][optional][in] */ long iFlags = 0) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Destroy( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Move( 
            /* [in] */ IVSSItem __RPC_FAR *pINewParent) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Label( 
            /* [in] */ BSTR Label,
            /* [defaultvalue][optional][in] */ BSTR Comment = L"") = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Versions( 
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ IVSSVersions __RPC_FAR *__RPC_FAR *pIVersions) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Version( 
            /* [optional][in] */ VARIANT Version,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSItemOldVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSItemOld __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSItemOld __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSItemOld __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IVSSItemOld __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IVSSItemOld __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IVSSItemOld __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IVSSItemOld __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Spec )( 
            IVSSItemOld __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pSpec);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Binary )( 
            IVSSItemOld __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbBinary);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Binary )( 
            IVSSItemOld __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL pbBinary);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Deleted )( 
            IVSSItemOld __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDeleted);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Deleted )( 
            IVSSItemOld __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL pbDeleted);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Type )( 
            IVSSItemOld __RPC_FAR * This,
            /* [retval][out] */ int __RPC_FAR *piType);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalSpec )( 
            IVSSItemOld __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pLocal);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LocalSpec )( 
            IVSSItemOld __RPC_FAR * This,
            /* [in] */ BSTR pLocal);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IVSSItemOld __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pName);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Name )( 
            IVSSItemOld __RPC_FAR * This,
            /* [in] */ BSTR pName);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IVSSItemOld __RPC_FAR * This,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIParent);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VersionNumber )( 
            IVSSItemOld __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *piVersion);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Items )( 
            IVSSItemOld __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL IncludeDeleted,
            /* [retval][out] */ IVSSItems __RPC_FAR *__RPC_FAR *ppIItems);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Get )( 
            IVSSItemOld __RPC_FAR * This,
            /* [optional][out][in] */ BSTR __RPC_FAR *Local,
            /* [defaultvalue][optional][in] */ long iFlags);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Checkout )( 
            IVSSItemOld __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [defaultvalue][optional][in] */ BSTR Local,
            /* [defaultvalue][optional][in] */ long iFlags);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Checkin )( 
            IVSSItemOld __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [defaultvalue][optional][in] */ BSTR Local,
            /* [defaultvalue][optional][in] */ long iFlags);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UndoCheckout )( 
            IVSSItemOld __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR Local,
            /* [defaultvalue][optional][in] */ long iFlags);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsCheckedOut )( 
            IVSSItemOld __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *piStatus);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Checkouts )( 
            IVSSItemOld __RPC_FAR * This,
            /* [retval][out] */ IVSSCheckouts __RPC_FAR *__RPC_FAR *ppICheckouts);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsDifferent )( 
            IVSSItemOld __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR Local,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDifferent);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Add )( 
            IVSSItemOld __RPC_FAR * This,
            /* [in] */ BSTR Local,
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NewSubproject )( 
            IVSSItemOld __RPC_FAR * This,
            /* [in] */ BSTR Name,
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Share )( 
            IVSSItemOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [defaultvalue][optional][in] */ long iFlags);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Destroy )( 
            IVSSItemOld __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Move )( 
            IVSSItemOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pINewParent);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Label )( 
            IVSSItemOld __RPC_FAR * This,
            /* [in] */ BSTR Label,
            /* [defaultvalue][optional][in] */ BSTR Comment);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Versions )( 
            IVSSItemOld __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ IVSSVersions __RPC_FAR *__RPC_FAR *pIVersions);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Version )( 
            IVSSItemOld __RPC_FAR * This,
            /* [optional][in] */ VARIANT Version,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);
        
        END_INTERFACE
    } IVSSItemOldVtbl;

    interface IVSSItemOld
    {
        CONST_VTBL struct IVSSItemOldVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSItemOld_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSItemOld_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSItemOld_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSItemOld_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVSSItemOld_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVSSItemOld_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVSSItemOld_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVSSItemOld_get_Spec(This,pSpec)	\
    (This)->lpVtbl -> get_Spec(This,pSpec)

#define IVSSItemOld_get_Binary(This,pbBinary)	\
    (This)->lpVtbl -> get_Binary(This,pbBinary)

#define IVSSItemOld_put_Binary(This,pbBinary)	\
    (This)->lpVtbl -> put_Binary(This,pbBinary)

#define IVSSItemOld_get_Deleted(This,pbDeleted)	\
    (This)->lpVtbl -> get_Deleted(This,pbDeleted)

#define IVSSItemOld_put_Deleted(This,pbDeleted)	\
    (This)->lpVtbl -> put_Deleted(This,pbDeleted)

#define IVSSItemOld_get_Type(This,piType)	\
    (This)->lpVtbl -> get_Type(This,piType)

#define IVSSItemOld_get_LocalSpec(This,pLocal)	\
    (This)->lpVtbl -> get_LocalSpec(This,pLocal)

#define IVSSItemOld_put_LocalSpec(This,pLocal)	\
    (This)->lpVtbl -> put_LocalSpec(This,pLocal)

#define IVSSItemOld_get_Name(This,pName)	\
    (This)->lpVtbl -> get_Name(This,pName)

#define IVSSItemOld_put_Name(This,pName)	\
    (This)->lpVtbl -> put_Name(This,pName)

#define IVSSItemOld_get_Parent(This,ppIParent)	\
    (This)->lpVtbl -> get_Parent(This,ppIParent)

#define IVSSItemOld_get_VersionNumber(This,piVersion)	\
    (This)->lpVtbl -> get_VersionNumber(This,piVersion)

#define IVSSItemOld_get_Items(This,IncludeDeleted,ppIItems)	\
    (This)->lpVtbl -> get_Items(This,IncludeDeleted,ppIItems)

#define IVSSItemOld_Get(This,Local,iFlags)	\
    (This)->lpVtbl -> Get(This,Local,iFlags)

#define IVSSItemOld_Checkout(This,Comment,Local,iFlags)	\
    (This)->lpVtbl -> Checkout(This,Comment,Local,iFlags)

#define IVSSItemOld_Checkin(This,Comment,Local,iFlags)	\
    (This)->lpVtbl -> Checkin(This,Comment,Local,iFlags)

#define IVSSItemOld_UndoCheckout(This,Local,iFlags)	\
    (This)->lpVtbl -> UndoCheckout(This,Local,iFlags)

#define IVSSItemOld_get_IsCheckedOut(This,piStatus)	\
    (This)->lpVtbl -> get_IsCheckedOut(This,piStatus)

#define IVSSItemOld_get_Checkouts(This,ppICheckouts)	\
    (This)->lpVtbl -> get_Checkouts(This,ppICheckouts)

#define IVSSItemOld_get_IsDifferent(This,Local,pbDifferent)	\
    (This)->lpVtbl -> get_IsDifferent(This,Local,pbDifferent)

#define IVSSItemOld_Add(This,Local,Comment,iFlags,ppIItem)	\
    (This)->lpVtbl -> Add(This,Local,Comment,iFlags,ppIItem)

#define IVSSItemOld_NewSubproject(This,Name,Comment,ppIItem)	\
    (This)->lpVtbl -> NewSubproject(This,Name,Comment,ppIItem)

#define IVSSItemOld_Share(This,pIItem,Comment,iFlags)	\
    (This)->lpVtbl -> Share(This,pIItem,Comment,iFlags)

#define IVSSItemOld_Destroy(This)	\
    (This)->lpVtbl -> Destroy(This)

#define IVSSItemOld_Move(This,pINewParent)	\
    (This)->lpVtbl -> Move(This,pINewParent)

#define IVSSItemOld_Label(This,Label,Comment)	\
    (This)->lpVtbl -> Label(This,Label,Comment)

#define IVSSItemOld_get_Versions(This,iFlags,pIVersions)	\
    (This)->lpVtbl -> get_Versions(This,iFlags,pIVersions)

#define IVSSItemOld_get_Version(This,Version,ppIItem)	\
    (This)->lpVtbl -> get_Version(This,Version,ppIItem)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_Spec_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pSpec);


void __RPC_STUB IVSSItemOld_get_Spec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_Binary_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbBinary);


void __RPC_STUB IVSSItemOld_get_Binary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_put_Binary_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL pbBinary);


void __RPC_STUB IVSSItemOld_put_Binary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_Deleted_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDeleted);


void __RPC_STUB IVSSItemOld_get_Deleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_put_Deleted_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL pbDeleted);


void __RPC_STUB IVSSItemOld_put_Deleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_Type_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [retval][out] */ int __RPC_FAR *piType);


void __RPC_STUB IVSSItemOld_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_LocalSpec_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pLocal);


void __RPC_STUB IVSSItemOld_get_LocalSpec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_put_LocalSpec_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [in] */ BSTR pLocal);


void __RPC_STUB IVSSItemOld_put_LocalSpec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_Name_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pName);


void __RPC_STUB IVSSItemOld_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_put_Name_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [in] */ BSTR pName);


void __RPC_STUB IVSSItemOld_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_Parent_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIParent);


void __RPC_STUB IVSSItemOld_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_VersionNumber_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *piVersion);


void __RPC_STUB IVSSItemOld_get_VersionNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_Items_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL IncludeDeleted,
    /* [retval][out] */ IVSSItems __RPC_FAR *__RPC_FAR *ppIItems);


void __RPC_STUB IVSSItemOld_get_Items_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_Get_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [optional][out][in] */ BSTR __RPC_FAR *Local,
    /* [defaultvalue][optional][in] */ long iFlags);


void __RPC_STUB IVSSItemOld_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_Checkout_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR Comment,
    /* [defaultvalue][optional][in] */ BSTR Local,
    /* [defaultvalue][optional][in] */ long iFlags);


void __RPC_STUB IVSSItemOld_Checkout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_Checkin_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR Comment,
    /* [defaultvalue][optional][in] */ BSTR Local,
    /* [defaultvalue][optional][in] */ long iFlags);


void __RPC_STUB IVSSItemOld_Checkin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_UndoCheckout_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR Local,
    /* [defaultvalue][optional][in] */ long iFlags);


void __RPC_STUB IVSSItemOld_UndoCheckout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_IsCheckedOut_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *piStatus);


void __RPC_STUB IVSSItemOld_get_IsCheckedOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_Checkouts_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [retval][out] */ IVSSCheckouts __RPC_FAR *__RPC_FAR *ppICheckouts);


void __RPC_STUB IVSSItemOld_get_Checkouts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_IsDifferent_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR Local,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDifferent);


void __RPC_STUB IVSSItemOld_get_IsDifferent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_Add_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [in] */ BSTR Local,
    /* [defaultvalue][optional][in] */ BSTR Comment,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);


void __RPC_STUB IVSSItemOld_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_NewSubproject_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [in] */ BSTR Name,
    /* [defaultvalue][optional][in] */ BSTR Comment,
    /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);


void __RPC_STUB IVSSItemOld_NewSubproject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_Share_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [defaultvalue][optional][in] */ BSTR Comment,
    /* [defaultvalue][optional][in] */ long iFlags);


void __RPC_STUB IVSSItemOld_Share_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_Destroy_Proxy( 
    IVSSItemOld __RPC_FAR * This);


void __RPC_STUB IVSSItemOld_Destroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_Move_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pINewParent);


void __RPC_STUB IVSSItemOld_Move_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_Label_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [in] */ BSTR Label,
    /* [defaultvalue][optional][in] */ BSTR Comment);


void __RPC_STUB IVSSItemOld_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_Versions_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ IVSSVersions __RPC_FAR *__RPC_FAR *pIVersions);


void __RPC_STUB IVSSItemOld_get_Versions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItemOld_get_Version_Proxy( 
    IVSSItemOld __RPC_FAR * This,
    /* [optional][in] */ VARIANT Version,
    /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);


void __RPC_STUB IVSSItemOld_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSItemOld_INTERFACE_DEFINED__ */


#ifndef __IVSSItem_INTERFACE_DEFINED__
#define __IVSSItem_INTERFACE_DEFINED__

/* interface IVSSItem */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A0DE0E7-2E9F-11D0-9236-00AA00A1EB95")
    IVSSItem : public IVSSItemOld
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Links( 
            /* [retval][out] */ IVSSItems __RPC_FAR *__RPC_FAR *ppIItems) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Branch( 
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSItem __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSItem __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSItem __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IVSSItem __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IVSSItem __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IVSSItem __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IVSSItem __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Spec )( 
            IVSSItem __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pSpec);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Binary )( 
            IVSSItem __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbBinary);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Binary )( 
            IVSSItem __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL pbBinary);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Deleted )( 
            IVSSItem __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDeleted);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Deleted )( 
            IVSSItem __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL pbDeleted);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Type )( 
            IVSSItem __RPC_FAR * This,
            /* [retval][out] */ int __RPC_FAR *piType);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalSpec )( 
            IVSSItem __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pLocal);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LocalSpec )( 
            IVSSItem __RPC_FAR * This,
            /* [in] */ BSTR pLocal);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IVSSItem __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pName);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Name )( 
            IVSSItem __RPC_FAR * This,
            /* [in] */ BSTR pName);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IVSSItem __RPC_FAR * This,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIParent);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VersionNumber )( 
            IVSSItem __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *piVersion);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Items )( 
            IVSSItem __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL IncludeDeleted,
            /* [retval][out] */ IVSSItems __RPC_FAR *__RPC_FAR *ppIItems);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Get )( 
            IVSSItem __RPC_FAR * This,
            /* [optional][out][in] */ BSTR __RPC_FAR *Local,
            /* [defaultvalue][optional][in] */ long iFlags);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Checkout )( 
            IVSSItem __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [defaultvalue][optional][in] */ BSTR Local,
            /* [defaultvalue][optional][in] */ long iFlags);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Checkin )( 
            IVSSItem __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [defaultvalue][optional][in] */ BSTR Local,
            /* [defaultvalue][optional][in] */ long iFlags);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UndoCheckout )( 
            IVSSItem __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR Local,
            /* [defaultvalue][optional][in] */ long iFlags);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsCheckedOut )( 
            IVSSItem __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *piStatus);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Checkouts )( 
            IVSSItem __RPC_FAR * This,
            /* [retval][out] */ IVSSCheckouts __RPC_FAR *__RPC_FAR *ppICheckouts);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsDifferent )( 
            IVSSItem __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR Local,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDifferent);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Add )( 
            IVSSItem __RPC_FAR * This,
            /* [in] */ BSTR Local,
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NewSubproject )( 
            IVSSItem __RPC_FAR * This,
            /* [in] */ BSTR Name,
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Share )( 
            IVSSItem __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [defaultvalue][optional][in] */ long iFlags);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Destroy )( 
            IVSSItem __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Move )( 
            IVSSItem __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pINewParent);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Label )( 
            IVSSItem __RPC_FAR * This,
            /* [in] */ BSTR Label,
            /* [defaultvalue][optional][in] */ BSTR Comment);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Versions )( 
            IVSSItem __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ IVSSVersions __RPC_FAR *__RPC_FAR *pIVersions);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Version )( 
            IVSSItem __RPC_FAR * This,
            /* [optional][in] */ VARIANT Version,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Links )( 
            IVSSItem __RPC_FAR * This,
            /* [retval][out] */ IVSSItems __RPC_FAR *__RPC_FAR *ppIItems);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Branch )( 
            IVSSItem __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR Comment,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);
        
        END_INTERFACE
    } IVSSItemVtbl;

    interface IVSSItem
    {
        CONST_VTBL struct IVSSItemVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSItem_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVSSItem_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVSSItem_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVSSItem_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVSSItem_get_Spec(This,pSpec)	\
    (This)->lpVtbl -> get_Spec(This,pSpec)

#define IVSSItem_get_Binary(This,pbBinary)	\
    (This)->lpVtbl -> get_Binary(This,pbBinary)

#define IVSSItem_put_Binary(This,pbBinary)	\
    (This)->lpVtbl -> put_Binary(This,pbBinary)

#define IVSSItem_get_Deleted(This,pbDeleted)	\
    (This)->lpVtbl -> get_Deleted(This,pbDeleted)

#define IVSSItem_put_Deleted(This,pbDeleted)	\
    (This)->lpVtbl -> put_Deleted(This,pbDeleted)

#define IVSSItem_get_Type(This,piType)	\
    (This)->lpVtbl -> get_Type(This,piType)

#define IVSSItem_get_LocalSpec(This,pLocal)	\
    (This)->lpVtbl -> get_LocalSpec(This,pLocal)

#define IVSSItem_put_LocalSpec(This,pLocal)	\
    (This)->lpVtbl -> put_LocalSpec(This,pLocal)

#define IVSSItem_get_Name(This,pName)	\
    (This)->lpVtbl -> get_Name(This,pName)

#define IVSSItem_put_Name(This,pName)	\
    (This)->lpVtbl -> put_Name(This,pName)

#define IVSSItem_get_Parent(This,ppIParent)	\
    (This)->lpVtbl -> get_Parent(This,ppIParent)

#define IVSSItem_get_VersionNumber(This,piVersion)	\
    (This)->lpVtbl -> get_VersionNumber(This,piVersion)

#define IVSSItem_get_Items(This,IncludeDeleted,ppIItems)	\
    (This)->lpVtbl -> get_Items(This,IncludeDeleted,ppIItems)

#define IVSSItem_Get(This,Local,iFlags)	\
    (This)->lpVtbl -> Get(This,Local,iFlags)

#define IVSSItem_Checkout(This,Comment,Local,iFlags)	\
    (This)->lpVtbl -> Checkout(This,Comment,Local,iFlags)

#define IVSSItem_Checkin(This,Comment,Local,iFlags)	\
    (This)->lpVtbl -> Checkin(This,Comment,Local,iFlags)

#define IVSSItem_UndoCheckout(This,Local,iFlags)	\
    (This)->lpVtbl -> UndoCheckout(This,Local,iFlags)

#define IVSSItem_get_IsCheckedOut(This,piStatus)	\
    (This)->lpVtbl -> get_IsCheckedOut(This,piStatus)

#define IVSSItem_get_Checkouts(This,ppICheckouts)	\
    (This)->lpVtbl -> get_Checkouts(This,ppICheckouts)

#define IVSSItem_get_IsDifferent(This,Local,pbDifferent)	\
    (This)->lpVtbl -> get_IsDifferent(This,Local,pbDifferent)

#define IVSSItem_Add(This,Local,Comment,iFlags,ppIItem)	\
    (This)->lpVtbl -> Add(This,Local,Comment,iFlags,ppIItem)

#define IVSSItem_NewSubproject(This,Name,Comment,ppIItem)	\
    (This)->lpVtbl -> NewSubproject(This,Name,Comment,ppIItem)

#define IVSSItem_Share(This,pIItem,Comment,iFlags)	\
    (This)->lpVtbl -> Share(This,pIItem,Comment,iFlags)

#define IVSSItem_Destroy(This)	\
    (This)->lpVtbl -> Destroy(This)

#define IVSSItem_Move(This,pINewParent)	\
    (This)->lpVtbl -> Move(This,pINewParent)

#define IVSSItem_Label(This,Label,Comment)	\
    (This)->lpVtbl -> Label(This,Label,Comment)

#define IVSSItem_get_Versions(This,iFlags,pIVersions)	\
    (This)->lpVtbl -> get_Versions(This,iFlags,pIVersions)

#define IVSSItem_get_Version(This,Version,ppIItem)	\
    (This)->lpVtbl -> get_Version(This,Version,ppIItem)


#define IVSSItem_get_Links(This,ppIItems)	\
    (This)->lpVtbl -> get_Links(This,ppIItems)

#define IVSSItem_Branch(This,Comment,iFlags,ppIItem)	\
    (This)->lpVtbl -> Branch(This,Comment,iFlags,ppIItem)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItem_get_Links_Proxy( 
    IVSSItem __RPC_FAR * This,
    /* [retval][out] */ IVSSItems __RPC_FAR *__RPC_FAR *ppIItems);


void __RPC_STUB IVSSItem_get_Links_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IVSSItem_Branch_Proxy( 
    IVSSItem __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR Comment,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);


void __RPC_STUB IVSSItem_Branch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSItem_INTERFACE_DEFINED__ */


#ifndef __IVSSVersions_INTERFACE_DEFINED__
#define __IVSSVersions_INTERFACE_DEFINED__

/* interface IVSSVersions */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSVersions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("783CD4E7-9D54-11CF-B8EE-00608CC9A71F")
    IVSSVersions : public IDispatch
    {
    public:
        virtual /* [hidden][restricted][id] */ HRESULT STDMETHODCALLTYPE _NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppIEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSVersionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSVersions __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSVersions __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSVersions __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IVSSVersions __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IVSSVersions __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IVSSVersions __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IVSSVersions __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [hidden][restricted][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *_NewEnum )( 
            IVSSVersions __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppIEnum);
        
        END_INTERFACE
    } IVSSVersionsVtbl;

    interface IVSSVersions
    {
        CONST_VTBL struct IVSSVersionsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSVersions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSVersions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSVersions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSVersions_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVSSVersions_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVSSVersions_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVSSVersions_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVSSVersions__NewEnum(This,ppIEnum)	\
    (This)->lpVtbl -> _NewEnum(This,ppIEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [hidden][restricted][id] */ HRESULT STDMETHODCALLTYPE IVSSVersions__NewEnum_Proxy( 
    IVSSVersions __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppIEnum);


void __RPC_STUB IVSSVersions__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSVersions_INTERFACE_DEFINED__ */


#ifndef __IVSSVersionOld_INTERFACE_DEFINED__
#define __IVSSVersionOld_INTERFACE_DEFINED__

/* interface IVSSVersionOld */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSVersionOld;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("783CD4E8-9D54-11CF-B8EE-00608CC9A71F")
    IVSSVersionOld : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Username( 
            /* [retval][out] */ BSTR __RPC_FAR *pUsername) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_VersionNumber( 
            /* [retval][out] */ long __RPC_FAR *piVersion) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Action( 
            /* [retval][out] */ BSTR __RPC_FAR *pAction) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Date( 
            /* [retval][out] */ DATE __RPC_FAR *pDate) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Comment( 
            /* [retval][out] */ BSTR __RPC_FAR *pComment) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Label( 
            /* [retval][out] */ BSTR __RPC_FAR *pLabel) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_VSSItem( 
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSVersionOldVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSVersionOld __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSVersionOld __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSVersionOld __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IVSSVersionOld __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IVSSVersionOld __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IVSSVersionOld __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IVSSVersionOld __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Username )( 
            IVSSVersionOld __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pUsername);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VersionNumber )( 
            IVSSVersionOld __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *piVersion);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Action )( 
            IVSSVersionOld __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pAction);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Date )( 
            IVSSVersionOld __RPC_FAR * This,
            /* [retval][out] */ DATE __RPC_FAR *pDate);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Comment )( 
            IVSSVersionOld __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pComment);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Label )( 
            IVSSVersionOld __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pLabel);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VSSItem )( 
            IVSSVersionOld __RPC_FAR * This,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);
        
        END_INTERFACE
    } IVSSVersionOldVtbl;

    interface IVSSVersionOld
    {
        CONST_VTBL struct IVSSVersionOldVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSVersionOld_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSVersionOld_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSVersionOld_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSVersionOld_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVSSVersionOld_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVSSVersionOld_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVSSVersionOld_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVSSVersionOld_get_Username(This,pUsername)	\
    (This)->lpVtbl -> get_Username(This,pUsername)

#define IVSSVersionOld_get_VersionNumber(This,piVersion)	\
    (This)->lpVtbl -> get_VersionNumber(This,piVersion)

#define IVSSVersionOld_get_Action(This,pAction)	\
    (This)->lpVtbl -> get_Action(This,pAction)

#define IVSSVersionOld_get_Date(This,pDate)	\
    (This)->lpVtbl -> get_Date(This,pDate)

#define IVSSVersionOld_get_Comment(This,pComment)	\
    (This)->lpVtbl -> get_Comment(This,pComment)

#define IVSSVersionOld_get_Label(This,pLabel)	\
    (This)->lpVtbl -> get_Label(This,pLabel)

#define IVSSVersionOld_get_VSSItem(This,ppIItem)	\
    (This)->lpVtbl -> get_VSSItem(This,ppIItem)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSVersionOld_get_Username_Proxy( 
    IVSSVersionOld __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pUsername);


void __RPC_STUB IVSSVersionOld_get_Username_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSVersionOld_get_VersionNumber_Proxy( 
    IVSSVersionOld __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *piVersion);


void __RPC_STUB IVSSVersionOld_get_VersionNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSVersionOld_get_Action_Proxy( 
    IVSSVersionOld __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pAction);


void __RPC_STUB IVSSVersionOld_get_Action_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSVersionOld_get_Date_Proxy( 
    IVSSVersionOld __RPC_FAR * This,
    /* [retval][out] */ DATE __RPC_FAR *pDate);


void __RPC_STUB IVSSVersionOld_get_Date_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSVersionOld_get_Comment_Proxy( 
    IVSSVersionOld __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pComment);


void __RPC_STUB IVSSVersionOld_get_Comment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSVersionOld_get_Label_Proxy( 
    IVSSVersionOld __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pLabel);


void __RPC_STUB IVSSVersionOld_get_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSVersionOld_get_VSSItem_Proxy( 
    IVSSVersionOld __RPC_FAR * This,
    /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);


void __RPC_STUB IVSSVersionOld_get_VSSItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSVersionOld_INTERFACE_DEFINED__ */


#ifndef __IVSSVersion_INTERFACE_DEFINED__
#define __IVSSVersion_INTERFACE_DEFINED__

/* interface IVSSVersion */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSVersion;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A0DE0E9-2E9F-11D0-9236-00AA00A1EB95")
    IVSSVersion : public IVSSVersionOld
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_LabelComment( 
            /* [retval][out] */ BSTR __RPC_FAR *pComment) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSVersionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSVersion __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSVersion __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSVersion __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IVSSVersion __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IVSSVersion __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IVSSVersion __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IVSSVersion __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Username )( 
            IVSSVersion __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pUsername);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VersionNumber )( 
            IVSSVersion __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *piVersion);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Action )( 
            IVSSVersion __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pAction);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Date )( 
            IVSSVersion __RPC_FAR * This,
            /* [retval][out] */ DATE __RPC_FAR *pDate);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Comment )( 
            IVSSVersion __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pComment);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Label )( 
            IVSSVersion __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pLabel);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VSSItem )( 
            IVSSVersion __RPC_FAR * This,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LabelComment )( 
            IVSSVersion __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pComment);
        
        END_INTERFACE
    } IVSSVersionVtbl;

    interface IVSSVersion
    {
        CONST_VTBL struct IVSSVersionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSVersion_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSVersion_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSVersion_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSVersion_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVSSVersion_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVSSVersion_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVSSVersion_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVSSVersion_get_Username(This,pUsername)	\
    (This)->lpVtbl -> get_Username(This,pUsername)

#define IVSSVersion_get_VersionNumber(This,piVersion)	\
    (This)->lpVtbl -> get_VersionNumber(This,piVersion)

#define IVSSVersion_get_Action(This,pAction)	\
    (This)->lpVtbl -> get_Action(This,pAction)

#define IVSSVersion_get_Date(This,pDate)	\
    (This)->lpVtbl -> get_Date(This,pDate)

#define IVSSVersion_get_Comment(This,pComment)	\
    (This)->lpVtbl -> get_Comment(This,pComment)

#define IVSSVersion_get_Label(This,pLabel)	\
    (This)->lpVtbl -> get_Label(This,pLabel)

#define IVSSVersion_get_VSSItem(This,ppIItem)	\
    (This)->lpVtbl -> get_VSSItem(This,ppIItem)


#define IVSSVersion_get_LabelComment(This,pComment)	\
    (This)->lpVtbl -> get_LabelComment(This,pComment)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSVersion_get_LabelComment_Proxy( 
    IVSSVersion __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pComment);


void __RPC_STUB IVSSVersion_get_LabelComment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSVersion_INTERFACE_DEFINED__ */


#ifndef __IVSSItems_INTERFACE_DEFINED__
#define __IVSSItems_INTERFACE_DEFINED__

/* interface IVSSItems */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSItems;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("783CD4E5-9D54-11CF-B8EE-00608CC9A71F")
    IVSSItems : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *piCount) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ VARIANT sItem,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem) = 0;
        
        virtual /* [hidden][restricted][id] */ HRESULT STDMETHODCALLTYPE _NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppIEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSItemsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSItems __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSItems __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSItems __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IVSSItems __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IVSSItems __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IVSSItems __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IVSSItems __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IVSSItems __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *piCount);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Item )( 
            IVSSItems __RPC_FAR * This,
            /* [in] */ VARIANT sItem,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);
        
        /* [hidden][restricted][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *_NewEnum )( 
            IVSSItems __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppIEnum);
        
        END_INTERFACE
    } IVSSItemsVtbl;

    interface IVSSItems
    {
        CONST_VTBL struct IVSSItemsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSItems_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSItems_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSItems_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSItems_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVSSItems_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVSSItems_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVSSItems_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVSSItems_get_Count(This,piCount)	\
    (This)->lpVtbl -> get_Count(This,piCount)

#define IVSSItems_get_Item(This,sItem,ppIItem)	\
    (This)->lpVtbl -> get_Item(This,sItem,ppIItem)

#define IVSSItems__NewEnum(This,ppIEnum)	\
    (This)->lpVtbl -> _NewEnum(This,ppIEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItems_get_Count_Proxy( 
    IVSSItems __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *piCount);


void __RPC_STUB IVSSItems_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSItems_get_Item_Proxy( 
    IVSSItems __RPC_FAR * This,
    /* [in] */ VARIANT sItem,
    /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIItem);


void __RPC_STUB IVSSItems_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][restricted][id] */ HRESULT STDMETHODCALLTYPE IVSSItems__NewEnum_Proxy( 
    IVSSItems __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppIEnum);


void __RPC_STUB IVSSItems__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSItems_INTERFACE_DEFINED__ */


#ifndef __IVSSCheckouts_INTERFACE_DEFINED__
#define __IVSSCheckouts_INTERFACE_DEFINED__

/* interface IVSSCheckouts */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSCheckouts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8903A770-F55F-11CF-9227-00AA00A1EB95")
    IVSSCheckouts : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *piCount) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ VARIANT sItem,
            /* [retval][out] */ IVSSCheckout __RPC_FAR *__RPC_FAR *ppICheckout) = 0;
        
        virtual /* [hidden][restricted][id] */ HRESULT STDMETHODCALLTYPE _NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppIEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSCheckoutsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSCheckouts __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSCheckouts __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSCheckouts __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IVSSCheckouts __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IVSSCheckouts __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IVSSCheckouts __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IVSSCheckouts __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IVSSCheckouts __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *piCount);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Item )( 
            IVSSCheckouts __RPC_FAR * This,
            /* [in] */ VARIANT sItem,
            /* [retval][out] */ IVSSCheckout __RPC_FAR *__RPC_FAR *ppICheckout);
        
        /* [hidden][restricted][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *_NewEnum )( 
            IVSSCheckouts __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppIEnum);
        
        END_INTERFACE
    } IVSSCheckoutsVtbl;

    interface IVSSCheckouts
    {
        CONST_VTBL struct IVSSCheckoutsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSCheckouts_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSCheckouts_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSCheckouts_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSCheckouts_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVSSCheckouts_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVSSCheckouts_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVSSCheckouts_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVSSCheckouts_get_Count(This,piCount)	\
    (This)->lpVtbl -> get_Count(This,piCount)

#define IVSSCheckouts_get_Item(This,sItem,ppICheckout)	\
    (This)->lpVtbl -> get_Item(This,sItem,ppICheckout)

#define IVSSCheckouts__NewEnum(This,ppIEnum)	\
    (This)->lpVtbl -> _NewEnum(This,ppIEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSCheckouts_get_Count_Proxy( 
    IVSSCheckouts __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *piCount);


void __RPC_STUB IVSSCheckouts_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSCheckouts_get_Item_Proxy( 
    IVSSCheckouts __RPC_FAR * This,
    /* [in] */ VARIANT sItem,
    /* [retval][out] */ IVSSCheckout __RPC_FAR *__RPC_FAR *ppICheckout);


void __RPC_STUB IVSSCheckouts_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][restricted][id] */ HRESULT STDMETHODCALLTYPE IVSSCheckouts__NewEnum_Proxy( 
    IVSSCheckouts __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppIEnum);


void __RPC_STUB IVSSCheckouts__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSCheckouts_INTERFACE_DEFINED__ */


#ifndef __IVSSCheckout_INTERFACE_DEFINED__
#define __IVSSCheckout_INTERFACE_DEFINED__

/* interface IVSSCheckout */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSCheckout;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("783CD4E6-9D54-11CF-B8EE-00608CC9A71F")
    IVSSCheckout : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Username( 
            /* [retval][out] */ BSTR __RPC_FAR *pUsername) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Date( 
            /* [retval][out] */ DATE __RPC_FAR *pDate) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_LocalSpec( 
            /* [retval][out] */ BSTR __RPC_FAR *pLocal) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Machine( 
            /* [retval][out] */ BSTR __RPC_FAR *pMachine) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Project( 
            /* [retval][out] */ BSTR __RPC_FAR *pProject) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Comment( 
            /* [retval][out] */ BSTR __RPC_FAR *pComment) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_VersionNumber( 
            /* [retval][out] */ long __RPC_FAR *piVersion) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSCheckoutVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSCheckout __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSCheckout __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSCheckout __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IVSSCheckout __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IVSSCheckout __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IVSSCheckout __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IVSSCheckout __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Username )( 
            IVSSCheckout __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pUsername);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Date )( 
            IVSSCheckout __RPC_FAR * This,
            /* [retval][out] */ DATE __RPC_FAR *pDate);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalSpec )( 
            IVSSCheckout __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pLocal);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Machine )( 
            IVSSCheckout __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pMachine);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Project )( 
            IVSSCheckout __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pProject);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Comment )( 
            IVSSCheckout __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pComment);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VersionNumber )( 
            IVSSCheckout __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *piVersion);
        
        END_INTERFACE
    } IVSSCheckoutVtbl;

    interface IVSSCheckout
    {
        CONST_VTBL struct IVSSCheckoutVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSCheckout_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSCheckout_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSCheckout_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSCheckout_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVSSCheckout_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVSSCheckout_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVSSCheckout_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVSSCheckout_get_Username(This,pUsername)	\
    (This)->lpVtbl -> get_Username(This,pUsername)

#define IVSSCheckout_get_Date(This,pDate)	\
    (This)->lpVtbl -> get_Date(This,pDate)

#define IVSSCheckout_get_LocalSpec(This,pLocal)	\
    (This)->lpVtbl -> get_LocalSpec(This,pLocal)

#define IVSSCheckout_get_Machine(This,pMachine)	\
    (This)->lpVtbl -> get_Machine(This,pMachine)

#define IVSSCheckout_get_Project(This,pProject)	\
    (This)->lpVtbl -> get_Project(This,pProject)

#define IVSSCheckout_get_Comment(This,pComment)	\
    (This)->lpVtbl -> get_Comment(This,pComment)

#define IVSSCheckout_get_VersionNumber(This,piVersion)	\
    (This)->lpVtbl -> get_VersionNumber(This,piVersion)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSCheckout_get_Username_Proxy( 
    IVSSCheckout __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pUsername);


void __RPC_STUB IVSSCheckout_get_Username_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSCheckout_get_Date_Proxy( 
    IVSSCheckout __RPC_FAR * This,
    /* [retval][out] */ DATE __RPC_FAR *pDate);


void __RPC_STUB IVSSCheckout_get_Date_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSCheckout_get_LocalSpec_Proxy( 
    IVSSCheckout __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pLocal);


void __RPC_STUB IVSSCheckout_get_LocalSpec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSCheckout_get_Machine_Proxy( 
    IVSSCheckout __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pMachine);


void __RPC_STUB IVSSCheckout_get_Machine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSCheckout_get_Project_Proxy( 
    IVSSCheckout __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pProject);


void __RPC_STUB IVSSCheckout_get_Project_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSCheckout_get_Comment_Proxy( 
    IVSSCheckout __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pComment);


void __RPC_STUB IVSSCheckout_get_Comment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSCheckout_get_VersionNumber_Proxy( 
    IVSSCheckout __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *piVersion);


void __RPC_STUB IVSSCheckout_get_VersionNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSCheckout_INTERFACE_DEFINED__ */


#ifndef __IVSSDatabaseOld_INTERFACE_DEFINED__
#define __IVSSDatabaseOld_INTERFACE_DEFINED__

/* interface IVSSDatabaseOld */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSDatabaseOld;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("783CD4E2-9D54-11CF-B8EE-00608CC9A71F")
    IVSSDatabaseOld : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Open( 
            /* [defaultvalue][optional][in] */ BSTR SrcSafeIni = L"",
            /* [defaultvalue][optional][in] */ BSTR Username = L"",
            /* [defaultvalue][optional][in] */ BSTR Password = L"") = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_SrcSafeIni( 
            /* [retval][out] */ BSTR __RPC_FAR *pSrcSafeIni) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_DatabaseName( 
            /* [retval][out] */ BSTR __RPC_FAR *pDatabaseName) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Username( 
            /* [retval][out] */ BSTR __RPC_FAR *pUsername) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_CurrentProject( 
            /* [retval][out] */ BSTR __RPC_FAR *pPrj) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_CurrentProject( 
            /* [in] */ BSTR pPrj) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_VSSItem( 
            /* [in] */ BSTR Spec,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL Deleted,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIVSSItem) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSDatabaseOldVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSDatabaseOld __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSDatabaseOld __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSDatabaseOld __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IVSSDatabaseOld __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IVSSDatabaseOld __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IVSSDatabaseOld __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IVSSDatabaseOld __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            IVSSDatabaseOld __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR SrcSafeIni,
            /* [defaultvalue][optional][in] */ BSTR Username,
            /* [defaultvalue][optional][in] */ BSTR Password);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SrcSafeIni )( 
            IVSSDatabaseOld __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pSrcSafeIni);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DatabaseName )( 
            IVSSDatabaseOld __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pDatabaseName);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Username )( 
            IVSSDatabaseOld __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pUsername);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CurrentProject )( 
            IVSSDatabaseOld __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pPrj);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CurrentProject )( 
            IVSSDatabaseOld __RPC_FAR * This,
            /* [in] */ BSTR pPrj);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VSSItem )( 
            IVSSDatabaseOld __RPC_FAR * This,
            /* [in] */ BSTR Spec,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL Deleted,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIVSSItem);
        
        END_INTERFACE
    } IVSSDatabaseOldVtbl;

    interface IVSSDatabaseOld
    {
        CONST_VTBL struct IVSSDatabaseOldVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSDatabaseOld_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSDatabaseOld_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSDatabaseOld_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSDatabaseOld_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVSSDatabaseOld_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVSSDatabaseOld_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVSSDatabaseOld_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVSSDatabaseOld_Open(This,SrcSafeIni,Username,Password)	\
    (This)->lpVtbl -> Open(This,SrcSafeIni,Username,Password)

#define IVSSDatabaseOld_get_SrcSafeIni(This,pSrcSafeIni)	\
    (This)->lpVtbl -> get_SrcSafeIni(This,pSrcSafeIni)

#define IVSSDatabaseOld_get_DatabaseName(This,pDatabaseName)	\
    (This)->lpVtbl -> get_DatabaseName(This,pDatabaseName)

#define IVSSDatabaseOld_get_Username(This,pUsername)	\
    (This)->lpVtbl -> get_Username(This,pUsername)

#define IVSSDatabaseOld_get_CurrentProject(This,pPrj)	\
    (This)->lpVtbl -> get_CurrentProject(This,pPrj)

#define IVSSDatabaseOld_put_CurrentProject(This,pPrj)	\
    (This)->lpVtbl -> put_CurrentProject(This,pPrj)

#define IVSSDatabaseOld_get_VSSItem(This,Spec,Deleted,ppIVSSItem)	\
    (This)->lpVtbl -> get_VSSItem(This,Spec,Deleted,ppIVSSItem)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT STDMETHODCALLTYPE IVSSDatabaseOld_Open_Proxy( 
    IVSSDatabaseOld __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR SrcSafeIni,
    /* [defaultvalue][optional][in] */ BSTR Username,
    /* [defaultvalue][optional][in] */ BSTR Password);


void __RPC_STUB IVSSDatabaseOld_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSDatabaseOld_get_SrcSafeIni_Proxy( 
    IVSSDatabaseOld __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pSrcSafeIni);


void __RPC_STUB IVSSDatabaseOld_get_SrcSafeIni_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSDatabaseOld_get_DatabaseName_Proxy( 
    IVSSDatabaseOld __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pDatabaseName);


void __RPC_STUB IVSSDatabaseOld_get_DatabaseName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSDatabaseOld_get_Username_Proxy( 
    IVSSDatabaseOld __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pUsername);


void __RPC_STUB IVSSDatabaseOld_get_Username_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSDatabaseOld_get_CurrentProject_Proxy( 
    IVSSDatabaseOld __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pPrj);


void __RPC_STUB IVSSDatabaseOld_get_CurrentProject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IVSSDatabaseOld_put_CurrentProject_Proxy( 
    IVSSDatabaseOld __RPC_FAR * This,
    /* [in] */ BSTR pPrj);


void __RPC_STUB IVSSDatabaseOld_put_CurrentProject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSDatabaseOld_get_VSSItem_Proxy( 
    IVSSDatabaseOld __RPC_FAR * This,
    /* [in] */ BSTR Spec,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL Deleted,
    /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIVSSItem);


void __RPC_STUB IVSSDatabaseOld_get_VSSItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSDatabaseOld_INTERFACE_DEFINED__ */


#ifndef __IVSSDatabase_INTERFACE_DEFINED__
#define __IVSSDatabase_INTERFACE_DEFINED__

/* interface IVSSDatabase */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSDatabase;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A0DE0E2-2E9F-11D0-9236-00AA00A1EB95")
    IVSSDatabase : public IVSSDatabaseOld
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE AddUser( 
            /* [in] */ BSTR User,
            /* [in] */ BSTR Password,
            /* [in] */ VARIANT_BOOL ReadOnly,
            /* [retval][out] */ IVSSUser __RPC_FAR *__RPC_FAR *ppIUser) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_User( 
            /* [in] */ BSTR Name,
            /* [retval][out] */ IVSSUser __RPC_FAR *__RPC_FAR *ppIUser) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Users( 
            /* [retval][out] */ IVSSUsers __RPC_FAR *__RPC_FAR *ppIUsers) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ProjectRightsEnabled( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pEnabled) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_ProjectRightsEnabled( 
            /* [in] */ VARIANT_BOOL pEnabled) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_DefaultProjectRights( 
            /* [retval][out] */ long __RPC_FAR *pRights) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_DefaultProjectRights( 
            /* [in] */ long pRights) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSDatabaseVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSDatabase __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSDatabase __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSDatabase __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IVSSDatabase __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IVSSDatabase __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IVSSDatabase __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IVSSDatabase __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            IVSSDatabase __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR SrcSafeIni,
            /* [defaultvalue][optional][in] */ BSTR Username,
            /* [defaultvalue][optional][in] */ BSTR Password);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SrcSafeIni )( 
            IVSSDatabase __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pSrcSafeIni);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DatabaseName )( 
            IVSSDatabase __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pDatabaseName);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Username )( 
            IVSSDatabase __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pUsername);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CurrentProject )( 
            IVSSDatabase __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pPrj);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CurrentProject )( 
            IVSSDatabase __RPC_FAR * This,
            /* [in] */ BSTR pPrj);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VSSItem )( 
            IVSSDatabase __RPC_FAR * This,
            /* [in] */ BSTR Spec,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL Deleted,
            /* [retval][out] */ IVSSItem __RPC_FAR *__RPC_FAR *ppIVSSItem);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddUser )( 
            IVSSDatabase __RPC_FAR * This,
            /* [in] */ BSTR User,
            /* [in] */ BSTR Password,
            /* [in] */ VARIANT_BOOL ReadOnly,
            /* [retval][out] */ IVSSUser __RPC_FAR *__RPC_FAR *ppIUser);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_User )( 
            IVSSDatabase __RPC_FAR * This,
            /* [in] */ BSTR Name,
            /* [retval][out] */ IVSSUser __RPC_FAR *__RPC_FAR *ppIUser);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Users )( 
            IVSSDatabase __RPC_FAR * This,
            /* [retval][out] */ IVSSUsers __RPC_FAR *__RPC_FAR *ppIUsers);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProjectRightsEnabled )( 
            IVSSDatabase __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pEnabled);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ProjectRightsEnabled )( 
            IVSSDatabase __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL pEnabled);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DefaultProjectRights )( 
            IVSSDatabase __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pRights);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DefaultProjectRights )( 
            IVSSDatabase __RPC_FAR * This,
            /* [in] */ long pRights);
        
        END_INTERFACE
    } IVSSDatabaseVtbl;

    interface IVSSDatabase
    {
        CONST_VTBL struct IVSSDatabaseVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSDatabase_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSDatabase_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSDatabase_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSDatabase_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVSSDatabase_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVSSDatabase_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVSSDatabase_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVSSDatabase_Open(This,SrcSafeIni,Username,Password)	\
    (This)->lpVtbl -> Open(This,SrcSafeIni,Username,Password)

#define IVSSDatabase_get_SrcSafeIni(This,pSrcSafeIni)	\
    (This)->lpVtbl -> get_SrcSafeIni(This,pSrcSafeIni)

#define IVSSDatabase_get_DatabaseName(This,pDatabaseName)	\
    (This)->lpVtbl -> get_DatabaseName(This,pDatabaseName)

#define IVSSDatabase_get_Username(This,pUsername)	\
    (This)->lpVtbl -> get_Username(This,pUsername)

#define IVSSDatabase_get_CurrentProject(This,pPrj)	\
    (This)->lpVtbl -> get_CurrentProject(This,pPrj)

#define IVSSDatabase_put_CurrentProject(This,pPrj)	\
    (This)->lpVtbl -> put_CurrentProject(This,pPrj)

#define IVSSDatabase_get_VSSItem(This,Spec,Deleted,ppIVSSItem)	\
    (This)->lpVtbl -> get_VSSItem(This,Spec,Deleted,ppIVSSItem)


#define IVSSDatabase_AddUser(This,User,Password,ReadOnly,ppIUser)	\
    (This)->lpVtbl -> AddUser(This,User,Password,ReadOnly,ppIUser)

#define IVSSDatabase_get_User(This,Name,ppIUser)	\
    (This)->lpVtbl -> get_User(This,Name,ppIUser)

#define IVSSDatabase_get_Users(This,ppIUsers)	\
    (This)->lpVtbl -> get_Users(This,ppIUsers)

#define IVSSDatabase_get_ProjectRightsEnabled(This,pEnabled)	\
    (This)->lpVtbl -> get_ProjectRightsEnabled(This,pEnabled)

#define IVSSDatabase_put_ProjectRightsEnabled(This,pEnabled)	\
    (This)->lpVtbl -> put_ProjectRightsEnabled(This,pEnabled)

#define IVSSDatabase_get_DefaultProjectRights(This,pRights)	\
    (This)->lpVtbl -> get_DefaultProjectRights(This,pRights)

#define IVSSDatabase_put_DefaultProjectRights(This,pRights)	\
    (This)->lpVtbl -> put_DefaultProjectRights(This,pRights)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT STDMETHODCALLTYPE IVSSDatabase_AddUser_Proxy( 
    IVSSDatabase __RPC_FAR * This,
    /* [in] */ BSTR User,
    /* [in] */ BSTR Password,
    /* [in] */ VARIANT_BOOL ReadOnly,
    /* [retval][out] */ IVSSUser __RPC_FAR *__RPC_FAR *ppIUser);


void __RPC_STUB IVSSDatabase_AddUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSDatabase_get_User_Proxy( 
    IVSSDatabase __RPC_FAR * This,
    /* [in] */ BSTR Name,
    /* [retval][out] */ IVSSUser __RPC_FAR *__RPC_FAR *ppIUser);


void __RPC_STUB IVSSDatabase_get_User_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSDatabase_get_Users_Proxy( 
    IVSSDatabase __RPC_FAR * This,
    /* [retval][out] */ IVSSUsers __RPC_FAR *__RPC_FAR *ppIUsers);


void __RPC_STUB IVSSDatabase_get_Users_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSDatabase_get_ProjectRightsEnabled_Proxy( 
    IVSSDatabase __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pEnabled);


void __RPC_STUB IVSSDatabase_get_ProjectRightsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IVSSDatabase_put_ProjectRightsEnabled_Proxy( 
    IVSSDatabase __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL pEnabled);


void __RPC_STUB IVSSDatabase_put_ProjectRightsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSDatabase_get_DefaultProjectRights_Proxy( 
    IVSSDatabase __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pRights);


void __RPC_STUB IVSSDatabase_get_DefaultProjectRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IVSSDatabase_put_DefaultProjectRights_Proxy( 
    IVSSDatabase __RPC_FAR * This,
    /* [in] */ long pRights);


void __RPC_STUB IVSSDatabase_put_DefaultProjectRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSDatabase_INTERFACE_DEFINED__ */


#ifndef __IVSSUser_INTERFACE_DEFINED__
#define __IVSSUser_INTERFACE_DEFINED__

/* interface IVSSUser */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSUser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A0DE0E3-2E9F-11D0-9236-00AA00A1EB95")
    IVSSUser : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR __RPC_FAR *pName) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pName) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Password( 
            /* [in] */ BSTR rhs) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ReadOnly( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pReadOnly) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_ReadOnly( 
            /* [in] */ VARIANT_BOOL pReadOnly) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ProjectRights( 
            /* [defaultvalue][optional][in] */ BSTR Project,
            /* [retval][out] */ long __RPC_FAR *piRightsOut) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_ProjectRights( 
            /* [defaultvalue][optional][in] */ BSTR Project,
            /* [in] */ long piRightsOut) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE RemoveProjectRights( 
            /* [in] */ BSTR Project) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSUserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSUser __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSUser __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSUser __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IVSSUser __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IVSSUser __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IVSSUser __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IVSSUser __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Delete )( 
            IVSSUser __RPC_FAR * This);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IVSSUser __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pName);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Name )( 
            IVSSUser __RPC_FAR * This,
            /* [in] */ BSTR pName);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Password )( 
            IVSSUser __RPC_FAR * This,
            /* [in] */ BSTR rhs);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ReadOnly )( 
            IVSSUser __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pReadOnly);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ReadOnly )( 
            IVSSUser __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL pReadOnly);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProjectRights )( 
            IVSSUser __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR Project,
            /* [retval][out] */ long __RPC_FAR *piRightsOut);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ProjectRights )( 
            IVSSUser __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR Project,
            /* [in] */ long piRightsOut);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveProjectRights )( 
            IVSSUser __RPC_FAR * This,
            /* [in] */ BSTR Project);
        
        END_INTERFACE
    } IVSSUserVtbl;

    interface IVSSUser
    {
        CONST_VTBL struct IVSSUserVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSUser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSUser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSUser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSUser_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVSSUser_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVSSUser_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVSSUser_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVSSUser_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define IVSSUser_get_Name(This,pName)	\
    (This)->lpVtbl -> get_Name(This,pName)

#define IVSSUser_put_Name(This,pName)	\
    (This)->lpVtbl -> put_Name(This,pName)

#define IVSSUser_put_Password(This,rhs)	\
    (This)->lpVtbl -> put_Password(This,rhs)

#define IVSSUser_get_ReadOnly(This,pReadOnly)	\
    (This)->lpVtbl -> get_ReadOnly(This,pReadOnly)

#define IVSSUser_put_ReadOnly(This,pReadOnly)	\
    (This)->lpVtbl -> put_ReadOnly(This,pReadOnly)

#define IVSSUser_get_ProjectRights(This,Project,piRightsOut)	\
    (This)->lpVtbl -> get_ProjectRights(This,Project,piRightsOut)

#define IVSSUser_put_ProjectRights(This,Project,piRightsOut)	\
    (This)->lpVtbl -> put_ProjectRights(This,Project,piRightsOut)

#define IVSSUser_RemoveProjectRights(This,Project)	\
    (This)->lpVtbl -> RemoveProjectRights(This,Project)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT STDMETHODCALLTYPE IVSSUser_Delete_Proxy( 
    IVSSUser __RPC_FAR * This);


void __RPC_STUB IVSSUser_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSUser_get_Name_Proxy( 
    IVSSUser __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pName);


void __RPC_STUB IVSSUser_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IVSSUser_put_Name_Proxy( 
    IVSSUser __RPC_FAR * This,
    /* [in] */ BSTR pName);


void __RPC_STUB IVSSUser_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IVSSUser_put_Password_Proxy( 
    IVSSUser __RPC_FAR * This,
    /* [in] */ BSTR rhs);


void __RPC_STUB IVSSUser_put_Password_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSUser_get_ReadOnly_Proxy( 
    IVSSUser __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pReadOnly);


void __RPC_STUB IVSSUser_get_ReadOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IVSSUser_put_ReadOnly_Proxy( 
    IVSSUser __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL pReadOnly);


void __RPC_STUB IVSSUser_put_ReadOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSUser_get_ProjectRights_Proxy( 
    IVSSUser __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR Project,
    /* [retval][out] */ long __RPC_FAR *piRightsOut);


void __RPC_STUB IVSSUser_get_ProjectRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IVSSUser_put_ProjectRights_Proxy( 
    IVSSUser __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR Project,
    /* [in] */ long piRightsOut);


void __RPC_STUB IVSSUser_put_ProjectRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IVSSUser_RemoveProjectRights_Proxy( 
    IVSSUser __RPC_FAR * This,
    /* [in] */ BSTR Project);


void __RPC_STUB IVSSUser_RemoveProjectRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSUser_INTERFACE_DEFINED__ */


#ifndef __IVSSUsers_INTERFACE_DEFINED__
#define __IVSSUsers_INTERFACE_DEFINED__

/* interface IVSSUsers */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSUsers;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A0DE0E4-2E9F-11D0-9236-00AA00A1EB95")
    IVSSUsers : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *piCount) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ VARIANT sItem,
            /* [retval][out] */ IVSSUser __RPC_FAR *__RPC_FAR *ppIUser) = 0;
        
        virtual /* [hidden][restricted][id] */ HRESULT STDMETHODCALLTYPE _NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppIEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSUsersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSUsers __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSUsers __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSUsers __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IVSSUsers __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IVSSUsers __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IVSSUsers __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IVSSUsers __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IVSSUsers __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *piCount);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Item )( 
            IVSSUsers __RPC_FAR * This,
            /* [in] */ VARIANT sItem,
            /* [retval][out] */ IVSSUser __RPC_FAR *__RPC_FAR *ppIUser);
        
        /* [hidden][restricted][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *_NewEnum )( 
            IVSSUsers __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppIEnum);
        
        END_INTERFACE
    } IVSSUsersVtbl;

    interface IVSSUsers
    {
        CONST_VTBL struct IVSSUsersVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSUsers_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSUsers_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSUsers_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSUsers_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVSSUsers_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVSSUsers_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVSSUsers_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVSSUsers_get_Count(This,piCount)	\
    (This)->lpVtbl -> get_Count(This,piCount)

#define IVSSUsers_get_Item(This,sItem,ppIUser)	\
    (This)->lpVtbl -> get_Item(This,sItem,ppIUser)

#define IVSSUsers__NewEnum(This,ppIEnum)	\
    (This)->lpVtbl -> _NewEnum(This,ppIEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSUsers_get_Count_Proxy( 
    IVSSUsers __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *piCount);


void __RPC_STUB IVSSUsers_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSSUsers_get_Item_Proxy( 
    IVSSUsers __RPC_FAR * This,
    /* [in] */ VARIANT sItem,
    /* [retval][out] */ IVSSUser __RPC_FAR *__RPC_FAR *ppIUser);


void __RPC_STUB IVSSUsers_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][restricted][id] */ HRESULT STDMETHODCALLTYPE IVSSUsers__NewEnum_Proxy( 
    IVSSUsers __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppIEnum);


void __RPC_STUB IVSSUsers__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSUsers_INTERFACE_DEFINED__ */


#ifndef __IVSSEventsOld_INTERFACE_DEFINED__
#define __IVSSEventsOld_INTERFACE_DEFINED__

/* interface IVSSEventsOld */
/* [object][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSEventsOld;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("783CD4E9-9D54-11CF-B8EE-00608CC9A71F")
    IVSSEventsOld : public IUnknown
    {
    public:
        virtual HRESULT __stdcall BeforeAdd( 
            /* [in] */ IVSSItem __RPC_FAR *pIPrj,
            /* [in] */ BSTR Local,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterAdd( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local) = 0;
        
        virtual HRESULT __stdcall BeforeCheckout( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterCheckout( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local) = 0;
        
        virtual HRESULT __stdcall BeforeCheckin( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterCheckin( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local) = 0;
        
        virtual HRESULT __stdcall BeforeUndoCheckout( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterUndoCheckout( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local) = 0;
        
        virtual HRESULT __stdcall BeforeRename( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR NewName,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterRename( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR OldName) = 0;
        
        virtual HRESULT __stdcall BeforeBranch( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterBranch( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem) = 0;
        
        virtual HRESULT __stdcall BeforeEvent( 
            /* [in] */ long iEvent,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Str,
            /* [in] */ VARIANT var,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterEvent( 
            /* [in] */ long iEvent,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Str,
            /* [in] */ VARIANT var) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSEventsOldVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSEventsOld __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSEventsOld __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeAdd )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIPrj,
            /* [in] */ BSTR Local,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterAdd )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeCheckout )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterCheckout )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeCheckin )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterCheckin )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeUndoCheckout )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterUndoCheckout )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeRename )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR NewName,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterRename )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR OldName);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeBranch )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterBranch )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeEvent )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ long iEvent,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Str,
            /* [in] */ VARIANT var,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterEvent )( 
            IVSSEventsOld __RPC_FAR * This,
            /* [in] */ long iEvent,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Str,
            /* [in] */ VARIANT var);
        
        END_INTERFACE
    } IVSSEventsOldVtbl;

    interface IVSSEventsOld
    {
        CONST_VTBL struct IVSSEventsOldVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSEventsOld_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSEventsOld_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSEventsOld_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSEventsOld_BeforeAdd(This,pIPrj,Local,pbContinue)	\
    (This)->lpVtbl -> BeforeAdd(This,pIPrj,Local,pbContinue)

#define IVSSEventsOld_AfterAdd(This,pIItem,Local)	\
    (This)->lpVtbl -> AfterAdd(This,pIItem,Local)

#define IVSSEventsOld_BeforeCheckout(This,pIItem,Local,pbContinue)	\
    (This)->lpVtbl -> BeforeCheckout(This,pIItem,Local,pbContinue)

#define IVSSEventsOld_AfterCheckout(This,pIItem,Local)	\
    (This)->lpVtbl -> AfterCheckout(This,pIItem,Local)

#define IVSSEventsOld_BeforeCheckin(This,pIItem,Local,pbContinue)	\
    (This)->lpVtbl -> BeforeCheckin(This,pIItem,Local,pbContinue)

#define IVSSEventsOld_AfterCheckin(This,pIItem,Local)	\
    (This)->lpVtbl -> AfterCheckin(This,pIItem,Local)

#define IVSSEventsOld_BeforeUndoCheckout(This,pIItem,Local,pbContinue)	\
    (This)->lpVtbl -> BeforeUndoCheckout(This,pIItem,Local,pbContinue)

#define IVSSEventsOld_AfterUndoCheckout(This,pIItem,Local)	\
    (This)->lpVtbl -> AfterUndoCheckout(This,pIItem,Local)

#define IVSSEventsOld_BeforeRename(This,pIItem,NewName,pbContinue)	\
    (This)->lpVtbl -> BeforeRename(This,pIItem,NewName,pbContinue)

#define IVSSEventsOld_AfterRename(This,pIItem,OldName)	\
    (This)->lpVtbl -> AfterRename(This,pIItem,OldName)

#define IVSSEventsOld_BeforeBranch(This,pIItem,pbContinue)	\
    (This)->lpVtbl -> BeforeBranch(This,pIItem,pbContinue)

#define IVSSEventsOld_AfterBranch(This,pIItem)	\
    (This)->lpVtbl -> AfterBranch(This,pIItem)

#define IVSSEventsOld_BeforeEvent(This,iEvent,pIItem,Str,var,pbContinue)	\
    (This)->lpVtbl -> BeforeEvent(This,iEvent,pIItem,Str,var,pbContinue)

#define IVSSEventsOld_AfterEvent(This,iEvent,pIItem,Str,var)	\
    (This)->lpVtbl -> AfterEvent(This,iEvent,pIItem,Str,var)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IVSSEventsOld_BeforeAdd_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIPrj,
    /* [in] */ BSTR Local,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEventsOld_BeforeAdd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEventsOld_AfterAdd_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local);


void __RPC_STUB IVSSEventsOld_AfterAdd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEventsOld_BeforeCheckout_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEventsOld_BeforeCheckout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEventsOld_AfterCheckout_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local);


void __RPC_STUB IVSSEventsOld_AfterCheckout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEventsOld_BeforeCheckin_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEventsOld_BeforeCheckin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEventsOld_AfterCheckin_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local);


void __RPC_STUB IVSSEventsOld_AfterCheckin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEventsOld_BeforeUndoCheckout_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEventsOld_BeforeUndoCheckout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEventsOld_AfterUndoCheckout_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local);


void __RPC_STUB IVSSEventsOld_AfterUndoCheckout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEventsOld_BeforeRename_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR NewName,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEventsOld_BeforeRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEventsOld_AfterRename_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR OldName);


void __RPC_STUB IVSSEventsOld_AfterRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEventsOld_BeforeBranch_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEventsOld_BeforeBranch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEventsOld_AfterBranch_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem);


void __RPC_STUB IVSSEventsOld_AfterBranch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEventsOld_BeforeEvent_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ long iEvent,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Str,
    /* [in] */ VARIANT var,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEventsOld_BeforeEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEventsOld_AfterEvent_Proxy( 
    IVSSEventsOld __RPC_FAR * This,
    /* [in] */ long iEvent,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Str,
    /* [in] */ VARIANT var);


void __RPC_STUB IVSSEventsOld_AfterEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSEventsOld_INTERFACE_DEFINED__ */


#ifndef __IVSSEvents_INTERFACE_DEFINED__
#define __IVSSEvents_INTERFACE_DEFINED__

/* interface IVSSEvents */
/* [object][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A0DE0E8-2E9F-11D0-9236-00AA00A1EB95")
    IVSSEvents : public IUnknown
    {
    public:
        virtual HRESULT __stdcall BeforeAdd( 
            /* [in] */ IVSSItem __RPC_FAR *pIPrj,
            /* [in] */ BSTR Local,
            /* [in] */ BSTR Comment,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterAdd( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [in] */ BSTR Comment) = 0;
        
        virtual HRESULT __stdcall BeforeCheckout( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [in] */ BSTR Comment,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterCheckout( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [in] */ BSTR Comment) = 0;
        
        virtual HRESULT __stdcall BeforeCheckin( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [in] */ BSTR Comment,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterCheckin( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [in] */ BSTR Comment) = 0;
        
        virtual HRESULT __stdcall BeforeUndoCheckout( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterUndoCheckout( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local) = 0;
        
        virtual HRESULT __stdcall BeforeRename( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR NewName,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterRename( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR OldName) = 0;
        
        virtual HRESULT __stdcall BeforeBranch( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Comment,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterBranch( 
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Comment) = 0;
        
        virtual HRESULT __stdcall BeginCommand( 
            /* [in] */ long unused,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall EndCommand( 
            /* [in] */ long unused) = 0;
        
        virtual HRESULT __stdcall BeforeEvent( 
            /* [in] */ long iEvent,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Str,
            /* [in] */ VARIANT var,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue) = 0;
        
        virtual HRESULT __stdcall AfterEvent( 
            /* [in] */ long iEvent,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Str,
            /* [in] */ VARIANT var) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSEvents __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeAdd )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIPrj,
            /* [in] */ BSTR Local,
            /* [in] */ BSTR Comment,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterAdd )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [in] */ BSTR Comment);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeCheckout )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [in] */ BSTR Comment,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterCheckout )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [in] */ BSTR Comment);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeCheckin )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [in] */ BSTR Comment,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterCheckin )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [in] */ BSTR Comment);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeUndoCheckout )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterUndoCheckout )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Local);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeRename )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR NewName,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterRename )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR OldName);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeBranch )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Comment,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterBranch )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Comment);
        
        HRESULT ( __stdcall __RPC_FAR *BeginCommand )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ long unused,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *EndCommand )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ long unused);
        
        HRESULT ( __stdcall __RPC_FAR *BeforeEvent )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ long iEvent,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Str,
            /* [in] */ VARIANT var,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);
        
        HRESULT ( __stdcall __RPC_FAR *AfterEvent )( 
            IVSSEvents __RPC_FAR * This,
            /* [in] */ long iEvent,
            /* [in] */ IVSSItem __RPC_FAR *pIItem,
            /* [in] */ BSTR Str,
            /* [in] */ VARIANT var);
        
        END_INTERFACE
    } IVSSEventsVtbl;

    interface IVSSEvents
    {
        CONST_VTBL struct IVSSEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSEvents_BeforeAdd(This,pIPrj,Local,Comment,pbContinue)	\
    (This)->lpVtbl -> BeforeAdd(This,pIPrj,Local,Comment,pbContinue)

#define IVSSEvents_AfterAdd(This,pIItem,Local,Comment)	\
    (This)->lpVtbl -> AfterAdd(This,pIItem,Local,Comment)

#define IVSSEvents_BeforeCheckout(This,pIItem,Local,Comment,pbContinue)	\
    (This)->lpVtbl -> BeforeCheckout(This,pIItem,Local,Comment,pbContinue)

#define IVSSEvents_AfterCheckout(This,pIItem,Local,Comment)	\
    (This)->lpVtbl -> AfterCheckout(This,pIItem,Local,Comment)

#define IVSSEvents_BeforeCheckin(This,pIItem,Local,Comment,pbContinue)	\
    (This)->lpVtbl -> BeforeCheckin(This,pIItem,Local,Comment,pbContinue)

#define IVSSEvents_AfterCheckin(This,pIItem,Local,Comment)	\
    (This)->lpVtbl -> AfterCheckin(This,pIItem,Local,Comment)

#define IVSSEvents_BeforeUndoCheckout(This,pIItem,Local,pbContinue)	\
    (This)->lpVtbl -> BeforeUndoCheckout(This,pIItem,Local,pbContinue)

#define IVSSEvents_AfterUndoCheckout(This,pIItem,Local)	\
    (This)->lpVtbl -> AfterUndoCheckout(This,pIItem,Local)

#define IVSSEvents_BeforeRename(This,pIItem,NewName,pbContinue)	\
    (This)->lpVtbl -> BeforeRename(This,pIItem,NewName,pbContinue)

#define IVSSEvents_AfterRename(This,pIItem,OldName)	\
    (This)->lpVtbl -> AfterRename(This,pIItem,OldName)

#define IVSSEvents_BeforeBranch(This,pIItem,Comment,pbContinue)	\
    (This)->lpVtbl -> BeforeBranch(This,pIItem,Comment,pbContinue)

#define IVSSEvents_AfterBranch(This,pIItem,Comment)	\
    (This)->lpVtbl -> AfterBranch(This,pIItem,Comment)

#define IVSSEvents_BeginCommand(This,unused,pbContinue)	\
    (This)->lpVtbl -> BeginCommand(This,unused,pbContinue)

#define IVSSEvents_EndCommand(This,unused)	\
    (This)->lpVtbl -> EndCommand(This,unused)

#define IVSSEvents_BeforeEvent(This,iEvent,pIItem,Str,var,pbContinue)	\
    (This)->lpVtbl -> BeforeEvent(This,iEvent,pIItem,Str,var,pbContinue)

#define IVSSEvents_AfterEvent(This,iEvent,pIItem,Str,var)	\
    (This)->lpVtbl -> AfterEvent(This,iEvent,pIItem,Str,var)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IVSSEvents_BeforeAdd_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIPrj,
    /* [in] */ BSTR Local,
    /* [in] */ BSTR Comment,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEvents_BeforeAdd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_AfterAdd_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local,
    /* [in] */ BSTR Comment);


void __RPC_STUB IVSSEvents_AfterAdd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_BeforeCheckout_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local,
    /* [in] */ BSTR Comment,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEvents_BeforeCheckout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_AfterCheckout_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local,
    /* [in] */ BSTR Comment);


void __RPC_STUB IVSSEvents_AfterCheckout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_BeforeCheckin_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local,
    /* [in] */ BSTR Comment,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEvents_BeforeCheckin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_AfterCheckin_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local,
    /* [in] */ BSTR Comment);


void __RPC_STUB IVSSEvents_AfterCheckin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_BeforeUndoCheckout_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEvents_BeforeUndoCheckout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_AfterUndoCheckout_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Local);


void __RPC_STUB IVSSEvents_AfterUndoCheckout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_BeforeRename_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR NewName,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEvents_BeforeRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_AfterRename_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR OldName);


void __RPC_STUB IVSSEvents_AfterRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_BeforeBranch_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Comment,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEvents_BeforeBranch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_AfterBranch_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Comment);


void __RPC_STUB IVSSEvents_AfterBranch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_BeginCommand_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ long unused,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEvents_BeginCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_EndCommand_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ long unused);


void __RPC_STUB IVSSEvents_EndCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_BeforeEvent_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ long iEvent,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Str,
    /* [in] */ VARIANT var,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbContinue);


void __RPC_STUB IVSSEvents_BeforeEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IVSSEvents_AfterEvent_Proxy( 
    IVSSEvents __RPC_FAR * This,
    /* [in] */ long iEvent,
    /* [in] */ IVSSItem __RPC_FAR *pIItem,
    /* [in] */ BSTR Str,
    /* [in] */ VARIANT var);


void __RPC_STUB IVSSEvents_AfterEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSEvents_INTERFACE_DEFINED__ */


#ifndef __IVSS_INTERFACE_DEFINED__
#define __IVSS_INTERFACE_DEFINED__

/* interface IVSS */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSS;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("783CD4EB-9D54-11CF-B8EE-00608CC9A71F")
    IVSS : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_VSSDatabase( 
            /* [retval][out] */ IVSSDatabase __RPC_FAR *__RPC_FAR *ppIVSSDatabase) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSS __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSS __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSS __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IVSS __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IVSS __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IVSS __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IVSS __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VSSDatabase )( 
            IVSS __RPC_FAR * This,
            /* [retval][out] */ IVSSDatabase __RPC_FAR *__RPC_FAR *ppIVSSDatabase);
        
        END_INTERFACE
    } IVSSVtbl;

    interface IVSS
    {
        CONST_VTBL struct IVSSVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSS_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSS_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSS_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSS_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IVSS_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IVSS_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IVSS_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IVSS_get_VSSDatabase(This,ppIVSSDatabase)	\
    (This)->lpVtbl -> get_VSSDatabase(This,ppIVSSDatabase)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE IVSS_get_VSSDatabase_Proxy( 
    IVSS __RPC_FAR * This,
    /* [retval][out] */ IVSSDatabase __RPC_FAR *__RPC_FAR *ppIVSSDatabase);


void __RPC_STUB IVSS_get_VSSDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSS_INTERFACE_DEFINED__ */


#ifndef __IVSSEventHandler_INTERFACE_DEFINED__
#define __IVSSEventHandler_INTERFACE_DEFINED__

/* interface IVSSEventHandler */
/* [object][helpstring][uuid] */ 


EXTERN_C const IID IID_IVSSEventHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("783CD4EA-9D54-11CF-B8EE-00608CC9A71F")
    IVSSEventHandler : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Init( 
            /* [in] */ IVSS __RPC_FAR *pIVSS) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVSSEventHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVSSEventHandler __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVSSEventHandler __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVSSEventHandler __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Init )( 
            IVSSEventHandler __RPC_FAR * This,
            /* [in] */ IVSS __RPC_FAR *pIVSS);
        
        END_INTERFACE
    } IVSSEventHandlerVtbl;

    interface IVSSEventHandler
    {
        CONST_VTBL struct IVSSEventHandlerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVSSEventHandler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVSSEventHandler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVSSEventHandler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVSSEventHandler_Init(This,pIVSS)	\
    (This)->lpVtbl -> Init(This,pIVSS)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IVSSEventHandler_Init_Proxy( 
    IVSSEventHandler __RPC_FAR * This,
    /* [in] */ IVSS __RPC_FAR *pIVSS);


void __RPC_STUB IVSSEventHandler_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IVSSEventHandler_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_VSSItem;

#ifdef __cplusplus

class DECLSPEC_UUID("783CD4E3-9D54-11CF-B8EE-00608CC9A71F")
VSSItem;
#endif

EXTERN_C const CLSID CLSID_VSSItems;

#ifdef __cplusplus

class DECLSPEC_UUID("783CD4F0-9D54-11CF-B8EE-00608CC9A71F")
VSSItems;
#endif

EXTERN_C const CLSID CLSID_VSSVersions;

#ifdef __cplusplus

class DECLSPEC_UUID("783CD4F1-9D54-11CF-B8EE-00608CC9A71F")
VSSVersions;
#endif

EXTERN_C const CLSID CLSID_VSSVersion;

#ifdef __cplusplus

class DECLSPEC_UUID("783CD4EC-9D54-11CF-B8EE-00608CC9A71F")
VSSVersion;
#endif

EXTERN_C const CLSID CLSID_VSSCheckout;

#ifdef __cplusplus

class DECLSPEC_UUID("2A0DE0E0-2E9F-11D0-9236-00AA00A1EB95")
VSSCheckout;
#endif

EXTERN_C const CLSID CLSID_VSSDatabase;

#ifdef __cplusplus

class DECLSPEC_UUID("783CD4E4-9D54-11CF-B8EE-00608CC9A71F")
VSSDatabase;
#endif

EXTERN_C const CLSID CLSID_VSSUser;

#ifdef __cplusplus

class DECLSPEC_UUID("2A0DE0E5-2E9F-11D0-9236-00AA00A1EB95")
VSSUser;
#endif

EXTERN_C const CLSID CLSID_VSSApp;

#ifdef __cplusplus

class DECLSPEC_UUID("2A0DE0E1-2E9F-11D0-9236-00AA00A1EB95")
VSSApp;
#endif
#endif /* __SourceSafeTypeLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif

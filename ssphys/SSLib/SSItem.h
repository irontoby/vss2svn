// SSItem.h: interface for the SSItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSITEM_H__CB9B12A5_D0F5_4015_BB9A_18DEC6265E3A__INCLUDED_)
#define AFX_SSITEM_H__CB9B12A5_D0F5_4015_BB9A_18DEC6265E3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <SSPhysLib\SSFiles.h>
#include <SSPhysLib\SSProjectObject.h>
#include <SSPhysLib\SSVersionObject.h>
#include <vector>
#include <boost\enable_shared_from_this.hpp>

// forward declaration
// #include "SSDatabase.h"
class SSDatabase;
class SSItems;
class SSVersions;

class SSItem;
typedef boost::shared_ptr <SSItem> SSItemPtr;

class SSItem : public boost::enable_shared_from_this<SSItem>
{
public:
//	SSItem();
//	SSItem(SSHistoryFile& rFile, int nVersion);
//  SSItem(SSDatabase* pDb);
  SSItem(SSDatabase* pDb, SSItemPtr pParent, SSProjectObject project, int version);
  SSItem(SSDatabase* pDb, SSItemPtr pParent, SSProjectObject project);
	virtual ~SSItem();

//  _bstr_t GetSpec ( );
  std::string GetSpec ();

//  VARIANT_BOOL GetBinary ( );
//  void PutBinary (
//      VARIANT_BOOL pbBinary );
  bool GetBinary ();

//  VARIANT_BOOL GetDeleted ( );
//  void PutDeleted (
//      VARIANT_BOOL pbDeleted );
  bool GetDeleted ();

//  int GetType ( );
  int GetType ( );

//  _bstr_t GetLocalSpec ( );
//  void PutLocalSpec (
//      _bstr_t pLocal );
  std::string GetLocalSpec ();

//  _bstr_t GetName ( );
//  void PutName (
//      _bstr_t pName );
  std::string GetName ();

//  IVSSItemPtr GetParent ( );
  SSItemPtr GetParent ();

//  long GetVersionNumber ( );
  long GetVersionNumber ( );

//  IVSSItemsPtr GetItems (
//      VARIANT_BOOL IncludeDeleted );
  SSItems* GetItems (bool includeDeleted);

//  HRESULT Get (
//      BSTR * Local,
//      long iFlags );

//  HRESULT Checkout (
//      _bstr_t Comment,
//      _bstr_t Local,
//      long iFlags );

//  HRESULT Checkin (
//      _bstr_t Comment,
//      _bstr_t Local,
//      long iFlags );

//  HRESULT UndoCheckout (
//      _bstr_t Local,
//      long iFlags );

  long GetIsCheckedOut ( );

//  IVSSCheckoutsPtr GetCheckouts ( );

//  VARIANT_BOOL GetIsDifferent (
//      _bstr_t Local );

//  IVSSItemPtr Add (
//      _bstr_t Local,
//      _bstr_t Comment,
//      long iFlags );

//  IVSSItemPtr NewSubproject (
//      _bstr_t Name,
//      _bstr_t Comment );

//  HRESULT Share (
//      struct IVSSItem * pIItem,
//      _bstr_t Comment,
//      long iFlags );

//  HRESULT Destroy ( );

//  HRESULT Move (
//      struct IVSSItem * pINewParent );

//  HRESULT Label (
//      _bstr_t Label,
//      _bstr_t Comment );

//  IVSSVersionsPtr GetVersions (
//      long iFlags );
  SSVersions* GetVersions ( long iFlags );

//  IVSSItemPtr GetVersion (
//      const _variant_t & Version = vtMissing );
  SSItemPtr GetVersion (std::string v);

  SSDatabase* GetDatabase () { return m_pDatabase; }

  std::string GetPhysical ();

  void Delete ();
  void Recover ();
  void Rename (SSNAME oldName, SSNAME newName);

protected:

  SSProjectObject m_ItemDescr;
  SSItemPtr       m_pParent;
//  long            m_nVersionNumber;
  SSDatabase*     m_pDatabase;
};


class SSItems : public ISSActionVisitor
{
public:
  SSItems (SSItemPtr pParent, SSProjectFile& rFile, bool bIncludeDeleted);
  ~SSItems ();
  
  long GetCount ()          { return m_Items.size (); }
  SSItemPtr GetItem (long i);
  SSItemPtr GetPhysicalItem (std::string physical);

  // ISSActionVisitor Interface for Undo
  virtual bool Apply (const SSLabeledAction& rAction);
  virtual bool Apply (const SSCreatedProjectAction& rAction);
  virtual bool Apply (const SSCreatedFileAction& rAction);
  virtual bool Apply (const SSDestroyedFileAction& rAction);
  virtual bool Apply (const SSDestroyedProjectAction& rAction);
  virtual bool Apply (const SSAddedFileAction& rAction);
  virtual bool Apply (const SSAddedProjectAction& rAction);
  virtual bool Apply (const SSDeletedFileAction& rAction);
  virtual bool Apply (const SSDeletedProjectAction& rAction);
  virtual bool Apply (const SSRenamedProjectAction& rAction); //, SSNAME oldName, SSNAME newName);
  virtual bool Apply (const SSRenamedFileAction& rAction); //, SSNAME oldName, SSNAME newName);
  virtual bool Apply (const SSCheckedInAction& rAction);

//  bool DeleteFile (SSAction& rAction);
//  bool DeleteProject (SSAction& rAction);

  
  void Dump (std::ostream& os) const;
private:
  void BuildList (SSItemPtr rParent, SSProjectFile& rFile, bool bIncludeDeleted);

  SSItemPtr m_pParent;
  std::vector<SSItemPtr> m_Items;
};

#endif // !defined(AFX_SSITEM_H__CB9B12A5_D0F5_4015_BB9A_18DEC6265E3A__INCLUDED_)

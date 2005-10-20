// SSItemInfoObject.h: interface for the SSItems class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSITEMS_H__716758E6_2B77_471C_B08A_424412E9B758__INCLUDED_)
#define AFX_SSITEMS_H__716758E6_2B77_471C_B08A_424412E9B758__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SSObject.h"
#include "SSVersionObject.h"

//------------------------------------------------------------------------------
class SSItemInfoObject : public SSObject
{
protected:
  SSItemInfoObject (SSRecordPtr pRecord);

public:
  ~SSItemInfoObject ();
  static SSItemInfoObject* MakeItemInfo (SSRecordPtr pRecord);

//  const char* GetSpec ()        { return NULL; /* return m_InfoItem.spec; */ }
//  const char* GetLocalSpec ()   { return NULL; /* return m_InfoItem.spec; */ }
//  const char* GetName ()        { return NULL; /* return m_InfoItem.spec; */ }
//  SSItemInfoObject* GetParent ()          { return NULL; }
  
  // accessors
  // BOOST_PP_SEQ_FOR_EACH (DEFINE_ACCESSORS, GetData(), DH_SEQ);
  short       GetType ()                const { return GetData()->Type; }
  short       GetNumberOfActions ()     const { return GetData()->NumberOfActions; }
  SSName      GetSSName ()              const { return GetData()->SSName; }
  std::string GetLatestExt ()           const { return std::string (GetData()->LatestExt, 2); }
  ulong       GetHistoryOffsetBegin ()  const { return GetData()->HistoryOffsetBegin; }
  ulong       GetHistoryOffsetLast ()   const { return GetData()->HistoryOffsetLast; }
  ulong       GetHistoryOffsetEnd ()    const { return GetData()->HistoryOffsetEnd; }

  // return the last known name of the item
  std::string GetName ()                const { SSName name (GetSSName ()); return name.GetFullName(); }

  SSVersionObject GetHistoryLast ()     const { return SSVersionObject (GetFile ()->GetRecord (GetHistoryOffsetLast())); }
  std::string GetDataFileName ()        const { return GetFile ()->GetFileName () + GetLatestExt (); }
  
//  bool GetDeleted ()            { return 0; }
//  bool GetCheckedOut ()         { return 0; }
//  bool GetDifferent ()          { return 0; }
//  bool GetBinary ()             { return false; }
//  int GetVersionNumber ()       { return 0; }
  
//  SSItems* GetItems ();
//  SSItems* GetCheckouts ();
//  SSItem* GetVersion (time_t date)    { return NULL; }
//  SSItem* GetVersion (int v)    { return NULL; }
//  SSItems* GetVersions ()    { return NULL; }

  bool Get (int version, const char* dest);
  SSVersionObject GetVersion (int i);

  virtual void Accept (ISSObjectVisitor& rVisitor, const ISSContext* pCtx = NULL) const { rVisitor.Apply (*this, pCtx); }
  virtual bool Validate ();

  const DH* GetData () const { return reinterpret_cast <const DH*> (SSObject::GetData ()); }

  void ToXml (XMLNode* pParent) const;
  void Dump (std::ostream& os)  const;

private:
  // pointer to data for debugging convinience
  const DH* m_pInfoItem;
};


class SSProjectItem : public SSItemInfoObject
{
public:
  SSProjectItem (SSRecordPtr pRecord);

  // accessors
  std::string GetParentSpec ()            const { return GetData()->ParentSpec; }
  std::string GetParentPhys ()            const { return GetData()->ParentPhys; }
  short       GetNumberOfItems ()         const { return GetData()->NumberOfItems; }
  short       GetNumberOfProjects ()      const { return GetData()->NumberOfProjects; }

  virtual void Accept (ISSObjectVisitor& rVisitor, const ISSContext* pCtx = NULL) const { rVisitor.Apply (*this, pCtx); }

  const DH_PROJECT* GetData ()  const { return reinterpret_cast <const DH_PROJECT*> (SSObject::GetData ()); }

  void ToXml (XMLNode* pParent) const;
  void Dump (std::ostream& os)  const;

private:
  // pointer to data for debugging convinience
  const DH_PROJECT* m_pProjectInfo;
};


class SSFileItem : public SSItemInfoObject
{
public:
  SSFileItem (SSRecordPtr pRecord);

  // accessors
  short       GetFlag ()                  const { return GetData ()->Flag; }
  std::string GetShareSrcPhys ()          const { return std::string (GetData ()->ShareSrcSpec, 8); }
  ulong       GetOffsetBFRecord ()        const { return GetData ()->OffsetBFRecord; }
  ulong       GetOffsetPFRecord ()        const { return GetData ()->OffsetPFRecord; }
  short       GetNumberOfBranches ()      const { return GetData ()->NumberOfBranches; }
  short       GetNumberOfReferences ()    const { return GetData ()->NumberOfReferences; }
  ulong       GetOffsetCFRecord1()        const { return GetData ()->OffsetCFRecord1; }
  ulong       GetOffsetCFRecord2()        const { return GetData ()->OffsetCFRecord2; }
//  short       GetNumberOfItems()          const { return GetData ()->NumberOfItems; }
//  short       GetNumberOfProjects()       const { return GetData ()->NumberOfProjects; }

  // specialized accessors
  eFileType   GetFileType ()              const;
  bool        GetStoreOnlyLatestRev ()    const;
  bool        GetCheckedOut ()            const;
  bool        GetShared ()                const;

  SSParentFolderObject* GetFirstParentFolder ();
  SSBranchFileObject*   GetFirstBranchFile ();

  virtual void Accept (ISSObjectVisitor& rVisitor, const ISSContext* pCtx = NULL) const { rVisitor.Apply (*this, pCtx); }
  virtual bool Validate ();

  const DH_FILE* GetData () const { return reinterpret_cast <const DH_FILE*> (SSObject::GetData ()); }

  void ToXml (XMLNode* pParent) const;
  void Dump (std::ostream& os)  const;

private:
  // pointer to data for debugging convinience
  const DH_FILE* m_pFileInfo;
};

#endif // !defined(AFX_SSITEMS_H__716758E6_2B77_471C_B08A_424412E9B758__INCLUDED_)

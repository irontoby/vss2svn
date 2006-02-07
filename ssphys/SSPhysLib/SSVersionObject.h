// SSVersionObject.h: interface for the SSActions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSHISTORYOBJECT_H__58F12C7C_FE74_43B1_9BF6_9E803182D8D7__INCLUDED_)
#define AFX_SSHISTORYOBJECT_H__58F12C7C_FE74_43B1_9BF6_9E803182D8D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SSObject.h"
#include "SSCommentObject.h"
#include "SSName.h"
//#include "SSItem.h"

//class SSItemAction;
class SSLabeledAction;            // Labeled = 0,
class SSCreatedProjectAction;     // Created_Project = 1,
class SSAddedProjectAction;       // Added_Project = 2,
class SSAddedFileAction;          // Added_File = 3,
class SSDestroyedProjectAction;   // Destroyed_Project = 4,
class SSDestroyedFileAction;      // Destroyed_File = 5,
class SSDeletedProjectAction;     // Deleted_Project = 6,
class SSDeletedFileAction;        // Deleted_File = 7,
class SSRecoveredProjectAction;   // Recovered_Project = 8,
class SSRecoveredFileAction;      // Recovered_File = 9,
class SSRenamedProjectAction;     // Renamed_Project = 10,
class SSRenamedFileAction;        // Renamed_File = 11,
//class SSAction12                // missing action 12,
class SSMovedProjectAction;       // Moved_Project = 13,
class SSSharedAction;             // Shared_File = 14, 	
class SSBranchFileAction;         // Branch_File = 15,
class SSCreatedFileAction;        // Created_File = 16,
class SSCheckedInAction;          // Checked_in = 17,
//class SSAction18                // missing action 18
class SSRollbackAction;           // RollBack = 19
  
class ISSActionVisitor
{
public:
  virtual bool Apply (const SSLabeledAction& rAction) = 0;
  virtual bool Apply (const SSCreatedProjectAction& rAction) = 0;
  virtual bool Apply (const SSCreatedFileAction& rAction) = 0;
  virtual bool Apply (const SSAddedProjectAction& rAction) = 0;
  virtual bool Apply (const SSAddedFileAction& rAction) = 0;
  virtual bool Apply (const SSDeletedProjectAction& rAction) = 0;
  virtual bool Apply (const SSDeletedFileAction& rAction) = 0;
  virtual bool Apply (const SSRecoveredProjectAction& rAction) = 0;
  virtual bool Apply (const SSRecoveredFileAction& rAction) = 0;
  virtual bool Apply (const SSBranchFileAction& rAction) = 0;
  virtual bool Apply (const SSRollbackAction& rAction) = 0;

  virtual bool Apply (const SSDestroyedProjectAction& rAction) = 0;
  virtual bool Apply (const SSDestroyedFileAction& rAction) = 0;
  virtual bool Apply (const SSRenamedProjectAction& rAction) = 0;
  virtual bool Apply (const SSRenamedFileAction& rAction) = 0;
  virtual bool Apply (const SSCheckedInAction& rAction) = 0;
  virtual bool Apply (const SSSharedAction& rAction) = 0;
  virtual bool Apply (const SSMovedProjectAction& rAction) = 0;
};

//---------------------------------------------------------------------------
class SSAction
{
public:
  SSAction (SSRecordPtr pRecord);
  virtual ~SSAction ();

  static SSAction* MakeAction (SSRecordPtr pRecord);
  
  virtual eAction GetActionID () const         { return m_ActionId; }

  virtual std::string FormatActionString () = 0;
  virtual bool Accept (ISSActionVisitor& rVisitor) = 0;
  
  virtual void ToXml (XMLNode* pParent) const = 0;
  virtual void Dump (std::ostream& os) const;

protected:
  const VERSION_RECORD* GetHistoryRecordPtr (SSRecordPtr pRecord) const;
  eAction m_ActionId;
};

//---------------------------------------------------------------------------
class SSVersionObject : public SSObject
{
public:
  SSVersionObject (SSRecordPtr pRecord);
  virtual ~SSVersionObject ();

  // handle copy constructor und assigment
  SSVersionObject (SSVersionObject& object);
  SSVersionObject& operator= (SSVersionObject const & object);

//  BOOST_PP_SEQ_FOR_EACH(DEFINE_ACCESSORS, GetData(), VERSION_RECORD_SEQ);
  ulong   GetPrevious ()                    const { return GetData ()->previous; }
  eAction  GetActionID  ()                  const { return static_cast <eAction> (GetAction ()->GetActionID ()); }
  short   GetVersionNumber ()               const { return GetData ()->versionNumber; }
  __time32_t  GetDate ()                    const { return GetData ()->date; }
  std::string GetUsername ()                const { return std::string (GetData ()->username /*, 32*/); }
  std::string GetLabel ()                   const { return std::string (GetData ()->label    /*, 32*/); }
  ulong   GetOffsetToNextRecordOrComment () const { return GetData ()->offsetToNextRecordOrComment; }
  ulong   GetOffsetToLabelComment ()        const { return GetData ()->offsetToLabelComment; }
  short   GetLengthComment ()               const { return GetData ()->lengthComment; }
  short   GetLengthLabelComment ()          const { return GetData ()->lengthLabelComment; }

//  std::string GetUsername ()      const { return GetData ()->user; }
//  int         GetVersionNumber () const { return GetData ()->version; }
//  eAction     GetActionId ()      const { return static_cast <eAction> (GetData ()->action); }
  std::string GetActionString ()  const { return m_pAction ? m_pAction->FormatActionString () : CAction::ActionToString (GetActionID()); }
//  __time32_t      GetDate ()      const { return GetData ()->date; }
  std::string GetComment ()       const;

  // special Label Action functions
//  std::string GetLabel ()         const;
  std::string GetLabelComment ()  const;

  // reporting functions
  virtual void Accept (ISSObjectVisitor& rVisitor, const ISSContext* pCtx = NULL) const { rVisitor.Apply (*this, pCtx); }
  
  const VERSION_RECORD* GetData () const { return (const VERSION_RECORD*) SSObject::GetData (); }
    
  SSAction* GetAction ()          const  { return m_pAction; }
  SSVersionObject GetPreviousObject ()  const;
  SSCommentObject GetCommentObject () const;

  virtual bool Validate ();

  virtual void ToXml (XMLNode* pParent) const;
  virtual void Dump (std::ostream& os) const;

protected:
  
  SSAction* m_pAction;
};


//---------------------------------------------------------------------------
template <class ACTION>
class SSActionEx : public SSAction
{
public:
  SSActionEx (SSRecordPtr pRecord)
    : SSAction (pRecord)
  {
  }
  
  bool Accept (ISSActionVisitor& rVisitor)
  {
    return rVisitor.Apply(*This());
  }

  // return a pointer to the most derived class
  ACTION * This()
  {
      return static_cast<ACTION *>(this);
  }
};


//---------------------------------------------------------------------------
class SSLabeledAction : public SSActionEx<SSLabeledAction>
{
public:
  SSLabeledAction (SSRecordPtr pRecord);
  virtual ~SSLabeledAction ();

  std::string GetLabel () const         { return m_Label; }
  std::string GetLabelComment () const  { return m_LabelComment; }
  
  virtual std::string FormatActionString ();

  virtual void ToXml (XMLNode* pParent) const;
  virtual void Dump (std::ostream& os) const;

private:
  std::string m_Label;
  std::string m_LabelComment;
};

//---------------------------------------------------------------------------
class ISSItemAction
{
public:
  virtual std::string GetPhysical () const = 0;
  virtual SSName GetSSName ()        const = 0;
};

//---------------------------------------------------------------------------
template <class ACTION, class STRUCT>
class SSItemAction : public SSActionEx<ACTION>, public ISSItemAction
{
public:
  SSItemAction (SSRecordPtr pRecord, std::string actionString);

  std::string GetPhysical () const { return std::string (m_Action.physical, 8); }
  SSName GetSSName ()        const { return m_Action.name; }

  std::string GetName ()     const { SSName name (GetSSName ()); return name.GetFullName (); }

  virtual std::string FormatActionString ();

  virtual void ToXml (XMLNode* pParent) const;
  virtual void Dump (std::ostream& os) const;

protected:
  STRUCT m_Action;
  std::string m_ActionString;
};

class SSCreatedProjectAction : public SSItemAction<SSCreatedProjectAction, ITEM_ACTION>
{
public:
  SSCreatedProjectAction (SSRecordPtr pRecord)
    : SSItemAction<SSCreatedProjectAction, ITEM_ACTION> (pRecord, "Created Project $")
  {
  }
};

class SSCreatedFileAction : public SSItemAction<SSCreatedFileAction, ITEM_ACTION>
{
public:
  SSCreatedFileAction (SSRecordPtr pRecord)
    : SSItemAction<SSCreatedFileAction, ITEM_ACTION> (pRecord, "Created File ")
  {
  }
};

class SSAddedProjectAction : public SSItemAction<SSAddedProjectAction, ITEM_ACTION>
{
public:
  SSAddedProjectAction (SSRecordPtr pRecord)
    : SSItemAction<SSAddedProjectAction, ITEM_ACTION> (pRecord, "Added Project $")
  {
  }
};

class SSAddedFileAction : public SSItemAction<SSAddedFileAction, ITEM_ACTION>
{
public:
  SSAddedFileAction (SSRecordPtr pRecord)
    : SSItemAction<SSAddedFileAction, ITEM_ACTION> (pRecord, "Added File ")
  {
  }
};

class SSDeletedProjectAction : public SSItemAction<SSDeletedProjectAction, ITEM_ACTION>
{
public:
  SSDeletedProjectAction (SSRecordPtr pRecord)
    : SSItemAction<SSDeletedProjectAction, ITEM_ACTION> (pRecord, "Deleted Project $")
  {
  }
};

class SSDeletedFileAction : public SSItemAction<SSDeletedFileAction, ITEM_ACTION>
{
public:
  SSDeletedFileAction (SSRecordPtr pRecord)
    : SSItemAction<SSDeletedFileAction, ITEM_ACTION> (pRecord, "Deleted File ")
  {
  }
};

class SSRecoveredProjectAction : public SSItemAction<SSRecoveredProjectAction, ITEM_ACTION>
{
public:
  SSRecoveredProjectAction (SSRecordPtr pRecord)
    : SSItemAction<SSRecoveredProjectAction, ITEM_ACTION> (pRecord, "Recovered Project ")
  {
  }
};

class SSRecoveredFileAction : public SSItemAction<SSRecoveredFileAction, ITEM_ACTION>
{
public:
  SSRecoveredFileAction (SSRecordPtr pRecord)
    : SSItemAction<SSRecoveredFileAction, ITEM_ACTION> (pRecord, "Recovered File ")
  {
  }
};

//---------------------------------------------------------------------------
class SSBranchFileAction : public SSItemAction<SSBranchFileAction, BRANCH_FILE_ACTION>
{
public:
  SSBranchFileAction (SSRecordPtr pRecord)
    : SSItemAction<SSBranchFileAction, BRANCH_FILE_ACTION> (pRecord, "Branched file ")
  {
  }

  std::string GetParent ()  const { return std::string (m_Action.parent, 8); }

  virtual void ToXml (XMLNode* pParent) const;
  virtual void Dump (std::ostream& os) const;
};

//---------------------------------------------------------------------------
class SSRollbackAction : public SSItemAction<SSRollbackAction, ROLLBACK_ACTION>
{
public:
  SSRollbackAction (SSRecordPtr pRecord)
    : SSItemAction<SSRollbackAction, ROLLBACK_ACTION> (pRecord, "Rolled back ")
  {
  }

  std::string GetParent ()  const { return std::string (m_Action.parent, 8); }
 
  virtual void ToXml (XMLNode* pParent) const;
  virtual void Dump (std::ostream& os) const;
};

//---------------------------------------------------------------------------

template <class ACTION>
class SSDestroyedAction : public SSItemAction<ACTION, DESTROYED_ACTION>
{
public:
  SSDestroyedAction (SSRecordPtr pRecord, std::string prefix);

  virtual std::string FormatActionString ();

private:
  std::string m_Prefix;
};

class SSDestroyedProjectAction : public SSDestroyedAction<SSDestroyedProjectAction>
{
public:
  SSDestroyedProjectAction (SSRecordPtr pRecord)
    : SSDestroyedAction<SSDestroyedProjectAction> (pRecord, "$")
  {}
};

class SSDestroyedFileAction : public SSDestroyedAction<SSDestroyedFileAction>
{
public:
  SSDestroyedFileAction (SSRecordPtr pRecord)
    : SSDestroyedAction<SSDestroyedFileAction> (pRecord, "")
  {}
};


//---------------------------------------------------------------------------

template <class ACTION>
class SSRenamedAction : public SSItemAction<ACTION, RENAMED_ACTION>
{
public:
  SSRenamedAction (SSRecordPtr pRecord, std::string prefix);

  std::string GetNewName ()   const { SSName name (m_Action.newName); return name.GetFullName (); }
  SSName GetNewSSName ()      const { return m_Action.newName; }

  virtual std::string FormatActionString ();

  virtual void ToXml (XMLNode* pParent) const;
  virtual void Dump (std::ostream& os) const;

private:
  std::string m_Prefix;
};

class SSRenamedProjectAction : public SSRenamedAction<SSRenamedProjectAction>
{
public:
  SSRenamedProjectAction (SSRecordPtr pRecord)
    : SSRenamedAction<SSRenamedProjectAction> (pRecord, "$")
  {}
};

class SSRenamedFileAction : public SSRenamedAction<SSRenamedFileAction>
{
public:
  SSRenamedFileAction (SSRecordPtr pRecord)
    : SSRenamedAction<SSRenamedFileAction> (pRecord, "")
  {}
};

//---------------------------------------------------------------------------

class SSCheckedInAction : public SSActionEx<SSCheckedInAction>
{
public:
  SSCheckedInAction ( SSRecordPtr pRecord);
  ~SSCheckedInAction ();

  std::string GetFileSepc () const    { return m_Action.checkInSpec; }
  long GetOffset () const             { return m_Action.offsetFileDelta; }
  SSRecordPtr GetFileDelta () const;

  virtual std::string FormatActionString ();
  virtual bool Accept (ISSActionVisitor& rVisitor) { return rVisitor.Apply (*this); }

  virtual void ToXml (XMLNode* pParent) const;
  virtual void Dump (std::ostream& os) const;

private:
  CHECKED_IN_ACTION m_Action;
  mutable SSRecordPtr m_pFileDelta;
  SSFileImpPtr m_FilePtr;
};


//---------------------------------------------------------------------------

class SSSharedAction : public SSItemAction<SSSharedAction, SHARED_FILE_ACTION>
{
public:
  SSSharedAction ( SSRecordPtr pRecord);
  ~SSSharedAction ();

  std::string GetSrcPathSpec () const   { return m_Action.srcPathSpec; }
  short GetPinnedToVersion () const     { return m_Action.pinnedToVersion; }
  short GetSubActionAndVersion () const { return m_Action.subActionAndVersion; }

  short GetUnpinnedVersion () const     { return m_Action.subActionAndVersion >= 0 ? m_Action.subActionAndVersion : 0; }
  
  virtual eAction GetActionID () const
  { 
    if (m_Action.subActionAndVersion < 0)
      return Shared_File;
    else if (m_Action.subActionAndVersion == 0)
      return Pinned_File;
    /* else if (m_Action.subActionAndVersion > 0) */
    return Unpinned_File;
  }

  virtual std::string FormatActionString ();

  virtual void ToXml (XMLNode* pParent) const;
  virtual void Dump (std::ostream& os) const;
};


//---------------------------------------------------------------------------

class SSMovedProjectAction : public SSItemAction<SSMovedProjectAction, MOVED_PROJECT_ACTION>
{
public:
  SSMovedProjectAction ( SSRecordPtr pRecord);
  ~SSMovedProjectAction ();

  std::string GetPathSpec () const   { return m_Action.pathSpec; }

//   virtual eAction GetActionID () const
//   { 
//     if (m_Action.subActionAndVersion < 0)
//       return Shared_File;
//     else if (m_Action.subActionAndVersion == 0)
//       return Pinned_File;
//     /* else if (m_Action.subActionAndVersion > 0) */
//     return Unpinned_File;
//   }

  virtual std::string FormatActionString ();

  virtual void ToXml (XMLNode* pParent) const;
  virtual void Dump (std::ostream& os) const;
};
#endif // !defined(AFX_SSHISTORYOBJECT_H__58F12C7C_FE74_43B1_9BF6_9E803182D8D7__INCLUDED_)

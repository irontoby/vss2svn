// SSVersionObject.cpp: implementation of the SSVersionObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSVersionObject.h"
#include "SSFiles.h"
#include "SSName.h"

//---------------------------------------------------------------------------
#include "LeakWatcher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SSVersionObject::SSVersionObject (SSRecordPtr pRecord)
  : SSObject (pRecord, eHistoryRecord), 
    m_pAction (NULL)
{
  if (pRecord)
  {
    if (pRecord->GetLen() < sizeof (VERSION_RECORD))
      throw SSRecordException ("not enough data for version object");
  
    m_pAction = SSAction::MakeAction (pRecord);
  }
}

SSVersionObject::~SSVersionObject ()
{
  delete m_pAction;
}

SSVersionObject::SSVersionObject (SSVersionObject& object)
  : SSObject (object), 
    m_pAction (NULL)
{
  m_pAction = SSAction::MakeAction (GetRecord());
}

SSVersionObject& SSVersionObject::operator= (SSVersionObject const & object)
{
  if (&object != this)
  {
    *(SSObject*)this = object;
    m_pAction = SSAction::MakeAction (GetRecord());
  }
  return *this;
}

std::string SSVersionObject::GetComment () const
{
  if (GetOffsetToNextRecordOrComment() && GetLengthComment())
  {
    SSCommentObject comment (GetFile ()->GetRecord (GetOffsetToNextRecordOrComment()));
    return comment.GetComment();
  }
  return "";
}

//std::string SSVersionObject::GetLabel () const
//{ 
//  const SSLabeledAction* pLabeledAction = dynamic_cast<const SSLabeledAction*> (m_pAction); 
//  return pLabeledAction ? pLabeledAction->GetLabel () : "";
//}
std::string SSVersionObject::GetLabelComment () const
{ 
  const SSLabeledAction* pLabeledAction = dynamic_cast<const SSLabeledAction*> (m_pAction); 
  return pLabeledAction ? pLabeledAction->GetLabelComment () : "";
}

SSVersionObject SSVersionObject::GetPreviousObject () const
{
  if (GetPrevious () == 0)
    return SSVersionObject (SSRecordPtr());

  return SSVersionObject (GetFile ()->GetRecord (GetPrevious ()));
}

bool SSVersionObject::Validate ()
{
  bool retval = true;

  SSRecordPtr pLabelCommentRecord;
  SSRecordPtr pNext;
  SSRecordPtr pPrevious;

  const VERSION_RECORD* pVersion = GetData ();
  if (pVersion->offsetToLabelComment)
    pLabelCommentRecord = GetFile ()->GetRecord (pVersion->offsetToLabelComment);
  if (pVersion->offsetToNextRecordOrComment)
    pNext = GetFile ()->GetRecord (pVersion->offsetToNextRecordOrComment);
  if (pVersion->previous)
    pPrevious = GetFile ()->GetRecord (pVersion->previous);

  if (   (pVersion->lengthLabelComment == 0 && pVersion->offsetToLabelComment != 0)
      || (pVersion->lengthLabelComment != 0 && pVersion->offsetToLabelComment == 0) )
  {
    Warning ("invalid length and offset combination");
    retval &= false;
  }

  if (pVersion->lengthLabelComment)
  {
    retval &= warn_with_msg_if (!pLabelCommentRecord, "the expected comment record is invalid")

    retval &= warn_with_msg_if (pLabelCommentRecord->GetType () != eCommentRecord, 
                                "the record pointed to by offsetToLabelComment is expected to be a comment record");
    return false;
  }

  if (pVersion->lengthComment)
  {
    retval &= warn_with_msg_if (!pNext, "the expected comment record is invalid");
    
    retval &= warn_with_msg_if (pNext->GetType () != eCommentRecord, 
                                "the record pointed to by offsetToNextRecordOrComment is expected to be a comment record");
  }

  if (pVersion->previous && !pPrevious)
  {
    Warning ("a previous record is specified, but the record could not be read");
    retval &= false;
  }

  if (pPrevious)
  {
    retval = warn_with_msg_if (pPrevious->GetType () != eHistoryRecord, 
                               "the record pointed to by previous is expected to be a history record");
  }

  return retval;
}
  // ---------------------------------------------------------------
  class ActionNode : public XMLNode
  {
  public:
    ActionNode (XMLNode* pParent, eAction actionid)
      : XMLNode (pParent, "Action", ToAttribMap(actionid))
    {
    }

    static AttribMap ToAttribMap (eAction actionid)
    {
      std::string actionString = CAction::ActionToString (actionid);
      actionString.erase (std::remove (actionString.begin (), actionString.end (), ' '));

      return ToAttribMap (actionString);
    }
    static AttribMap ToAttribMap (std::string actionString)
    {
      AttribMap map;
      map["ActionId"] = actionString;
      return map;
    }
  };


void SSVersionObject::ToXml (XMLNode* pParent) const
{
  XMLElement versionNumber (pParent, "VersionNumber", GetVersionNumber());
  XMLElement useName (pParent, "UserName", GetUsername());
  XMLElement date (pParent, "Date", GetDate());
  if (!GetComment ().empty ())
  {
    XMLElement date (pParent, "Comment", GetComment());
  }

  if (GetAction ())
  {
    SSAction* pAction = GetAction ();
    ActionNode node (pParent, pAction->GetActionID());

    pAction->ToXml (&node);
  }
}

void SSVersionObject::Dump (std::ostream& os) const
{
  SSRecordPtr pLabelCommentRecord;
  SSRecordPtr pNext;
  const VERSION_RECORD* pVersion = GetData ();

  if (pVersion->offsetToLabelComment)
    pLabelCommentRecord = GetFile ()->GetRecord (pVersion->offsetToLabelComment);
  if (pVersion->offsetToNextRecordOrComment)
    pNext = GetFile ()->GetRecord (pVersion->offsetToNextRecordOrComment);

  // dump basic information
  SSObject::Dump (os);

  SSVersionObject previous = GetPreviousObject ();
  os << "Previous: ";
  if (previous)
  {
    os << "Type " << SSRecord::TypeToString (previous.GetType ()) << ", ";
    os << "Offset 0x" << std::hex << previous.GetOffset () << std::dec;
    os << std::endl;
  }
  else
    os << "NULL" << std::endl;

  os << "Action " << GetActionID() << " : " << CAction::ActionToString (GetActionID())  << std::endl;
  os << "ActionStr: " << GetActionString ()  << std::endl;
  os << "Version:  " << GetVersionNumber () << std::endl;
//  os << "Date" << std::endl;
  os << "User:     " << GetUsername () << std::endl;
  os << "Comment:  " << GetComment () << std::endl;

  os << "Comment Offset      0x" << std::hex << pVersion->offsetToNextRecordOrComment << std::dec << ", Length " << pVersion->lengthComment;
  if (pNext)
      os << ", Type " << pNext->GetRecordType ();
  os << std::endl;

  os << "LabelComment Offset 0x" << std::hex << pVersion->offsetToLabelComment << std::dec << ", Length " << pVersion->lengthLabelComment;
  if (pLabelCommentRecord)
      os << ", Type " << pLabelCommentRecord->GetRecordType ();
  os << std::endl;

    SSAction* pAction = GetAction ();
    if (pAction)
      pAction->Dump (os);
}

//---------------------------------------------------------------------------
SSAction::SSAction (SSRecordPtr pRecord)
{
  const VERSION_RECORD* pVersion = GetHistoryRecordPtr (pRecord);
  
  m_ActionId = static_cast <eAction> (pVersion->actionID);
}

SSAction::~SSAction ()
{
}

SSAction* SSAction::MakeAction (SSRecordPtr pRecord)
{
  if (!pRecord)
    return NULL;
  
  if (pRecord->GetLen () < sizeof (VERSION_RECORD))
    throw SSException ("not enough bytes for Version Header in record");

  const VERSION_RECORD* pVersion = reinterpret_cast<const VERSION_RECORD*> (pRecord->GetBuffer ());
  
  if (pVersion->actionID == Labeled)
    return new SSLabeledAction (pRecord);
  else if (pVersion->actionID == Created_Project)
    return new SSCreatedProjectAction (pRecord);
  else if (pVersion->actionID == Added_Project)
    return new SSAddedProjectAction (pRecord);
  else if (pVersion->actionID == Added_File)
    return new SSAddedFileAction (pRecord);
  else if (pVersion->actionID == Destroyed_Project)
    return new SSDestroyedProjectAction(pRecord);
  else if (pVersion->actionID == Destroyed_File)
    return new SSDestroyedFileAction (pRecord);
  else if (pVersion->actionID == Deleted_Project)
    return new SSDeletedProjectAction (pRecord);
  else if (pVersion->actionID == Deleted_File)
    return new SSDeletedFileAction (pRecord);
  else if (pVersion->actionID == Recovered_Project)
    return new SSRecoveredProjectAction (pRecord);
  else if (pVersion->actionID == Recovered_File)
    return new SSRecoveredFileAction (pRecord);
  else if (pVersion->actionID == Renamed_Project)
    return new SSRenamedProjectAction (pRecord);
  else if (pVersion->actionID == Renamed_File)
    return new SSRenamedFileAction (pRecord);
//  else if (pVersion->actionID == missing action 12)
//    return new SSVersionObject (pRecord);
//  else if (pVersion->actionID == missing action 13)
//    return new SSVersionObject (pRecord);
  else if (pVersion->actionID == Shared_File)
    return new SSSharedAction (pRecord);
  else if (pVersion->actionID == Branch_File)
    return new SSBranchFileAction (pRecord);
  else if (pVersion->actionID == Created_File)
    return new SSCreatedFileAction (pRecord);
  else if (pVersion->actionID == Checked_in)
    return new SSCheckedInAction (pRecord);
//  else if (pVersion->actionID == // missing action 18)
//    return new SSVersionObject (pRecord);
  else if (pVersion->actionID == RollBack)
    return new SSRollbackAction (pRecord);
  else 
    throw SSUnknownActionException (pVersion->actionID, pRecord);

  return NULL;
}

const VERSION_RECORD* SSAction::GetHistoryRecordPtr (SSRecordPtr pRecord) const
{
  assert (pRecord->GetBuffer ());
  return reinterpret_cast<const VERSION_RECORD*> (pRecord->GetBuffer ());
}

void SSAction::Dump (std::ostream& os) const
{
}

//---------------------------------------------------------------------------
SSLabeledAction::SSLabeledAction (SSRecordPtr pRecord)
  : SSActionEx<SSLabeledAction> (pRecord)
{
  const VERSION_RECORD* pVersion = GetHistoryRecordPtr (pRecord);
  
  m_Label = std::string (pVersion->label);

  if (pVersion->offsetToLabelComment && pVersion->lengthLabelComment > 0)
  {
    try {
      SSCommentObject commentObject (pRecord->GetFileImp ()->GetRecord (pVersion->offsetToLabelComment));
      m_LabelComment = commentObject.GetComment();
    }
    catch (SSRecordException&)
    {
    }
  }
}

SSLabeledAction::~SSLabeledAction ()
{
}

std::string SSLabeledAction::FormatActionString ()   
{
  return "Labeled " + m_Label; 
}

void SSLabeledAction::ToXml (XMLNode* pParent) const
{
  XMLElement label   (pParent, "Label", GetLabel());
  XMLElement comment (pParent, "LabelComment", GetLabelComment());

}

void SSLabeledAction::Dump (std::ostream& os) const
{
  SSActionEx<SSLabeledAction>::Dump (os);
}

//---------------------------------------------------------------------------
template <class ACTION, class STRUCT>
SSItemAction<ACTION, STRUCT>::SSItemAction (SSRecordPtr pRecord, std::string actionString)
  : SSActionEx<ACTION> (pRecord),
    m_ActionString (actionString)
{
  memcpy (&m_Action, pRecord->GetBuffer()+sizeof(VERSION_RECORD), sizeof(STRUCT));
}

template <class T, class ACTION>
std::string SSItemAction<T, ACTION>::FormatActionString ()   
{ 
  return m_ActionString + GetName (); 
}

template <class T, class ACTION>
void SSItemAction<T, ACTION>::ToXml (XMLNode* pParent) const
{
  XMLElement physical (pParent, "Physical", GetPhysical());
  GetSSName().ToXml (pParent);
}

template <class T, class ACTION>
void SSItemAction<T, ACTION>::Dump (std::ostream& os) const
{
  SSActionEx<T>::Dump (os);
}

//---------------------------------------------------------------------------
template <class ACTION>
SSDestroyedAction<ACTION>::SSDestroyedAction (SSRecordPtr pRecord, std::string prefix)
  : SSItemAction<ACTION, DESTROYED_ACTION> (pRecord, ""),
    m_Prefix (prefix)
{
}

template <class ACTION>
std::string SSDestroyedAction<ACTION>::FormatActionString ()   
{ 
  std::string action (m_Prefix);
  action += GetName () + " Destroyed";
  return action; 
}

//---------------------------------------------------------------------------
template <class ACTION>
SSRenamedAction<ACTION>::SSRenamedAction (SSRecordPtr pRecord, std::string prefix)
  : SSItemAction<ACTION, RENAMED_ACTION> (pRecord, ""),
    m_Prefix (prefix)
{
}

template <class ACTION>
std::string SSRenamedAction<ACTION>::FormatActionString ()   
{ 
  SSName ssOldName (GetSSName ());
  SSName ssNewName (m_Action.newName);
  std::string action (m_Prefix);
  action += ssOldName.GetFullName ();
  action += " renamedto " + m_Prefix;
  action += ssNewName.GetFullName ();
  return action; 
}

template <class ACTION>
void SSRenamedAction<ACTION>::ToXml (XMLNode* pParent) const
{
  SSItemAction<ACTION,RENAMED_ACTION>::ToXml (pParent);

  GetNewSSName().ToXml (pParent, "NewSSName");
}

template <class ACTION>
void SSRenamedAction<ACTION>::Dump (std::ostream& os) const
{
  SSItemAction<ACTION, RENAMED_ACTION>::Dump (os);
}

//---------------------------------------------------------------------------
SSCheckedInAction::SSCheckedInAction (SSRecordPtr pRecord)
  : SSActionEx<SSCheckedInAction> (pRecord),
    m_FilePtr (pRecord->GetFileImp ())
{
  memcpy (&m_Action, pRecord->GetBuffer()+sizeof(VERSION_RECORD), sizeof(CHECKED_IN_ACTION));
}

SSCheckedInAction::~SSCheckedInAction ()
{
}

std::string SSCheckedInAction::FormatActionString ()   
{ 
  std::string action = "Checked In "; 
  action += std::string (m_Action.checkInSpec);
  return action;
}

SSRecordPtr SSCheckedInAction::GetFileDelta () const
{
  if (!m_pFileDelta && m_Action.offsetFileDelta)
  {
    m_pFileDelta = m_FilePtr->GetRecord (m_Action.offsetFileDelta);
  }
  
  return m_pFileDelta;
}

void SSCheckedInAction::ToXml (XMLNode* pParent) const
{
  XMLElement checkInNode (pParent, "CheckInSpec", GetFileSepc());
  XMLElement offsetNode  (pParent, "Offset", GetOffset());
}

void SSCheckedInAction::Dump (std::ostream& os) const
{
  SSActionEx<SSCheckedInAction>::Dump (os);
}

//---------------------------------------------------------------------------
SSSharedAction::SSSharedAction (SSRecordPtr pRecord)
  : SSItemAction<SSSharedAction, SHARED_FILE_ACTION> (pRecord, "")
{
}

SSSharedAction::~SSSharedAction ()
{
}

std::string SSSharedAction::FormatActionString ()   
{ 
  std::ostringstream str;
  SSName name (m_Action.name);
  if (m_Action.subActionAndVersion == -1)
  {
    str << m_Action.srcPathSpec << "/" << name.GetFullName();
    if (m_Action.pinnedToVersion > 0)
      str << ";" << m_Action.pinnedToVersion;
    str << " shared";
  }
  else if (m_Action.subActionAndVersion == 0)
    str << m_Action.srcPathSpec << "/" << name.GetFullName() << " pinned to version " << m_Action.pinnedToVersion;
  else if (m_Action.subActionAndVersion > 0)
    str << m_Action.srcPathSpec << "/" << name.GetFullName() << " unpinned version " << m_Action.subActionAndVersion;

  return str.str();
}


void SSSharedAction::ToXml (XMLNode* pParent) const
{
  SSItemAction<SSSharedAction, SHARED_FILE_ACTION>::ToXml(pParent);

  XMLElement srcPathNode (pParent, "SrcPath", GetSrcPathSpec ());
  if (GetActionID () == Unpinned_File)
    XMLElement unpinnedNode (pParent, "UnpinnedFromVersion", GetUnpinnedVersion());
  if (GetPinnedToVersion () > 0)
    XMLElement pinnedNode  (pParent, "PinnedToVersion", GetPinnedToVersion ());
}

void SSSharedAction::Dump (std::ostream& os) const
{
  SSItemAction<SSSharedAction, SHARED_FILE_ACTION>::Dump(os);
}

//---------------------------------------------------------------------------
void SSBranchFileAction::ToXml (XMLNode* pParent) const
{
  SSItemAction<SSBranchFileAction, BRANCH_FILE_ACTION>::ToXml (pParent);

  XMLElement parentNode (pParent, "Parent", GetParent());
}

void SSBranchFileAction::Dump (std::ostream& os) const
{
  SSItemAction<SSBranchFileAction, BRANCH_FILE_ACTION>::Dump (os);
}

//---------------------------------------------------------------------------
void SSRollbackAction::ToXml (XMLNode* pParent) const
{
  SSItemAction<SSRollbackAction, ROLLBACK_ACTION>::ToXml (pParent);

  XMLElement parentNode (pParent, "Parent", GetParent());
}

void SSRollbackAction::Dump (std::ostream& os) const
{
  SSItemAction<SSRollbackAction, ROLLBACK_ACTION>::Dump (os);
}

// SSFiles.h: interface for the SSFiles class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSFILES_H__A748503F_FA76_42CB_9EE0_A4FED9F1779B__INCLUDED_)
#define AFX_SSFILES_H__A748503F_FA76_42CB_9EE0_A4FED9F1779B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <boost/shared_ptr.hpp>

class SSItemInfoObject;
class SSVersionObject;
class SSRecord;
typedef boost::shared_ptr<SSRecord> SSRecordPtr;
class SSFileImp;
typedef boost::shared_ptr<SSFileImp> SSFileImpPtr;



class SSFileImp : public boost::enable_shared_from_this<SSFileImp>
{
public:
  SSFileImp (const std::string& fileName);
  SSFileImp (std::istream* pInput);
  virtual ~SSFileImp ();

  bool Seek (size_t offset, std::ios_base::seekdir way);
  bool Read (long offset, void* ptr, size_t size);
  bool Read (void* ptr, size_t size);
//  size_t Write (const void* ptr, size_t size, size_t count);
  long Size ();

  std::string GetFileName () const;

  SSRecordPtr GetRecord (long offset);

protected:
  SSRecord* ReadRecord (long offset);
//  friend SSRecord;
//  void ReleaseRecord (SSRecord* record);

//  std::map <long, SSRecordPtr > m_Records;
  std::istream* m_pInput;
  std::string m_FileName;
};



class SSFile
{
public:
  SSFile (const std::string& fileName);
  SSFile (std::istream* pInput);
  virtual ~SSFile ();
 
  virtual bool Validate () { return true; }

protected:
  SSFileImpPtr m_FileImpPtr;
};

class SSTextFile : public SSFile
{
public:
  SSTextFile (const std::string& fileName);
};

class SSBinaryFile : public SSFile
{
public:
  SSBinaryFile (const std::string& fileName);
  SSBinaryFile (std::istream* pInput);
};

class SSRecordFile : public SSBinaryFile
{
public:
  SSRecordFile (const std::string& fileName);
  SSRecordFile (std::istream* pInput);
  virtual ~SSRecordFile ();

  static SSRecordFile* MakeFile (const std::string& fileName);
  
  virtual bool CheckHeader ()   { return true; }
  virtual long GetHeaderLength () = 0;
  
  SSRecordPtr GetRecord (long offset);
  SSRecordPtr GetFirstRecord ();
  SSRecordPtr GetNextRecord (SSRecordPtr pRecord);
  SSRecordPtr FindNextRecord (SSRecordPtr pRecord);

  virtual void Dump (std::ostream& os);
  virtual bool Validate ();
};


class SSHeaderFile : public SSRecordFile
{
public:
  SSHeaderFile (const std::string& fileName);

  virtual long GetHeaderLength ();

  virtual void Dump (std::ostream& os);

protected:
  char m_Header[52];
};

class SSPlainFile : public SSRecordFile
{
public:
  SSPlainFile (const std::string& fileName);
  SSPlainFile (std::istream* pInput);

  virtual long GetHeaderLength ();

  SSItemInfoObject* GetItemInfo ();
  virtual void Dump (std::ostream& os);

protected:
};

class SSHistoryFile : public SSHeaderFile
{
public:
  SSHistoryFile (const std::string& fileName);
  ~SSHistoryFile ();

  virtual bool CheckHeader ();

  bool IsProject ();
  bool IsFile ();
  std::string GetLatestExt ();

  SSVersionObject GetLastVersion ();
  SSVersionObject GetPrevVersion (const SSVersionObject& version);

  virtual void Dump (std::ostream& os);

  std::auto_ptr<SSItemInfoObject> GetItemInfo ();

protected:
  SSItemInfoObject* m_pItemInfo;
};

class SSNamesCacheFile : public SSPlainFile
{
public:
  SSNamesCacheFile (const std::string& fileName);
  virtual void Dump (std::ostream& os);
};

class SSProjectFile : public SSPlainFile
{
public:
  SSProjectFile (const std::string& fileName);
  SSProjectFile (std::istream* pInput);

  virtual void Dump (std::ostream& os);
};

class SSUserFile : public SSHeaderFile
{
public:
  SSUserFile (const std::string& fileName);

  virtual bool CheckHeader ();

  virtual void Dump (std::ostream& os);

private:
};

#endif // !defined(AFX_SSFILES_H__A748503F_FA76_42CB_9EE0_A4FED9F1779B__INCLUDED_)

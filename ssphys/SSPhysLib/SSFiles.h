// SSFiles.h: interface for the SSFiles class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSFILES_H__A748503F_FA76_42CB_9EE0_A4FED9F1779B__INCLUDED_)
#define AFX_SSFILES_H__A748503F_FA76_42CB_9EE0_A4FED9F1779B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class SSItemInfoObject;
class SSVersionObject;
class SSRecord;
typedef boost::shared_ptr<SSRecord> SSRecordPtr;

class CBaseIO
{
public:
  virtual ~CBaseIO ();

  virtual bool Open (const char* mode) = 0;
  virtual void Close () = 0;
  virtual bool Seek (size_t offset, int whence) = 0;
  virtual size_t Read (void* ptr, size_t size, size_t count) = 0;
  virtual size_t Write (const void* ptr, size_t size, size_t count) = 0;
  virtual long Size () = 0;

  virtual std::string FileName () = 0;
};

class CFileIO : public CBaseIO
{
public:
  CFileIO (const std::string& fileName);
  virtual ~CFileIO ();

  virtual bool Open (const char* mode);
  virtual void Close ();
  virtual bool Seek (size_t offset, int whence);
  virtual size_t Read (void* ptr, size_t size, size_t count);
  virtual size_t Write (const void* ptr, size_t size, size_t count);
  virtual long Size ();

  virtual std::string FileName ();
private:
  FILE* m_pFile;
  std::string m_FileName;
};

class CMemoryIO : public CBaseIO
{
public:
  CMemoryIO (const void* ptr, long size);
  virtual ~CMemoryIO ();

  virtual bool Open (const char* mode);
  virtual void Close ();
  virtual bool Seek (size_t offset, int whence);
  virtual size_t Read (void* ptr, size_t size, size_t count);
  virtual size_t Write (const void* ptr, size_t size, size_t count);
  virtual long Size ();

  virtual std::string FileName ();
private:
  const void* m_Ptr;
  long m_Size;
  long m_CurrentPos;
};

class SSFileImp;
typedef boost::shared_ptr<SSFileImp> SSFileImpPtr;

class SSFileImp : public boost::enable_shared_from_this<SSFileImp>
{
public:
  SSFileImp (const std::string& fileName, bool bOpen = false);
  SSFileImp (CBaseIO* pio, bool bOpen = false);
  virtual ~SSFileImp ();

  bool Open (const char*) const;
  void Close () const;

  bool Seek (size_t offset, int pos) const;
  bool Read (long offset, void* ptr, int len) const;
  size_t Read (void* ptr, size_t size, size_t count) const;
  size_t Write (const void* ptr, size_t size, size_t count) const;
  long Size ();

  std::string GetFileName ();

  SSRecordPtr GetRecord (long offset);

protected:
  SSRecord* ReadRecord (SSFileImpPtr fileImp, long offset);
//  friend SSRecord;
//  void ReleaseRecord (SSRecord* record);

//  std::map <long, SSRecordPtr > m_Records;
  CBaseIO* m_pIO;
};



class SSFile
{
public:
  SSFile ();
  SSFile (const std::string& fileName, bool bOpen = false);
  SSFile (SSFileImpPtr filePtr);
  SSFile (CBaseIO* pio, bool bOpen = false);
  virtual ~SSFile ();
 
  std::string GetFileName (); //  { return m_FileName; }

//  bool IsOpen ();
  bool Open ();
  void Close ();

  virtual bool Validate () { return true; }

protected:
  SSFileImpPtr m_FileImpPtr;
};

class SSTextFile : public SSFile
{
public:
  SSTextFile (const std::string& fileName, bool bOpen = false);

};

class SSBinaryFile : public SSFile
{
public:
  SSBinaryFile (const std::string& fileName, bool bOpen = false);
  SSBinaryFile (SSFileImpPtr filePtr);
  SSBinaryFile (CBaseIO* pio);
};


class SSRecordFile : public SSBinaryFile
{
public:
//  SSRecordFile ();
  SSRecordFile (const std::string& fileName, bool bOpen = false);
  SSRecordFile (CBaseIO* pio);
  virtual ~SSRecordFile ();

  static SSRecordFile* MakeFile (const std::string& fileName);
  
  virtual bool CheckHeader ()   { return true; }
  virtual long GetHeaderLength () = 0;
  
  SSRecordPtr GetRecord (long offset);
  SSRecordPtr GetFirstRecord ();
  SSRecordPtr GetNextRecord (SSRecordPtr pRecord);
  SSRecordPtr FindNextRecord (SSRecordPtr pRecord);

  virtual void Dump (std::ostream& os);
//  virtual void DumpRecords (std::ostream& os);
  virtual bool Validate ();
};


class SSHeaderFile : public SSRecordFile
{
public:
//  SSHeaderFile ();
  SSHeaderFile (const std::string& fileName, bool bOpen = false);

  virtual long GetHeaderLength ();

  virtual void Dump (std::ostream& os);

protected:
  char m_Header[52];
};

class SSPlainFile : public SSRecordFile
{
public:
//  SSPlainFile ();
  SSPlainFile (const std::string& fileName, bool bOpen = false);
  SSPlainFile (CBaseIO* pio);

  virtual long GetHeaderLength ();

  SSItemInfoObject* GetItemInfo ();
  virtual void Dump (std::ostream& os);

protected:
};

class SSHistoryFile : public SSHeaderFile
{
public:
//  SSHistoryFile ();
  SSHistoryFile (const std::string& fileName, bool bOpen = false);
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
//  SSNamesCacheFile ();
  SSNamesCacheFile (const std::string& fileName, bool bOpen = false);
  virtual void Dump (std::ostream& os);
};

class SSProjectFile : public SSPlainFile
{
public:
//  SSProjectFile ();
  SSProjectFile (const std::string& fileName, bool bOpen = false);
  SSProjectFile (CBaseIO* pio);

  virtual void Dump (std::ostream& os);
};

class SSUserFile : public SSHeaderFile
{
public:
//  SSUserFile ();
  SSUserFile (const std::string& fileName, bool bOpen = false);

  virtual bool CheckHeader ();

  virtual void Dump (std::ostream& os);

private:
};

#endif // !defined(AFX_SSFILES_H__A748503F_FA76_42CB_9EE0_A4FED9F1779B__INCLUDED_)

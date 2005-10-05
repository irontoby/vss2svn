// FileTypeCommand.h: interface for the CFileTypeCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILETYPECOMMAND_H__A66D5425_5DF0_46BD_8664_EF74FE905B1B__INCLUDED_)
#define AFX_FILETYPECOMMAND_H__A66D5425_5DF0_46BD_8664_EF74FE905B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Command.h"

class CFileTypeCommand : public CCommand
{
public:
  CFileTypeCommand ();

  virtual COptionInfoList GetOptionsInfo () const;
  virtual bool SetOption (const COption& option);
  virtual bool SetArguments (CArguments& args);
  void Execute ();

protected:
  std::string m_PhysFile;
};

#endif // !defined(AFX_FILETYPECOMMAND_H__A66D5425_5DF0_46BD_8664_EF74FE905B1B__INCLUDED_)

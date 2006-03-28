// FileTypeCommand.h: interface for the CFileTypeCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILETYPECOMMAND_H__A66D5425_5DF0_46BD_8664_EF74FE905B1B__INCLUDED_)
#define AFX_FILETYPECOMMAND_H__A66D5425_5DF0_46BD_8664_EF74FE905B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Command.h"

class CFileTypeCommand : public CMultiArgCommand
{
public:
  CFileTypeCommand ();

  virtual void Execute (const po::variables_map& vm, std::string const& arg);

protected:
};

#endif // !defined(AFX_FILETYPECOMMAND_H__A66D5425_5DF0_46BD_8664_EF74FE905B1B__INCLUDED_)

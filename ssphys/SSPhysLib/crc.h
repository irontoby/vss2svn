// crc.h: interface for the crc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CRC_H__47066454_F128_4446_94CE_29295817E6CE__INCLUDED_)
#define AFX_CRC_H__47066454_F128_4446_94CE_29295817E6CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

unsigned calc_crc32(const void* buf, unsigned size);
unsigned calc_crc16(const void* buf, unsigned size);

#endif // !defined(AFX_CRC_H__47066454_F128_4446_94CE_29295817E6CE__INCLUDED_)

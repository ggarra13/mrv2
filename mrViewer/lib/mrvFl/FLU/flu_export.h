// $Id: flu_export.h,v 1.2 2003/08/20 16:29:44 jbryan Exp $

/***************************************************************
 *                FLU - FLTK Utility Widgets 
 *  Copyright (C) 2002 Ohio Supercomputer Center, Ohio State University
 *
 * This file and its content is protected by a software license.
 * You should have received a copy of this license with this file.
 * If not, please contact the Ohio Supercomputer Center immediately:
 * Attn: Jason Bryan Re: FLU 1224 Kinnear Rd, Columbus, Ohio 43212
 * 
 ***************************************************************/

#ifndef _FLU_EXPORT_H
#define _FLU_EXPORT_H

#ifdef FLU_DLL
 #ifdef FLU_LIBRARY
  #define FLU_EXPORT __declspec(dllexport)
 #else
  #define FLU_EXPORT __declspec(dllimport)
 #endif
#else
 #define FLU_EXPORT
#endif

#endif

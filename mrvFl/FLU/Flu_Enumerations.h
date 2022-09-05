// $Id: Flu_Enumerations.h,v 1.16 2004/04/03 17:35:41 jbryan Exp $

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

#pragma once

#include "mrvFl/FLU/flu_export.h"

/* these enums are all global to conform to the fltk standard */

/*! Selection modes for FLU widgets that select stuff.
  Used by:
  Flu_Tree_Browser
*/
enum {
  FLU_NO_SELECT,
  FLU_SINGLE_SELECT,
  FLU_MULTI_SELECT
};

/*! Data insertion modes for FLU widgets that insert stuff.
  Used by:
  Flu_Tree_Browser
*/
enum {
  FLU_INSERT_FRONT,
  FLU_INSERT_BACK,
  FLU_INSERT_SORTED,
  FLU_INSERT_SORTED_REVERSE
};

/*! Selection drag modes for FLU widgets that select stuff (used while the mouse is being dragged).
  Used by:
  Flu_Tree_Browser
*/
enum {
  FLU_DRAG_IGNORE,
  FLU_DRAG_TO_SELECT,
  FLU_DRAG_TO_MOVE
};

/*! Callback reasons for FLU widgets that select stuff.
  Used by:
  Flu_Tree_Browser
*/
enum {
  FLU_HILIGHTED,
  FLU_UNHILIGHTED,
  FLU_SELECTED,
  FLU_UNSELECTED,
  FLU_OPENED,
  FLU_CLOSED,
  FLU_DOUBLE_CLICK,
  FLU_WIDGET_CALLBACK,
  FLU_MOVED_NODE,
  FLU_NEW_NODE,
  FLU_NOTHING
};

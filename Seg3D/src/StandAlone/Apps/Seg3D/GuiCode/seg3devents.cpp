/////////////////////////////////////////////////////////////////////////////
// Name:        seg3devents.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     10/08/2007 11:53:59
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "seg3devents.h"


DEFINE_EVENT_TYPE(wxEVT_CURSOR_INFORMATION_CHANGE)
DEFINE_EVENT_TYPE(wxEVT_BRUSH_RADIUS_CHANGE)
DEFINE_EVENT_TYPE(wxEVT_CROP_SET_RANGE)
DEFINE_EVENT_TYPE(wxEVT_CROP_SET_MINMAX)
DEFINE_EVENT_TYPE(wxEVT_SET_PROGRESS)
DEFINE_EVENT_TYPE(wxEVT_THRESHOLDTOOL_CHANGE)
DEFINE_EVENT_TYPE(wxEVT_MOVESCALETOOL_CHANGE)
DEFINE_EVENT_TYPE(wxEVT_VOLUME_INFO_PANEL)
DEFINE_EVENT_TYPE(wxEVT_MEASUREMENT_UPDATE)


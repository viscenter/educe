/////////////////////////////////////////////////////////////////////////////
// Name:        croppanel.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     10/08/2007 11:54:54
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 10/08/2007 11:54:54

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "croppanel.h"

////@begin XPM images
////@end XPM images


/*!
 * CropPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CropPanel, wxPanel )


/*!
 * CropPanel event table definition
 */

BEGIN_EVENT_TABLE( CropPanel, wxPanel )

////@begin CropPanel event table entries
////@end CropPanel event table entries

END_EVENT_TABLE()


/*!
 * CropPanel constructors
 */

CropPanel::CropPanel()
{
    Init();
}

CropPanel::CropPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CropPanel creator
 */

bool CropPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CropPanel creation
    SetParent(parent);
    CreateControls();
////@end CropPanel creation
    return true;
}


/*!
 * CropPanel destructor
 */

CropPanel::~CropPanel()
{
////@begin CropPanel destruction
////@end CropPanel destruction
}


/*!
 * Member initialisation
 */

void CropPanel::Init()
{
////@begin CropPanel member initialisation
////@end CropPanel member initialisation
}


/*!
 * Control creation for CropPanel
 */

void CropPanel::CreateControls()
{    
////@begin CropPanel content construction
    // Generated by DialogBlocks, 21/08/2007 10:54:10 (unregistered)

    if (!wxXmlResource::Get()->LoadPanel(this, GetParent(), _T("ID_CROPPANEL")))
        wxLogError(wxT("Missing wxXmlResource::Get()->Load() in OnInit()?"));
////@end CropPanel content construction

    // Create custom windows not generated automatically here.
////@begin CropPanel content initialisation
////@end CropPanel content initialisation
}


/*!
 * Should we show tooltips?
 */

bool CropPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CropPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CropPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CropPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CropPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CropPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CropPanel icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */



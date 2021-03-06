/////////////////////////////////////////////////////////////////////////////
// Name:        brushpanel.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     10/08/2007 11:53:59
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 10/08/2007 11:53:59

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include <StandAlone/Apps/Seg3D/Painter.h>
#include <Core/Containers/StringUtil.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "brushpanel.h"
#include "seg3devents.h"

////@begin XPM images
////@end XPM images


/*!
 * BrushPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( BrushPanel, wxPanel )


/*!
 * BrushPanel event table definition
 */

BEGIN_EVENT_TABLE( BrushPanel, wxPanel )

////@begin BrushPanel event table entries
    EVT_BUTTON( XRCID("CLOSE_BUTTON"), BrushPanel::OnCloseButtonClick )

////@end BrushPanel event table entries

    EVT_BUTTON( XRCID("FLOODFILL_BUTTON"), BrushPanel::OnFloodFillButtonClick )
    EVT_BUTTON( XRCID("FLOODERASE_BUTTON"), BrushPanel::OnFloodEraseButtonClick )

    // Custom entries
    EVT_SPINCTRL( XRCID("SPIN_BRUSH_RADIUS"), BrushPanel::SpinRadius)
    EVT_COMMAND( wxID_ANY, wxEVT_BRUSH_RADIUS_CHANGE, BrushPanel::UpdateRadius)

    EVT_CHECKBOX( XRCID("ERASEBRUSH"), BrushPanel::OnEraseBrushChange)
END_EVENT_TABLE()


/*!
 * BrushPanel constructors
 */

BrushPanel::BrushPanel()
{
    Init();
}

BrushPanel::BrushPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * BrushPanel creator
 */

bool BrushPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin BrushPanel creation
    SetParent(parent);
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
////@end BrushPanel creation
    return true;
}


/*!
 * BrushPanel destructor
 */

BrushPanel::~BrushPanel()
{
////@begin BrushPanel destruction
////@end BrushPanel destruction
}


/*!
 * Member initialisation
 */

void BrushPanel::Init()
{
////@begin BrushPanel member initialisation
    mRadius = NULL;
    mBrushErase = NULL;
////@end BrushPanel member initialisation
}


/*!
 * Control creation for BrushPanel
 */

void BrushPanel::CreateControls()
{    
////@begin BrushPanel content construction
    if (!wxXmlResource::Get()->LoadPanel(this, GetParent(), _T("ID_BRUSHPANEL")))
        wxLogError(wxT("Missing wxXmlResource::Get()->Load() in OnInit()?"));
    mRadius = XRCCTRL(*this, "SPIN_BRUSH_RADIUS", wxSpinCtrl);
    mBrushErase = XRCCTRL(*this, "ERASEBRUSH", wxCheckBox);
////@end BrushPanel content construction

    // Set default radius values
    mRadius->SetRange(1, 40);

    // Create custom windows not generated automatically here.
////@begin BrushPanel content initialisation
////@end BrushPanel content initialisation
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for FLOOD_FILL_BUTTON
 */
void BrushPanel::OnFloodFillButtonClick( wxCommandEvent& event )
{
  wxBeginBusyCursor();
  SCIRun::ThrowSkinnerSignalEvent *tsse =
    new SCIRun::ThrowSkinnerSignalEvent("Painter::FloodFillLayer");
  SCIRun::Painter::ThrowSkinnerSignal(tsse);
  wxEndBusyCursor();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for FLOOD_ERASE_BUTTON
 */
void BrushPanel::OnFloodEraseButtonClick( wxCommandEvent& event )
{
  wxBeginBusyCursor();
  SCIRun::ThrowSkinnerSignalEvent *tsse =
    new SCIRun::ThrowSkinnerSignalEvent("Painter::FloodFillClear");
  SCIRun::Painter::ThrowSkinnerSignal(tsse);
  wxEndBusyCursor();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for CLOSE_BUTTON
 */

void BrushPanel::OnCloseButtonClick( wxCommandEvent& event )
{
  SCIRun::Painter::global_seg3dframe_pointer_->HideTool();
}

void
BrushPanel::SpinRadius( wxSpinEvent &event)
{
  SCIRun::ThrowSkinnerSignalEvent *tsse =
    new SCIRun::ThrowSkinnerSignalEvent("Painter::UpdateBrushRadius");
  tsse->add_var("Painter::brush_radius",
                SCIRun::to_string(event.GetPosition()));
  tsse->add_var("Painter::brush_force_erasing",
                SCIRun::to_string(mBrushErase->GetValue()));
  SCIRun::Painter::ThrowSkinnerSignal(tsse);
}


void
BrushPanel::UpdateRadius( wxCommandEvent &event)
{
    mRadius->SetValue(event.GetInt());
}


void
BrushPanel::OnEraseBrushChange( wxCommandEvent &event)
{
  SCIRun::ThrowSkinnerSignalEvent *tsse =
    new SCIRun::ThrowSkinnerSignalEvent("Painter::UpdateBrushRadius");
  tsse->add_var("Painter::brush_radius",
                SCIRun::to_string(mRadius->GetValue()));
  tsse->add_var("Painter::brush_force_erasing",
                SCIRun::to_string(mBrushErase->GetValue()));
  SCIRun::Painter::ThrowSkinnerSignal(tsse);
}


/*!
 * Should we show tooltips?
 */

bool BrushPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap BrushPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin BrushPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end BrushPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon BrushPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin BrushPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end BrushPanel icon retrieval
}


/////////////////////////////////////////////////////////////////////////////
// Name:        measurementtool.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     Wed 09 Jul 2008 10:46:45 MDT
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include <StandAlone/Apps/Seg3D/Painter.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "measurementtool.h"
#include "seg3devents.h"
#include <StandAlone/Apps/Seg3D/Seg3DwxGuiUtils.h>

////@begin XPM images
////@end XPM images


/*!
 * MeasurementTool type definition
 */

IMPLEMENT_DYNAMIC_CLASS( MeasurementTool, wxPanel )


/*!
 * MeasurementTool event table definition
 */

BEGIN_EVENT_TABLE( MeasurementTool, wxPanel )

////@begin MeasurementTool event table entries
////@end MeasurementTool event table entries

    EVT_BUTTON( XRCID("CLEAR_SEEDS_BUTTON"), MeasurementTool::OnClearSeedsButtonClick )
    EVT_BUTTON( XRCID("CLOSE_BUTTON"), MeasurementTool::OnCloseButtonClick )
    EVT_COMMAND(wxID_ANY, wxEVT_MEASUREMENT_UPDATE, MeasurementTool::OnUpdateMeasurements)

END_EVENT_TABLE()


/*!
 * MeasurementTool constructors
 */

MeasurementTool::MeasurementTool()
{
    Init();
}

MeasurementTool::MeasurementTool( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * MeasurementTool creator
 */

bool MeasurementTool::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin MeasurementTool creation
    SetParent(parent);
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end MeasurementTool creation
    return true;
}


/*!
 * MeasurementTool destructor
 */

MeasurementTool::~MeasurementTool()
{
////@begin MeasurementTool destruction
////@end MeasurementTool destruction
}


/*!
 * Member initialisation
 */

void MeasurementTool::Init()
{
////@begin MeasurementTool member initialisation
    mInfoText = NULL;
////@end MeasurementTool member initialisation
}


/*!
 * Control creation for MeasurementTool
 */

void MeasurementTool::CreateControls()
{    
////@begin MeasurementTool content construction
    if (!wxXmlResource::Get()->LoadPanel(this, GetParent(), _T("ID_MEASUREMENTTOOL")))
        wxLogError(wxT("Missing wxXmlResource::Get()->Load() in OnInit()?"));
    mInfoText = XRCCTRL(*this, "wxID_STATIC", wxStaticText);
////@end MeasurementTool content construction

    // Create custom windows not generated automatically here.
////@begin MeasurementTool content initialisation
////@end MeasurementTool content initialisation
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for CLEAR_SEEDS_BUTTON
 */
void MeasurementTool::OnClearSeedsButtonClick( wxCommandEvent& event )
{
  SCIRun::Painter::ThrowSkinnerSignal("Painter::SetLayer");
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for CLOSE_BUTTON
 */
void MeasurementTool::OnCloseButtonClick( wxCommandEvent& event )
{
  SCIRun::Painter::global_seg3dframe_pointer_->HideTool();
}

void MeasurementTool::OnUpdateMeasurements( wxCommandEvent& event )
{
  vector<SCIRun::Point> *points =
    (vector<SCIRun::Point> *)event.GetClientData();
  
  std::ostringstream ostrm;
  if (points->size() == 0)
  {
    ostrm << "No points.";
  }
  for (size_t i = 0; i < points->size(); i++)
  {
    ostrm << "Point #" << i << " " << (*points)[i].x() << " " << (*points)[i].y() << " " << (*points)[i].z() << "\n";
    if (i < points->size()-1)
    {
      ostrm << "  Length = " <<
        ((*points)[i+1] - (*points)[i]).length() << "\n";
    }
  }
  mInfoText->SetLabel(std2wx(ostrm.str()));
  delete points;
}


/*!
 * Should we show tooltips?
 */

bool MeasurementTool::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap MeasurementTool::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin MeasurementTool bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end MeasurementTool bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon MeasurementTool::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin MeasurementTool icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end MeasurementTool icon retrieval
}

/////////////////////////////////////////////////////////////////////////////
// Name:        itkconfidenceconnectedfilter.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     14/08/2007 13:04:00
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 14/08/2007 13:04:00

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

#include "itkconfidenceconnectedfilter.h"
#include "seg3devents.h"

////@begin XPM images
////@end XPM images


/*!
 * ITKConfidenceConnectedFilter type definition
 */

IMPLEMENT_DYNAMIC_CLASS( ITKConfidenceConnectedFilter, wxPanel )


/*!
 * ITKConfidenceConnectedFilter event table definition
 */

BEGIN_EVENT_TABLE( ITKConfidenceConnectedFilter, wxPanel )

////@begin ITKConfidenceConnectedFilter event table entries
////@end ITKConfidenceConnectedFilter event table entries
    EVT_BUTTON( XRCID("CLEAR_SEEDS_BUTTON"), ITKConfidenceConnectedFilter::OnClearSeedsButtonClick )
    EVT_BUTTON( XRCID("START_BUTTON"), ITKConfidenceConnectedFilter::OnStartButtonClick )
    EVT_BUTTON( XRCID("STOP_BUTTON"), ITKConfidenceConnectedFilter::OnStopButtonClick )
    EVT_BUTTON( XRCID("CLOSE_BUTTON"), ITKConfidenceConnectedFilter::OnCloseButtonClick )

    EVT_COMMAND( wxID_ANY, wxEVT_SET_PROGRESS, ITKConfidenceConnectedFilter::OnSetProgress)

END_EVENT_TABLE()


/*!
 * ITKConfidenceConnectedFilter constructors
 */

ITKConfidenceConnectedFilter::ITKConfidenceConnectedFilter()
{
    Init();
}

ITKConfidenceConnectedFilter::ITKConfidenceConnectedFilter( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * ITKConfidenceConnectedFilter creator
 */

bool ITKConfidenceConnectedFilter::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ITKConfidenceConnectedFilter creation
    SetParent(parent);
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
////@end ITKConfidenceConnectedFilter creation
    return true;
}


/*!
 * ITKConfidenceConnectedFilter destructor
 */

ITKConfidenceConnectedFilter::~ITKConfidenceConnectedFilter()
{
////@begin ITKConfidenceConnectedFilter destruction
////@end ITKConfidenceConnectedFilter destruction
}


/*!
 * Member initialisation
 */

void ITKConfidenceConnectedFilter::Init()
{
////@begin ITKConfidenceConnectedFilter member initialisation
    mIterations = NULL;
    mMultipliers = NULL;
    mPercentage = NULL;
////@end ITKConfidenceConnectedFilter member initialisation

    disabler_ = NULL;
}


/*!
 * Control creation for ITKConfidenceConnectedFilter
 */

void ITKConfidenceConnectedFilter::CreateControls()
{    
////@begin ITKConfidenceConnectedFilter content construction
    if (!wxXmlResource::Get()->LoadPanel(this, GetParent(), _T("ID_ITKCONFIDENCECONNECTEDFILTER")))
        wxLogError(wxT("Missing wxXmlResource::Get()->Load() in OnInit()?"));
    mIterations = XRCCTRL(*this, "SPIN_ITERATIONS", wxSpinCtrl);
    mMultipliers = XRCCTRL(*this, "SPIN_MULTIPLIER", wxSpinCtrl);
    mPercentage = XRCCTRL(*this, "ID_GAUGE1", wxGauge);
////@end ITKConfidenceConnectedFilter content construction

    // Create custom windows not generated automatically here.
////@begin ITKConfidenceConnectedFilter content initialisation
////@end ITKConfidenceConnectedFilter content initialisation
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for CLEAR_SEEDS_BUTTON
 */
void ITKConfidenceConnectedFilter::OnClearSeedsButtonClick( wxCommandEvent& event )
{
  SCIRun::Painter::ThrowSkinnerSignal("Painter::SetLayer");
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for START_BUTTON
 */
void ITKConfidenceConnectedFilter::OnStartButtonClick( wxCommandEvent& event )
{
  SCIRun::ThrowSkinnerSignalEvent *tsse =
    new SCIRun::ThrowSkinnerSignalEvent("Painter::FinishTool");
  tsse->add_var("ITKConfidenceConnectedImageFilterTool::numberOfIterations",
                SCIRun::to_string(mIterations->GetValue()));
  tsse->add_var("ITKConfidenceConnectedImageFilterTool::multiplier",
                SCIRun::to_string(mMultipliers->GetValue()));
  SCIRun::Painter::ThrowSkinnerSignal(tsse, false);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for STOP_BUTTON
 */
void ITKConfidenceConnectedFilter::OnStopButtonClick( wxCommandEvent& event )
{
  //SCIRun::Painter::ThrowSkinnerSignal("Painter::AbortFilterOn");
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for CLOSE_BUTTON
 */
void ITKConfidenceConnectedFilter::OnCloseButtonClick( wxCommandEvent& event )
{
  SCIRun::Painter::global_seg3dframe_pointer_->HideTool();
}


void ITKConfidenceConnectedFilter::OnSetProgress( wxCommandEvent &event)
{
  int progress = event.GetInt();
  
  if (progress < 0)
  {
    wxBeginBusyCursor();
    disabler_ = new wxWindowDisabler();
    progress = 0;
  }
  if (progress > 100)
  {
    if (disabler_) { delete disabler_; disabler_ = 0; }
    wxEndBusyCursor();
    progress = 100;
  }
  if (progress == 0 || progress > mPercentage->GetValue())
  {
    mPercentage->SetValue(progress);
  }
}

/*!
 * Should we show tooltips?
 */

bool ITKConfidenceConnectedFilter::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap ITKConfidenceConnectedFilter::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin ITKConfidenceConnectedFilter bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end ITKConfidenceConnectedFilter bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon ITKConfidenceConnectedFilter::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin ITKConfidenceConnectedFilter icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end ITKConfidenceConnectedFilter icon retrieval
}

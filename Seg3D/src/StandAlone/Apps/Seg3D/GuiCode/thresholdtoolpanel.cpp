/////////////////////////////////////////////////////////////////////////////
// Name:        thresholdtoolpanel.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     Wed 09 Apr 2008 17:18:05 MDT
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include <StandAlone/Apps/Seg3D/Painter.h>
#include <StandAlone/Apps/Seg3D/Seg3DwxGuiUtils.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "thresholdtoolpanel.h"
#include <sstream>

////@begin XPM images
////@end XPM images


/*!
 * ThresholdToolPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( ThresholdToolPanel, wxPanel )


/*!
 * ThresholdToolPanel event table definition
 */

BEGIN_EVENT_TABLE( ThresholdToolPanel, wxPanel )

////@begin ThresholdToolPanel event table entries
////@end ThresholdToolPanel event table entries

  EVT_BUTTON( XRCID("CLEAR_SEEDS_BUTTON"), ThresholdToolPanel::OnClearSeedsButtonClick )
  EVT_BUTTON( XRCID("START_BUTTON"), ThresholdToolPanel::OnStartButtonClick )
  EVT_BUTTON( XRCID("CLOSE_BUTTON"), ThresholdToolPanel::OnCloseButtonClick )
  EVT_COMMAND_SCROLL( XRCID("ID_SLIDER1"), ThresholdToolPanel::OnLowerSlider )
  EVT_COMMAND_SCROLL( XRCID("ID_SLIDER"), ThresholdToolPanel::OnUpperSlider )
  EVT_TEXT( XRCID("ID_TEXTCTRL"), ThresholdToolPanel::OnLowerText )
  EVT_TEXT( XRCID("ID_TEXTCTRL1"), ThresholdToolPanel::OnUpperText )
  EVT_COMMAND (wxID_ANY, wxEVT_THRESHOLDTOOL_CHANGE, ThresholdToolPanel::OnThresholdToolChange )

END_EVENT_TABLE()


/*!
 * ThresholdToolPanel constructors
 */

ThresholdToolPanel::ThresholdToolPanel()
{
    Init();
}

ThresholdToolPanel::ThresholdToolPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * ThresholdToolPanel creator
 */

bool ThresholdToolPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ThresholdToolPanel creation
    SetParent(parent);
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
////@end ThresholdToolPanel creation
    return true;
}


/*!
 * ThresholdToolPanel destructor
 */

ThresholdToolPanel::~ThresholdToolPanel()
{
////@begin ThresholdToolPanel destruction
////@end ThresholdToolPanel destruction
}


/*!
 * Member initialisation
 */

void ThresholdToolPanel::Init()
{
////@begin ThresholdToolPanel member initialisation
    mMinValue = NULL;
    mMaxValue = NULL;
    mLowerValue = NULL;
    mLowerSlider = NULL;
    mUpperValue = NULL;
    mUpperSlider = NULL;
////@end ThresholdToolPanel member initialisation

    minval_ = 0.0;
    maxval_ = 0.0;
    lower_ = AIR_NAN;
    upper_ = AIR_NAN;
}


/*!
 * Control creation for ThresholdToolPanel
 */

void ThresholdToolPanel::CreateControls()
{    
////@begin ThresholdToolPanel content construction
    if (!wxXmlResource::Get()->LoadPanel(this, GetParent(), _T("ID_THRESHOLDTOOLPANEL")))
        wxLogError(wxT("Missing wxXmlResource::Get()->Load() in OnInit()?"));
    mMinValue = XRCCTRL(*this, "ID_TEXTCTRL2", wxTextCtrl);
    mMaxValue = XRCCTRL(*this, "ID_TEXTCTRL3", wxTextCtrl);
    mLowerValue = XRCCTRL(*this, "ID_TEXTCTRL", wxTextCtrl);
    mLowerSlider = XRCCTRL(*this, "ID_SLIDER1", wxSlider);
    mUpperValue = XRCCTRL(*this, "ID_TEXTCTRL1", wxTextCtrl);
    mUpperSlider = XRCCTRL(*this, "ID_SLIDER", wxSlider);
    // Set validators
    if (FindWindow(XRCID("ID_TEXTCTRL2")))
        FindWindow(XRCID("ID_TEXTCTRL2"))->SetValidator( wxTextValidator(wxFILTER_NUMERIC) );
    if (FindWindow(XRCID("ID_TEXTCTRL3")))
        FindWindow(XRCID("ID_TEXTCTRL3"))->SetValidator( wxTextValidator(wxFILTER_NUMERIC) );
    if (FindWindow(XRCID("ID_TEXTCTRL")))
        FindWindow(XRCID("ID_TEXTCTRL"))->SetValidator( wxTextValidator(wxFILTER_NUMERIC) );
    if (FindWindow(XRCID("ID_TEXTCTRL1")))
        FindWindow(XRCID("ID_TEXTCTRL1"))->SetValidator( wxTextValidator(wxFILTER_NUMERIC) );
////@end ThresholdToolPanel content construction

    // Create custom windows not generated automatically here.
////@begin ThresholdToolPanel content initialisation
////@end ThresholdToolPanel content initialisation
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for CLEAR_SEEDS_BUTTON
 */
void ThresholdToolPanel::OnClearSeedsButtonClick( wxCommandEvent& event )
{
  SCIRun::Painter::ThrowSkinnerSignal("Painter::SetLayer");
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for START_BUTTON
 */
void ThresholdToolPanel::OnStartButtonClick( wxCommandEvent& event )
{
  wxBeginBusyCursor();
  SCIRun::Painter::ThrowSkinnerSignal("Painter::FinishTool");
  wxEndBusyCursor();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for CLOSE_BUTTON
 */
void ThresholdToolPanel::OnCloseButtonClick( wxCommandEvent& event )
{
  SCIRun::Painter::global_seg3dframe_pointer_->HideTool();
}

void ThresholdToolPanel::OnThresholdToolChange( wxCommandEvent& event )
{
  ThresholdToolChangeStruct *info =
    (ThresholdToolChangeStruct *)event.GetClientData();

  minval_ = info->minval;
  maxval_ = info->maxval;
  mMinValue->Clear(); *mMinValue << info->minval;
  mMaxValue->Clear(); *mMaxValue << info->maxval;
  mLowerValue->Clear(); *mLowerValue << info->lower;
  mUpperValue->Clear(); *mUpperValue << info->upper;

  // Update the slider
  double lowern = (info->lower - minval_) / (maxval_ - minval_);
  if (lowern < 0.0) lowern = 0.0;
  if (lowern > 1.0) lowern = 1.0;
  mLowerSlider->SetValue((int)(1000.0 * lowern));

  double uppern = (info->upper - minval_) / (maxval_ - minval_);
  if (uppern < 0.0) uppern = 0.0;
  if (uppern > 1.0) uppern = 1.0;
  mUpperSlider->SetValue((int)(1000.0 * uppern));

  delete info;
}

void ThresholdToolPanel::OnLowerSlider( wxScrollEvent& event )
{
  double lower =
    mLowerSlider->GetValue() / 1000.0 * (maxval_ - minval_) + minval_;
  if (lower != lower_)
  {
    lower_ = lower;
    mLowerValue->Clear();
    *mLowerValue << lower_;

    PushToSkinner();
  }
}

void ThresholdToolPanel::OnUpperSlider( wxScrollEvent& event )
{
  double upper =
    mUpperSlider->GetValue() / 1000.0 * (maxval_ - minval_) + minval_;
  if (upper != upper_)
  {
    upper_ = upper;
    mUpperValue->Clear();
    *mUpperValue << upper_;

    PushToSkinner();
  }
}

void ThresholdToolPanel::OnLowerText( wxCommandEvent& event )
{
  if (mLowerValue && mLowerValue->IsModified())
  {
    const string str = wx2std(mLowerValue->GetValue());
    std::istringstream sstr(str);
    sstr >> lower_;

    // Update the slider
    double lowern = (lower_ - minval_) / (maxval_ - minval_);
    if (lowern < 0.0) lowern = 0.0;
    if (lowern > 1.0) lowern = 1.0;
    mLowerSlider->SetValue((int)(1000.0 * lowern));

    PushToSkinner();
  }
}

void ThresholdToolPanel::OnUpperText( wxCommandEvent& event )
{
  if (mUpperValue && mUpperValue->IsModified())
  {
    const string str = wx2std(mUpperValue->GetValue());
    std::istringstream sstr(str);
    sstr >> upper_;

    // Update the slider
    double uppern = (upper_ - minval_) / (maxval_ - minval_);
    if (uppern < 0.0) uppern = 0.0;
    if (uppern > 1.0) uppern = 1.0;
    mUpperSlider->SetValue((int)(1000.0 * uppern));

    PushToSkinner();
  }
}


/*!
 * should we show tooltips?
 */

bool ThresholdToolPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap ThresholdToolPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin ThresholdToolPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end ThresholdToolPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon ThresholdToolPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin ThresholdToolPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end ThresholdToolPanel icon retrieval
}


void ThresholdToolPanel::PushToSkinner()
{
  SCIRun::ThrowSkinnerSignalEvent *tsse =
    new SCIRun::ThrowSkinnerSignalEvent("Painter::UpdateThresholdTool");
  tsse->add_var("Painter::thresholdtool_lower",
                wx2std(mLowerValue->GetValue()));
  tsse->add_var("Painter::thresholdtool_upper",
                wx2std(mUpperValue->GetValue()));
  SCIRun::Painter::ThrowSkinnerSignal(tsse, false);
}

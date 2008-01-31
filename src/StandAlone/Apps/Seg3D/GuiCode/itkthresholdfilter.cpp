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

#include "itkthresholdfilter.h"
#include "seg3devents.h"
#include <StandAlone/Apps/Seg3D/Seg3DwxGuiUtils.h>

////@begin XPM images
////@end XPM images



/*!
 * ITKThresholdFilter type definition
 */

IMPLEMENT_DYNAMIC_CLASS( ITKThresholdFilter, wxPanel )


/*!
 * ITKThresholdFilter event table definition
 */

BEGIN_EVENT_TABLE( ITKThresholdFilter, wxPanel )

  EVT_BUTTON( XRCID("START_BUTTON"), ITKThresholdFilter::OnStartButtonClick )
  EVT_BUTTON( XRCID("CLOSE_BUTTON"), ITKThresholdFilter::OnCloseButtonClick )

  EVT_COMMAND( wxID_ANY, wxEVT_SET_PROGRESS, ITKThresholdFilter::OnSetProgress)

////@begin ITKThresholdFilter event table entries
////@end ITKThresholdFilter event table entries

END_EVENT_TABLE()


/*!
 * ITKThresholdFilter constructors
 */

ITKThresholdFilter::ITKThresholdFilter()
{
    Init();
}

ITKThresholdFilter::ITKThresholdFilter( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * ITKThresholdFilter creator
 */

bool ITKThresholdFilter::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ITKThresholdFilter creation
    SetParent(parent);
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
////@end ITKThresholdFilter creation
    return true;
}


/*!
 * ITKThresholdFilter destructor
 */

ITKThresholdFilter::~ITKThresholdFilter()
{
////@begin ITKThresholdFilter destruction
////@end ITKThresholdFilter destruction
}


/*!
 * Member initialisation
 */

void ITKThresholdFilter::Init()
{
////@begin ITKThresholdFilter member initialisation
    mMinValue = NULL;
    mMaxValue = NULL;
    mPercentage = NULL;
////@end ITKThresholdFilter member initialisation

    disabler_ = NULL;
}


/*!
 * Control creation for ITKThresholdFilter
 */

void ITKThresholdFilter::CreateControls()
{    
////@begin ITKThresholdFilter content construction
    if (!wxXmlResource::Get()->LoadPanel(this, GetParent(), _T("ID_ITKTHRESHOLDFILTER")))
        wxLogError(wxT("Missing wxXmlResource::Get()->Load() in OnInit()?"));
    mMinValue = XRCCTRL(*this, "MINVALUE", wxTextCtrl);
    mMaxValue = XRCCTRL(*this, "MAXVALUE", wxTextCtrl);
    mPercentage = XRCCTRL(*this, "ID_GAUGE", wxGauge);
    // Set validators
    if (FindWindow(XRCID("MINVALUE")))
        FindWindow(XRCID("MINVALUE"))->SetValidator( wxTextValidator(wxFILTER_NUMERIC) );
    if (FindWindow(XRCID("MAXVALUE")))
        FindWindow(XRCID("MAXVALUE"))->SetValidator( wxTextValidator(wxFILTER_NUMERIC) );
////@end ITKThresholdFilter content construction

    // Create custom windows not generated automatically here.
////@begin ITKThresholdFilter content initialisation
////@end ITKThresholdFilter content initialisation
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for START_BUTTON
 */
void ITKThresholdFilter::OnStartButtonClick( wxCommandEvent& event )
{
  wxBeginBusyCursor();
  SCIRun::ThrowSkinnerSignalEvent *tsse = new SCIRun::ThrowSkinnerSignalEvent("Painter::FinishTool");
  tsse->add_var("ITKThresholdFilter::mMinValue", wx2std(mMinValue->GetValue()));
  tsse->add_var("ITKThresholdFilter::mMaxValue", wx2std(mMaxValue->GetValue()));
  SCIRun::Painter::ThrowSkinnerSignal(tsse);
  wxEndBusyCursor();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for CLOSE_BUTTON
 */
void ITKThresholdFilter::OnCloseButtonClick( wxCommandEvent& event )
{
  SCIRun::Painter::global_seg3dframe_pointer_->HideTool();
}

void ITKThresholdFilter::OnSetProgress( wxCommandEvent &event)
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

bool ITKThresholdFilter::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap ITKThresholdFilter::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin ITKThresholdFilter bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end ITKThresholdFilter bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon ITKThresholdFilter::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin ITKThresholdFilter icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end ITKThresholdFilter icon retrieval
}


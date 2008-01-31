/////////////////////////////////////////////////////////////////////////////
// Name:        thresholdfilter.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     19/12/2007 11:54:43
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

////@begin includes
////@end includes

#include "thresholdfilter.h"

////@begin XPM images
////@end XPM images


/*!
 * ThresholdFilter type definition
 */

IMPLEMENT_DYNAMIC_CLASS( ThresholdFilter, wxPanel )


/*!
 * ThresholdFilter event table definition
 */

BEGIN_EVENT_TABLE( ThresholdFilter, wxPanel )

////@begin ThresholdFilter event table entries
////@end ThresholdFilter event table entries

END_EVENT_TABLE()


/*!
 * ThresholdFilter constructors
 */

ThresholdFilter::ThresholdFilter()
{
    Init();
}

ThresholdFilter::ThresholdFilter( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * ThresholdFilter creator
 */

bool ThresholdFilter::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ThresholdFilter creation
    SetParent(parent);
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
////@end ThresholdFilter creation
    return true;
}


/*!
 * ThresholdFilter destructor
 */

ThresholdFilter::~ThresholdFilter()
{
////@begin ThresholdFilter destruction
////@end ThresholdFilter destruction
}


/*!
 * Member initialisation
 */

void ThresholdFilter::Init()
{
////@begin ThresholdFilter member initialisation
    mMinValue = NULL;
    mMaxValue = NULL;
    mPercentage = NULL;
////@end ThresholdFilter member initialisation
}


/*!
 * Control creation for ThresholdFilter
 */

void ThresholdFilter::CreateControls()
{    
////@begin ThresholdFilter content construction
    if (!wxXmlResource::Get()->LoadPanel(this, GetParent(), _T("ID_THRESHOLDFILTER")))
        wxLogError(wxT("Missing wxXmlResource::Get()->Load() in OnInit()?"));
    mMinValue = XRCCTRL(*this, "MINVALUE", wxTextCtrl);
    mMaxValue = XRCCTRL(*this, "MAXVALUE", wxTextCtrl);
    mPercentage = XRCCTRL(*this, "ID_GAUGE", wxGauge);
    // Set validators
    if (FindWindow(XRCID("MINVALUE")))
        FindWindow(XRCID("MINVALUE"))->SetValidator( wxTextValidator(wxFILTER_NUMERIC) );
    if (FindWindow(XRCID("MAXVALUE")))
        FindWindow(XRCID("MAXVALUE"))->SetValidator( wxTextValidator(wxFILTER_NUMERIC) );
////@end ThresholdFilter content construction

    // Create custom windows not generated automatically here.
////@begin ThresholdFilter content initialisation
////@end ThresholdFilter content initialisation
}


/*!
 * Should we show tooltips?
 */

bool ThresholdFilter::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap ThresholdFilter::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin ThresholdFilter bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end ThresholdFilter bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon ThresholdFilter::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin ThresholdFilter icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end ThresholdFilter icon retrieval
}

/////////////////////////////////////////////////////////////////////////////
// Name:        thresholdfilter.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     19/12/2007 11:54:43
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _THRESHOLDFILTER_H_
#define _THRESHOLDFILTER_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/xrc/xmlres.h"
#include "wx/valtext.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_THRESHOLDFILTER 10039
#define SYMBOL_THRESHOLDFILTER_STYLE 0
#define SYMBOL_THRESHOLDFILTER_TITLE _("ThresholdFilter")
#define SYMBOL_THRESHOLDFILTER_IDNAME ID_THRESHOLDFILTER
#define SYMBOL_THRESHOLDFILTER_SIZE wxSize(200, -1)
#define SYMBOL_THRESHOLDFILTER_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * ThresholdFilter class declaration
 */

class ThresholdFilter: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( ThresholdFilter )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ThresholdFilter();
    ThresholdFilter( wxWindow* parent, wxWindowID id = SYMBOL_THRESHOLDFILTER_IDNAME, const wxPoint& pos = SYMBOL_THRESHOLDFILTER_POSITION, const wxSize& size = SYMBOL_THRESHOLDFILTER_SIZE, long style = SYMBOL_THRESHOLDFILTER_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_THRESHOLDFILTER_IDNAME, const wxPoint& pos = SYMBOL_THRESHOLDFILTER_POSITION, const wxSize& size = SYMBOL_THRESHOLDFILTER_SIZE, long style = SYMBOL_THRESHOLDFILTER_STYLE );

    /// Destructor
    ~ThresholdFilter();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin ThresholdFilter event handler declarations

////@end ThresholdFilter event handler declarations

////@begin ThresholdFilter member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end ThresholdFilter member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin ThresholdFilter member variables
    wxTextCtrl* mMinValue;
    wxTextCtrl* mMaxValue;
    wxGauge* mPercentage;
////@end ThresholdFilter member variables
};

#endif
    // _THRESHOLDFILTER_H_

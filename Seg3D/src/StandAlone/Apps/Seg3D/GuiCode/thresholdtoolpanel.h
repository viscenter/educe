/////////////////////////////////////////////////////////////////////////////
// Name:        thresholdtoolpanel.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     Wed 09 Apr 2008 17:18:05 MDT
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _THRESHOLDTOOLPANEL_H_
#define _THRESHOLDTOOLPANEL_H_


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
#define ID_THRESHOLDTOOLPANEL 10012
#define SYMBOL_THRESHOLDTOOLPANEL_STYLE 0
#define SYMBOL_THRESHOLDTOOLPANEL_TITLE _("ThresholdToolPanel")
#define SYMBOL_THRESHOLDTOOLPANEL_IDNAME ID_THRESHOLDTOOLPANEL
#define SYMBOL_THRESHOLDTOOLPANEL_SIZE wxSize(200, -1)
#define SYMBOL_THRESHOLDTOOLPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * ThresholdToolPanel class declaration
 */

class ThresholdToolPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( ThresholdToolPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ThresholdToolPanel();
    ThresholdToolPanel( wxWindow* parent, wxWindowID id = SYMBOL_THRESHOLDTOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_THRESHOLDTOOLPANEL_POSITION, const wxSize& size = SYMBOL_THRESHOLDTOOLPANEL_SIZE, long style = SYMBOL_THRESHOLDTOOLPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_THRESHOLDTOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_THRESHOLDTOOLPANEL_POSITION, const wxSize& size = SYMBOL_THRESHOLDTOOLPANEL_SIZE, long style = SYMBOL_THRESHOLDTOOLPANEL_STYLE );

    /// Destructor
    ~ThresholdToolPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    void OnClearSeedsButtonClick( wxCommandEvent& event );
    void OnStartButtonClick( wxCommandEvent& event );
    void OnCloseButtonClick( wxCommandEvent& event );
    void OnThresholdToolChange( wxCommandEvent& event );
    void OnLowerSlider( wxScrollEvent& event );
    void OnUpperSlider( wxScrollEvent& event );
    void OnLowerText ( wxCommandEvent& event );
    void OnUpperText ( wxCommandEvent& event );

////@begin ThresholdToolPanel event handler declarations

////@end ThresholdToolPanel event handler declarations

////@begin ThresholdToolPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end ThresholdToolPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void PushToSkinner();

////@begin ThresholdToolPanel member variables
    wxTextCtrl* mMinValue;
    wxTextCtrl* mMaxValue;
    wxTextCtrl* mLowerValue;
    wxSlider* mLowerSlider;
    wxTextCtrl* mUpperValue;
    wxSlider* mUpperSlider;
////@end ThresholdToolPanel member variables

    double minval_;
    double maxval_;
    double upper_;
    double lower_;
};

#endif
    // _THRESHOLDTOOLPANEL_H_

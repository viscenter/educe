/////////////////////////////////////////////////////////////////////////////
// Name:        intensitycorrectionfilterpanel.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     Tue 08 Jul 2008 16:45:24 MDT
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _INTENSITYCORRECTIONFILTERPANEL_H_
#define _INTENSITYCORRECTIONFILTERPANEL_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/xrc/xmlres.h"
#include "wx/spinctrl.h"
#include "wx/valtext.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxSpinCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_INTENSITYCORRECTIONFILTER 10039
#define SYMBOL_INTENSITYCORRECTIONFILTERPANEL_STYLE 0
#define SYMBOL_INTENSITYCORRECTIONFILTERPANEL_TITLE _("IntensityCorrectionFilter")
#define SYMBOL_INTENSITYCORRECTIONFILTERPANEL_IDNAME ID_INTENSITYCORRECTIONFILTER
#define SYMBOL_INTENSITYCORRECTIONFILTERPANEL_SIZE wxSize(200, -1)
#define SYMBOL_INTENSITYCORRECTIONFILTERPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * IntensityCorrectionFilterPanel class declaration
 */

class IntensityCorrectionFilterPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( IntensityCorrectionFilterPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    IntensityCorrectionFilterPanel();
    IntensityCorrectionFilterPanel( wxWindow* parent, wxWindowID id = SYMBOL_INTENSITYCORRECTIONFILTERPANEL_IDNAME, const wxPoint& pos = SYMBOL_INTENSITYCORRECTIONFILTERPANEL_POSITION, const wxSize& size = SYMBOL_INTENSITYCORRECTIONFILTERPANEL_SIZE, long style = SYMBOL_INTENSITYCORRECTIONFILTERPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_INTENSITYCORRECTIONFILTERPANEL_IDNAME, const wxPoint& pos = SYMBOL_INTENSITYCORRECTIONFILTERPANEL_POSITION, const wxSize& size = SYMBOL_INTENSITYCORRECTIONFILTERPANEL_SIZE, long style = SYMBOL_INTENSITYCORRECTIONFILTERPANEL_STYLE );

    /// Destructor
    ~IntensityCorrectionFilterPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void OnStartButtonClick( wxCommandEvent& event );
    void OnCloseButtonClick( wxCommandEvent& event );
    void OnSetProgress( wxCommandEvent& event );

////@begin IntensityCorrectionFilterPanel event handler declarations

////@end IntensityCorrectionFilterPanel event handler declarations

////@begin IntensityCorrectionFilterPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end IntensityCorrectionFilterPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin IntensityCorrectionFilterPanel member variables
    wxSpinCtrl* mOrder;
    wxTextCtrl* mEdge;
    wxGauge* mPercentage;
////@end IntensityCorrectionFilterPanel member variables

    wxWindowDisabler *disabler_;
};

#endif
    // _INTENSITYCORRECTIONFILTERPANEL_H_

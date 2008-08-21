/////////////////////////////////////////////////////////////////////////////
// Name:        measurementtool.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     Wed 09 Jul 2008 10:46:45 MDT
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _MEASUREMENTTOOL_H_
#define _MEASUREMENTTOOL_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/xrc/xmlres.h"
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
#define ID_MEASUREMENTTOOL 10069
#define SYMBOL_MEASUREMENTTOOL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_MEASUREMENTTOOL_TITLE _("MeasurementTool")
#define SYMBOL_MEASUREMENTTOOL_IDNAME ID_MEASUREMENTTOOL
#define SYMBOL_MEASUREMENTTOOL_SIZE wxSize(200, -1)
#define SYMBOL_MEASUREMENTTOOL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * MeasurementTool class declaration
 */

class MeasurementTool: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( MeasurementTool )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    MeasurementTool();
    MeasurementTool( wxWindow* parent, wxWindowID id = SYMBOL_MEASUREMENTTOOL_IDNAME, const wxPoint& pos = SYMBOL_MEASUREMENTTOOL_POSITION, const wxSize& size = SYMBOL_MEASUREMENTTOOL_SIZE, long style = SYMBOL_MEASUREMENTTOOL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_MEASUREMENTTOOL_IDNAME, const wxPoint& pos = SYMBOL_MEASUREMENTTOOL_POSITION, const wxSize& size = SYMBOL_MEASUREMENTTOOL_SIZE, long style = SYMBOL_MEASUREMENTTOOL_STYLE );

    /// Destructor
    ~MeasurementTool();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void OnClearSeedsButtonClick( wxCommandEvent& event );
    void OnCloseButtonClick( wxCommandEvent& event );
    void OnUpdateMeasurements( wxCommandEvent& event );

////@begin MeasurementTool event handler declarations

////@end MeasurementTool event handler declarations

////@begin MeasurementTool member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end MeasurementTool member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin MeasurementTool member variables
    wxStaticText* mInfoText;
////@end MeasurementTool member variables
};

#endif
    // _MEASUREMENTTOOL_H_

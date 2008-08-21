/////////////////////////////////////////////////////////////////////////////
// Name:        polylinetoolpanel.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     Wed 09 Apr 2008 16:29:38 MDT
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _POLYLINETOOLPANEL_H_
#define _POLYLINETOOLPANEL_H_


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
#define ID_POLYLINETOOL 10012
#define SYMBOL_POLYLINETOOLPANEL_STYLE 0
#define SYMBOL_POLYLINETOOLPANEL_TITLE _("PolylineTool")
#define SYMBOL_POLYLINETOOLPANEL_IDNAME ID_POLYLINETOOL
#define SYMBOL_POLYLINETOOLPANEL_SIZE wxSize(200, -1)
#define SYMBOL_POLYLINETOOLPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * PolylineToolPanel class declaration
 */

class PolylineToolPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( PolylineToolPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    PolylineToolPanel();
    PolylineToolPanel( wxWindow* parent, wxWindowID id = SYMBOL_POLYLINETOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_POLYLINETOOLPANEL_POSITION, const wxSize& size = SYMBOL_POLYLINETOOLPANEL_SIZE, long style = SYMBOL_POLYLINETOOLPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_POLYLINETOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_POLYLINETOOLPANEL_POSITION, const wxSize& size = SYMBOL_POLYLINETOOLPANEL_SIZE, long style = SYMBOL_POLYLINETOOLPANEL_STYLE );

    /// Destructor
    ~PolylineToolPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    void OnClearSeedsButtonClick( wxCommandEvent& event );
    void OnStartButtonClick( wxCommandEvent& event );
    void OnEraseButtonClick( wxCommandEvent& event );
    void OnCloseButtonClick( wxCommandEvent& event );

////@begin PolylineToolPanel event handler declarations

////@end PolylineToolPanel event handler declarations

////@begin PolylineToolPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end PolylineToolPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin PolylineToolPanel member variables
////@end PolylineToolPanel member variables
};

#endif
    // _POLYLINETOOLPANEL_H_

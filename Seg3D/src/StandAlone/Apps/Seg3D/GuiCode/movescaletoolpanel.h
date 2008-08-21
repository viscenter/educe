/////////////////////////////////////////////////////////////////////////////
// Name:        movescaletoolpanel.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     Mon 21 Apr 2008 14:27:39 MDT
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _MOVESCALETOOLPANEL_H_
#define _MOVESCALETOOLPANEL_H_


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
#define ID_MOVESCALETOOLPANEL 10012
#define SYMBOL_MOVESCALETOOLPANEL_STYLE 0
#define SYMBOL_MOVESCALETOOLPANEL_TITLE _("MoveScaleToolPanel")
#define SYMBOL_MOVESCALETOOLPANEL_IDNAME ID_MOVESCALETOOLPANEL
#define SYMBOL_MOVESCALETOOLPANEL_SIZE wxSize(200, -1)
#define SYMBOL_MOVESCALETOOLPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * MoveScaleToolPanel class declaration
 */

class MoveScaleToolPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( MoveScaleToolPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    MoveScaleToolPanel();
    MoveScaleToolPanel( wxWindow* parent, wxWindowID id = SYMBOL_MOVESCALETOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_MOVESCALETOOLPANEL_POSITION, const wxSize& size = SYMBOL_MOVESCALETOOLPANEL_SIZE, long style = SYMBOL_MOVESCALETOOLPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_MOVESCALETOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_MOVESCALETOOLPANEL_POSITION, const wxSize& size = SYMBOL_MOVESCALETOOLPANEL_SIZE, long style = SYMBOL_MOVESCALETOOLPANEL_STYLE );

    /// Destructor
    ~MoveScaleToolPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void OnGetButtonClick( wxCommandEvent& event );
    void OnSetButtonClick( wxCommandEvent& event );
    void OnSetAllButtonClick( wxCommandEvent& event );
    void OnCloseButtonClick( wxCommandEvent& event );
    void OnMoveScaleToolChange( wxCommandEvent& event );

////@begin MoveScaleToolPanel event handler declarations

////@end MoveScaleToolPanel event handler declarations

////@begin MoveScaleToolPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end MoveScaleToolPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin MoveScaleToolPanel member variables
    wxTextCtrl* mXOrigin;
    wxTextCtrl* mYOrigin;
    wxTextCtrl* mZOrigin;
    wxTextCtrl* mXSpacing;
    wxTextCtrl* mYSpacing;
    wxTextCtrl* mZSpacing;
////@end MoveScaleToolPanel member variables
};

#endif
    // _MOVESCALETOOLPANEL_H_

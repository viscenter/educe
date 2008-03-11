/////////////////////////////////////////////////////////////////////////////
// Name:        brushpanel.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     10/08/2007 11:53:59
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 10/08/2007 11:53:59

#ifndef _BRUSHPANEL_H_
#define _BRUSHPANEL_H_

/*!
 * Includes
 */

// BJW - had to add this manually
#include <wx/wx.h>

////@begin includes
#include "wx/xrc/xmlres.h"
#include "wx/spinctrl.h"
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
#define ID_BRUSHPANEL 10000
#define SYMBOL_BRUSHPANEL_STYLE 0
#define SYMBOL_BRUSHPANEL_TITLE _("BrushPanel")
#define SYMBOL_BRUSHPANEL_IDNAME ID_BRUSHPANEL
#define SYMBOL_BRUSHPANEL_SIZE wxSize(200, -1)
#define SYMBOL_BRUSHPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * BrushPanel class declaration
 */

class BrushPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( BrushPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    BrushPanel();
    BrushPanel( wxWindow* parent, wxWindowID id = SYMBOL_BRUSHPANEL_IDNAME, const wxPoint& pos = SYMBOL_BRUSHPANEL_POSITION, const wxSize& size = SYMBOL_BRUSHPANEL_SIZE, long style = SYMBOL_BRUSHPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_BRUSHPANEL_IDNAME, const wxPoint& pos = SYMBOL_BRUSHPANEL_POSITION, const wxSize& size = SYMBOL_BRUSHPANEL_SIZE, long style = SYMBOL_BRUSHPANEL_STYLE );

    /// Destructor
    ~BrushPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin BrushPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for CLOSE_BUTTON
    void OnCloseButtonClick( wxCommandEvent& event );

////@end BrushPanel event handler declarations

    void OnFloodFillButtonClick( wxCommandEvent& event );
    void OnFloodEraseButtonClick( wxCommandEvent& event );

////@begin BrushPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end BrushPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void SpinRadius( wxSpinEvent &event);
    void UpdateRadius( wxCommandEvent &event);
    void OnEraseBrushChange( wxCommandEvent &event);
////@begin BrushPanel member variables
    wxSpinCtrl* mRadius;
    wxCheckBox* mBrushErase;
////@end BrushPanel member variables
};

#endif
    // _BRUSHPANEL_H_
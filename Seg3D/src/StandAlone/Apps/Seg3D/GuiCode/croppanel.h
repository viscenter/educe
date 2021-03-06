/////////////////////////////////////////////////////////////////////////////
// Name:        croppanel.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     10/08/2007 11:54:54
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 10/08/2007 11:54:54

#ifndef _CROPPANEL_H_
#define _CROPPANEL_H_

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
#define ID_CROPPANEL 10004
#define SYMBOL_CROPPANEL_STYLE 0
#define SYMBOL_CROPPANEL_TITLE _("CropPanel")
#define SYMBOL_CROPPANEL_IDNAME ID_CROPPANEL
#define SYMBOL_CROPPANEL_SIZE wxDefaultSize
#define SYMBOL_CROPPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CropPanel class declaration
 */

class CropPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CropPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CropPanel();
    CropPanel( wxWindow* parent, wxWindowID id = SYMBOL_CROPPANEL_IDNAME, const wxPoint& pos = SYMBOL_CROPPANEL_POSITION, const wxSize& size = SYMBOL_CROPPANEL_SIZE, long style = SYMBOL_CROPPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CROPPANEL_IDNAME, const wxPoint& pos = SYMBOL_CROPPANEL_POSITION, const wxSize& size = SYMBOL_CROPPANEL_SIZE, long style = SYMBOL_CROPPANEL_STYLE );

    /// Destructor
    ~CropPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CropPanel event handler declarations

////@end CropPanel event handler declarations

////@begin CropPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CropPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CropPanel member variables
////@end CropPanel member variables
};

#endif
    // _CROPPANEL_H_

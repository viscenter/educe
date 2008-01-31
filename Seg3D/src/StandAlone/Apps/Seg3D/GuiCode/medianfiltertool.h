/////////////////////////////////////////////////////////////////////////////
// Name:        medianfiltertool.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     12/12/2007 10:09:58
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _MEDIANFILTERTOOL_H_
#define _MEDIANFILTERTOOL_H_


/*!
 * Includes
 */

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
#define ID_MEDIANFILTERTOOL 10062
#define SYMBOL_MEDIANFILTERTOOL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_MEDIANFILTERTOOL_TITLE _("MedianFilterTool")
#define SYMBOL_MEDIANFILTERTOOL_IDNAME ID_MEDIANFILTERTOOL
#define SYMBOL_MEDIANFILTERTOOL_SIZE wxSize(200, -1)
#define SYMBOL_MEDIANFILTERTOOL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * MedianFilterTool class declaration
 */

class MedianFilterTool: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( MedianFilterTool )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    MedianFilterTool();
    MedianFilterTool( wxWindow* parent, wxWindowID id = SYMBOL_MEDIANFILTERTOOL_IDNAME, const wxPoint& pos = SYMBOL_MEDIANFILTERTOOL_POSITION, const wxSize& size = SYMBOL_MEDIANFILTERTOOL_SIZE, long style = SYMBOL_MEDIANFILTERTOOL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_MEDIANFILTERTOOL_IDNAME, const wxPoint& pos = SYMBOL_MEDIANFILTERTOOL_POSITION, const wxSize& size = SYMBOL_MEDIANFILTERTOOL_SIZE, long style = SYMBOL_MEDIANFILTERTOOL_STYLE );

    /// Destructor
    ~MedianFilterTool();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin MedianFilterTool event handler declarations

////@end MedianFilterTool event handler declarations

////@begin MedianFilterTool member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end MedianFilterTool member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void OnStartButtonClick( wxCommandEvent& event );
    void OnCloseButtonClick( wxCommandEvent& event );
    void OnSetProgress( wxCommandEvent& event );

////@begin MedianFilterTool member variables
    wxSpinCtrl* mRadius;
    wxGauge* mPercentage;
////@end MedianFilterTool member variables
};

#endif
    // _MEDIANFILTERTOOL_H_

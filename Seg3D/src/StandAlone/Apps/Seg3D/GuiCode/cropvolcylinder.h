/////////////////////////////////////////////////////////////////////////////
// Name:        cropvolcylinder.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     20/08/2007 08:51:42
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 20/08/2007 08:51:42

#ifndef _CROPVOLCYLINDER_H_
#define _CROPVOLCYLINDER_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/xrc/xmlres.h"
#include "wx/spinctrl.h"
////@end includes
#include "wx/spinctrl.h"

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
#define ID_CROPVOLCYLINDER 10031
#define SYMBOL_CROPVOLCYLINDER_STYLE 0
#define SYMBOL_CROPVOLCYLINDER_TITLE _("CropVolCylinder")
#define SYMBOL_CROPVOLCYLINDER_IDNAME ID_CROPVOLCYLINDER
#define SYMBOL_CROPVOLCYLINDER_SIZE wxSize(200, -1)
#define SYMBOL_CROPVOLCYLINDER_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CropVolCylinder class declaration
 */

struct CropVolCylinderSetRangeStruct {
  int min_x, min_y, min_z;
  int max_x, max_y, max_z;
};


class CropVolCylinder: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CropVolCylinder )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CropVolCylinder();
    CropVolCylinder( wxWindow* parent, wxWindowID id = SYMBOL_CROPVOLCYLINDER_IDNAME, const wxPoint& pos = SYMBOL_CROPVOLCYLINDER_POSITION, const wxSize& size = SYMBOL_CROPVOLCYLINDER_SIZE, long style = SYMBOL_CROPVOLCYLINDER_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CROPVOLCYLINDER_IDNAME, const wxPoint& pos = SYMBOL_CROPVOLCYLINDER_POSITION, const wxSize& size = SYMBOL_CROPVOLCYLINDER_SIZE, long style = SYMBOL_CROPVOLCYLINDER_STYLE );

    /// Destructor
    ~CropVolCylinder();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
	void OnCropSetRange( wxCommandEvent& event );
	void OnCropSetMinMax( wxCommandEvent& event );
	void OnStartButtonClick( wxCommandEvent& event );	
	void OnCloseButtonClick( wxCommandEvent& event );
    void OnSpinMinX (wxSpinEvent &event);
    void OnSpinMaxX (wxSpinEvent &event);
    void OnSpinMinY (wxSpinEvent &event);
    void OnSpinMaxY (wxSpinEvent &event);
    void OnSpinMinZ (wxSpinEvent &event);
    void OnSpinMaxZ (wxSpinEvent &event);

////@begin CropVolCylinder event handler declarations

////@end CropVolCylinder event handler declarations

////@begin CropVolCylinder member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CropVolCylinder member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CropVolCylinder member variables
    wxSpinCtrl* mSpinMinX;
    wxSpinCtrl* mSpinMaxX;
    wxSpinCtrl* mSpinMinY;
    wxSpinCtrl* mSpinMaxY;
    wxSpinCtrl* mSpinMinZ;
    wxSpinCtrl* mSpinMaxZ;
////@end CropVolCylinder member variables
};

#endif
    // _CROPVOLCYLINDER_H_

/////////////////////////////////////////////////////////////////////////////
// Name:        cursorinfo.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     21/08/2007 15:01:49
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 21/08/2007 15:01:49

#ifndef _CURSORINFO_H_
#define _CURSORINFO_H_

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
#define ID_CURSORINFORMATION 10026
#define SYMBOL_CURSORINFORMATION_STYLE 0
#define SYMBOL_CURSORINFORMATION_TITLE _("CursorInformation")
#define SYMBOL_CURSORINFORMATION_IDNAME ID_CURSORINFORMATION
#define SYMBOL_CURSORINFORMATION_SIZE wxDefaultSize
#define SYMBOL_CURSORINFORMATION_POSITION wxDefaultPosition
////@end control identifiers

struct CursorInformationStruct {
	double x,y,z,value;
};


/*!
 * CursorInformation class declaration
 */

class CursorInformation: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CursorInformation )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CursorInformation();
    CursorInformation( wxWindow* parent, wxWindowID id = SYMBOL_CURSORINFORMATION_IDNAME, const wxPoint& pos = SYMBOL_CURSORINFORMATION_POSITION, const wxSize& size = SYMBOL_CURSORINFORMATION_SIZE, long style = SYMBOL_CURSORINFORMATION_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CURSORINFORMATION_IDNAME, const wxPoint& pos = SYMBOL_CURSORINFORMATION_POSITION, const wxSize& size = SYMBOL_CURSORINFORMATION_SIZE, long style = SYMBOL_CURSORINFORMATION_STYLE );

    /// Destructor
    ~CursorInformation();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

	void SetMousePositionAndValue(wxCommandEvent &event);
	void SetMousePosition(double x, double y, double z);
	void SetInfoValue(double value);
	
	
////@begin CursorInformation event handler declarations

////@end CursorInformation event handler declarations

////@begin CursorInformation member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CursorInformation member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CursorInformation member variables
////@end CursorInformation member variables
	
	
	wxTextCtrl* mValue_;
    wxTextCtrl* mPosX_;
    wxTextCtrl* mPosY_;
    wxTextCtrl* mPosZ_;
	
};

#endif
    // _CURSORINFO_H_

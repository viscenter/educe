/////////////////////////////////////////////////////////////////////////////
// Name:        itkspeedtopathiterateneighborhoodfilter.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     12/03/2008 15:18:27
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_H_
#define _ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_H_


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
#define ID_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER 10014
#define SYMBOL_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_STYLE 0
#define SYMBOL_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_TITLE _("ITKSpeedToPathIterateNeighborhoodFilter")
#define SYMBOL_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_IDNAME ID_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER
#define SYMBOL_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_SIZE wxSize(200, -1)
#define SYMBOL_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * ITKSpeedToPathIterateNeighborhoodFilter class declaration
 */

class ITKSpeedToPathIterateNeighborhoodFilter: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( ITKSpeedToPathIterateNeighborhoodFilter )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ITKSpeedToPathIterateNeighborhoodFilter();
    ITKSpeedToPathIterateNeighborhoodFilter( wxWindow* parent, wxWindowID id = SYMBOL_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_IDNAME, const wxPoint& pos = SYMBOL_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_POSITION, const wxSize& size = SYMBOL_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_SIZE, long style = SYMBOL_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_IDNAME, const wxPoint& pos = SYMBOL_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_POSITION, const wxSize& size = SYMBOL_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_SIZE, long style = SYMBOL_ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_STYLE );

    /// Destructor
    ~ITKSpeedToPathIterateNeighborhoodFilter();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
	
	void OnStartButtonClick( wxCommandEvent& event );
	void OnCloseButtonClick( wxCommandEvent& event );
	void OnClearSeedsButtonClick( wxCommandEvent& event );
	void OnCreateSpeedButtonClick( wxCommandEvent& event );
	void OnAddSeedsCheck( wxCommandEvent& event );
	void OnSetProgress( wxCommandEvent& event );

////@begin ITKSpeedToPathIterateNeighborhoodFilter event handler declarations

////@end ITKSpeedToPathIterateNeighborhoodFilter event handler declarations

////@begin ITKSpeedToPathIterateNeighborhoodFilter member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end ITKSpeedToPathIterateNeighborhoodFilter member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin ITKSpeedToPathIterateNeighborhoodFilter member variables
    wxTextCtrl* mTermination;
    wxTextCtrl* mStepLengthFactor;
    wxCheckBox* mSeedPoints;
    wxGauge* mPercentage;
////@end ITKSpeedToPathIterateNeighborhoodFilter member variables
		
	wxWindowDisabler *disabler_;

};

#endif
    // _ITKSPEEDTOPATHITERATENEIGHBORHOODFILTER_H_

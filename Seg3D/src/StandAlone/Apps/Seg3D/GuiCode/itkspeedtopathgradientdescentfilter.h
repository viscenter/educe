/////////////////////////////////////////////////////////////////////////////
// Name:        itkspeedtopathgradientdescentfilter.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     12/03/2008 13:58:25
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _ITKSPEEDTOPATHGRADIENTDESCENTFILTER_H_
#define _ITKSPEEDTOPATHGRADIENTDESCENTFILTER_H_


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
#define ID_ITKSPEEDTOPATHGRADIENTDESCENTFILTER 10013
#define SYMBOL_ITKSPEEDTOPATHGRADIENTDESCENTFILTER_STYLE 0
#define SYMBOL_ITKSPEEDTOPATHGRADIENTDESCENTFILTER_TITLE _("ITKSpeedToPathGradientDescentFilter")
#define SYMBOL_ITKSPEEDTOPATHGRADIENTDESCENTFILTER_IDNAME ID_ITKSPEEDTOPATHGRADIENTDESCENTFILTER
#define SYMBOL_ITKSPEEDTOPATHGRADIENTDESCENTFILTER_SIZE wxSize(200, -1)
#define SYMBOL_ITKSPEEDTOPATHGRADIENTDESCENTFILTER_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * ITKSpeedToPathGradientDescentFilter class declaration
 */

class ITKSpeedToPathGradientDescentFilter: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( ITKSpeedToPathGradientDescentFilter )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ITKSpeedToPathGradientDescentFilter();
    ITKSpeedToPathGradientDescentFilter( wxWindow* parent, wxWindowID id = SYMBOL_ITKSPEEDTOPATHGRADIENTDESCENTFILTER_IDNAME, const wxPoint& pos = SYMBOL_ITKSPEEDTOPATHGRADIENTDESCENTFILTER_POSITION, const wxSize& size = SYMBOL_ITKSPEEDTOPATHGRADIENTDESCENTFILTER_SIZE, long style = SYMBOL_ITKSPEEDTOPATHGRADIENTDESCENTFILTER_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ITKSPEEDTOPATHGRADIENTDESCENTFILTER_IDNAME, const wxPoint& pos = SYMBOL_ITKSPEEDTOPATHGRADIENTDESCENTFILTER_POSITION, const wxSize& size = SYMBOL_ITKSPEEDTOPATHGRADIENTDESCENTFILTER_SIZE, long style = SYMBOL_ITKSPEEDTOPATHGRADIENTDESCENTFILTER_STYLE );

    /// Destructor
    ~ITKSpeedToPathGradientDescentFilter();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
	void OnStartButtonClick( wxCommandEvent& event );
	void OnCloseButtonClick( wxCommandEvent& event );
	void OnClearSeedsButtonClick( wxCommandEvent& event );
	void OnAddSeedsCheck( wxCommandEvent& event );
	void OnCreateSpeedButtonClick( wxCommandEvent& event );
	void OnSetProgress( wxCommandEvent& event );

////@begin ITKSpeedToPathGradientDescentFilter event handler declarations

////@end ITKSpeedToPathGradientDescentFilter event handler declarations

////@begin ITKSpeedToPathGradientDescentFilter member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end ITKSpeedToPathGradientDescentFilter member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin ITKSpeedToPathGradientDescentFilter member variables
    wxSpinCtrl* mIterations;
    wxTextCtrl* mTermination;
    wxCheckBox* mSeedPoints;
    wxGauge* mPercentage;
////@end ITKSpeedToPathGradientDescentFilter member variables
    
	wxWindowDisabler *disabler_;
};

#endif
    // _ITKSPEEDTOPATHGRADIENTDESCENTFILTER_H_

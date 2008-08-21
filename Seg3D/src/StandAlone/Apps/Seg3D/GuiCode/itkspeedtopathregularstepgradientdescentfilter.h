/////////////////////////////////////////////////////////////////////////////
// Name:        itkspeedtopathregularstepgradientdescentfilter.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     12/03/2008 15:08:43
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_H_
#define _ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_H_


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
#define ID_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER 10014
#define SYMBOL_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_STYLE 0
#define SYMBOL_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_TITLE _("ITKSpeedToPathRegularStepGradientDescentFilter")
#define SYMBOL_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_IDNAME ID_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER
#define SYMBOL_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_SIZE wxSize(200, -1)
#define SYMBOL_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * ITKSpeedToPathRegularStepGradientDescentFilter class declaration
 */

class ITKSpeedToPathRegularStepGradientDescentFilter: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( ITKSpeedToPathRegularStepGradientDescentFilter )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ITKSpeedToPathRegularStepGradientDescentFilter();
    ITKSpeedToPathRegularStepGradientDescentFilter( wxWindow* parent, wxWindowID id = SYMBOL_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_IDNAME, const wxPoint& pos = SYMBOL_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_POSITION, const wxSize& size = SYMBOL_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_SIZE, long style = SYMBOL_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_IDNAME, const wxPoint& pos = SYMBOL_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_POSITION, const wxSize& size = SYMBOL_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_SIZE, long style = SYMBOL_ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_STYLE );

    /// Destructor
    ~ITKSpeedToPathRegularStepGradientDescentFilter();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

	void OnStartButtonClick( wxCommandEvent& event );
	void OnClearSeedsButtonClick( wxCommandEvent& event );
	void OnCreateSpeedButtonClick( wxCommandEvent& event );
	void OnCloseButtonClick( wxCommandEvent& event );
	void OnAddSeedsCheck( wxCommandEvent& event );
	void OnSetProgress( wxCommandEvent &event );


////@begin ITKSpeedToPathRegularStepGradientDescentFilter event handler declarations

////@end ITKSpeedToPathRegularStepGradientDescentFilter event handler declarations

////@begin ITKSpeedToPathRegularStepGradientDescentFilter member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end ITKSpeedToPathRegularStepGradientDescentFilter member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin ITKSpeedToPathRegularStepGradientDescentFilter member variables
    wxSpinCtrl* mIterations;
    wxTextCtrl* mTermination;
    wxTextCtrl* mStepLengthFactor;
    wxTextCtrl* mStepLengthRelax;
    wxCheckBox* mSeedPoints;
    wxGauge* mPercentage;
////@end ITKSpeedToPathRegularStepGradientDescentFilter member variables
		
	wxWindowDisabler *disabler_;

};

#endif
    // _ITKSPEEDTOPATHREGULARSTEPGRADIENTDESCENTFILTER_H_

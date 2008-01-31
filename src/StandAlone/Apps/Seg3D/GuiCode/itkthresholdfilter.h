#ifndef _ITKTHRESHOLDFILTER_H_
#define _ITKTHRESHOLDFILTER_H_


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
#define ID_ITKTHRESHOLDFILTER 10039
#define SYMBOL_ITKTHRESHOLDFILTER_STYLE 0
#define SYMBOL_ITKTHRESHOLDFILTER_TITLE _("ITKThresholdFilter")
#define SYMBOL_ITKTHRESHOLDFILTER_IDNAME ID_ITKTHRESHOLDFILTER
#define SYMBOL_ITKTHRESHOLDFILTER_SIZE wxSize(200, -1)
#define SYMBOL_ITKTHRESHOLDFILTER_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * ITKThresholdFilter class declaration
 */

class ITKThresholdFilter: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( ITKThresholdFilter )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ITKThresholdFilter();
    ITKThresholdFilter( wxWindow* parent, wxWindowID id = SYMBOL_ITKTHRESHOLDFILTER_IDNAME, const wxPoint& pos = SYMBOL_ITKTHRESHOLDFILTER_POSITION, const wxSize& size = SYMBOL_ITKTHRESHOLDFILTER_SIZE, long style = SYMBOL_ITKTHRESHOLDFILTER_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ITKTHRESHOLDFILTER_IDNAME, const wxPoint& pos = SYMBOL_ITKTHRESHOLDFILTER_POSITION, const wxSize& size = SYMBOL_ITKTHRESHOLDFILTER_SIZE, long style = SYMBOL_ITKTHRESHOLDFILTER_STYLE );

    /// Destructor
    ~ITKThresholdFilter();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin ITKThresholdFilter event handler declarations

////@end ITKThresholdFilter event handler declarations

////@begin ITKThresholdFilter member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end ITKThresholdFilter member function declarations

	void OnStartButtonClick( wxCommandEvent& event );
	void OnCloseButtonClick( wxCommandEvent& event );
	void OnSetProgress( wxCommandEvent &event);


    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin ITKThresholdFilter member variables
    wxTextCtrl* mMinValue;
    wxTextCtrl* mMaxValue;
    wxGauge* mPercentage;
////@end ITKThresholdFilter member variables

    wxWindowDisabler *disabler_;
};

#endif


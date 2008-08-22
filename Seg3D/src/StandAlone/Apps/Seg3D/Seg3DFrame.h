//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2006 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
//    File   : Seg3DFrame.h
//    Author : David Brayford
//    Date   : May 2008

#ifndef SEG3D_SEG3DFRAME_H
#define SEG3D_SEG3DFRAME_H

#include <StandAlone/Apps/Seg3D/WXOpenGLContext.h>
#include <StandAlone/Apps/Seg3D/Painter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/seg3devents.h>
#include <StandAlone/Apps/Seg3D/plugins.h>

#include <sci_gl.h>

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/help.h>

#include <opencv/cv.h>
#include <opencv/cxcore.h>

#if !wxUSE_GLCANVAS
#error Need WX GL Canvas!
#endif

#define SEG3D_VERSION_STRING "1.10.0"

#include <StandAlone/Apps/Seg3D/share.h>

class BrushPanel;

class CursorInformation;

class ITKCurvatureAnistopricDiffusionFilter;
class ITKConfidenceConnectedFilter;
class ITKNeighbourhoodConnectedFilter;
class ITKBinaryDilateErodeFilter;
class ITKThresholdSegmentationLevelSetFilter;
class CropVolume;
class CropVolCylinder;
class FlipTool;
class ResampleTool;
class MedianFilterTool;
class OptionlessFilter;
class MaskFilter;
class itkDiscreteGaussianImageFilter;
class HistoEqFilter;
class PolylineToolPanel;
class ThresholdToolPanel;
class MoveScaleToolPanel;
class MeasurementTool;
class IntensityCorrectionFilterPanel;

class ITKSpeedToPathGradientDescentFilter;
class ITKSpeedToPathRegularStepGradientDescentFilter;
class ITKSpeedToPathIterateNeighborhoodFilter;
class SpeedFunction;

#define PANEL_WIDTH 200
#define INFO_HEIGHT 250

enum
{
    MENU_FILE_LOAD_VOLUME = 100,
    MENU_FILE_LOAD_SESSION,
    MENU_FILE_SAVE_VOLUME,
    MENU_FILE_SAVE_SESSION,
    MENU_FILE_IMPORT_SEGMENTATION,
    MENU_FILE_EXPORT_SEGMENTATION,
    MENU_FILE_QUIT = wxID_EXIT,

    MENU_VIEW_TWO_BY_TWO = 200,
    MENU_VIEW_ONE_BY_THREE,
    MENU_VIEW_SINGLE,
    MENU_VIEW_VOLUME_RENDER,

    MENU_EDIT_AUTOVIEW = 300,
    MENU_EDIT_SET_MASK_LAYER,
    MENU_EDIT_CLEAR_MASK_LAYER,
    MENU_EDIT_ISOSURFACE_ONE,
    MENU_EDIT_ISOSURFACE_ALL,
    MENU_EDIT_SET_VRTARGET,
    MENU_EDIT_RESET_CLUT,
    MENU_EDIT_MOVE_LAYER_UP,
    MENU_EDIT_MOVE_LAYER_DOWN,
    MENU_EDIT_UNDO,
    MENU_EDIT_PREFS,
    
    MENU_PREFS_FATLINES = 350,
    MENU_PREFS_STIPPLE,

    MENU_TOOL_PAINT_BRUSH = 400,
    MENU_TOOL_CROP_VOLUME,
    MENU_TOOL_CROP_CYLINDER,
    MENU_TOOL_FLIP,
    MENU_TOOL_RESAMPLE,
    MENU_TOOL_POLYLINE,
    MENU_TOOL_THRESHOLD,
    MENU_TOOL_MOVESCALE,
    MENU_TOOL_MEASUREMENT,

    MENU_FILTER_C_A_D_F = 500,
    MENU_FILTER_C_C_F,
    MENU_FILTER_N_C_F,
    MENU_FILTER_B_D_E_F,
    MENU_FILTER_T_S_L_S_F,
    MENU_FILTER_OTSU_T_F,
    MENU_FILTER_G_M_F,
    MENU_FILTER_LABEL_INVERT_FILTER,
    MENU_FILTER_LABEL_EXTRACT,
    MENU_FILTER_FILL_HOLE,
    MENU_FILTER_FLOOD_FILL_COPY,
    MENU_FILTER_MEDIAN_FILTER,
    MENU_FILTER_THRESHOLD_FILTER,
    MENU_FILTER_DISCRETE_GAUSSIAN_FILTER,
    MENU_FILTER_HISTO_EQ,
    MENU_FILTER_MASK_DATA,
    MENU_FILTER_MASK_LABEL,
    MENU_FILTER_MASK_AND,
    MENU_FILTER_MASK_REMOVE,
    MENU_FILTER_MASK_OR,
    MENU_FILTER_MASK_XOR,
    MENU_FILTER_INHOMO_CORRECTION,

    MENU_FILTER_S_T_P_G_D_F,
    MENU_FILTER_S_T_P_R_S_G_D_F,
    MENU_FILTER_S_T_P_I_N_F,

    MENU_HELP_UNDO,
    MENU_HELP_INDEX, // TODO: Fix later (wxID_HELP or wxID_HELP_INDEX?),
    MENU_HELP_ABOUT = wxID_ABOUT,
		
		MENU_PLUGIN_START = 700,

    VOLUME_INFO_OK_BUTTON = 600
    // Don't add any here without setting the value correctly.
};

namespace SCIRun {


struct colour_picker_data_t
{
  double r;
  double g;
  double b;
  string layername;
};

struct export_label_selection_data_t
{
  export_label_selection_data_t() : choices_(NULL), nchoices_(0) {}
  wxString *choices_;
  size_t    nchoices_;
  wxArrayInt selections_;
};

struct update_undo_state_t
{
  bool enable_;
  string label_;
};

DECLARE_EVENT_TYPE(wxEVT_COMMAND_STATUS_BAR_TEXT_CHANGE, -100)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_COLOUR_PICKER, -101)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_HIDE_TOOL, -102)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_OK_DIALOG, -103)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_LAYER_DELETE_DIALOG, -104)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_EXPORT_LABEL_SELECTION, -105)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_UPDATE_UNDO_STATE, -106)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_UNWRAP_WINDOW, -107)
 

class Seg3DFrame : public wxFrame
{
public:
  Seg3DFrame(const string& target, wxFrame *frame,
             const wxString& title, const wxPoint& pos,
             const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

  WXOpenGLContext* getContext() { return context_; }
  
  void FileLoadVolume( wxCommandEvent& WXUNUSED(event) );
  void FileLoadSession( wxCommandEvent& WXUNUSED(event) );
  void FileSaveVolume( wxCommandEvent& WXUNUSED(event) );
  void FileSaveSession( wxCommandEvent& WXUNUSED(event) );
  void FileImportSegmentation( wxCommandEvent& WXUNUSED(event) );
  void FileExportSegmentation( wxCommandEvent& WXUNUSED(event) );
  void FileQuit( wxCommandEvent& WXUNUSED(event) );

  void ViewTwoByTwo( wxCommandEvent& WXUNUSED(event) );
  void ViewOneByThree( wxCommandEvent& WXUNUSED(event) );
  void ViewSingle( wxCommandEvent& WXUNUSED(event) );
  void ViewVolumeRender( wxCommandEvent& WXUNUSED(event) );
  void EditAutoview( wxCommandEvent& WXUNUSED(event) );
  void EditSetMaskLayer( wxCommandEvent& WXUNUSED(event) );
  void EditClearMaskLayer( wxCommandEvent& WXUNUSED(event) );
  void EditIsosurfaceOne( wxCommandEvent& WXUNUSED(event) );
  void EditIsosurfaceAll( wxCommandEvent& WXUNUSED(event) );
  void EditSetVRTarget( wxCommandEvent& WXUNUSED(event) );
  void EditResetCLUT( wxCommandEvent& WXUNUSED(event) );
  void EditMoveLayerUp( wxCommandEvent& WXUNUSED(event) );
  void EditMoveLayerDown( wxCommandEvent& WXUNUSED(event) );
  void EditUndo( wxCommandEvent& WXUNUSED(event) );
 
  void ToolPaintBrush( wxCommandEvent& WXUNUSED(event) );


  void ToolCropVolume( wxCommandEvent& WXUNUSED(event) );
  void ToolCropCylinder( wxCommandEvent& WXUNUSED(event) );
  void ToolFlip( wxCommandEvent& WXUNUSED(event) );

  void ToolResample( wxCommandEvent& WXUNUSED(event) );
  void ToolPolyline( wxCommandEvent& WXUNUSED(event) );
  void ToolThreshold( wxCommandEvent& WXUNUSED(event) );
  void ToolMoveScale( wxCommandEvent& WXUNUSED(event) );
  void ToolMeasurement( wxCommandEvent& WXUNUSED(event) );
 

	void PluginEvent( wxCommandEvent& event );

  void Filter_CADF( wxCommandEvent& WXUNUSED(event) );
  void Filter_CCF( wxCommandEvent& WXUNUSED(event) );
  void Filter_NCF( wxCommandEvent& WXUNUSED(event) );
  void Filter_BDEF( wxCommandEvent& WXUNUSED(event) );
  void Filter_TSLSF( wxCommandEvent& WXUNUSED(event) );
  void Filter_OTSUTF( wxCommandEvent& WXUNUSED(event) );
  void Filter_GMF( wxCommandEvent& WXUNUSED(event) );
  void Filter_LabelInvertFilter( wxCommandEvent& WXUNUSED(event) );
  void Filter_LabelExtract( wxCommandEvent& WXUNUSED(event) );
  void Filter_FillHole( wxCommandEvent& WXUNUSED(event) );
  void Filter_FloodFillCopy( wxCommandEvent &WXUNUSED(event) );
  void Filter_MedianFilter( wxCommandEvent& WXUNUSED(event) );
  void Filter_DiscreteGaussianFilter( wxCommandEvent& WXUNUSED(event) );
  void Filter_HistoEq( wxCommandEvent& WXUNUSED(event) );
  void Filter_InhomoCorrection( wxCommandEvent& WXUNUSED(event) );

  void Filter_SpeedToPathGradientDescent( wxCommandEvent& WXUNUSED(event) );
  void Filter_SpeedToPathRegularStepGradientDescent( wxCommandEvent& WXUNUSED(event) );
  void Filter_SpeedToPathIterateNeighborhood( wxCommandEvent& WXUNUSED(event) );

  void Filter_MaskData( wxCommandEvent& WXUNUSED(event) );
  void Filter_MaskLabel( wxCommandEvent& WXUNUSED(event) );
  void Filter_MaskAnd( wxCommandEvent& WXUNUSED(event) );
  void Filter_MaskRemove( wxCommandEvent& WXUNUSED(event) );
  void Filter_MaskOr( wxCommandEvent& WXUNUSED(event) );
  void Filter_MaskXor( wxCommandEvent& WXUNUSED(event) );


  void About( wxCommandEvent& WXUNUSED(event) );
  void Index( wxCommandEvent& WXUNUSED(event) );
  void Undo( wxCommandEvent& WXUNUSED(event) );

  void OnUpdatePrefsFatlines( wxCommandEvent& WXUNUSED(event) );
  void OnUpdatePrefsStipple( wxCommandEvent& WXUNUSED(event) );

  void OnVolumeInfoPanel( wxCommandEvent& WXUNUSED(event) );

  bool Init();

  ::CursorInformation* cursorInformation_;

  BrushPanel* brushPanel_;
  CropVolume* cropVolume_;
  CropVolCylinder* cropCylinder_;
  FlipTool* flipTools_;
  HistoEqFilter *histoEqTool_;
  ResampleTool* resampleTool_;
  MedianFilterTool* medianFilterTool_;
  // New wx itk test
  ITKCurvatureAnistopricDiffusionFilter* itk_CADF_;
  ITKConfidenceConnectedFilter* itk_CCF_;
  ITKNeighbourhoodConnectedFilter* itk_NCF_;
  ITKBinaryDilateErodeFilter* itk_BDEF_;
  ITKThresholdSegmentationLevelSetFilter* itk_TSLSF_;
  OptionlessFilter* optionless_;
  itkDiscreteGaussianImageFilter* itk_discretegaussianfilter_;
  MaskFilter* maskfilter_;
  PolylineToolPanel *polylinetoolpanel_;
  ThresholdToolPanel *thresholdtoolpanel_;
  MoveScaleToolPanel *movescaletoolpanel_;
  MeasurementTool *measurementtoolpanel_;
  IntensityCorrectionFilterPanel *intensitycorrectionfilterpanel_;

  ITKSpeedToPathGradientDescentFilter* itk_STPGDF_;
  ITKSpeedToPathRegularStepGradientDescentFilter* itk_STPRSGDF_;
  ITKSpeedToPathIterateNeighborhoodFilter* itk_STPINF_;

  void ShowTool(wxPanel* tool, const char* event_name,
                const char *title0, const char *title1 = "");  

  void HideTool();
  wxPanel *CurrentToolPanel();

	PluginManager* plugin_manager_;
	IplImage * unwrapped_image_;

private:
  void OnCloseWindow(wxCloseEvent &event);
  void OnStatusBarTextChange(wxCommandEvent &event);
  void OnColourPicker(wxCommandEvent &event);
  void OnHideTool(wxCommandEvent &event);
  void OnOKDialog(wxCommandEvent &event);
  void OnLayerDeleteDialog(wxCommandEvent &event);
  void OnExportLabelSelection(wxCommandEvent &event);
  void OnUpdateUndoState(wxCommandEvent &event);
 
  void OnUnwrapWindow(wxCommandEvent &event);

  static void PainterShowVisibleItem(const string &id, const string &group);

  wxString CurrentDocPath;

  wxPanel* toolsPanel_;
  wxPanel* infoPanel_;
  wxPanel* current_tool_;

  wxMenu* editMenu_;
  wxMenu* prefsMenu_;

  wxStaticText* tool_label0_;
  wxStaticText* tool_label1_;
  
  WXOpenGLContext* context_;
  wxHelpController* help_;

  DECLARE_EVENT_TABLE();
};

}
#endif

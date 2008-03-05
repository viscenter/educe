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

#define SEG3D_VERSION_STRING "1.9.1"

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
class ITKThresholdFilter;
class MaskFilter;
class itkDiscreteGaussianImageFilter;
class HistoEqFilter;

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

    MENU_TOOL_AUTOVIEW = 300,
    MENU_TOOL_PAINT_BRUSH,
    MENU_TOOL_CROP_VOLUME,
    MENU_TOOL_CROP_CYLINDER,
		MENU_TOOL_ISOSURFACE_ONE,
    MENU_TOOL_ISOSURFACE_ALL,
    MENU_TOOL_SET_VRTARGET,
    MENU_TOOL_FLIP,
    MENU_TOOL_RESAMPLE,
    MENU_TOOL_RESET_CLUT,
    MENU_TOOL_SET_MASK_LAYER,
    MENU_TOOL_CLEAR_MASK_LAYER,
    MENU_TOOL_POLYLINE,

    MENU_FILTER_C_A_D_F = 400,
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
    MENU_FILTER_MASK_OR,

    MENU_HELP_UNDO,
    MENU_HELP_INDEX, // TODO: Fix later (wxID_HELP or wxID_HELP_INDEX?),
    MENU_HELP_ABOUT = wxID_ABOUT,
		
		MENU_PLUGIN_START = 500

    // Don't add any here without setting the value correctly.
};

namespace SCIRun {


struct colour_picker_data
{
  double r;
  double g;
  double b;
  string layername;
};


DECLARE_EVENT_TYPE(wxEVT_COMMAND_STATUS_BAR_TEXT_CHANGE, -100)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_COLOUR_PICKER, -101)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_HIDE_TOOL, -102)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_OK_DIALOG, -103)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_LAYER_DELETE_DIALOG, -104)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_UNWRAP_WINDOW, -105)


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

  void ToolAutoview( wxCommandEvent& WXUNUSED(event) );
  void ToolIsosurfaceOne( wxCommandEvent& WXUNUSED(event) );
  void ToolIsosurfaceAll( wxCommandEvent& WXUNUSED(event) );
  void ToolSetVRTarget( wxCommandEvent& WXUNUSED(event) );
  void ToolPaintBrush( wxCommandEvent& WXUNUSED(event) );
  void ToolCrop( wxCommandEvent& WXUNUSED(event) );

  void ToolCropVolume( wxCommandEvent& WXUNUSED(event) );
  void ToolCropCylinder( wxCommandEvent& WXUNUSED(event) );
  void ToolFlip( wxCommandEvent& WXUNUSED(event) );

  void ToolFlipC( wxCommandEvent& WXUNUSED(event) );
  void ToolFlipA( wxCommandEvent& WXUNUSED(event) );
  void ToolFlipS( wxCommandEvent& WXUNUSED(event) );
  void ToolResample( wxCommandEvent& WXUNUSED(event) );
  void ToolResetCLUT( wxCommandEvent& WXUNUSED(event) );
  void ToolSetMaskLayer( wxCommandEvent& WXUNUSED(event) );
  void ToolClearMaskLayer( wxCommandEvent& WXUNUSED(event) );
  void ToolPolyline( wxCommandEvent& WXUNUSED(event) );

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
  void Filter_ThresholdFilter( wxCommandEvent& WXUNUSED(event) );
  void Filter_DiscreteGaussianFilter( wxCommandEvent& WXUNUSED(event) );
  void Filter_HistoEq( wxCommandEvent& WXUNUSED(event) );

  void Filter_MaskData( wxCommandEvent& WXUNUSED(event) );
  void Filter_MaskLabel( wxCommandEvent& WXUNUSED(event) );
  void Filter_MaskAnd( wxCommandEvent& WXUNUSED(event) );
  void Filter_MaskOr( wxCommandEvent& WXUNUSED(event) );


  void About( wxCommandEvent& WXUNUSED(event) );
  void Index( wxCommandEvent& WXUNUSED(event) );
  void Undo( wxCommandEvent& WXUNUSED(event) );


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
  ITKThresholdFilter* itk_thresholdfilter_;
  itkDiscreteGaussianImageFilter* itk_discretegaussianfilter_;
  MaskFilter* maskfilter_;

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
  void OnUnwrapWindow(wxCommandEvent &event);

  static void PainterShowVisibleItem(const string &id, const string &group);

  wxString CurrentDocPath;

  wxPanel* toolsPanel_;
  wxPanel* infoPanel_;
  wxPanel* current_tool_;

  wxStaticText* tool_label0_;
  wxStaticText* tool_label1_;
  
  WXOpenGLContext* context_;
  wxHelpController* help_;

  DECLARE_EVENT_TABLE();
};

}
#endif

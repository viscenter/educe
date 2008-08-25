/*
  For more information, please see: http://software.sci.utah.edu

  The MIT License

  Copyright (c) 2004 Scientific Computing and Imaging Institute,
  University of Utah.

   
  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/


/*
 *  Painter.cc
 *
 *  Written by:
 *   SCI Institute
 *   University of Utah
 *   July 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */


#include <StandAlone/Apps/Seg3D/Seg3DFrame.h>
#include <StandAlone/Apps/Seg3D/WXOpenGLContext.h>
#include <StandAlone/Apps/Seg3D/plugins.h>
#include <StandAlone/Apps/Seg3D/UnwrapPlugin.h>
#include <Core/Malloc/Allocator.h>
#include <Core/Events/EventManager.h>
#include <Core/Events/BaseEvent.h>
#include <signal.h>

#include <StandAlone/Apps/Seg3D/Seg3DwxGuiUtils.h>

#include <StandAlone/Apps/Seg3D/GuiCode/brushpanel.h>

//New wx ITK test
#include <StandAlone/Apps/Seg3D/GuiCode/itkcurvatureanistopricdiffusionfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkconfidenceconnectedfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkbinarydilateerodefilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkneighbourhoodconnectedfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkthresholdsegmentationlevelsetfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/cropvolume.h>
#include <StandAlone/Apps/Seg3D/GuiCode/cropvolcylinder.h>
#include <StandAlone/Apps/Seg3D/GuiCode/brushpanel.h>
#include <StandAlone/Apps/Seg3D/GuiCode/cursorinformation.h>
#include <StandAlone/Apps/Seg3D/GuiCode/fliptool.h>
#include <StandAlone/Apps/Seg3D/GuiCode/histoeqfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/intensitycorrectionfilterpanel.h>
#include <StandAlone/Apps/Seg3D/GuiCode/medianfiltertool.h>
#include <StandAlone/Apps/Seg3D/GuiCode/optionlessfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkDiscreteGaussianImageFilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkbinarydilateerodefilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkconfidenceconnectedfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkcurvatureanistopricdiffusionfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkneighbourhoodconnectedfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkspeedtopathgradientdescentfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkspeedtopathiterateneighborhoodfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkspeedtopathregularstepgradientdescentfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkthresholdsegmentationlevelsetfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/maskfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/measurementtool.h>
#include <StandAlone/Apps/Seg3D/GuiCode/movescaletoolpanel.h>
#include <StandAlone/Apps/Seg3D/GuiCode/polylinetoolpanel.h>
#include <StandAlone/Apps/Seg3D/GuiCode/resampletool.h>
#include <StandAlone/Apps/Seg3D/GuiCode/thresholdtoolpanel.h>

// #define USING_HELP_FILES

#include <wx/filedlg.h>
#include <wx/colordlg.h>
#include <wx/aboutdlg.h>
#include <wx/tokenzr.h>
#include <wx/fs_zip.h>


// These defines changed in the 2.8 release to use FD_
#if (wxMAJOR_VERSION <= 2) && (wxMINOR_VERSION <= 6)
#define wxFD_OPEN wxOPEN
#define wxFD_FILE_MUST_EXIST wxFILE_MUST_EXIST
#define wxFD_MULTIPLE wxMULTIPLE
#define wxFD_SAVE wxSAVE
#define wxFD_OVERWRITE_PROMPT wxOVERWRITE_PROMPT
#define wxFD_CHANGE_DIR wxCHANGE_DIR
#endif

namespace SCIRun {


BEGIN_EVENT_TABLE(Seg3DFrame, wxFrame)
  EVT_CLOSE(Seg3DFrame::OnCloseWindow)
  EVT_COMMAND (wxID_ANY, wxEVT_COMMAND_STATUS_BAR_TEXT_CHANGE, Seg3DFrame::OnStatusBarTextChange)
  EVT_COMMAND (wxID_ANY, wxEVT_COMMAND_COLOUR_PICKER, Seg3DFrame::OnColourPicker)
  EVT_COMMAND (wxID_ANY, wxEVT_COMMAND_HIDE_TOOL, Seg3DFrame::OnHideTool)
  EVT_COMMAND (wxID_ANY, wxEVT_COMMAND_OK_DIALOG, Seg3DFrame::OnOKDialog)
  EVT_COMMAND (wxID_ANY, wxEVT_COMMAND_LAYER_DELETE_DIALOG, Seg3DFrame::OnLayerDeleteDialog)
  EVT_COMMAND (wxID_ANY, wxEVT_COMMAND_UNWRAP_WINDOW, Seg3DFrame::OnUnwrapWindow)

  EVT_MENU(MENU_FILE_LOAD_VOLUME, Seg3DFrame::FileLoadVolume)
  EVT_MENU(MENU_FILE_LOAD_SESSION, Seg3DFrame::FileLoadSession)
  EVT_MENU(MENU_FILE_SAVE_VOLUME, Seg3DFrame::FileSaveVolume)
  EVT_MENU(MENU_FILE_SAVE_SESSION, Seg3DFrame::FileSaveSession)
  EVT_MENU(MENU_FILE_IMPORT_SEGMENTATION, Seg3DFrame::FileImportSegmentation)
  EVT_MENU(MENU_FILE_EXPORT_SEGMENTATION, Seg3DFrame::FileExportSegmentation)
  EVT_MENU(MENU_FILE_QUIT, Seg3DFrame::FileQuit)

  EVT_MENU(MENU_VIEW_TWO_BY_TWO, Seg3DFrame::ViewTwoByTwo)
  EVT_MENU(MENU_VIEW_ONE_BY_THREE, Seg3DFrame::ViewOneByThree)
  EVT_MENU(MENU_VIEW_SINGLE, Seg3DFrame::ViewSingle)
  EVT_MENU(MENU_VIEW_VOLUME_RENDER, Seg3DFrame::ViewVolumeRender)

  EVT_MENU(MENU_EDIT_AUTOVIEW, Seg3DFrame::EditAutoview)
  EVT_MENU(MENU_EDIT_SET_MASK_LAYER, Seg3DFrame::EditSetMaskLayer)
  EVT_MENU(MENU_EDIT_CLEAR_MASK_LAYER, Seg3DFrame::EditClearMaskLayer)
  EVT_MENU(MENU_EDIT_ISOSURFACE_ONE, Seg3DFrame::EditIsosurfaceOne)
  EVT_MENU(MENU_EDIT_ISOSURFACE_ALL, Seg3DFrame::EditIsosurfaceAll)
  EVT_MENU(MENU_EDIT_SET_VRTARGET, Seg3DFrame::EditSetVRTarget)
  EVT_MENU(MENU_EDIT_RESET_CLUT, Seg3DFrame::EditResetCLUT)
  EVT_MENU(MENU_EDIT_MOVE_LAYER_UP, Seg3DFrame::EditMoveLayerUp)
  EVT_MENU(MENU_EDIT_MOVE_LAYER_DOWN, Seg3DFrame::EditMoveLayerDown)
  EVT_MENU(MENU_EDIT_UNDO, Seg3DFrame::EditUndo)

  EVT_MENU(MENU_PREFS_FATLINES, Seg3DFrame::OnUpdatePrefsFatlines)
  EVT_MENU(MENU_PREFS_STIPPLE, Seg3DFrame::OnUpdatePrefsStipple)

  EVT_MENU(MENU_TOOL_PAINT_BRUSH, Seg3DFrame::ToolPaintBrush)
  EVT_MENU(MENU_TOOL_CROP_VOLUME, Seg3DFrame::ToolCropVolume)
  EVT_MENU(MENU_TOOL_CROP_CYLINDER, Seg3DFrame::ToolCropCylinder)
  EVT_MENU(MENU_TOOL_FLIP, Seg3DFrame::ToolFlip)
  EVT_MENU(MENU_TOOL_RESAMPLE, Seg3DFrame::ToolResample)
  EVT_MENU(MENU_TOOL_POLYLINE, Seg3DFrame::ToolPolyline)
  EVT_MENU(MENU_TOOL_THRESHOLD, Seg3DFrame::ToolThreshold)
  EVT_MENU(MENU_TOOL_MOVESCALE, Seg3DFrame::ToolMoveScale)
	EVT_MENU(MENU_TOOL_MEASUREMENT, Seg3DFrame::ToolMeasurement)

  EVT_MENU(MENU_FILTER_C_A_D_F, Seg3DFrame::Filter_CADF)
  EVT_MENU(MENU_FILTER_C_C_F, Seg3DFrame::Filter_CCF)
  EVT_MENU(MENU_FILTER_N_C_F, Seg3DFrame::Filter_NCF)
  EVT_MENU(MENU_FILTER_B_D_E_F, Seg3DFrame::Filter_BDEF)
  EVT_MENU(MENU_FILTER_T_S_L_S_F, Seg3DFrame::Filter_TSLSF)
  EVT_MENU(MENU_FILTER_OTSU_T_F, Seg3DFrame::Filter_OTSUTF)
  EVT_MENU(MENU_FILTER_G_M_F, Seg3DFrame::Filter_GMF)
  EVT_MENU(MENU_FILTER_INHOMO_CORRECTION, Seg3DFrame::Filter_InhomoCorrection)
  EVT_MENU(MENU_FILTER_LABEL_EXTRACT, Seg3DFrame::Filter_LabelExtract)
  EVT_MENU(MENU_FILTER_FILL_HOLE, Seg3DFrame::Filter_FillHole)
  EVT_MENU(MENU_FILTER_FLOOD_FILL_COPY, Seg3DFrame::Filter_FloodFillCopy)
  EVT_MENU(MENU_FILTER_MEDIAN_FILTER, Seg3DFrame::Filter_MedianFilter)
  EVT_MENU(MENU_FILTER_DISCRETE_GAUSSIAN_FILTER, Seg3DFrame::Filter_DiscreteGaussianFilter)
  EVT_MENU(MENU_FILTER_HISTO_EQ, Seg3DFrame::Filter_HistoEq)
  EVT_MENU(MENU_FILTER_MASK_DATA, Seg3DFrame::Filter_MaskData)
  EVT_MENU(MENU_FILTER_MASK_LABEL, Seg3DFrame::Filter_MaskLabel)
  EVT_MENU(MENU_FILTER_LABEL_INVERT_FILTER, Seg3DFrame::Filter_LabelInvertFilter)
  EVT_MENU(MENU_FILTER_MASK_AND, Seg3DFrame::Filter_MaskAnd)
  EVT_MENU(MENU_FILTER_MASK_REMOVE, Seg3DFrame::Filter_MaskRemove)
  EVT_MENU(MENU_FILTER_MASK_OR, Seg3DFrame::Filter_MaskOr)
  EVT_MENU(MENU_FILTER_MASK_XOR, Seg3DFrame::Filter_MaskXor)

  EVT_MENU(MENU_FILTER_S_T_P_G_D_F, Seg3DFrame::Filter_SpeedToPathGradientDescent)
  EVT_MENU(MENU_FILTER_S_T_P_R_S_G_D_F, Seg3DFrame::Filter_SpeedToPathRegularStepGradientDescent)
  EVT_MENU(MENU_FILTER_S_T_P_I_N_F, Seg3DFrame::Filter_SpeedToPathIterateNeighborhood)

  EVT_MENU(MENU_HELP_UNDO, Seg3DFrame::Undo)
  EVT_MENU(MENU_HELP_INDEX, Seg3DFrame::Index)
  EVT_MENU(MENU_HELP_ABOUT, Seg3DFrame::About)

  EVT_COMMAND(wxID_ANY, wxEVT_VOLUME_INFO_PANEL, Seg3DFrame::OnVolumeInfoPanel)
END_EVENT_TABLE()


Seg3DFrame::Seg3DFrame(const std::string& target, wxFrame *frame,
                       const wxString& title, const wxPoint& pos,
                       const wxSize& size, long style)
  : wxFrame(frame, wxID_ANY, title, pos, size, style)
{
  wxBoxSizer* main_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(main_sizer);
  wxSize panel_size(PANEL_WIDTH, wxDefaultSize.y);
  toolsPanel_ = new wxPanel(this, wxID_ANY, wxDefaultPosition, panel_size, style);
  toolsPanel_->SetMinSize(panel_size);
  main_sizer->Add(left_sizer, 0, wxEXPAND);
  left_sizer->Add(toolsPanel_, 1, wxEXPAND, 0);

  // default tools label
  wxBoxSizer* tools_sizer = new wxBoxSizer(wxVERTICAL);
  toolsPanel_->SetSizer(tools_sizer);
  tool_label0_ = new wxStaticText(toolsPanel_, wxID_ANY, wxT(""));
  tool_label1_ = new wxStaticText(toolsPanel_, wxID_ANY, wxT(""));
  tools_sizer->Add(tool_label0_, 0, wxALIGN_CENTER, 0); 
  tools_sizer->Add(tool_label1_, 0, wxALIGN_CENTER, 0); 
  wxFont font = tool_label0_->GetFont();
  font.SetWeight(wxBOLD);
  tool_label0_->SetFont(font);
  tool_label1_->SetFont(font);

  cursorInformation_ =
    new ::CursorInformation(this, wxID_ANY, wxDefaultPosition, panel_size);
  left_sizer->Add(cursorInformation_, 0, wxEXPAND, 0);

  brushPanel_ = new BrushPanel(toolsPanel_);
  tools_sizer->Add(brushPanel_, 0, 0, 0);
  tools_sizer->Show(brushPanel_, false);
  tools_sizer->Layout();

  // new wx crop volume
  cropVolume_ = new CropVolume(toolsPanel_);
  tools_sizer->Add(cropVolume_, 0, 0, 0);
  tools_sizer->Show(cropVolume_, false);
  tools_sizer->Layout();
  
	// new wx crop cylinder
	/*
  cropCylinder_ = scinew CropVolCylinder(toolsPanel_);
  tools_sizer->Add(cropCylinder_, 0, 0, 0);
  tools_sizer->Show(cropCylinder_, false);
  tools_sizer->Layout();
	*/
  
	// new wx median filter volume
  medianFilterTool_ = new MedianFilterTool(toolsPanel_);
  tools_sizer->Add(medianFilterTool_, 0, 0, 0);
  tools_sizer->Show(medianFilterTool_, false);
  tools_sizer->Layout();

  // new wx flip volume
  flipTools_ = new FlipTool(toolsPanel_);
  tools_sizer->Add(flipTools_, 0, 0, 0);
  tools_sizer->Show(flipTools_, false);
  tools_sizer->Layout();

  histoEqTool_ = new HistoEqFilter(toolsPanel_);
  tools_sizer->Add(histoEqTool_, 0, 0, 0);
  tools_sizer->Show(histoEqTool_, false);
  tools_sizer->Layout();

  // new wx resample volume
  resampleTool_ = new ResampleTool(toolsPanel_);
  tools_sizer->Add(resampleTool_, 0, 0, 0);
  tools_sizer->Show(resampleTool_, false);
  tools_sizer->Layout();

  // new wx ITK test
  itk_CADF_ = new ITKCurvatureAnistopricDiffusionFilter(toolsPanel_);
  tools_sizer->Add(itk_CADF_, 0, 0, 0);
  tools_sizer->Show(itk_CADF_, false);
  tools_sizer->Layout();

  itk_CCF_ = new ITKConfidenceConnectedFilter(toolsPanel_);
  tools_sizer->Add(itk_CCF_, 0, 0, 0);
  tools_sizer->Show(itk_CCF_, false);
  tools_sizer->Layout();  

  itk_NCF_ = new ITKNeighbourhoodConnectedFilter(toolsPanel_);
  tools_sizer->Add(itk_NCF_, 0, 0, 0);
  tools_sizer->Show(itk_NCF_, false);
  tools_sizer->Layout();

  polylinetoolpanel_ = new PolylineToolPanel(toolsPanel_);
  tools_sizer->Add(polylinetoolpanel_, 0, 0, 0);
  tools_sizer->Show(polylinetoolpanel_, false);
  tools_sizer->Layout();

  thresholdtoolpanel_ = new ThresholdToolPanel(toolsPanel_);
  tools_sizer->Add(thresholdtoolpanel_, 0, 0, 0);
  tools_sizer->Show(thresholdtoolpanel_, false);
  tools_sizer->Layout();

  measurementtoolpanel_ = new MeasurementTool(toolsPanel_);
  tools_sizer->Add(measurementtoolpanel_, 0, 0, 0);
  tools_sizer->Show(measurementtoolpanel_, false);
  tools_sizer->Layout();

  movescaletoolpanel_ = new MoveScaleToolPanel(toolsPanel_);
  tools_sizer->Add(movescaletoolpanel_, 0, 0, 0);
  tools_sizer->Show(movescaletoolpanel_, false);
  tools_sizer->Layout();

  intensitycorrectionfilterpanel_ = new IntensityCorrectionFilterPanel(toolsPanel_);
  tools_sizer->Add(intensitycorrectionfilterpanel_, 0, 0, 0);
  tools_sizer->Show(intensitycorrectionfilterpanel_, false);
  tools_sizer->Layout();
  
  
  itk_BDEF_ = new ITKBinaryDilateErodeFilter(toolsPanel_);
  tools_sizer->Add(itk_BDEF_, 0, 0, 0);
  tools_sizer->Show(itk_BDEF_, false);
  tools_sizer->Layout();

  itk_TSLSF_ = new ITKThresholdSegmentationLevelSetFilter(toolsPanel_);
  tools_sizer->Add(itk_TSLSF_, 0, 0, 0);
  tools_sizer->Show(itk_TSLSF_, false);
  tools_sizer->Layout();

  optionless_ = new OptionlessFilter(toolsPanel_);
  tools_sizer->Add(optionless_, 0, 0, 0);
  tools_sizer->Show(optionless_, false);
  tools_sizer->Layout();

  itk_discretegaussianfilter_ = new itkDiscreteGaussianImageFilter(toolsPanel_);
  tools_sizer->Add(itk_discretegaussianfilter_, 0, 0, 0);
  tools_sizer->Show(itk_discretegaussianfilter_, false);
  tools_sizer->Layout();

  maskfilter_ = new MaskFilter(toolsPanel_);
  tools_sizer->Add(maskfilter_, 0, 0, 0);
  tools_sizer->Show(maskfilter_, false);
  tools_sizer->Layout();

  itk_STPGDF_ = new ITKSpeedToPathGradientDescentFilter(toolsPanel_);
  tools_sizer->Add(itk_STPGDF_, 0, 0, 0);
  tools_sizer->Show(itk_STPGDF_, false);
  tools_sizer->Layout();

  itk_STPRSGDF_ = new ITKSpeedToPathRegularStepGradientDescentFilter(toolsPanel_);
  tools_sizer->Add(itk_STPRSGDF_, 0, 0, 0);
  tools_sizer->Show(itk_STPRSGDF_, false);
  tools_sizer->Layout();

  itk_STPINF_ = new ITKSpeedToPathIterateNeighborhoodFilter(toolsPanel_);
  tools_sizer->Add(itk_STPINF_, 0, 0, 0);
  tools_sizer->Show(itk_STPINF_, false);
  tools_sizer->Layout();


  context_ =
    new WXOpenGLContext(target, this, wxID_ANY,
                           wxDefaultPosition, wxDefaultSize, style);
  main_sizer->Add(context_, 1, wxEXPAND, 0);

  current_tool_ = NULL;

	plugin_manager_ = new PluginManager(MENU_PLUGIN_START);

  Init();
}


void
Seg3DFrame::HideTool()
{
  tool_label0_->SetLabel("");
  tool_label1_->SetLabel("");

  // Too many copies of the brush radius, sync them to the brush panel
  // on tool closure.
  if (current_tool_ == itk_TSLSF_)
  {
    brushPanel_->mRadius->SetValue(itk_TSLSF_->mRadius->GetValue());
  }

  // Reset the optionless_ callback to the default.
  optionless_->SetSkinnerCallback();
  
  if (current_tool_)
  {
    current_tool_->Hide();
    current_tool_ = NULL;
  }

  SCIRun::Painter::ThrowSkinnerSignal("Painter::ClearTools");
}


// Create a new frame window with menu bar attached
bool
Seg3DFrame::Init()
{
  wxMenuBar * menuBar = new wxMenuBar;

  // File menu dialog
  wxMenu * winMenu = new wxMenu;
  winMenu->Append( MENU_FILE_LOAD_VOLUME, _T("Open &Volume") );
  winMenu->Append( MENU_FILE_LOAD_SESSION, _T("&Open Session") );
  winMenu->Append( MENU_FILE_IMPORT_SEGMENTATION, _T("&Import Segmentation") );
  winMenu->Append( MENU_FILE_SAVE_VOLUME, _T("Save Volume") );
  winMenu->Append( MENU_FILE_SAVE_SESSION, _T("&Save Session") );
  winMenu->Append( MENU_FILE_EXPORT_SEGMENTATION, _T("&Export Segmentation") );
  winMenu->Append( MENU_FILE_QUIT, _T("&Quit") );
  menuBar->Append( winMenu, _T("&File") );

  // View menu dialog
  winMenu = new wxMenu;
  winMenu->Append(MENU_VIEW_TWO_BY_TWO, _T("&Two by Two"));
  winMenu->Append(MENU_VIEW_ONE_BY_THREE, _T("&One and Three") );
  winMenu->Append(MENU_VIEW_SINGLE, _T("&Single") );
  winMenu->Append(MENU_VIEW_VOLUME_RENDER, _T("&Volume Rendering") );
  menuBar->Append(winMenu, _T("&Views"));

  prefsMenu_ = new wxMenu;
  prefsMenu_->AppendCheckItem(MENU_PREFS_FATLINES, _T("Fat label outlines"));
  prefsMenu_->Check(MENU_PREFS_FATLINES, true);
  prefsMenu_->AppendCheckItem(MENU_PREFS_STIPPLE, _T("Stippled labels"));
  prefsMenu_->Check(MENU_PREFS_STIPPLE, true);

  editMenu_ = new wxMenu;
  editMenu_->Append(MENU_EDIT_UNDO, _T("&Undo") );
  editMenu_->Enable(MENU_EDIT_UNDO, false);
  editMenu_->AppendSeparator();
  editMenu_->Append(MENU_EDIT_AUTOVIEW, _T("&Autoview All"));
  editMenu_->Append(MENU_EDIT_SET_MASK_LAYER, _T("Set &Mask Label"));
  editMenu_->Append(MENU_EDIT_CLEAR_MASK_LAYER, _T("&Clear Mask Label"));
  editMenu_->Append(MENU_EDIT_ISOSURFACE_ONE, _T("&Isosurface Current Label"));
  editMenu_->Append(MENU_EDIT_ISOSURFACE_ALL, _T("Isosurface All Labels"));
  editMenu_->Append(MENU_EDIT_SET_VRTARGET, _T("Set &Volume Rendering Target"));
  editMenu_->Append(MENU_EDIT_RESET_CLUT, _T("Reset &Brightness/Contrast"));
  editMenu_->Append(MENU_EDIT_MOVE_LAYER_UP, _T("Move Current Volume &Up"));
  editMenu_->Append(MENU_EDIT_MOVE_LAYER_DOWN, _T("Move Current Volume &Down"));
  editMenu_->AppendSeparator();
  editMenu_->Append(MENU_EDIT_PREFS, _T("Preferences"), prefsMenu_);
  menuBar->Append(editMenu_, _T("&Edit"));

  // Tools menu dialog
  winMenu = new wxMenu;
  winMenu->Append(MENU_TOOL_PAINT_BRUSH, _T("Paint &Brush"));
  winMenu->Append(MENU_TOOL_CROP_VOLUME, _T("&Crop Tool"));
  winMenu->Append(MENU_TOOL_CROP_CYLINDER, _T("Crop C&ylinder"));
  winMenu->Append(MENU_TOOL_FLIP, _T("&Flip Tool"));
  winMenu->Append(MENU_TOOL_RESAMPLE, _T("&Resample Tool"));
  winMenu->Append(MENU_TOOL_POLYLINE, _T("&Polyline Tool"));
  winMenu->Append(MENU_TOOL_THRESHOLD, _T("&Threshold Tool"));
  winMenu->Append(MENU_TOOL_MOVESCALE, _T("&Move/Scale Tool"));
  winMenu->Append(MENU_TOOL_MEASUREMENT, _T("Measurement Tool"));

  winMenu->AppendSeparator();
  // Speedlines tools
  winMenu->Append(MENU_FILTER_S_T_P_I_N_F, _T("&Speedline Iterate Neighborhood"));
  winMenu->Append(MENU_FILTER_S_T_P_G_D_F, _T("&Speedline Gradient Descent"));
  winMenu->Append(MENU_FILTER_S_T_P_R_S_G_D_F, _T("&Speedline Regular Step"));

  menuBar->Append(winMenu, _T("&Tools"));

  // ITK menu dialog
  winMenu = new wxMenu;

	// Plugins
	
	// winMenu->Append(MENU_PLUGIN_START, _T("Test"));
	plugin_manager_->LoadPlugins(winMenu,PLUGIN_FILTER,(wxFrame*)this);

  winMenu->AppendSeparator();
  
	// Data enhancers.
  winMenu->Append(MENU_FILTER_C_A_D_F, _T("Curvature &Anisotropic Diffusion Filter"));
  winMenu->Append(MENU_FILTER_MEDIAN_FILTER, _T("&Median Filter"));
  winMenu->Append(MENU_FILTER_DISCRETE_GAUSSIAN_FILTER, _T("&Discrete Gaussian Filter"));
  winMenu->Append(MENU_FILTER_G_M_F, _T("&Gradient Magnitude Filter"));
  winMenu->Append(MENU_FILTER_INHOMO_CORRECTION, _T("&Intensity Correction Filter"));
  winMenu->Append(MENU_FILTER_HISTO_EQ, _T("&Histogram Equalization"));
  winMenu->Append(MENU_FILTER_MASK_DATA, _T("Mask Data"));

  winMenu->AppendSeparator();

  // Segmenters
  winMenu->Append(MENU_FILTER_C_C_F, _T("&Confidence Connected Filter"));
  winMenu->Append(MENU_FILTER_N_C_F, _T("&Neighborhood Connected Filter"));
  winMenu->Append(MENU_FILTER_OTSU_T_F, _T("&Otsu Threshold Filter"));
  winMenu->Append(MENU_FILTER_T_S_L_S_F, _T("&Threshold Segmentation Level Set Filter"));

  winMenu->AppendSeparator();

  // Label manipulators.
  winMenu->Append(MENU_FILTER_LABEL_EXTRACT, _T("Copy Label Filter"));
  winMenu->Append(MENU_FILTER_LABEL_INVERT_FILTER, _T("&Label Invert Filter"));
  winMenu->Append(MENU_FILTER_MASK_AND, _T("Combine Labels with Logical And"));
  winMenu->Append(MENU_FILTER_MASK_REMOVE, _T("Combine Labels with Logical Remove"));
  winMenu->Append(MENU_FILTER_MASK_OR, _T("Combine Labels with Logical Or"));
  winMenu->Append(MENU_FILTER_MASK_XOR, _T("Combine Labels with Logical Xor"));
  winMenu->Append(MENU_FILTER_B_D_E_F, _T("&Binary Dilate -> Erode Filter"));
  winMenu->Append(MENU_FILTER_FLOOD_FILL_COPY, _T("Connected Component Filter"));
  winMenu->Append(MENU_FILTER_FILL_HOLE, _T("&Fill Holes Filter"));

  menuBar->Append(winMenu, _T("F&ilters"));

  // Tools menu dialog
  winMenu = new wxMenu;
#ifdef USING_HELP_FILES
  winMenu->Append(MENU_HELP_INDEX, _T("&Contents"));
#endif
  //winMenu->Append( MENU_HELP_UNDO, _T("&Undo") ); // Not yet
  winMenu->Append(MENU_HELP_ABOUT, _T("&About Seg3D"));
  menuBar->Append(winMenu, _T("&Help"));

#ifdef USING_HELP_FILES
  // uses .chm for windows, .htb (which is a .zip renamed .htb) for everything else
  help_ = new wxHelpController;
  wxFileSystem::AddHandler(new wxZipFSHandler);
  help_->Initialize(wxT("doc/Seg3D"));
#endif

  SetMenuBar(menuBar);
  CreateStatusBar();

  Show();
  return true;
}


void
Seg3DFrame::OnCloseWindow(wxCloseEvent &event)
{
  if (event.CanVeto())
  {
    const int answer =
      wxMessageBox(wxT("Quit Seg3D?"), wxT("Confirm"), wxOK | wxCANCEL, this);
    if (answer != wxOK)
    {
      event.Veto();
      return;
    }
  }

#ifdef USING_HELP_FILES
  delete help_;
#endif
  // TODO: Fix the destroy function rather than using the signal.

#ifndef _WIN32
  raise(SIGKILL);
#else
  // do this instead of Destroy until we figure out how to stop the hang. 
  // (the Draw thread is locking in the GUIMutex)
  Thread::exitAll(0);
  //this->Destroy();
#endif
}


void
Seg3DFrame::OnStatusBarTextChange(wxCommandEvent &event)
{
  SetStatusText(event.GetString());
}


void
Seg3DFrame::OnColourPicker(wxCommandEvent &event)
{
  colour_picker_data_t *cpd = (colour_picker_data_t *)event.GetClientData();
  const unsigned char r = (unsigned char)(cpd->r * 255.0);
  const unsigned char g = (unsigned char)(cpd->g * 255.0);
  const unsigned char b = (unsigned char)(cpd->b * 255.0);
  wxColourData wxcdata;
  wxcdata.SetColour(wxColour(r, g, b));
  wxColourDialog wxdialog(this, &wxcdata);
  if (wxdialog.ShowModal() == wxID_OK)
  {
    const double rr = wxdialog.GetColourData().GetColour().Red() / 255.0;
    const double gg = wxdialog.GetColourData().GetColour().Green() / 255.0;
    const double bb = wxdialog.GetColourData().GetColour().Blue() / 255.0;

    SCIRun::ThrowSkinnerSignalEvent *tsse =
      new SCIRun::ThrowSkinnerSignalEvent("Painter::SetLabelColor");
    tsse->add_var("Painter::SetLabelColor::r", to_string(rr));
    tsse->add_var("Painter::SetLabelColor::g", to_string(gg));
    tsse->add_var("Painter::SetLabelColor::b", to_string(bb));
    tsse->add_var("Painter::SetLabelColor::layername", cpd->layername);
    SCIRun::Painter::ThrowSkinnerSignal(tsse);
  }
}


void
Seg3DFrame::OnHideTool(wxCommandEvent &event)
{
  HideTool();
}

void
Seg3DFrame::OnOKDialog(wxCommandEvent &event)
{
  wxMessageDialog dialog(this,
                         event.GetString(),
                         _T("Seg3D error message"),
                         wxOK | wxICON_ERROR);
  dialog.ShowModal();
}

void
Seg3DFrame::OnLayerDeleteDialog(wxCommandEvent &event)
{
  // Verify the deletion with the user.
  wxMessageDialog dialog(Painter::global_seg3dframe_pointer_,
                         event.GetString(),
                         _T("Delete volume"),
                         wxYES_NO | wxICON_ERROR);
  if (dialog.ShowModal() == wxID_YES)
  {
    SCIRun::Painter::ThrowSkinnerSignal("Painter::DeleteLayer2");
  }
  else
  {
    SetStatusText("Delete volume canceled by user.");
  }    
}

void
Seg3DFrame::OnUnwrapWindow(wxCommandEvent &event)
{
	UnwrapPluginWindow * unwrap_win = new UnwrapPluginWindow(_T("Unwrapped View"), Painter::global_seg3dframe_pointer_,
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, ((Unwrapping*)event.GetClientData())->num_unwraps);
	unwrap_win->scroll->set_image((Unwrapping*)event.GetClientData());
	unwrap_win->slide->set_scroll(unwrap_win->scroll);
	unwrap_win->Show(true);
}

void
Seg3DFrame::OnUpdateUndoState(wxCommandEvent &event)
{
  update_undo_state_t *uus = (update_undo_state_t *)event.GetClientData();
  if (uus && uus->enable_)
  {
    editMenu_->SetLabel(MENU_EDIT_UNDO, std2wx(uus->label_));
    editMenu_->Enable(MENU_EDIT_UNDO, true);
  }
  else
  {
    editMenu_->SetLabel(MENU_EDIT_UNDO, _T("Undo"));
    editMenu_->Enable(MENU_EDIT_UNDO, false);
  }

  if (uus) { delete uus; }
}

DEFINE_EVENT_TYPE(wxEVT_COMMAND_STATUS_BAR_TEXT_CHANGE)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_COLOUR_PICKER)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_HIDE_TOOL)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_OK_DIALOG)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_LAYER_DELETE_DIALOG)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_EXPORT_LABEL_SELECTION)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_UPDATE_UNDO_STATE)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_UNWRAP_WINDOW)


void
Seg3DFrame::FileLoadVolume( wxCommandEvent& WXUNUSED(event) )
{
  static const wxChar *formats =
    _T("All files(*.*)|*|"
       "Basic volume files (*.nrrd;*.nhdr;*.mha;*.mhd;*.vff)|"
       "*.nrrd;*.nhdr;*.mha;*.mhd;*.vff;"
       "*.NRRD;*.NHDR;*.MHA;*.MHD;*.VFF|"
       "Other volume files (*.pic;*.vtk;*.gipl;*.lsm;*.img;*.hdr;*.spr;*.mat)|"
       "*.pic;*.vtk;*.gipl;*.lsm;*.img;*.hdr;*.spr;*.mat;"
       "*.PIC;*.VTK;*.GIPL;*.LSM;*.IMG;*.HDR;*.SPR;*.MAT|"
       "Image sequences (*.tiff;*.tif;*.bmp;*.dcm;*.dicom;*.png;*.jpeg;*.jpg)|"
       "*.tiff;*.tif;*.bmp;*.png;*.jpeg;*.jpg;"
       "*.TIFF;*.TIF;*.BMP;*.PNG;*.JPEG;*.JPG"
       );

  wxFileDialog dialog(this, _T("Seg3D open volume dialog"),
                      CurrentDocPath, wxEmptyString, formats);
	  
  dialog.CentreOnParent();

  if( CurrentDocPath.IsEmpty() )
  {
    dialog.SetDirectory(wxGetHomeDir());
  }
	  
  if (dialog.ShowModal() == wxID_OK)
  {
    const string filename = wx2std(dialog.GetPath(), &wxConvFile);

    // Busy cursor the load operation.
    SetStatusText(std2wx("Loading " + filename));
    wxBusyCursor(); // Busy cursor until this leaves scope.

    SCIRun::ThrowSkinnerSignalEvent *tsse =
      new SCIRun::ThrowSkinnerSignalEvent("Painter::LoadVolume");
    tsse->add_var("Painter::LoadVolume::filename", filename);
    SCIRun::Painter::ThrowSkinnerSignal(tsse);

    CurrentDocPath = wxFileName::FileName(dialog.GetPath()).GetPath();
  }
}


void
Seg3DFrame::FileLoadSession( wxCommandEvent& WXUNUSED(event) )
{
  wxFileDialog dialog(this, _T("Seg3D open session dialog"),
                      CurrentDocPath, wxEmptyString,
#ifdef __WXMOTIF__
                      _T("Session files(*.ses)|*.ses")
#else
                      _T("Session files(*.ses)|*.ses|All files(*.*)|*"));
#endif

  dialog.CentreOnParent();

  if( CurrentDocPath.IsEmpty() )
  {
    dialog.SetDirectory(wxGetHomeDir());
  }
	  
  if (dialog.ShowModal() == wxID_OK)
  {
    const string filename = wx2std(dialog.GetPath(), &wxConvFile);

    SetStatusText(std2wx("Loading " + filename));
    wxBusyCursor cursor; // Busy cursor until this leaves scope.

    SCIRun::ThrowSkinnerSignalEvent *tsse =
      new SCIRun::ThrowSkinnerSignalEvent("Painter::LoadSession");
    tsse->add_var("Painter::LoadSession::filename", filename);
    SCIRun::Painter::ThrowSkinnerSignal(tsse);

    CurrentDocPath = wxFileName::FileName(dialog.GetPath()).GetPath();
  }
  else
  {
    SetStatusText(wxT("Load Volume canceled."));
  }
}


static string
generate_long_filename(const string &stuff)
{
  string result;
  for (size_t i = 0; i < stuff.size(); i++)
  {
    if (isalnum(stuff[i])) result.push_back(stuff[i]);
  }
  return result;
}

static string
generate_short_filename(const string &stuff)
{
  string result;
  for (size_t i = 0; i < stuff.size(); i++)
  {
    if (stuff[i] == ' ') break;
    result.push_back(stuff[i]);
  }
  return result;
}


void
Seg3DFrame::FileSaveVolume( wxCommandEvent& WXUNUSED(event) )
{
  static const wxChar *formats = _T("All files(*.*)|*");
  
  static string last_filename = "";
  string default_filename = last_filename;
  if (default_filename == "")
  {
    default_filename =
      generate_long_filename(SCIRun::Painter::get_current_layer_name())
      + ".nrrd";
  }

  wxFileDialog dialog(this, _T("Save Volume"), CurrentDocPath,
                      std2wx(default_filename),
                      formats, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

  dialog.SetFilterIndex(1);

  if (dialog.ShowModal() == wxID_OK)
  {
    const string filename = wx2std(dialog.GetPath(), &wxConvFile);
    last_filename = filename;

    SetStatusText(std2wx("Saving " + filename));
    wxBusyCursor cursor; // Busy cursor until this leaves scope.

    SCIRun::ThrowSkinnerSignalEvent *tsse =
      new SCIRun::ThrowSkinnerSignalEvent("Painter::SaveVolume");
    tsse->add_var("Painter::SaveVolume::filename", filename);
    SCIRun::Painter::ThrowSkinnerSignal(tsse);

    CurrentDocPath = wxFileName::FileName(dialog.GetPath()).GetPath();
  }
  else
  {
    SetStatusText(wxT("Save Volume canceled."));
  }
}


void
Seg3DFrame::FileSaveSession( wxCommandEvent& WXUNUSED(event) )
{
  static string last_filename = "";
  string default_filename = last_filename;
  if (default_filename == "")
  {
    default_filename =
      generate_short_filename(SCIRun::Painter::get_current_layer_name())
      + ".ses";
  }

  wxFileDialog dialog(this, _T("Save Session"), CurrentDocPath,
                      std2wx(default_filename),
                      _T("Session files (*.ses)|*.ses"),
                      wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

  dialog.SetFilterIndex(1);

  if (dialog.ShowModal() == wxID_OK)
  {
    const string filename = wx2std(dialog.GetPath(), &wxConvFile);
    last_filename = filename;

    SetStatusText(std2wx("Saving " + filename));
    wxBusyCursor cursor; // Busy cursor until this leaves scope.

    SCIRun::ThrowSkinnerSignalEvent *tsse =
      new SCIRun::ThrowSkinnerSignalEvent("Painter::SaveSession");
    tsse->add_var("Painter::SaveSession::filename", filename);
    SCIRun::Painter::ThrowSkinnerSignal(tsse);

    CurrentDocPath = wxFileName::FileName(dialog.GetPath()).GetPath();
  }
  else
  {
    SetStatusText(wxT("Save Session canceled."));
  }
}


void
Seg3DFrame::FileImportSegmentation( wxCommandEvent& WXUNUSED(event) )
{
  static const wxChar *formats =
    _T("All files(*.*)|*|"
       "Basic volume files (*.nrrd;*.nhdr;*.mha;*.mhd;*.vff)|"
       "*.nrrd;*.nhdr;*.mha;*.mhd;*.vff;"
       "*.NRRD;*.NHDR;*.MHA;*.MHD;*.VFF|"
       "Other volume files (*.pic;*.vtk;*.gipl;*.lsm;*.img;*.hdr;*.spr;*.mat)|"
       "*.pic;*.vtk;*.gipl;*.lsm;*.img;*.hdr;*.spr;*.mat;"
       "*.PIC;*.VTK;*.GIPL;*.LSM;*.IMG;*.HDR;*.SPR;*.MAT|"
       "Image sequences (*.tiff;*.tif;*.bmp;*.dcm;*.dicom;*.png;*.jpeg;*.jpg)|"
       "*.tiff;*.tif;*.bmp;*.png;*.jpeg;*.jpg;"
       "*.TIFF;*.TIF;*.BMP;*.PNG;*.JPEG;*.JPG"
       );

  wxFileDialog dialog(this, _T("Seg3D import segmentation dialog"),
                      CurrentDocPath, wxEmptyString, formats);
  
  dialog.CentreOnParent();

  if( CurrentDocPath.IsEmpty() )
  {
    dialog.SetDirectory(wxGetHomeDir());
  }
	  
  if (dialog.ShowModal() == wxID_OK)
  {
    const string filename = wx2std(dialog.GetPath(), &wxConvFile);

    // Busy cursor the load operation.
    SetStatusText(std2wx("Importing " + filename));
    wxBusyCursor(); // Busy cursor until this leaves scope.

    SCIRun::ThrowSkinnerSignalEvent *tsse =
      new SCIRun::ThrowSkinnerSignalEvent("Painter::ImportSegmentation");
    tsse->add_var("Painter::ImportSegmentation::filename", filename);
    SCIRun::Painter::ThrowSkinnerSignal(tsse);

    CurrentDocPath = wxFileName::FileName(dialog.GetPath()).GetPath();
  }
}


void
Seg3DFrame::FileExportSegmentation( wxCommandEvent& WXUNUSED(event) )
{
  SCIRun::Painter::ThrowSkinnerSignal("Painter::ExportSegmentation1");
}


void
Seg3DFrame::OnExportLabelSelection( wxCommandEvent& event )
{
  export_label_selection_data_t *data =
    (export_label_selection_data_t *)event.GetClientData();
  
  const size_t nselections =
    wxGetMultipleChoices(data->selections_,
                         _T("Select which labels to export."),
                         _T("Segmentation export picker"),
                         data->nchoices_, data->choices_,
                         this);

  delete [] data->choices_;
  data->choices_ = NULL;

  // Check for positive count.
  if (nselections == 0)
  {
    wxMessageDialog dialog(this, _T("No labels were selected for export."),
                           _T("Seg3D error message"),
                           wxOK | wxICON_ERROR);
    dialog.ShowModal();
    return;
  }

  static const wxChar *formats = _T("All files(*.*)|*");

  static string last_filename = "";
  string default_filename = last_filename;
  if (default_filename == "")
  {
    default_filename =
      generate_short_filename(SCIRun::Painter::get_current_layer_name())
      + "-seg.nrrd";
  }

  wxFileDialog dialog(this, _T("Export Segmentation"), CurrentDocPath,
                      std2wx(default_filename),
                      formats, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
  
  dialog.SetFilterIndex(1);

  if (dialog.ShowModal() == wxID_OK)
  {
    const string filename = wx2std(dialog.GetPath(), &wxConvFile);
    last_filename = filename;

    SetStatusText(std2wx("Exporting " + filename));
    wxBusyCursor cursor; // Busy cursor until this leaves scope.

    SCIRun::ThrowSkinnerSignalEvent *tsse =
      new SCIRun::ThrowSkinnerSignalEvent("Painter::ExportSegmentation2");
    tsse->add_var("Painter::ExportSegmentation::filename", filename);
    SCIRun::Painter::ThrowSkinnerSignal(tsse);

    CurrentDocPath = wxFileName::FileName(dialog.GetPath()).GetPath();
  }
  else
  {
    SetStatusText(wxT("Export Segmentation canceled."));
  }
}


void
Seg3DFrame::Undo( wxCommandEvent& WXUNUSED(event) )
{
}


void
Seg3DFrame::ViewTwoByTwo( wxCommandEvent& WXUNUSED(event) )
{
  PainterShowVisibleItem("Two by Two", "MainLayout");
  Painter::ThrowSkinnerSignal("Painter::Autoview");
}


void
Seg3DFrame::ViewOneByThree( wxCommandEvent& WXUNUSED(event) )
{
  PainterShowVisibleItem("One and Three", "MainLayout");
  Painter::ThrowSkinnerSignal("Painter::Autoview");
}


void
Seg3DFrame::ViewSingle( wxCommandEvent& WXUNUSED(event) )
{
  PainterShowVisibleItem("Single", "MainLayout");
  Painter::ThrowSkinnerSignal("Painter::Autoview");
}


void
Seg3DFrame::ViewVolumeRender( wxCommandEvent& WXUNUSED(event) )
{
  // Busy cursor this one, turning on volume rendering can be slow.
  SetStatusText(wxT("Automatically enabling volume rendering."));
  wxBusyCursor cursor; // Busy cursor until this leaves scope.

  // Turn on volume rendering if switching to the volume rendering view.
  SCIRun::ThrowSkinnerSignalEvent *tsse =
    new SCIRun::ThrowSkinnerSignalEvent("Painter::ShowVolumeRendering2");
  tsse->add_var("Painter::volume_visible", "1");
  SCIRun::Painter::ThrowSkinnerSignal(tsse);

  // Switch the view.
  PainterShowVisibleItem("Volume Rendering", "MainLayout");

  Painter::ThrowSkinnerSignal("Painter::Autoview");
}


void
Seg3DFrame::FileQuit( wxCommandEvent& WXUNUSED(event) )
{
  Close();
}


void
Seg3DFrame::EditAutoview( wxCommandEvent& WXUNUSED(event) )
{
  Painter::ThrowSkinnerSignal("Painter::Autoview");
  SetStatusText(wxT("Autoview complete."));
}


void
Seg3DFrame::ToolPaintBrush( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(brushPanel_, "Painter::StartBrushTool", "Paint Brush");
}

void
Seg3DFrame::ToolCropVolume( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(cropVolume_, "Painter::StartCropTool", "Crop Tool");
}

void
Seg3DFrame::ToolCropCylinder( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(cropCylinder_, "Painter::StartCropCylinder", "Crop Cylinder");
}

void
Seg3DFrame::ToolFlip( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(flipTools_, "", "Flip Tool");
}

void
Seg3DFrame::ToolResample( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(resampleTool_, "Painter::StartResampleTool", "Resample Tool");
}

void
Seg3DFrame::EditIsosurfaceOne( wxCommandEvent& WXUNUSED(event) )
{
  SetStatusText(wxT("Computing label isosurface."));
  wxBusyCursor cursor; // Busy cursor until this leaves scope.
  Painter::ThrowSkinnerSignal("Painter::ShowOneIsosurface");
}

void
Seg3DFrame::EditIsosurfaceAll( wxCommandEvent& WXUNUSED(event) )
{
  SetStatusText(wxT("Computing all label isosurfaces."));
  wxBusyCursor cursor; // Busy cursor until this leaves scope.
  Painter::ThrowSkinnerSignal("Painter::ShowAllIsosurfaces");
}

void
Seg3DFrame::EditSetVRTarget( wxCommandEvent& WXUNUSED(event) )
{
  SetStatusText(wxT("Setting new volume rendering target."));
  wxBusyCursor(); // Busy cursor until this leaves scope.
  Painter::ThrowSkinnerSignal("Painter::ShowVolumeRendering");
}

void
Seg3DFrame::EditResetCLUT( wxCommandEvent& WXUNUSED(event) )
{
  Painter::ThrowSkinnerSignal("Painter::ResetCLUT");
  SetStatusText("Brightness and contrast reset for the active volume.");
}

void
Seg3DFrame::EditMoveLayerUp( wxCommandEvent& WXUNUSED(event) )
{
  Painter::ThrowSkinnerSignal("Painter::MoveLayerUp");
}

void
Seg3DFrame::EditMoveLayerDown( wxCommandEvent& WXUNUSED(event) )
{
  Painter::ThrowSkinnerSignal("Painter::MoveLayerDown");
}

void
Seg3DFrame::EditSetMaskLayer( wxCommandEvent& WXUNUSED(event) )
{
  Painter::ThrowSkinnerSignal("Painter::SetMaskLayer");
  SetStatusText("Set the mask volume.");
}

void
Seg3DFrame::EditUndo( wxCommandEvent& WXUNUSED(event) )
{
  wxBusyCursor();
  Painter::ThrowSkinnerSignal("Painter::Undo");
}

	void
Seg3DFrame::OnUpdatePrefsFatlines( wxCommandEvent& WXUNUSED(event) )
{
  const bool fatlines = prefsMenu_->IsChecked(MENU_PREFS_FATLINES);

  SCIRun::ThrowSkinnerSignalEvent *tsse =
    new SCIRun::ThrowSkinnerSignalEvent("Painter::RedrawAll");
  tsse->add_var("Painter::appearance::fatlines", fatlines?"1":"0");
  SCIRun::Painter::ThrowSkinnerSignal(tsse);
}

void
Seg3DFrame::OnUpdatePrefsStipple( wxCommandEvent& WXUNUSED(event) )
{
  const bool stipple = prefsMenu_->IsChecked(MENU_PREFS_STIPPLE);

  SCIRun::ThrowSkinnerSignalEvent *tsse =
    new SCIRun::ThrowSkinnerSignalEvent("Painter::RedrawAll");
  tsse->add_var("Painter::appearance::stipple", stipple?"1":"0");
  SCIRun::Painter::ThrowSkinnerSignal(tsse);
}

void
Seg3DFrame::EditClearMaskLayer( wxCommandEvent& WXUNUSED(event) )
{
  Painter::ThrowSkinnerSignal("Painter::ClearMaskLayer");
  SetStatusText("Cleared the mask layer.");
}

void
Seg3DFrame::ToolPolyline( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_NCF_, "Painter::StartPolylineTool", "Polyline Tool");
  itk_NCF_->SetShowProgress(false);
}

void
Seg3DFrame::ToolThreshold( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(thresholdtoolpanel_,
           "Painter::StartThresholdTool", "Threshold Tool");
  itk_NCF_->SetShowProgress(false);
}

void
Seg3DFrame::ToolMoveScale( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(movescaletoolpanel_, "", "Move/Scale Tool");
  SCIRun::Painter::ThrowSkinnerSignal("Painter::MoveScaleLayerUpdateGUI");
}

void
Seg3DFrame::ToolMeasurement( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(measurementtoolpanel_, "", "Measurement Tool");
  SCIRun::Painter::ThrowSkinnerSignal("Painter::StartMeasurementTool");
}

void
Seg3DFrame::PluginEvent( wxCommandEvent& event )
{
	printf("PluginEvent called\n");
	plugin_manager_->HandleEvent(event);

	// Begin ShowTool
	// Clean up the current panel.
  HideTool();

  // Set the current tool to this one.
  current_tool_ = optionless_;

  // Reset the progress meter.
  SCIRun::Painter::update_progress(0);

  // Set the title text.
  tool_label0_->SetLabel(plugin_manager_->GetPluginTitle(event.GetId()).c_str());
  tool_label1_->SetLabel("");

  // Display the new tool panel.
  optionless_->Show();
  this->toolsPanel_->GetSizer()->Show(optionless_, true);
  this->toolsPanel_->GetSizer()->Layout();

  // Tell the skinner side which tool is active.
	SCIRun::ThrowSkinnerSignalEvent *tsse =
      new SCIRun::ThrowSkinnerSignalEvent("Painter::PluginFilter");
  tsse->add_var("Painter::PluginFilter::id", SCIRun::to_string(event.GetId()));
  SCIRun::Painter::ThrowSkinnerSignal(tsse,true);
  // End ShowTool

	// optionless_->SetSkinnerCallback("Painter::PluginFilter");
	optionless_->SetShowProgress(true);
}

void
Seg3DFrame::Filter_CADF( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_CADF_, "Painter::ITKCurvatureAnisotropic",
           "Curvature Anisotropic", "Diffusion Filter");
}


void
Seg3DFrame::Filter_CCF( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_CCF_, "Painter::start_ITKConfidenceConnectedImageFilterTool",
           "Confidence Connected", "Filter");
}


void
Seg3DFrame::Filter_NCF( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_NCF_, "Painter::start_ITKNeighborhoodConnectedImageFilterTool",
           "Neighborhood Connected", "Filter");
  itk_NCF_->SetShowProgress(true);
}


void
Seg3DFrame::Filter_BDEF( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_BDEF_, "Painter::ITKBinaryDilateErodeImageFilter",
           "Binary Dilate -> Erode", "Filter");
}


void
Seg3DFrame::Filter_TSLSF( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_TSLSF_, "Painter::start_ITKThresholdSegmentationLevelSetImageFilterTool",
           "Threshold Segmentation", "Level Set Filter");
}


void
Seg3DFrame::Filter_OTSUTF( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(optionless_, "Painter::ITKOtsuFilter",
           "Otsu Threshold Filter");
  optionless_->SetShowProgress(true);
}


void
Seg3DFrame::Filter_GMF( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(optionless_, "Painter::ITKGradientMagnitude",
           "Gradient Magnitude Filter");
  optionless_->SetShowProgress(true);
}


void
Seg3DFrame::Filter_InhomoCorrection( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(intensitycorrectionfilterpanel_,
           "Painter::InhomogeneityCorrectionFilter",
           "Intensity Correction", "Filter");
}


void
Seg3DFrame::Filter_LabelInvertFilter( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(optionless_, "", "Label Invert Filter");
  optionless_->SetSkinnerCallback("Painter::LabelInvertFilter");
  optionless_->SetShowProgress(false);
}


void
Seg3DFrame::Filter_LabelExtract( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(optionless_, "", "Copy Label Filter");
  optionless_->SetSkinnerCallback("Painter::CopyLabel");
  optionless_->SetShowProgress(false);
}

void
Seg3DFrame::Filter_FillHole( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_NCF_, "Painter::ITKHoleFillFilter", "Fill Holes Filter");
  itk_NCF_->SetShowProgress(true);
}


void
Seg3DFrame::Filter_FloodFillCopy( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_NCF_, "Painter::FloodFillCopyFilter", "Connected Component", "Filter");
  itk_NCF_->SetShowProgress(true);
}


void
Seg3DFrame::Filter_MedianFilter( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(medianFilterTool_, "", "Median Filter");
}


void
Seg3DFrame::Filter_DiscreteGaussianFilter( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_discretegaussianfilter_, "Painter::start_ITKDiscreteGaussianImageFilterTool", "Discrete Gaussian Filter");
}


void
Seg3DFrame::Filter_HistoEq( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(histoEqTool_, "", "Histogram Equalization");
}


void
Seg3DFrame::Filter_MaskData( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(maskfilter_, "", "Mask Data");
  maskfilter_->SetSkinnerCallback("Painter::MaskDataFilter");
}


void
Seg3DFrame::Filter_MaskLabel( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(maskfilter_, "", "Mask Label");
  maskfilter_->SetSkinnerCallback("Painter::MaskLabelFilter");
}

void
Seg3DFrame::Filter_MaskAnd( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(maskfilter_, "", "Combine Labels with", "Logical And");
  maskfilter_->SetSkinnerCallback("Painter::CombineMaskAnd");
}

void
Seg3DFrame::Filter_MaskRemove( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(maskfilter_, "", "Combine Labels with", "Logical Remove");
  maskfilter_->SetSkinnerCallback("Painter::CombineMaskRemove");
}

void
Seg3DFrame::Filter_MaskOr( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(maskfilter_, "", "Combine Labels with", "Logical Or");
  maskfilter_->SetSkinnerCallback("Painter::CombineMaskOr");
}

void
Seg3DFrame::Filter_MaskXor( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(maskfilter_, "", "Combine Labels with", "Logical Xor");
  maskfilter_->SetSkinnerCallback("Painter::CombineMaskXor");
}

void
Seg3DFrame::Filter_SpeedToPathGradientDescent( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_STPGDF_, "Painter::start_ITKSpeedToPathGradientDescentImageFilterTool", "Speedline", "Gradient Descent");
}

void
Seg3DFrame::Filter_SpeedToPathRegularStepGradientDescent( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_STPRSGDF_, "Painter::start_ITKSpeedToPathRegularStepGradientDescentImageFilterTool", "Speedline", "Regular Step");
}


void
Seg3DFrame::Filter_SpeedToPathIterateNeighborhood( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_STPINF_, "Painter::start_ITKSpeedToPathIterateNeighborhoodImageFilterTool", "Speedline", "Iterate Neighborhood");
}

void
Seg3DFrame::ShowTool(wxPanel* tool, const char* event_name,
                     const char *title0, const char *title1)
{
  // Clean up the current panel.
  HideTool();

  // Set the current tool to this one.
  current_tool_ = tool;

  // Reset the progress meter.
  SCIRun::Painter::update_progress(0);

  // Set the title text.
  tool_label0_->SetLabel(title0);
  tool_label1_->SetLabel(title1);

  // Display the new tool panel.
  tool->Show();
  this->toolsPanel_->GetSizer()->Show(tool, true);
  this->toolsPanel_->GetSizer()->Layout();

  // Tell the skinner side which tool is active.
  if (strcmp(event_name, "") != 0)
  {
    Painter::ThrowSkinnerSignal(event_name);
  }

  // Always update the brush size from the wx brush tool because there
  // are too many copies of the radius.
  if (tool == brushPanel_ || tool == itk_TSLSF_)
  {
    itk_TSLSF_->mRadius->SetValue(brushPanel_->mRadius->GetValue());
    SCIRun::ThrowSkinnerSignalEvent *tsse =
      new SCIRun::ThrowSkinnerSignalEvent("Painter::UpdateBrushRadius");
    tsse->add_var("Painter::brush_radius",
                  SCIRun::to_string(brushPanel_->mRadius->GetValue()));
    SCIRun::Painter::ThrowSkinnerSignal(tsse);
  }
}
  

wxPanel *
Seg3DFrame::CurrentToolPanel()
{
  return current_tool_;
}


void
Seg3DFrame::About( wxCommandEvent& WXUNUSED(event) )
{
  wxAboutDialogInfo info;
  info.SetName(_T("Seg3D"));
  info.SetVersion(_T(SEG3D_VERSION_STRING));
  info.SetDescription(_T("Segmentation and Imaging Tool"));
  info.SetCopyright(_T("(C) 1993-2008 Scientific Computing and Imaging Institute, University of Utah"));

  info.AddDeveloper(_T("J.R. Blackham"));
  info.AddDeveloper(_T("David Brayford"));
  info.AddDeveloper(_T("Jason Callahan"));
  info.AddDeveloper(_T("Michael Callahan"));
  info.AddDeveloper(_T("Joshua Cates"));
  info.AddDeveloper(_T("Marty Cole"));
  info.AddDeveloper(_T("McKay Davis"));
  info.AddDeveloper(_T("Elizabeth Jurrus"));
  info.AddDeveloper(_T("Rob MacLeod"));
  info.AddDeveloper(_T("Jeroen Stinstra"));
  info.AddDeveloper(_T("Chems Touati"));
  info.AddDeveloper(_T("David Weinstein"));
  info.AddDeveloper(_T("Ross Whitaker"));
  info.AddDeveloper(_T("Brian Worthen"));

  info.SetWebSite(_T("http://www.seg3d.org"), _T("Seg3D web site"));
  info.SetLicense(wxString::FromAscii(
"Permission is hereby granted, free of charge, to any person obtaining a\n"
"copy of this software and associated documentation files (the \"Software\"),\n"
"to deal in the Software without restriction, including without limitation\n"
"the rights to use, copy, modify, merge, publish, distribute, sublicense,\n"
"and/or sell copies of the Software, and to permit persons to whom the\n"
"Software is furnished to do so, subject to the following conditions:\n"
"\n"
"The above copyright notice and this permission notice shall be included\n"
"in all copies or substantial portions of the Software.\n"
"\n"
"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS\n"
"OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL\n"
"THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n"
"FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER\n"
"DEALINGS IN THE SOFTWARE.\n"
));

  wxAboutBox(info);
}

void Seg3DFrame::Index( wxCommandEvent& WXUNUSED(event) )
{
#ifdef USING_HELP_FILES
  help_->DisplayContents();
#endif
}


void
Seg3DFrame::PainterShowVisibleItem(const string &id, const string &group)
{
  SCIRun::ThrowSkinnerSignalEvent *tsse =
    new SCIRun::ThrowSkinnerSignalEvent("Painter::ShowVisibleItem");
  tsse->add_var("Painter::ShowVisibleItem::id", id);
  tsse->add_var("Painter::ShowVisibleItem::group", group);
  SCIRun::Painter::ThrowSkinnerSignal(tsse);
}

class VolumeInfoDialog : public wxDialog
{
public:
  VolumeInfoDialog(wxWindow *parent, VolumeInfoPanelStruct *info)
    : wxDialog(parent, wxID_ANY, _T("Volume Information"),
               wxDefaultPosition, wxSize(PANEL_WIDTH, PANEL_WIDTH*2))
  {
    std::ostringstream ostrm;
    if (info->is_label)
    {
      ostrm << "This is a label volume.\n";
    }
    else
    {
      ostrm << "This is a data volume.\n";
    }
    ostrm << "Origin: " <<
      info->origin_x << " " <<
      info->origin_y << " " <<
      info->origin_z << "\n";
    ostrm << "Spacing: " <<
      info->spacing_x << " " <<
      info->spacing_y << " " <<
      info->spacing_z << "\n";
    ostrm << "Size: " <<
      info->size_x << " " <<
      info->size_y << " " <<
      info->size_z << "\n";
    ostrm << "Volume: " << info->volume << "\n";
    if (info->is_label)
    {
      ostrm << "Label Count: " << info->labelcount << "\n";
      ostrm << "Label Volume: " << info->labelvolume << "\n";
      ostrm << "Label Volume Percentage: " << info->labelvolume * 100.0 / info->volume << "\n";
      if (info->is_mask)
      {
        ostrm << "Mask Count: " << info->maskcount << "\n";
        ostrm << "Mask Volume: " << info->maskvolume << "\n";
        ostrm << "Percentage of Mask Volume: " << info->labelvolume * 100.0 / info->maskvolume << "\n";
      }
    }
    else
    {
      ostrm << "Min/Max: " << info->datamin << " " << info->datamax << "\n";
    }

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText *stext0 =
      new wxStaticText(this, wxID_ANY, std2wx(info->name));
    wxFont font = stext0->GetFont();
    font.SetWeight(wxBOLD);
    stext0->SetFont(font);
    wxStaticText *stext1 =
      new wxStaticText(this, wxID_ANY, std2wx(ostrm.str()));
    wxButton *button = new wxButton(this, VOLUME_INFO_OK_BUTTON, _T("OK"));
    sizer->Add(stext0, 0, wxEXPAND | wxALL, 5);
    sizer->Add(stext1, 1, wxEXPAND | wxALL, 5);
    sizer->Add(button, 0, wxALIGN_RIGHT | wxALL, 5);
    SetSizer(sizer);
    sizer->SetSizeHints(this);
    sizer->Fit(this);

    delete info;
  }
  
  void OnOK(wxCommandEvent &event)
  {
    Close();
  }
private:
  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(VolumeInfoDialog, wxDialog)
  EVT_BUTTON(VOLUME_INFO_OK_BUTTON, VolumeInfoDialog::OnOK)
END_EVENT_TABLE()

void
Seg3DFrame::OnVolumeInfoPanel( wxCommandEvent& event )
{
  VolumeInfoPanelStruct *info = (VolumeInfoPanelStruct *)event.GetClientData();
  VolumeInfoDialog *dialog = new VolumeInfoDialog(this, info);
  dialog->Show(true);
}


} // namespace SCIRun

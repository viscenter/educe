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
#include <StandAlone/Apps/Seg3D/GuiCode/resampletool.h>
#include <StandAlone/Apps/Seg3D/GuiCode/medianfiltertool.h>
#include <StandAlone/Apps/Seg3D/GuiCode/optionlessfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkthresholdfilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/itkDiscreteGaussianImageFilter.h>
#include <StandAlone/Apps/Seg3D/GuiCode/maskfilter.h>


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

  EVT_MENU(MENU_TOOL_AUTOVIEW, Seg3DFrame::ToolAutoview)
  EVT_MENU(MENU_TOOL_PAINT_BRUSH, Seg3DFrame::ToolPaintBrush)
  EVT_MENU(MENU_TOOL_CROP_VOLUME, Seg3DFrame::ToolCropVolume)
  EVT_MENU(MENU_TOOL_CROP_CYLINDER, Seg3DFrame::ToolCropCylinder)
  EVT_MENU(MENU_TOOL_ISOSURFACE, Seg3DFrame::ToolIsosurface)
  EVT_MENU(MENU_TOOL_SET_VRTARGET, Seg3DFrame::ToolSetVRTarget)
  EVT_MENU(MENU_TOOL_FLIP, Seg3DFrame::ToolFlip)
  EVT_MENU(MENU_TOOL_RESAMPLE, Seg3DFrame::ToolResample)
  EVT_MENU(MENU_TOOL_RESET_CLUT, Seg3DFrame::ToolResetCLUT)
  EVT_MENU(MENU_TOOL_SET_MASK_LAYER, Seg3DFrame::ToolSetMaskLayer)
  EVT_MENU(MENU_TOOL_CLEAR_MASK_LAYER, Seg3DFrame::ToolClearMaskLayer)

  EVT_MENU(MENU_FILTER_C_A_D_F, Seg3DFrame::Filter_CADF)
  EVT_MENU(MENU_FILTER_C_C_F, Seg3DFrame::Filter_CCF)
  EVT_MENU(MENU_FILTER_N_C_F, Seg3DFrame::Filter_NCF)
  EVT_MENU(MENU_FILTER_B_D_E_F, Seg3DFrame::Filter_BDEF)
  EVT_MENU(MENU_FILTER_T_S_L_S_F, Seg3DFrame::Filter_TSLSF)
  EVT_MENU(MENU_FILTER_OTSU_T_F, Seg3DFrame::Filter_OTSUTF)
  EVT_MENU(MENU_FILTER_G_M_F, Seg3DFrame::Filter_GMF)
  EVT_MENU(MENU_FILTER_LABEL_EXTRACT, Seg3DFrame::Filter_LabelExtract)
  EVT_MENU(MENU_FILTER_FILL_HOLE, Seg3DFrame::Filter_FillHole)
  EVT_MENU(MENU_FILTER_FLOOD_FILL_COPY, Seg3DFrame::Filter_FloodFillCopy)
  EVT_MENU(MENU_FILTER_MEDIAN_FILTER, Seg3DFrame::Filter_MedianFilter)
  EVT_MENU(MENU_FILTER_THRESHOLD_FILTER, Seg3DFrame::Filter_ThresholdFilter)
  EVT_MENU(MENU_FILTER_DISCRETE_GAUSSIAN_FILTER, Seg3DFrame::Filter_DiscreteGaussianFilter)
  EVT_MENU(MENU_FILTER_HISTO_EQ, Seg3DFrame::Filter_HistoEq)
  EVT_MENU(MENU_FILTER_MASK_DATA, Seg3DFrame::Filter_MaskData)
  EVT_MENU(MENU_FILTER_MASK_LABEL, Seg3DFrame::Filter_MaskLabel)
  EVT_MENU(MENU_FILTER_LABEL_INVERT_FILTER, Seg3DFrame::Filter_LabelInvertFilter)
  EVT_MENU(MENU_FILTER_MASK_AND, Seg3DFrame::Filter_MaskAnd)
  EVT_MENU(MENU_FILTER_MASK_OR, Seg3DFrame::Filter_MaskOr)

  EVT_MENU(MENU_HELP_UNDO, Seg3DFrame::Undo)
  EVT_MENU(MENU_HELP_INDEX, Seg3DFrame::Index)
  EVT_MENU(MENU_HELP_ABOUT, Seg3DFrame::About)
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
  toolsPanel_ = scinew wxPanel(this, wxID_ANY, wxDefaultPosition, panel_size, style);
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

  cursorInformation_ = scinew ::CursorInformation(this, wxID_ANY, wxDefaultPosition, panel_size);
  left_sizer->Add(cursorInformation_, 0, wxEXPAND, 0);

  brushPanel_ = scinew BrushPanel(toolsPanel_);
  tools_sizer->Add(brushPanel_, 0, 0, 0);
  tools_sizer->Show(brushPanel_, false);
  tools_sizer->Layout();

  // new wx crop volume
  cropVolume_ = scinew CropVolume(toolsPanel_);
  tools_sizer->Add(cropVolume_, 0, 0, 0);
  tools_sizer->Show(cropVolume_, false);
  tools_sizer->Layout();
  
	// new wx crop cylinder
  cropCylinder_ = scinew CropVolCylinder(toolsPanel_);
  tools_sizer->Add(cropCylinder_, 0, 0, 0);
  tools_sizer->Show(cropCylinder_, false);
  tools_sizer->Layout();
  
	// new wx median filter volume
  medianFilterTool_ = scinew MedianFilterTool(toolsPanel_);
  tools_sizer->Add(medianFilterTool_, 0, 0, 0);
  tools_sizer->Show(medianFilterTool_, false);
  tools_sizer->Layout();

  // new wx flip volume
  flipTools_ = scinew FlipTool(toolsPanel_);
  tools_sizer->Add(flipTools_, 0, 0, 0);
  tools_sizer->Show(flipTools_, false);
  tools_sizer->Layout();

  // new wx resample volume
  resampleTool_ = scinew ResampleTool(toolsPanel_);
  tools_sizer->Add(resampleTool_, 0, 0, 0);
  tools_sizer->Show(resampleTool_, false);
  tools_sizer->Layout();

  // new wx ITK test
  itk_CADF_ = scinew ITKCurvatureAnistopricDiffusionFilter(toolsPanel_);
  tools_sizer->Add(itk_CADF_, 0, 0, 0);
  tools_sizer->Show(itk_CADF_, false);
  tools_sizer->Layout();

  itk_CCF_ = scinew ITKConfidenceConnectedFilter(toolsPanel_);
  tools_sizer->Add(itk_CCF_, 0, 0, 0);
  tools_sizer->Show(itk_CCF_, false);
  tools_sizer->Layout();  

  itk_NCF_ = scinew ITKNeighbourhoodConnectedFilter(toolsPanel_);
  tools_sizer->Add(itk_NCF_, 0, 0, 0);
  tools_sizer->Show(itk_NCF_, false);
  tools_sizer->Layout();

  itk_BDEF_ = scinew ITKBinaryDilateErodeFilter(toolsPanel_);
  tools_sizer->Add(itk_BDEF_, 0, 0, 0);
  tools_sizer->Show(itk_BDEF_, false);
  tools_sizer->Layout();

  itk_TSLSF_ = scinew ITKThresholdSegmentationLevelSetFilter(toolsPanel_);
  tools_sizer->Add(itk_TSLSF_, 0, 0, 0);
  tools_sizer->Show(itk_TSLSF_, false);
  tools_sizer->Layout();

  optionless_ = new OptionlessFilter(toolsPanel_);
  tools_sizer->Add(optionless_, 0, 0, 0);
  tools_sizer->Show(optionless_, false);
  tools_sizer->Layout();

  itk_thresholdfilter_ = new ITKThresholdFilter(toolsPanel_);
  tools_sizer->Add(itk_thresholdfilter_, 0, 0, 0);
  tools_sizer->Show(itk_thresholdfilter_, false);
  tools_sizer->Layout();

  itk_discretegaussianfilter_ = new itkDiscreteGaussianImageFilter(toolsPanel_);
  tools_sizer->Add(itk_discretegaussianfilter_, 0, 0, 0);
  tools_sizer->Show(itk_discretegaussianfilter_, false);
  tools_sizer->Layout();

  maskfilter_ = new MaskFilter(toolsPanel_);
  tools_sizer->Add(maskfilter_, 0, 0, 0);
  tools_sizer->Show(maskfilter_, false);
  tools_sizer->Layout();

  context_ =
    scinew WXOpenGLContext(target, this, wxID_ANY,
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
  wxMenu * winMenu = new wxMenu;
  wxMenuBar * menuBar = new wxMenuBar;

  CurrentDocPath = wxEmptyString;

  //#if wxUSE_FILEDG
  // File menu dialog
  winMenu->Append( MENU_FILE_LOAD_VOLUME, _T("Open &Volume") );
  winMenu->Append( MENU_FILE_LOAD_SESSION, _T("&Open Session") );
  winMenu->Append( MENU_FILE_IMPORT_SEGMENTATION, _T("&Import Segmentation") );
  winMenu->Append( MENU_FILE_SAVE_VOLUME, _T("Save Volume") );
  winMenu->Append( MENU_FILE_SAVE_SESSION, _T("&Save Session") );
  winMenu->Append( MENU_FILE_EXPORT_SEGMENTATION, _T("&Export Segmentation") );
  winMenu->Append( MENU_FILE_QUIT, _T("&Quit") );
  menuBar->Append( winMenu, _T("&File") );
  //#endif

  // View menu dialog
  winMenu = new wxMenu;
  winMenu->Append(MENU_VIEW_TWO_BY_TWO, _T("&Two by Two"));
  winMenu->Append(MENU_VIEW_ONE_BY_THREE, _T("&One and Three") );
  winMenu->Append(MENU_VIEW_SINGLE, _T("&Single") );
  winMenu->Append(MENU_VIEW_VOLUME_RENDER, _T("&Volume Rendering") );
  menuBar->Append(winMenu, _T("&Views"));

  // Tools menu dialog
  winMenu = new wxMenu;
  winMenu->Append(MENU_TOOL_AUTOVIEW, _T("&Autoview All"));
  winMenu->Append(MENU_TOOL_PAINT_BRUSH, _T("&Paint Brush"));
  winMenu->Append(MENU_TOOL_CROP_VOLUME, _T("&Crop Tool"));
  winMenu->Append(MENU_TOOL_CROP_CYLINDER, _T("Crop C&ylinder"));
  winMenu->Append(MENU_TOOL_ISOSURFACE, _T("&Isosurface Labels"));
  winMenu->Append(MENU_TOOL_SET_VRTARGET, _T("Set Volume &Rendering Target"));
  winMenu->Append(MENU_TOOL_FLIP, _T("&Flip Tool"));
  winMenu->Append(MENU_TOOL_RESAMPLE, _T("Resample Tool"));
  winMenu->Append(MENU_TOOL_RESET_CLUT, _T("Reset Brightness/Contrast"));
  winMenu->Append(MENU_TOOL_SET_MASK_LAYER, _T("Set Mask Label"));
  winMenu->Append(MENU_TOOL_CLEAR_MASK_LAYER, _T("Clear Mask Label"));
  menuBar->Append(winMenu, _T("&Tools"));

  // ITK menu dialog
  winMenu = new wxMenu;

	// Plugins
	
	// winMenu->Append(MENU_PLUGIN_START, _T("Test"));
	plugin_manager_->LoadPlugins(winMenu,PLUGIN_FILTER,(wxFrame*)this);

  winMenu->AppendSeparator();
  
	// Data enhancers.
  winMenu->Append(MENU_FILTER_C_A_D_F, _T("Curvature &Anisotropic Diffusion Filter"));
  winMenu->Append(MENU_FILTER_MEDIAN_FILTER, _T("Median Filter"));
  winMenu->Append(MENU_FILTER_DISCRETE_GAUSSIAN_FILTER, _T("Discrete Gaussian Filter"));
  winMenu->Append(MENU_FILTER_G_M_F, _T("&Gradient Magnitude Filter"));
  winMenu->Append(MENU_FILTER_HISTO_EQ, _T("Histogram Equalization"));
  winMenu->Append(MENU_FILTER_MASK_DATA, _T("Mask Data"));

  winMenu->AppendSeparator();

  // Segmenters
  winMenu->Append(MENU_FILTER_C_C_F, _T("&Confidence Connected Filter"));
  winMenu->Append(MENU_FILTER_N_C_F, _T("&Neighborhood Connected Filter"));
  winMenu->Append(MENU_FILTER_OTSU_T_F, _T("&Otsu Threshold Filter"));
  winMenu->Append(MENU_FILTER_THRESHOLD_FILTER, _T("Threshold Filter"));
  //winMenu->Append(MENU_FILTER_T_S_L_S_F, _T("&Threshold Segmentation Level Set Filter"));

  winMenu->AppendSeparator();

  // Label manipulators.
  winMenu->Append(MENU_FILTER_LABEL_EXTRACT, _T("Copy Label Filter"));
  winMenu->Append(MENU_FILTER_LABEL_INVERT_FILTER, _T("&Label Invert Filter"));
  winMenu->Append(MENU_FILTER_MASK_AND, _T("Combine Labels with Logical And"));
  winMenu->Append(MENU_FILTER_MASK_OR, _T("Combine Labels with Logical Or"));
  winMenu->Append(MENU_FILTER_B_D_E_F, _T("&Binary Dilate -> Erode Filter"));
  winMenu->Append(MENU_FILTER_FLOOD_FILL_COPY, _T("Flood Fill Copy Filter"));
  winMenu->Append(MENU_FILTER_FILL_HOLE, _T("Fill Holes Filter"));

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
  colour_picker_data *cpd = (colour_picker_data *)event.GetClientData();
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
                         _T("Delete layer"),
                         wxYES_NO | wxICON_ERROR);
  if (dialog.ShowModal() == wxID_YES)
  {
    SCIRun::Painter::ThrowSkinnerSignal("Painter::DeleteLayer2");
  }
  else
  {
    SetStatusText("Delete layer canceled by user.");
  }    
}

void
Seg3DFrame::OnUnwrapWindow(wxCommandEvent &event)
{
	UnwrapPluginWindow * unwrap_win = new UnwrapPluginWindow(_T("Unwrapped View"), Painter::global_seg3dframe_pointer_,
		wxDefaultPosition, wxDefaultSize);
	unwrap_win->scroll->set_image((Unwrapping*)event.GetClientData());
	unwrap_win->Show(true);
}


DEFINE_EVENT_TYPE(wxEVT_COMMAND_STATUS_BAR_TEXT_CHANGE)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_COLOUR_PICKER)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_HIDE_TOOL)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_OK_DIALOG)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_LAYER_DELETE_DIALOG)
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

void
Seg3DFrame::FileSaveVolume( wxCommandEvent& WXUNUSED(event) )
{
  static const wxChar *formats = _T("");

  wxFileDialog dialog(this, _T("Save Volume"), CurrentDocPath,
                      _T("sample_volume.nrrd"), 
                      formats, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

  dialog.SetFilterIndex(1);

  if (dialog.ShowModal() == wxID_OK)
  {
    const string filename = wx2std(dialog.GetPath(), &wxConvFile);

    SetStatusText(std2wx("Saving " + filename));
    wxBusyCursor cursor; // Busy cursor until this leaves scope.

    SCIRun::ThrowSkinnerSignalEvent *tsse =
      new SCIRun::ThrowSkinnerSignalEvent("Painter::SaveVolume");
    tsse->add_var("Painter::SaveVolume::filename", filename);
    SCIRun::Painter::ThrowSkinnerSignal(tsse);

    // TODO Filters broken on Unicode
    //wxLogMessage(_T("%s, filter %d"), filename.c_str(),
    //dialog.GetFilterIndex());
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
  wxFileDialog dialog(this, _T("Save Session"), CurrentDocPath, _T("sample_session.ses"),
                      _T("Session files (*.ses)|*.ses"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

  dialog.SetFilterIndex(1);

  if (dialog.ShowModal() == wxID_OK)
  {
    const string filename = wx2std(dialog.GetPath(), &wxConvFile);

    SetStatusText(std2wx("Saving " + filename));
    wxBusyCursor cursor; // Busy cursor until this leaves scope.

    SCIRun::ThrowSkinnerSignalEvent *tsse =
      new SCIRun::ThrowSkinnerSignalEvent("Painter::SaveSession");
    tsse->add_var("Painter::SaveSession::filename", filename);
    SCIRun::Painter::ThrowSkinnerSignal(tsse);

    // TODO Filters broken on Unicode
    //wxLogMessage(_T("%s, filter %d"), filename.c_str(),
    //dialog.GetFilterIndex());
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
  static const wxChar *formats = _T("");

  wxFileDialog dialog(this, _T("Export Segmentation"), CurrentDocPath,
                      _T("segmentation.nrrd"), 
                      formats, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

  dialog.SetFilterIndex(1);

  if (dialog.ShowModal() == wxID_OK)
  {
    const string filename = wx2std(dialog.GetPath(), &wxConvFile);

    SetStatusText(std2wx("Exporting " + filename));
    wxBusyCursor cursor; // Busy cursor until this leaves scope.

    SCIRun::ThrowSkinnerSignalEvent *tsse =
      new SCIRun::ThrowSkinnerSignalEvent("Painter::ExportSegmentation");
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
Seg3DFrame::ToolAutoview( wxCommandEvent& WXUNUSED(event) )
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
  ShowTool(resampleTool_, "", "Resample Tool");
}

void
Seg3DFrame::ToolIsosurface( wxCommandEvent& WXUNUSED(event) )
{
  SetStatusText(wxT("Computing label isosurfaces."));
  wxBusyCursor cursor; // Busy cursor until this leaves scope.
  Painter::ThrowSkinnerSignal("Painter::ShowIsosurface");
}


void
Seg3DFrame::ToolSetVRTarget( wxCommandEvent& WXUNUSED(event) )
{
  SetStatusText(wxT("Setting new volume rendering target."));
  wxBusyCursor(); // Busy cursor until this leaves scope.
  Painter::ThrowSkinnerSignal("Painter::ShowVolumeRendering");
}

void
Seg3DFrame::ToolResetCLUT( wxCommandEvent& WXUNUSED(event) )
{
  Painter::ThrowSkinnerSignal("Painter::ResetCLUT");
  SetStatusText("Brightness and contrast reset for the active layer.");
}


void
Seg3DFrame::ToolSetMaskLayer( wxCommandEvent& WXUNUSED(event) )
{
  Painter::ThrowSkinnerSignal("Painter::SetMaskLayer");
  SetStatusText("Set the mask layer.");
}


void
Seg3DFrame::ToolClearMaskLayer( wxCommandEvent& WXUNUSED(event) )
{
  Painter::ThrowSkinnerSignal("Painter::ClearMaskLayer");
  SetStatusText("Cleared the mask layer.");
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
  ShowTool(optionless_, "Painter::ITKHoleFillFilter", "Fill Holes Filter");
  optionless_->SetShowProgress(true);
}


void
Seg3DFrame::Filter_FloodFillCopy( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(optionless_, "Painter::FloodFillCopyFilter", "Flood Fill Copy Filter");
  optionless_->SetShowProgress(true);
}


void
Seg3DFrame::Filter_MedianFilter( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(medianFilterTool_, "", "Median Filter");
}

void
Seg3DFrame::Filter_ThresholdFilter( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_thresholdfilter_, "Painter::start_ITKThresholdImageFilterTool", "Threshold Filter");
}

void
Seg3DFrame::Filter_DiscreteGaussianFilter( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(itk_discretegaussianfilter_, "Painter::start_ITKDiscreteGaussianImageFilterTool", "Discrete Gaussian Filter");
}


void
Seg3DFrame::Filter_HistoEq( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(optionless_, "", "Histogram Equalization");
  optionless_->SetSkinnerCallback("Painter::HistoEqFilter");
  optionless_->SetShowProgress(false);
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
Seg3DFrame::Filter_MaskOr( wxCommandEvent& WXUNUSED(event) )
{
  ShowTool(maskfilter_, "", "Combine Labels with", "Logical Or");
  maskfilter_->SetSkinnerCallback("Painter::CombineMaskOr");
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
  info.AddDeveloper(_T("Michael Callahan"));
  info.AddDeveloper(_T("Joshua Cates"));
  info.AddDeveloper(_T("Marty Cole"));
  info.AddDeveloper(_T("McKay Davis"));
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


} // namespace SCIRun

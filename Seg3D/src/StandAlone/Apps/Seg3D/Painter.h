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
//    File   : Painter.h
//    Author : McKay Davis
//    Date   : November 2005

#ifndef StandAlone_Apps_Painter_Painter_h
#define StandAlone_Apps_Painter_Painter_h

#include <sci_comp_warn_fixes.h>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <algorithm>
#include <typeinfo>
#include <iostream>
#include <sci_gl.h>

#include <StandAlone/Apps/Seg3D/VolumeSlice.h>
#include <StandAlone/Apps/Seg3D/NrrdVolume.h>
#include <StandAlone/Apps/Seg3D/SliceWindow.h>
#include <StandAlone/Apps/Seg3D/LayerButton.h>
#include <StandAlone/Apps/Seg3D/UIvar.h>
#include <StandAlone/Apps/Seg3D/Undo.h>

#include <Core/Containers/StringUtil.h>
#include <Core/Geom/GeomGroup.h>

#include <Core/Math/MiscMath.h>
#include <Core/Math/MinMax.h>
#include <Core/Thread/Runnable.h>
#include <Core/Thread/Mutex.h>
#include <Core/Thread/ThreadLock.h>
#include <Core/Util/Timer.h>
#include <Core/Util/Environment.h>
#include <Core/Util/FileUtils.h>
#include <Core/Skinner/Parent.h>
#include <Core/Skinner/Variables.h>
#include <Core/Events/BaseTool.h>
#include <Core/Events/ToolManager.h>

#include <sci_defs/insight_defs.h>
#include <Core/Datatypes/ITKDatatype.h>
#include <Core/Datatypes/NrrdToITK.h>
#include <itkImageFileReader.h>
#include <itkImageSeriesReader.h>

#ifdef _WIN32
#  undef min
#  undef max
#endif 


#include <StandAlone/Apps/Seg3D/share.h>

namespace SCIRun {


using SCIRun::ITKDatatypeHandle;
typedef itk::Image<float, 3> ITKImageFloat3D;
typedef itk::Image<label_type, 3> ITKImageLabel3D;

typedef itk::Image<float, 2> ITKImageFloat2D;
typedef itk::Image<label_type, 2> ITKImageLabel2D;


class VolumeFilterBase;
class Seg3DFrame;

class ThrowSkinnerSignalEvent : public BaseEvent
{
  string name_;
  Semaphore semaphore_;
  vector<string> vars_;
public:
  ThrowSkinnerSignalEvent(const string &name) : name_(name), semaphore_("ThrowSkinnerSignalEvent", 0) {}
  virtual BaseEvent *clone() { return new ThrowSkinnerSignalEvent(name_); }
  virtual ~ThrowSkinnerSignalEvent() {}

  const string &get_name() { return name_; }
  void down() { semaphore_.down(); }
  void up() { semaphore_.up(); }
  void add_var(const string &var, const string &val)
  {
    vars_.push_back(var);
    vars_.push_back(val);
  }
  void set_vars(Skinner::Variables *svars)
  {
    for (size_t i = 0; i < vars_.size(); i+=2)
    {
      if (svars->exists(vars_[i+0]))
      {
        svars->set_by_string(vars_[i+0], vars_[i+1]);
      }
      else
      {
        svars->insert(vars_[i+0], vars_[i+1]);
      }
    }
  }
};


class Painter : public Skinner::Parent
{
public:
  Painter(Skinner::Variables *, VarContext* ctx);
  virtual ~Painter();

  static Skinner::DrawableMakerFunc_t   maker;
  static string                         class_name() { return "Painter"; }
  void                                  redraw_all();
  void                                  extract_all_window_slices();

  static Seg3DFrame *global_seg3dframe_pointer_;

  enum DisplayMode_e {
    normal_e,
    slab_e,
    mip_e,
    num_display_modes_e
  };

  // Methods for drawing to the GL window

  void                  set_probe();

  // these should probably be moved to NrrdVolume class
  vector<string>        get_filename_series(string reference_filename);
  template <class T>
  NrrdVolumeHandle      load_image_series(const vector<string> &filenames);
  template <class T>
  NrrdVolumeHandle      load_volume(string filename);
  template <class T>
  NrrdVolumeHandle      load_volume(string filename, int &status);
  bool                  save_volume(string filename, NrrdVolumeHandle &);
  
  void                  set_all_slices_tex_dirty();
  void                  rebuild_layer_buttons();
  void                  get_data_from_layer_buttons();

  void                  build_layer_button(unsigned int &, NrrdVolumeHandle &);
  void                  build_volume_list(NrrdVolumes &vols);

  void                  current_layer_up();
  void                  current_layer_down();

  void                  move_layer_up();
  void                  move_layer_down();

  void                  opacity_up();
  void                  opacity_down();
  NrrdVolumeHandle      find_volume_by_name(const string &, NrrdVolumeHandle parent=0);

  int                   isosurface_label_volumes(NrrdVolumes &vols);
  string		unique_layer_name(string);
  string		unique_layer_name_from_filename(string);

  NrrdVolumeHandle      copy_current_layer(string suffix = "");
  NrrdVolumeHandle      make_layer(string suffix,
                                   NrrdDataHandle &,
                                   unsigned int mask=0);

  template <class FilterType>
  NrrdDataHandle        do_itk_filter (FilterType *, NrrdDataHandle &);

  template <class FilterType>
  void                  filter_callback(itk::Object *, 
                                        const itk::EventObject &);

  void                  clear_all_volumes();
  size_t                remove_volume(NrrdVolumeHandle &volume, bool undo);
  void                  insert_volume(NrrdVolumeHandle &volume, size_t index);
  void                  push_undo(UndoHandle &undo);

  typedef set<VolumeFilterBase *> Filters_t;
  Filters_t filters_;

  double                scene_scale();

private:
  BaseTool::propagation_state_e LoadVolume(const string &filename);
  BaseTool::propagation_state_e LoadSession(const string &filename);
  BaseTool::propagation_state_e SaveSession(const string &filename);
  BaseTool::propagation_state_e SaveVolume(const string &filename);

  CatcherFunction_t     PluginFilter;
  
	CatcherFunction_t     UnwrapProbe;
  
	CatcherFunction_t     ITKOtsuFilter;
  CatcherFunction_t     ITKHoleFillFilter;
  CatcherFunction_t     FloodFillCopyFilter;
  
  CatcherFunction_t     InitializeSignalCatcherTargets;
  CatcherFunction_t     SliceWindow_Maker;
  CatcherFunction_t     LayerButton_Maker;

  CatcherFunction_t     StartBrushTool;
  CatcherFunction_t     StartCropTool;
  CatcherFunction_t     StartCropCylinder;
  CatcherFunction_t     StartPolylineTool;
  CatcherFunction_t     StartResampleTool;
  CatcherFunction_t     StartThresholdTool;
  CatcherFunction_t     StartMeasurementTool;

  CatcherFunction_t     VolumeInformation;

public:
  CatcherFunction_t     Autoview;
private:
  CatcherFunction_t     CopyLabel;
  CatcherFunction_t     DeleteLayer;
  CatcherFunction_t     DeleteLayer2;
  CatcherFunction_t     MoveLayerUp;
  CatcherFunction_t     MoveLayerDown;

  CatcherFunction_t     FinishTool;
  CatcherFunction_t     ClearTools;
  CatcherFunction_t     SetLayer;
  CatcherFunction_t     SetDataLayer;

  CatcherFunction_t     FlipVolume;
  CatcherFunction_t     HistoEqFilter;

  CatcherFunction_t     ITKBinaryDilate;
  CatcherFunction_t     ITKBinaryErode;

  CatcherFunction_t     ITKImageFileWrite;
  CatcherFunction_t     ITKGradientMagnitude;
  CatcherFunction_t     InhomogeneityCorrectionFilter;
  CatcherFunction_t     ITKBinaryDilateErode;
  CatcherFunction_t     ITKCurvatureAnisotropic;
  CatcherFunction_t     ITKBinaryDilateErodeImageFilter;
  CatcherFunction_t     start_ITKConfidenceConnectedImageFilterTool;
  CatcherFunction_t     start_ITKNeighborhoodConnectedImageFilterTool;
  CatcherFunction_t     start_ITKThresholdSegmentationLevelSetImageFilterTool;

  CatcherFunction_t     start_ITKDiscreteGaussianImageFilterTool;

  CatcherFunction_t     start_ITKSpeedToPathGradientDescentImageFilterTool;
  CatcherFunction_t     start_ITKSpeedToPathRegularStepGradientDescentImageFilterTool;
  CatcherFunction_t     start_ITKSpeedToPathIterateNeighborhoodImageFilterTool;

  CatcherFunction_t     ShowVolumeRendering;
public:
  CatcherFunction_t     ShowVolumeRendering2;
private:
  CatcherFunction_t     ShowOneIsosurface;
  CatcherFunction_t     ShowAllIsosurfaces;
  CatcherFunction_t     AbortFilterOn;

  CatcherFunction_t     LoadVolume;
  CatcherFunction_t     SaveVolume;
  CatcherFunction_t     LoadSession;
  CatcherFunction_t     SaveSession;
  CatcherFunction_t     ImportSegmentation;
  CatcherFunction_t     ExportSegmentation1;
  CatcherFunction_t     ExportSegmentation2;

  CatcherFunction_t     Undo;

  CatcherFunction_t     MedianFilterVolume;

  CatcherFunction_t     CreateLabelVolume;

  CatcherFunction_t     RebuildLayers;

  CatcherFunction_t     FloodFillLayer;
  CatcherFunction_t     FloodFillClear;
  
  CatcherFunction_t     RasterPolyline;
  
  CatcherFunction_t     ResetCLUT;
  CatcherFunction_t     UpdateBrushRadius;
  CatcherFunction_t     UpdateThresholdTool;

  CatcherFunction_t     OpenLabelColorDialog;
  CatcherFunction_t     SetLabelColor;

  CatcherFunction_t     LabelInvertFilter;
  CatcherFunction_t     SetMaskLayer;
  CatcherFunction_t     ClearMaskLayer;
  CatcherFunction_t     MaskDataFilter;
  CatcherFunction_t     MaskLabelFilter;
  CatcherFunction_t     CombineMaskAnd;
  CatcherFunction_t     CombineMaskRemove;
  CatcherFunction_t     CombineMaskOr;
  CatcherFunction_t     CombineMaskXor;

  CatcherFunction_t     MoveScaleLayerUpdateGUI;
  CatcherFunction_t     MoveScaleLayer;
  CatcherFunction_t     MoveScaleAllLayers;
  
  CatcherFunction_t     ShowVisibleItem;
  CatcherFunction_t     RedrawAll;

  Nrrd*                 do_CMedian_filter(Nrrd *nin, int radius);

  virtual CatcherFunction_t         process_event;
  KeyTool *             global_key_tool_;

public:
  static void ThrowSkinnerSignal(const string &name, bool sync = true);
  static void ThrowSkinnerSignal(ThrowSkinnerSignalEvent *tsse,
                                 bool sync = true);

  // Used to generate a reasonable default filename for saving.
  static string get_current_layer_name();

  static void update_progress(int progress); // 0-100
  static void start_progress();
  static void finish_progress();

  NrrdDataHandle        MatlabNrrd_reader(const string &filename);
  bool                  MatlabNrrd_writer(NrrdDataHandle nh, const char *);
  NrrdDataHandle        vff_reader(const string &filename);

  void set_pointer_position(const Point &p);
  
  SliceWindow *         cur_window_;
  ToolManager           tm_;
  Point                 pointer_pos_;
  SliceWindows		windows_;
  NrrdVolumes		volumes_;
  NrrdVolumeHandle      current_volume_;
  NrrdVolumeHandle      mask_volume_;
  NrrdVolumeHandle      current_vrender_target_;
  bool                  current_vrender_target_deferred_;
  ThreadLock            volume_lock_;

private:
  vector<LayerButton *> layer_buttons_;

public:
  bool check_for_active_data_volume(const char *msg);
  bool check_for_active_label_volume(const char *msg);
  bool check_for_valid_mask(const char *msg, bool strict = false);

  static void set_status_dialog(const string &s1, const string &s2);
  static void set_status(const string &s);
  static void set_status_safe(const string &s)
  {
    const string clean = replace_substring(s, "\n", " "); 
    set_status(clean);
  }
  
  // hide_tool_panel is called from the skinner side and throws the
  // appropriate HideTool event to the wx side.
  static void hide_tool_panel();

  void show_visible_item(const string &id, const string &group);

  NrrdVolumeHandle create_new_label(NrrdVolumeHandle &likethis,
                                    const string &name);

  // The session uses GUI vars for status, so update them before we write
  // the session.
  void set_session_appearance_vars();

  // Update the appearance from the session vars after reading.
  void get_session_appearance_vars(Skinner::Variables *vars);

  void toggle_current_volume_visibility();

private:
  UndoManager *undo_manager_;
};


class RedrawSliceWindowEvent : public RedrawEvent 
{
  SliceWindow &        window_;
public:
  RedrawSliceWindowEvent(SliceWindow &window) :
    RedrawEvent(),
    window_(window)
  {
  }
  
  ~RedrawSliceWindowEvent() {}
  SliceWindow &       get_window() { return window_; }
};


class UpdateBrushRadiusEvent : public BaseEvent
{
  double radius_;
  bool force_erasing_;

public:
  UpdateBrushRadiusEvent(double r, bool fe)
    : BaseEvent(), radius_(r), force_erasing_(fe) {}
  virtual BaseEvent *clone()
  { return new UpdateBrushRadiusEvent(radius_, force_erasing_); }
  virtual ~UpdateBrushRadiusEvent() {}

  double get_radius() { return radius_; }
  bool get_force_erasing() { return force_erasing_; }
};


class UpdateThresholdToolEvent : public BaseEvent
{
  double lower_;
  double upper_;

public:
  UpdateThresholdToolEvent(double lower, double upper)
    : BaseEvent(), lower_(lower), upper_(upper) {}
  virtual BaseEvent *clone()
  { return new UpdateThresholdToolEvent(lower_, upper_); }
  virtual ~UpdateThresholdToolEvent() {}

  double get_lower() { return lower_; }
  double get_upper() { return upper_; }
};



class DilateEvent : public QuitEvent 
{
public:
  DilateEvent() : QuitEvent() {}
  ~DilateEvent() {}
};

class ErodeEvent : public QuitEvent 
{
public:
  ErodeEvent() : QuitEvent() {}
  ~ErodeEvent() {}
};

class DilateErodeEvent : public QuitEvent 
{
public:
  DilateErodeEvent() : QuitEvent() {}
  ~DilateErodeEvent() {}
};

class FinishEvent : public QuitEvent 
{
public:
  FinishEvent() : QuitEvent() {}
  ~FinishEvent() {}
};



class SetLayerEvent : public RedrawEvent 
{
public:
  SetLayerEvent() : RedrawEvent() {}
  ~SetLayerEvent() {}
};



class SetDataLayerEvent : public RedrawEvent
{
public:
  SetDataLayerEvent() : RedrawEvent() {}
  ~SetDataLayerEvent() {}
};



template <class T>
unsigned int max_vector_magnitude_index(vector<T> array) {
  if (array.empty()) return 0;
  unsigned int index = 0;
  for (unsigned int i = 1; i < array.size(); ++i) 
    if (fabs(array[i]) > fabs(array[index]))
      index = i;
  return index;
}



template <class T>
NrrdVolumeHandle
Painter::load_image_series(const vector<string> &files)
{
  SCIRun::ITKDatatype *img = new SCIRun::ITKDatatype();
  NrrdDataHandle nrrd_handle = 0;

  typedef itk::ImageSeriesReader <itk::Image<T, 3> > FileReaderType;
  typename FileReaderType::Pointer reader = FileReaderType::New();
  reader->SetFileNames(files);
  
  try {
    reader->Update();  
  } catch  ( itk::ExceptionObject & err ) {
    cerr << "Painter::read_volume - ITK ExceptionObject caught!" << std::endl;
    cerr << err.GetDescription() << std::endl;
    set_status_safe(err.GetDescription());
    return 0;
  }
    
  img->data_ = reader->GetOutput();
    
  if (img->data_)
  {
    ITKDatatypeHandle img_handle = img;
    nrrd_handle = itk_image_to_nrrd<T, 3>(img_handle);
  }
  else
  {
    cerr << "no itk image\n";
    return 0;
  }

  if (!nrrd_handle.get_rep() || !nrrd_handle->nrrd_) return 0;

  pair<string, string> dir_file = split_filename(files[0]);
  if (dir_file.second.empty()) {
    dir_file.second = split_filename(dir_file.first).second;
  }
  const string name = unique_layer_name_from_filename(dir_file.second);
  NrrdVolume *vol = new NrrdVolume(this, name, nrrd_handle);
  vol->filename_ = dir_file.second;
  vol->full_path_ = files[0];

  return vol;
}


template <class T>
NrrdVolumeHandle
Painter::load_volume(string filename)
{
  int status;
  return load_volume<T>(filename, status);
}


template <class T>
NrrdVolumeHandle
Painter::load_volume(string filename, int &status)
{
  // Error status
  status = 1; // 1 general failure, 2 bad format, 3 invalid file.

  filename = substituteTilde(filename);
  convertToUnixPath(filename);
  
  SCIRun::ITKDatatype *img = new SCIRun::ITKDatatype();
  NrrdDataHandle nrrd_handle = 0;

  if (validFile(filename))
  {
    if (ends_with(string_tolower(filename),".mat"))
    {
      nrrd_handle = MatlabNrrd_reader(filename);
    }
    else if (ends_with(string_tolower(filename),".vff"))
    {
      nrrd_handle = vff_reader(filename);
    }
    else
    {
      // Create a new reader.
      typedef itk::ImageFileReader <itk::Image<T, 3> > FileReaderType;
      typename FileReaderType::Pointer reader = FileReaderType::New();
      
      reader->SetFileName(filename.c_str());
      
      try {
        reader->Update();  
        img->data_ = reader->GetOutput();
      }
      catch  ( itk::ExceptionObject & err )
      {
        cerr << "Painter::read_volume - ITK ExceptionObject caught!" << std::endl;
        const string msg = err.GetDescription();
        cerr << msg << std::endl;
        set_status_safe(msg);
        if (!strncmp(msg.c_str(), " Could not create IO object for file", 36))
        {
          status = 2;
        }
        return 0;
      }
      
      if (!img->data_)
      {
        cerr << "no itk image\n";
        return 0;
      }
    }
  }
  else
  {
    status = 3;
    cerr << "Filename not valid.\n";
  }
  
  if (img->data_)
  {
    ITKDatatypeHandle img_handle = img;
    nrrd_handle = itk_image_to_nrrd<T, 3>(img_handle);
  }

  if (!nrrd_handle.get_rep() || !nrrd_handle->nrrd_) return 0;

  pair<string, string> dir_file = split_filename(filename);
  if (dir_file.second.empty()) {
    dir_file.second = split_filename(dir_file.first).second;
  }
  const string name = unique_layer_name_from_filename(dir_file.second);
  NrrdVolume *vol = new NrrdVolume(this, name, nrrd_handle);
  vol->filename_ = dir_file.second;
  vol->full_path_ = filename;

  status = 0;
  return vol;
}


}
#endif

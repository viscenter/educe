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
 *  OpenGL.cc: Render geometry using opengl
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   December 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#include <sci_gl.h>
#include <sci_glx.h>
#include <sci_values.h>

#include <sci_defs/bits_defs.h>
#include <sci_defs/image_defs.h>

#include <teem/nrrd.h>
#include <png.h>

#include <Dataflow/Modules/Render/OpenGL.h>
#include <Core/Containers/StringUtil.h>
#include <Dataflow/GuiInterface/TCLTask.h>
#include <Core/Util/Environment.h>
#include <Core/Util/FileUtils.h>
#include <Core/Geom/GeomViewerItem.h>
#include <Core/Geom/GeomQuads.h>

#include <sgi_stl_warnings_off.h>
#include   <iostream>
#include <sgi_stl_warnings_on.h>

#ifdef _WIN32
#  include <Core/Thread/Time.h>
#  undef near
#  undef far
#  undef min
#  undef max
#  ifndef BUILD_SCIRUN_STATIC
#    define SCISHARE __declspec(dllimport)
#  else
#    define SCISHARE
#  endif
#else
#  define SCISHARE
#endif

extern "C" SCISHARE Tcl_Interp* the_interp;

namespace SCIRun {

#define DO_REDRAW     0
#define DO_PICK       1
#define DO_GETDATA    2
#define REDRAW_DONE   4
#define PICK_DONE     5
#define DO_IMAGE      6
#define IMAGE_DONE    7
#define DO_SYNC_FRAME 8


int CAPTURE_Z_DATA_HACK = 0;

static const int pick_buffer_size = 512;
static const double pick_window = 10.0;


OpenGL::OpenGL(GuiInterface* gui, ViewScene *viewer, ViewWindow *vw) :
  xres_(0),
  yres_(0),
  doing_image_p_(false),
  doing_movie_p_(false),
  make_MPEG_p_(false),
  current_movie_frame_(0),
  add_timestamp_to_movie_frame_name_(false),
  movie_name_("./movie.%04d"),
  doing_sync_frame_(false),
  dump_sync_frame_(false),
  tk_gl_context_(0),
  old_tk_gl_context_(0),
  myname_("Not Intialized"),
#ifdef __APPLE__
  apple_wait_a_second_(false),
#endif
  // private member variables
  gui_(gui),
  helper_(0),
  helper_thread_(0),
  viewer_(viewer),
  view_window_(vw),
  drawinfo_(scinew DrawInfoOpenGL),
  dead_(false),
  do_hi_res_(false),
  encoding_mpeg_(false),
  max_gl_lights_(0),
  animate_num_frames_(0),
  animate_time_begin_(0.0),
  animate_time_end_(0.0),
  animate_framerate_(0.0),
  znear_(0.0),
  zfar_(0.0),
  current_time_(0.0),
  frame_count_(1),
  total_frame_count_(1),
  cached_view_(),
  send_pick_x_(0),
  send_pick_y_(0),
  ret_pick_index_(0),
  ret_pick_obj_(0),
  ret_pick_pick_(0),
  mailbox_("OpenGL renderer send mailbox",50),
  render_buffer_id_(0),
  frame_buffer_id_(0)
{

  fps_timer_.start();
}


OpenGL::~OpenGL()
{
  // Finish up the mpeg that was in progress.
  if (encoding_mpeg_)
  {
    encoding_mpeg_ = false;
    EndMpeg();
  }
  kill_helper();
  
  RenderWindowMsgHandle rwm;
  while (mailbox_.tryReceive(rwm))
  {
    rwm->signal_done();
  }

  fps_timer_.stop();

  delete drawinfo_;
  drawinfo_ = 0;

  if (tk_gl_context_)
  {
    delete tk_gl_context_;
    tk_gl_context_ = 0;
  }

// The destructor is called from a different thread and hence crashes when
// Deleting objects here. As the context is destroyed anyway, mempory should
// be reclamed by opengl when the context is destroyed.
// Hence commenting out these calls.

//  if (render_buffer_id_)
//  {
//    gui_->lock();
//    glDeleteRenderbuffersEXT(1, &render_buffer_id_);
//    gui_->unlock();
//  }

//  if (frame_buffer_id_)
//  {
//    gui_->lock();
//    glDeleteFramebuffersEXT(1, &frame_buffer_id_);
//    gui_->unlock();
//  }

}


class OpenGLHelper : public Runnable
{
  OpenGL* opengl;
public:
  OpenGLHelper(OpenGL* opengl);
  virtual ~OpenGLHelper();
  virtual void run();
};


OpenGLHelper::OpenGLHelper(OpenGL* opengl) :
  opengl(opengl)
{
}


OpenGLHelper::~OpenGLHelper()
{

}


void
OpenGLHelper::run()
{
  Thread::allow_sgi_OpenGL_page0_sillyness();
  opengl->redraw_loop();
}


void
OpenGL::redraw(double tbeg, double tend, int nframes, double framerate)
{
  if (dead_) return;
  animate_time_begin_ = tbeg;
  animate_time_end_ = tend;
  animate_num_frames_ = nframes;
  animate_framerate_ = framerate;
  
  RenderWindowMsgHandle rwm = scinew RenderWindowMsg(DO_REDRAW);
  mailbox_.send(rwm);
  rwm->wait_signal();
}


void
OpenGL::start_helper()
{
  // This is the first redraw - if there is not an OpenGL thread,
  // start one...

#ifdef __APPLE__
  apple_wait_a_second_ = true;
#endif
  if (!helper_)
  {
    helper_ = scinew OpenGLHelper(this);
    helper_thread_ = scinew Thread(helper_,
                                   string("OpenGL: "+myname_).c_str(),
                                   0, Thread::NotActivated);
    helper_thread_->setStackSize(1024*1024);
    helper_thread_->activate(false);
  }
}


void
OpenGL::kill_helper()
{
  // kill the helper thread
  dead_ = true;
  if (helper_thread_)
  {
    RenderWindowMsgHandle rwm= scinew RenderWindowMsg(86);
    mailbox_.send(rwm);
    helper_thread_->join();
    helper_thread_ = 0;
  }
}


void
OpenGL::redraw_loop()
{
  int resx = -1;
  int resy = -1;
  string fname, ftype;
  TimeThrottle throttle;
  throttle.start();
  double newtime = 0;
  // This is the frame when inertia is started.  This is needed since
  // we need to compute the number of frames that have been rendered
  // since this initial frame.
  unsigned int inertia_frame_start = 0;
  bool do_sync_frame = false;
  for (;;)
  {
    if (reply_.size())
    {
      for (size_t j=0; j<reply_.size(); j++) reply_[j]->signal_done();
      reply_.clear();
    }
    
    view_window_->gui_inertia_mode_.reset();
    if(view_window_->gui_inertia_mode_.get())
    {
      current_time_ = throttle.time();
      if (animate_framerate_ == 0)
      {
        animate_framerate_ = 30;
      }
      double frametime = 1.0 / animate_framerate_;
      const double delta = current_time_ - newtime;
      if (delta > 1.5 * frametime)
      {
        animate_framerate_ = 1.0 / delta;
        frametime = delta;
        newtime = current_time_;
      }
      if (delta > 0.85 * frametime)
      {
        animate_framerate_ *= 0.9;
        frametime = 1.0 / animate_framerate_;
        newtime = current_time_;
      }
      else if (delta < 0.5 * frametime)
      {
        animate_framerate_ *= 1.1;
        if (animate_framerate_ > 30)
        {
          animate_framerate_ = 30;
        }
        frametime = 1.0 / animate_framerate_;
        newtime = current_time_;
      }
      newtime += frametime;
      throttle.wait_for_time(newtime);
    
      RenderWindowMsgHandle rwm;
      while (mailbox_.tryReceive(rwm))
      {
        if (rwm->message() == 86)
        {
          throttle.stop();
          // Send out replies to all threads waiting for this one
          if (reply_.size())
          {
            for (size_t j=0; j<reply_.size(); j++) reply_[j]->signal_done();
            reply_.clear();
          } 
          // Send a reply to thread that asked us to quit
          rwm->signal_done();
          return;
        }
        else if (rwm->message() == DO_PICK)
        {
          send_pick_x_ = rwm->send_pick_x();
          send_pick_y_ = rwm->send_pick_y();
          
          real_get_pick(send_pick_x_, send_pick_y_, ret_pick_obj_,
                        ret_pick_pick_, ret_pick_index_);
          rwm->ret_pick_index() = ret_pick_index_;
          rwm->ret_pick_obj() = ret_pick_obj_;
          rwm->ret_pick_pick() = ret_pick_pick_;
          
          rwm->signal_done();
        }
        else if (rwm->message() == DO_GETDATA)
        {
          int data_mask = rwm->data_mask(); 
          real_getData(data_mask, rwm->geom_data());
          rwm->signal_done();
        }
        else if (rwm->message() == DO_IMAGE)
        {
          do_hi_res_ = true;
          fname = rwm->file_name();
          ftype = rwm->file_type();
          resx = rwm->image_res_x();
          resy = rwm->image_res_y();
          rwm->signal_done();
        }
        else if (rwm->message() == DO_SYNC_FRAME)
        {
          do_sync_frame = true;
          rwm->signal_done();
        }
        else
        {
          reply_.push_back(rwm);
        }
      }

      view_window_->gui_inertia_recalculate_.reset();
      if (view_window_->gui_inertia_recalculate_.get()) 
      {
        if (view_window_->gui_inertia_recalculate_.get() == 1) 
        {
          view_window_->gui_inertia_recalculate_.set(0);
          view_window_->gui_inertia_x_.reset();
          view_window_->gui_inertia_y_.reset();
          view_window_->ball_->vDown = HVect(0.0, 0.0, 0.0, 1.0);
          view_window_->ball_->vNow = 
            HVect(view_window_->gui_inertia_x_.get()/2.0, 
          view_window_->gui_inertia_y_.get()/2.0, 0.0, 1.0);
          view_window_->ball_->dragging = 1;
          view_window_->ball_->Update();
          view_window_->ball_->qNorm = view_window_->ball_->qNow.Conj();
          const double c = 1.0/view_window_->ball_->qNow.VecMag();
          view_window_->ball_->qNorm.x *= c;
          view_window_->ball_->qNorm.y *= c;
          view_window_->ball_->qNorm.z *= c;
        }
        view_window_->gui_inertia_loop_count_.reset();
        view_window_->loop_count_ = view_window_->gui_inertia_loop_count_.get();  
        throttle.stop();
        throttle.clear();
        throttle.start();
        current_time_ = throttle.time(); 
        newtime = throttle.time()+frametime;
        inertia_frame_start = total_frame_count_;
        view_window_->gui_view_.reset();
        View tmpview(view_window_->gui_view_.get());
        view_window_->rot_view_ = tmpview;
        Vector y_axis = tmpview.up();
        Vector z_axis = tmpview.eyep() - tmpview.lookat();
        Vector x_axis = Cross(y_axis,z_axis);
        x_axis.normalize();
        y_axis.normalize();
        view_window_->eye_dist_ = z_axis.normalize();
        view_window_->prev_trans_.load_frame(x_axis,y_axis,z_axis);
      }
	

      // you want to just rotate around the current rotation
      // axis - the current quaternion is viewwindow->ball_->qNow       
      // the first 3 components of this

      // This used to use newtime with the angular velocity, but we
      // replaced angular velocity with the number of rendered frames
      // per loop.

      // This needs to compute the number of frames that have been
      // rendered from when we started inertia.  This is the 0 angle
      // in the rotation.
      double angle = ((total_frame_count_-inertia_frame_start)%view_window_->loop_count_/
                      (double)(view_window_->loop_count_)*(2*M_PI));
      view_window_->ball_->SetAngle(angle);
      View tmpview(view_window_->rot_view_);
      Transform tmp_trans;
      HMatrix mNow;
      view_window_->ball_->Value(mNow);
      tmp_trans.set(&mNow[0][0]);
      Transform prv = view_window_->prev_trans_;
      prv.post_trans(tmp_trans);
      HMatrix vmat;
      prv.get(&vmat[0][0]);
      Point y_a(vmat[0][1], vmat[1][1], vmat[2][1]);
      Point z_a(vmat[0][2], vmat[1][2], vmat[2][2]);
      tmpview.up(y_a.vector());
      if (view_window_->gui_inertia_mode_.get() == 1)
      {
        tmpview.eyep((z_a*(view_window_->eye_dist_))+tmpview.lookat().vector());
        view_window_->gui_view_.set(tmpview);
      }
      else if (view_window_->gui_inertia_mode_.get() == 2)
      {
        tmpview.lookat(tmpview.eyep()-
                       (z_a*(view_window_->eye_dist_)).vector());
        view_window_->gui_view_.set(tmpview);
      }

    }
    else
    {
      for (;;)
      {
        RenderWindowMsgHandle rwm;
        rwm = mailbox_.receive();
        if (rwm->message() == 86)
        {
          throttle.stop();
          // Send out replies to all threads waiting for this one
          if (reply_.size())
          {
            for (size_t j=0; j<reply_.size(); j++) reply_[j]->signal_done();
            reply_.clear();
          } 
          // Send a reply to thread that asked us to quit
          rwm->signal_done();
          return;
        }
        else if (rwm->message() == DO_PICK)
        {
          send_pick_x_ = rwm->send_pick_x();
          send_pick_y_ = rwm->send_pick_y();
          
          real_get_pick(send_pick_x_, send_pick_y_, ret_pick_obj_,
                        ret_pick_pick_, ret_pick_index_);
          rwm->ret_pick_index() = ret_pick_index_;
          rwm->ret_pick_obj() = ret_pick_obj_;
          rwm->ret_pick_pick() = ret_pick_pick_;
          
          rwm->signal_done();
        }
        else if (rwm->message() == DO_GETDATA)
        {
          int data_mask = rwm->data_mask(); 
          real_getData(data_mask, rwm->geom_data());
          rwm->signal_done();
        }
        else if (rwm->message() == DO_IMAGE)
        {
          do_hi_res_ = true;
          fname = rwm->file_name();
          ftype = rwm->file_type();
          resx = rwm->image_res_x();
          resy = rwm->image_res_y();
          rwm->signal_done();
        }
        else if (rwm->message() == DO_SYNC_FRAME)
        {
          do_sync_frame = true;
          rwm->signal_done();
        }
        else
        {
          reply_.push_back(rwm);
          break;
        }
      }
      
      newtime = throttle.time();
      throttle.stop();
      throttle.clear();
      throttle.start();
    }

    if (do_hi_res_)
    {
      render_and_save_image(resx, resy, fname, ftype);

      do_hi_res_ = false;
    }

#ifdef __APPLE__
    while (apple_wait_a_second_)
    {
      apple_wait_a_second_=false;
      sleep(1);
    }
#endif

    if (do_sync_frame) {
      dump_sync_frame_ = true;
      // Prevent dumping a frame on the next loop iteration.
      do_sync_frame = false;
    }

    redraw_frame();
    dump_sync_frame_ = false;
          
    if (reply_.size())
    {
      for (size_t j=0; j<reply_.size(); j++) reply_[j]->signal_done();
      reply_.clear();
    } 

    view_window_->gui_total_frames_.set(view_window_->gui_total_frames_.get()+1);
  } // end for (;;)
}


void
OpenGL::render_and_save_image( int x, int y,
                               const string & fname, const string & ftype )
                               
{
  // First figure out which type we are writing:
  // Natively we can write png, ppm, and raw

  // Determine file extension
  string fileext = fname.substr(fname.find_last_of('.')+1);

  // Copy file name so we can alter it (it is const on input)
  string filename = fname;
  string filename_convert = fname;
  
  // Convert file extension to lower case
  for (size_t p=0; p<fileext.size(); p++) fileext[p] = tolower(fileext[p]);

  bool write_nrrd = false;
  bool write_ppm = false;
  bool write_png = false;
  bool use_convert = false;
  
  if (ftype == "raw") 
  {
    // Indicate that we are writing a raw
    write_nrrd = true;
    // If extension is improper add extension
    if ((fileext != "nhdr")&&(fileext != "nrrd")) filename += string(".nhdr");
  }
  
  if (ftype == "png") 
  {
    // Indicate that we are writing a png
    write_png = true;
    // Make sure extension matches
    if (fileext != "png") filename += string(".png");
  }
  if (ftype == "ppm") 
  {
    write_ppm = true;
    // Make sure extension matches
    if (fileext != "ppm") filename += string(".ppm");
  }
  
  if (!write_nrrd && !write_ppm && !write_png)
  {
    if (fileext == "raw") 
    {
      filename += string(".nhdr");
      write_nrrd = true;
    }
    else if (fileext == "nrrd") write_nrrd = true;
    else if (fileext == "nhdr") write_nrrd = true;
    else if (fileext == "png") write_png = true;
    else if (fileext == "ppm") write_ppm = true;
    else
    {
      write_png = true;
      // Test whether image magick is available
      if (system("convert -version") != 0)
      {
        std::cout << "Unsupported image file format.\n";
        std::cout << "Put convert in your file path to support more image formats.\n";
        std::cout << "Using PNG fileformat instead.\n";
        filename += string(".png");
      }
      else
      {
        use_convert = true;
        string temp_dir = sci_getenv("SCIRUN_TMP_DIR");
        
        filename_convert = filename;
        filename = temp_dir + string("/scirun_temp_png.png");
      }
    }
  }
  
#ifndef HAVE_PBUFFER
  // Don't need to raise if using pbuffer.
  // FIXME: this next line was apparently meant to raise the Viewer to the
  //        top... but it doesn't actually seem to work
  if (tk_gl_context_)
  {
    Tk_RestackWindow(tk_gl_context_->tkwin_, Above, NULL);
  }
#endif
  
  // Do the context stuff
  gui_->lock();

  // Make sure our GL context is current
  if (tk_gl_context_ != old_tk_gl_context_)
  {
    old_tk_gl_context_ = tk_gl_context_;
    tk_gl_context_->make_current();
  }

  deriveFrustum();
  // Get Viewport dimensions
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  gui_->unlock();

  hi_res_.resx = x;
  hi_res_.resy = y;
  // The EXACT # of screen rows and columns needed to render the image
  hi_res_.ncols = (double)hi_res_.resx/(double)vp[2];
  hi_res_.nrows = (double)hi_res_.resy/(double)vp[3];

  // The # of screen rows and columns that will be rendered to make the image
  const int nrows = (int)ceil(hi_res_.nrows);
  const int ncols = (int)ceil(hi_res_.ncols);

  // Determine the type of image used
  
  int channel_bytes = 1;
  int num_channels = 3;  
  if (write_nrrd) num_channels = 4;
  int pix_size = channel_bytes * num_channels;


  // Obtain image (Render full image to memory)

  unsigned char* pixels = 0;
  unsigned char* tmp_row = 0;
  try
  {
    // Allocate enough memory to save the full image
    pixels = scinew unsigned char[hi_res_.resx*hi_res_.resy*pix_size];
    tmp_row = scinew unsigned char[hi_res_.resx*pix_size];   
  }
  catch (...)
  {
    if (pixels) delete pixels;
    if (tmp_row) delete tmp_row;
    std::cerr << "Not enough memory to generate image\n";
    throw;
  }
  
  // WORK AROUND FOR MAC, FIX ME
  // THE FIRST DRAW IS BAD. 
  // SOMEHOW WE DO NOT SET ALL STATE VARIABLES. AFTER THIS
  // CALL EVERYTHING SHOULD BE SET PROPERLY
  doing_image_p_ = true; // forces pbuffer if available
  redraw_frame();
  doing_image_p_ = false;
          
  for (hi_res_.row = nrows - 1; hi_res_.row >= 0; --hi_res_.row)
  {
    int read_height = hi_res_.resy - hi_res_.row * vp[3];
    read_height = (vp[3] < read_height) ? vp[3] : read_height;

    for (hi_res_.col = 0; hi_res_.col < ncols; hi_res_.col++)
    {
      // Render the col and row in the hi_res struct.
      doing_image_p_ = true; // forces pbuffer if available
      redraw_frame();
      doing_image_p_ = false;
      
      gui_->lock();
      // Tell OpenGL where to put the data in our pixel buffer
      // Read the data from OpenGL into our memory
      glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

      glPixelStorei(GL_PACK_ALIGNMENT,1);
      glPixelStorei(GL_PACK_SKIP_PIXELS, hi_res_.col * vp[2]);
      glPixelStorei(GL_PACK_SKIP_ROWS,0);
      glPixelStorei(GL_PACK_ROW_LENGTH, hi_res_.resx);

      int read_width = hi_res_.resx - hi_res_.col * vp[2];
      read_width = (vp[2] < read_width) ? vp[2] : read_width;
      
      glReadPixels(0,0,read_width, read_height,
		   (num_channels == 3) ? GL_RGB : GL_RGBA,
		   (channel_bytes == 1) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
		   pixels+(hi_res_.resx*(hi_res_.resy - read_height - 
					 vp[3]*hi_res_.row)*pix_size));
      gui_->unlock();
    }
    // OpenGL renders upside-down to image_file writing
    
    unsigned char *top_row, *bot_row;   
    int top, bot;
    
    for (top = read_height-1, bot = 0; bot < read_height/2; top--, bot++)
    {
      top_row = pixels+(hi_res_.resx*(hi_res_.resy - read_height - vp[3]*hi_res_.row)*pix_size) + hi_res_.resx*top*pix_size;
      bot_row = pixels+(hi_res_.resx*(hi_res_.resy - read_height - vp[3]*hi_res_.row)*pix_size) + hi_res_.resx*bot*pix_size;
      memcpy(tmp_row, top_row, hi_res_.resx*pix_size);
      memcpy(top_row, bot_row, hi_res_.resx*pix_size);
      memcpy(bot_row, tmp_row, hi_res_.resx*pix_size);
    }
  }

  // Need to figure out how we need to do locking properly here. We should freeze
  // SCIRun fully when writing an image
  gui_->lock();

  // Set OpenGL back to nice PixelStore values for somebody else
  glPixelStorei(GL_PACK_SKIP_PIXELS,0);
  glPixelStorei(GL_PACK_ROW_LENGTH,0);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  CHECK_OPENGL_ERROR();

  gui_->unlock();

  string netversion = gui_->eval("getNetVersion");

  // Now write image
  if (write_png)
  {
    Nrrd* nrrd = nrrdNew();
    size_t nrrddim[3]; nrrddim[0] = num_channels; nrrddim[1] = x; nrrddim[2] = y;
    nrrdWrap_nva(nrrd,pixels,nrrdTypeUChar,3,nrrddim);
    NrrdIoState *nio = nrrdIoStateNew();
    nio->encoding = nrrdEncodingRaw;
    nio->format = nrrdFormatPNG;
    nio->endian = airMyEndian;
    
    nrrdKeyValueAdd(nrrd,"netversion",netversion.c_str());
    
    if (nrrdSave(filename.c_str(),nrrd,nio))
    {
      // Cannot use nrrd error system as it is not thread safe
      // Hence error number may come from somewhere else
      std::cerr << "Error could not save PNG image file\n";
    }
    nrrdIoStateNix(nio);
    nrrdNix(nrrd);
  }

  if (write_ppm)
  {
    Nrrd* nrrd = nrrdNew();
    size_t nrrddim[3]; nrrddim[0] = num_channels; nrrddim[1] = x; nrrddim[2] = y;
    nrrdWrap_nva(nrrd,pixels,nrrdTypeUChar,3,nrrddim);
    NrrdIoState *nio = nrrdIoStateNew();
    nio->encoding = nrrdEncodingRaw;
    nio->format = nrrdFormatPNM;
    nio->endian = airMyEndian;

    nrrdKeyValueAdd(nrrd,"netversion",netversion.c_str());
    
    if (nrrdSave(filename.c_str(),nrrd,nio))
    {
      // Cannot use nrrd error system as it is not thread safe
      // Hence error number may come from somewhere else
      std::cerr << "Error could not save PNG image file\n";
    }
    nrrdIoStateNix(nio);
    nrrdNix(nrrd);
  }


  if (write_nrrd)
  {
    Nrrd* nrrd = nrrdNew();
    size_t nrrddim[3]; nrrddim[0] = num_channels; nrrddim[1] = x; nrrddim[2] = y;
    nrrdWrap_nva(nrrd,pixels,nrrdTypeUChar,3,nrrddim);
    NrrdIoState *nio = nrrdIoStateNew();
    nio->encoding = nrrdEncodingRaw;
    nio->format = nrrdFormatNRRD;
    nio->endian = airMyEndian;

    nrrdKeyValueAdd(nrrd,"netversion",netversion.c_str());
    
    if (nrrdSave(filename.c_str(),nrrd,nio))
    {
      // Cannot use nrrd error system as it is not thread safe
      // Hence error number may come from somewhere else
      std::cerr << "Error could not save PNG image file\n";
    }
    nrrdIoStateNix(nio);
    nrrdNix(nrrd);
  }

  // Free space used to store image
  if (pixels) delete [] pixels;
  if (tmp_row) delete [] tmp_row;



  if (use_convert) 
  {
    if (system(string("convert " + filename + " " + filename_convert).c_str()) != 0) 
    {
      std::cerr << "ERROR - Using convert to write image.\n";
    }
    deleteFile(filename);
  }

} // end render_and_save_image()


void
OpenGL::redraw_frame()
{
  if (dead_) return;
  if (!tk_gl_context_) return;
  gui_->lock();
  if (dead_)
  {
    // ViewWindow was deleted from gui_
    gui_->unlock();
    return;
  }

  // Increment the total_frame_count_ now that we know we will draw
  total_frame_count_++;
  
  // Make sure our GL context is current
  if ((tk_gl_context_ != old_tk_gl_context_))
  {
    tk_gl_context_->make_current();

    CHECK_OPENGL_ERROR();

    if (tk_gl_context_ != old_tk_gl_context_)
    {
      old_tk_gl_context_ = tk_gl_context_;
      sci_glew_init();
      GLint data[1];
      glGetIntegerv(GL_MAX_LIGHTS, data);
      max_gl_lights_=data[0];
      // Look for multisample extension...
#ifdef __sgi
      if (strstr((char*)glGetString(GL_EXTENSIONS), "GL_SGIS_multisample"))
      {
        cerr << "Enabling multisampling...\n";
        glEnable(GL_MULTISAMPLE_SGIS);
        glSamplePatternSGIS(GL_1PASS_SGIS);
      }
#endif
    }
  }
  CHECK_OPENGL_ERROR();

  // Get the window size
  xres_ = tk_gl_context_->width();
  yres_ = tk_gl_context_->height();

  gui_->unlock();

  // Start polygon counter...
  WallClockTimer timer;
  timer.clear();
  timer.start();

  // Get a lock on the geometry database...
  // Do this now to prevent a hold and wait condition with TCLTask
  viewer_->geomlock_.readLock();

  gui_->lock();
  const bool dump_frame =
    // Saving an image
    doing_image_p_
    // Recording a movie, but we don't care about synchronized frames
    || (doing_movie_p_ && !doing_sync_frame_)
    // Recording a movie, but we care about synchronized frames
    || (doing_movie_p_ && doing_sync_frame_ && dump_sync_frame_);

  // Set up a framebuffer object for image or movie making.
  if (dump_frame)
  {
    if (!frame_buffer_id_) {
      CHECK_OPENGL_ERROR();

      GLuint db;
      glGenFramebuffersEXT(1, &frame_buffer_id_);
      glGenRenderbuffersEXT(1, &render_buffer_id_);
      glGenRenderbuffersEXT(1, &db);
      
      //color buffer
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, render_buffer_id_);
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA, 
			       xres_, yres_);

      // depth buffer
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, db);
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT32, 
			       xres_, yres_);

      // frame buffer
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buffer_id_);
      glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, 
				   GL_COLOR_ATTACHMENT0_EXT, 
                                   GL_RENDERBUFFER_EXT, render_buffer_id_);

      glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, 
				   GL_DEPTH_ATTACHMENT_EXT,
				   GL_RENDERBUFFER_EXT, db);
      CHECK_OPENGL_ERROR();
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buffer_id_);
    CHECK_OPENGL_ERROR();
    GLint w, h;
    glGetRenderbufferParameterivEXT(GL_RENDERBUFFER_EXT, 
				    GL_RENDERBUFFER_WIDTH_EXT, &w);
    glGetRenderbufferParameterivEXT(GL_RENDERBUFFER_EXT, 
				    GL_RENDERBUFFER_HEIGHT_EXT, &h);
    CHECK_OPENGL_ERROR();
    if (w != xres_ || h != yres_) {
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA, 
			       xres_, yres_);
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, 
			       xres_, yres_);
    }
  }
  
  CHECK_OPENGL_ERROR();
  if (frame_buffer_id_ && dump_frame)
  {
    CHECK_OPENGL_ERROR();
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    CHECK_OPENGL_ERROR();
  }
  else
  {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    tk_gl_context_->make_current();
  }
  CHECK_OPENGL_ERROR(); 

  // Clear the screen.
  glViewport(0, 0, xres_, yres_);
  Color bg(view_window_->gui_bgcolor_.get());
  glClearColor(bg.r(), bg.g(), bg.b(), 0);

  CHECK_OPENGL_ERROR();

  // Setup the view...
  View view(view_window_->gui_view_.get());
  cached_view_ = view;
  const double aspect = double(xres_)/double(yres_);
  // XXX - UNICam change-- should be '1.0/aspect' not 'aspect' below.
  const double fovy = RtoD(2*Atan(1.0/aspect*Tan(DtoR(view.fov()/2.))));

  drawinfo_->reset();

  bool do_stereo = view_window_->gui_do_stereo_.get();

  CHECK_OPENGL_ERROR();

  if (do_stereo)
  {
    GLboolean supported;
    glGetBooleanv(GL_STEREO, &supported);
    if (!supported)
    {
      do_stereo = false;
      static bool warnonce = true;
      if (warnonce)
      {
        cout << "Stereo display selected but not supported.\n";
        warnonce = false;
      }
    }
    CHECK_OPENGL_ERROR();
  }
  
  drawinfo_->view_                  = view;
  drawinfo_->ambient_scale_ = view_window_->gui_ambient_scale_.get();
  drawinfo_->diffuse_scale_ = view_window_->gui_diffuse_scale_.get();
  drawinfo_->specular_scale_ = view_window_->gui_specular_scale_.get();
  drawinfo_->shininess_scale_ = view_window_->gui_shininess_scale_.get();
  drawinfo_->emission_scale_ = view_window_->gui_emission_scale_.get();
  drawinfo_->line_width_ = view_window_->gui_line_width_.get();
  drawinfo_->point_size_ = view_window_->gui_point_size_.get();
  drawinfo_->polygon_offset_factor_ =
    view_window_->gui_polygon_offset_factor_.get();
  drawinfo_->polygon_offset_units_ =
    view_window_->gui_polygon_offset_units_.get();

  if (compute_depth(view, znear_, zfar_))
  {
    // Set up graphics state.
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    view_window_->setState(drawinfo_, "global");
    drawinfo_->pickmode_=0;

    CHECK_OPENGL_ERROR();

    // Do the redraw loop for each time value.
    const double dt = ((animate_time_end_ - animate_time_begin_)
		       / animate_num_frames_);
    const double frametime = 
      animate_framerate_ == 0 ? 0 : 1.0 / animate_framerate_;
    TimeThrottle throttle;
    throttle.start();
    Vector eyesep(0.0, 0.0, 0.0);
    if (do_stereo)
    {
      const double eye_sep_dist = view_window_->gui_sbase_.get() *
        (view_window_->gui_sr_.get() ? 0.048 : 0.0125);
      Vector u, v;
      view.get_viewplane(aspect, 1.0, u, v);
      u.safe_normalize();
      const double zmid = (znear_+zfar_) / 2.0;
      eyesep = u * eye_sep_dist * zmid;
    }

    for (int t=0; t<animate_num_frames_; t++)
    {
      int n = 1;
      if ( do_stereo ) n = 2;
      for (int i=0; i<n; i++)
      {
        if ( do_stereo )
        {
          glDrawBuffer( (i == 0) ? GL_BACK_LEFT : GL_BACK_RIGHT);
        }
        else
        {
          CHECK_OPENGL_ERROR();
          if (frame_buffer_id_ && dump_frame)
          {
            //ASSERT(pbuffer_->is_current());
            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
            CHECK_OPENGL_ERROR();
          }
          else
          {
            glDrawBuffer(GL_BACK);
            glReadBuffer(GL_BACK);
          }
          CHECK_OPENGL_ERROR();
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, xres_, yres_);

        const double modeltime = t * dt + animate_time_begin_;
        view_window_->set_current_time(modeltime);
        
        { // render normal
          // Setup view.
          glMatrixMode(GL_PROJECTION);
          glLoadIdentity();

          if (view_window_->gui_ortho_view_.get())
          {
            drawinfo_->view_.set_is_ortho(true);
            const double len = (view.lookat() - view.eyep()).length();
            const double yval = tan(fovy * M_PI / 360.0) * len;
            const double xval = yval * aspect;
            glOrtho(-xval, xval, -yval, yval, znear_, zfar_);
          }
          else
          {
            drawinfo_->view_.set_is_ortho(false);
            gluPerspective(fovy, aspect, znear_, zfar_);
          }

          glMatrixMode(GL_MODELVIEW);

          glLoadIdentity();
          Point eyep(view.eyep());
          Point lookat(view.lookat());
          if (do_stereo)
          {
            if (i==0)
            {
              eyep -= eyesep;
              if (!view_window_->gui_sr_.get())
              {
                lookat-=eyesep;
              }
            }
            else
            {
              eyep += eyesep;
              if (!view_window_->gui_sr_.get())
              {
                lookat += eyesep;
              }
            }
          }
          Vector up(view.up());
          gluLookAt(eyep.x(), eyep.y(), eyep.z(),
                    lookat.x(), lookat.y(), lookat.z(),
                    up.x(), up.y(), up.z());
          if (do_hi_res_)
          {
            setFrustumToWindowPortion();
          }
        }

        // Set up Lighting
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        const Lighting& lighting = viewer_->lighting_;
        int idx=0;
        int ii;
        for (ii=0; ii<lighting.lights.size(); ii++)
        {
          LightHandle light = lighting.lights[ii];
          light->opengl_setup(view, drawinfo_, idx);
        }
        for (ii=0; ii<idx && ii<max_gl_lights_; ii++)
        {
          glEnable((GLenum)(GL_LIGHT0 + ii));
        }
        for (;ii<max_gl_lights_;ii++)
        {
          glDisable((GLenum)(GL_LIGHT0 + ii));
        }

        // Now set up the fog stuff.
        double fognear, fogfar;
        compute_fog_depth(view, fognear, fogfar,
                          view_window_->gui_fog_visibleonly_.get());
        glFogi(GL_FOG_MODE, GL_LINEAR);
        const float fnear =
          fognear + (fogfar - fognear) * view_window_->gui_fog_start_.get();
        glFogf(GL_FOG_START, fnear);
        const double ffar =
          fognear + (fogfar - fognear) /
          Max(view_window_->gui_fog_end_.get(), 0.001);
        glFogf(GL_FOG_END, ffar);
        GLfloat bgArray[4];
        if (view_window_->gui_fogusebg_.get())
        {
          bgArray[0] = bg.r();
          bgArray[1] = bg.g();
          bgArray[2] = bg.b();
        }
        else
        {
          Color fogcolor(view_window_->gui_fogcolor_.get());
          bgArray[0] = fogcolor.r();
          bgArray[1] = fogcolor.g();
          bgArray[2] = fogcolor.b();
        }       
        bgArray[3] = 1.0;
        glFogfv(GL_FOG_COLOR, bgArray);

        // now make the ViewWindow setup its clipping planes...
        view_window_->setClip(drawinfo_);
        view_window_->setMouse(drawinfo_);

        // UNICAM addition
        glGetDoublev (GL_MODELVIEW_MATRIX, modelview_matrix_);
        glGetDoublev (GL_PROJECTION_MATRIX, projection_matrix_);
        glGetIntegerv(GL_VIEWPORT, viewport_matrix_);

        // set up point size, line size, and polygon offset
        glPointSize(drawinfo_->point_size_);
        glLineWidth(drawinfo_->line_width_);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        if (drawinfo_->polygon_offset_factor_ ||
            drawinfo_->polygon_offset_units_)
        {
          glPolygonOffset(drawinfo_->polygon_offset_factor_,
                          drawinfo_->polygon_offset_units_);
          glEnable(GL_POLYGON_OFFSET_FILL);
        }
        else
        {
          glDisable(GL_POLYGON_OFFSET_FILL);
        }
        CHECK_OPENGL_ERROR();
        // Draw it all.
        current_time_ = modeltime;

        view_window_->do_for_visible(this, &OpenGL::redraw_obj);
        CHECK_OPENGL_ERROR();

        // Check clip planes again, but this time draw their outlining 
        // geometry if clipping plane frames are turned on.  Do this after 
        // drawing all of the other objects.
        if(drawinfo_->clip_planes_){
          render_clip_frames();
        }

        if (view_window_->gui_raxes_.get())
        {
          render_rotation_axis(view, do_stereo, i, eyesep);
        }
      }
        
      // Save z-buffer data.
      if (CAPTURE_Z_DATA_HACK)
      {
        CAPTURE_Z_DATA_HACK = 0;
        glReadPixels(0, 0, xres_, yres_, GL_DEPTH_COMPONENT, GL_FLOAT,
                     depth_buffer_ );
      }
        
      // Wait for the right time before swapping buffers
      //get_gui()->unlock();
      const double realtime = t * frametime;
      if (animate_num_frames_>1)
      {
        throttle.wait_for_time(realtime);
      }

      gui_->execute("update idletasks");

      view_window_->gui_total_frames_.set(
			view_window_->gui_total_frames_.get() + 1);

      // Show the pretty picture.
      if (!(frame_buffer_id_ && dump_frame)) {
        tk_gl_context_->swap();
      }
    }
    throttle.stop();
    double fps;
    if (throttle.time() > 0)
    {
      fps = animate_num_frames_ / throttle.time();
    }
    else
    {
      fps = animate_num_frames_;
    }
    view_window_->set_current_time(animate_time_end_);
    CHECK_OPENGL_ERROR();
  }
  else // !compute_depth()
  {
    // Just show the cleared screen
    view_window_->set_current_time(animate_time_end_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!(frame_buffer_id_ && dump_frame))
    {
      tk_gl_context_->swap();
    }
    CHECK_OPENGL_ERROR();
  }

  viewer_->geomlock_.readUnlock();

  // Look for errors.
  //CHECK_OPENGL_ERROR();

  // Report statistics
  timer.stop();
  fps_timer_.stop();
  ostringstream str;
  // Frame counter This is used to keep track of how many frames the
  // statistics are being used for.  When we go to update the on
  // screen statistics we will check to see if the elapsed time is
  // greater than .33 seconds.  If not we increment this counter and
  // don't display the statistics.  When the accumulated time becomes
  // greater than .33 seconds, we update our statistics.
  if (fps_timer_.time()>.33)
  {
    double fps = animate_num_frames_*frame_count_/fps_timer_.time();
    cerr.flush();
    frame_count_ = 1;
    // TODO: This looks incorrect, round and then convert to int?
    fps += 0.05;                // Round to nearest tenth
    const int fps_whole = (int)fps;
    const int fps_tenths = (int)((fps-fps_whole)*10);
    fps_timer_.clear();
    fps_timer_.start();         // Start it running for next time
    double pps;
    if (timer.time() > 0)
    {
      pps = drawinfo_->polycount_/timer.time();
    }
    else
    {
      pps = drawinfo_->polycount_;
    }
    str << view_window_->id_ << " updatePerf \"";
    str << drawinfo_->polycount_ << " polygons in " << timer.time()
        << " seconds\" \"" << pps
        << " polygons/second\"" << " \"" << fps_whole << "."
        << fps_tenths << " frames/sec\"" << '\0';
  }
  else if (fps_timer_.time() > 0)
  {
    frame_count_++;
    fps_timer_.start();
  }
  else
  {
    fps_timer_.start();
  }

  /*****************************************/
  /* movie-movie makin' movie-movie makin' */
  /*                                       */
  /* Only do this if we are making a movie */
  /* and we are dumping a frame.           */
  /*****************************************/
  if (dump_frame && doing_movie_p_)
  {
    CHECK_OPENGL_ERROR();
    if (make_MPEG_p_ )
    {
      if (!encoding_mpeg_)
      {
        CHECK_OPENGL_ERROR();
        string fname = movie_name_;

        // only add extension if not allready there
        if (!(fname.find(".mpg") != std::string::npos ||
              fname.find(".MPG") != std::string::npos ||
              fname.find(".mpeg") != std::string::npos ||
              fname.find(".MPEG") != std::string::npos))
        {
          fname = fname + string(".mpg");
        }
        
        // Dump the mpeg in the local dir ... ignoring any path since mpeg
        // can not handle it.
        //std::string::size_type pos = fname.find_last_of("/");
        //if ( pos != std::string::npos ) {
        //cerr << "Removing the mpeg path.\n";
        //fname = fname.erase(0, pos+1);
        //}

        if ( fname.find("%") != std::string::npos )
        {
          string message = "Bad File Name Format - Remove the frame numbering format, (i.e. the % and other C style formatting).";
          view_window_->setMovieMessage( message, true );
        }
        else
        {
          string message = "Dumping mpeg " + fname;
          view_window_->setMovieMessage( message );
          CHECK_OPENGL_ERROR();
          StartMpeg( fname );
          CHECK_OPENGL_ERROR();
          current_movie_frame_ = 0;
          encoding_mpeg_ = true;
        }
      }

      if (encoding_mpeg_)
      {
        const string message =
          "Adding Mpeg Frame " + to_string( current_movie_frame_ );

        view_window_->setMovieMessage( message );

        view_window_->setMovieFrame(current_movie_frame_);
        CHECK_OPENGL_ERROR();
        AddMpegFrame();
        CHECK_OPENGL_ERROR();
        current_movie_frame_++;
        view_window_->setMovieFrame(current_movie_frame_);
      }
    }
    else
    { // Dump each frame.
      CHECK_OPENGL_ERROR();
      if (encoding_mpeg_)
      {
        // Finish up mpeg that was in progress.
        encoding_mpeg_ = false;

        EndMpeg();
      }

      std::string::size_type pos = movie_name_.find_last_of("%0");

      if ( pos == std::string::npos ||
          movie_name_[pos+2] != 'd' ||
          movie_name_.find("%") != movie_name_.find_last_of("%") )
      {
        string message = "Bad File Name Format - Illegal Frame Format, no C stlye formating found for the frame number.";
        view_window_->setMovieMessage( message, true );
      }
      else
      {
        char fname[256];
        sprintf(fname, movie_name_.c_str(), current_movie_frame_);

        ostringstream timestr;
        if( add_timestamp_to_movie_frame_name_ ) {
          // Only add the timestamp if the user has requested it via the GUI.
#ifndef _WIN32
          timeval tv;
          gettimeofday(&tv, 0);
          timestr << "." << tv.tv_sec << ".";
          timestr.fill('0');
          timestr.width(6);
          timestr << tv.tv_usec;
#else
          long m_sec = Time::currentSeconds();
          timestr << "." << m_sec /1000 << ".";
          timestr.fill('0');
          timestr.width(3);
          timestr << m_sec % 1000;
#endif
        }
        string fullpath = string(fname) + timestr.str() + "." + movie_frame_extension_;
        
        string message = "Dumping " + fullpath;
        view_window_->setMovieMessage( message );
        dump_image(fullpath, movie_frame_extension_);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        glDrawBuffer(GL_BACK);
        glReadBuffer(GL_BACK);
        current_movie_frame_++;
        view_window_->setMovieFrame(current_movie_frame_);
      }
    }
  }

  // End the mpeg if we are no longer recording a movie
  if (!doing_movie_p_)
  {
    if (encoding_mpeg_)
    { // Finish up mpeg that was in progress.
      encoding_mpeg_ = false;
      EndMpeg();
    }
  }

  gui_->unlock();
  if (!dead_) {
    // may only pertain to exp tcl thread:
    // -  if the ViewWindow is going down, the ViewWindow tcl command may already be gone
    gui_->execute(str.str());
  }
  CHECK_OPENGL_ERROR();
}


void
OpenGL::get_pick(int x, int y,
                 GeomHandle& pick_obj, GeomPickHandle& pick_pick,
                 int& pick_index)
{
  RenderWindowMsgHandle rwm = scinew RenderWindowMsg(DO_PICK,x,y);
  
  mailbox_.send(rwm);
  rwm->wait_signal();

  pick_obj = rwm->ret_pick_obj();
  pick_pick = rwm->ret_pick_pick();
  pick_index = rwm->ret_pick_index();
}


void
OpenGL::real_get_pick(int x, int y,
                      GeomHandle& pick_obj, GeomPickHandle& pick_pick,
                      int& pick_index)
{
  pick_obj = 0;
  pick_pick = 0;
  pick_index = 0x12345678;
  // Make ourselves current

  // Make sure our GL context is current
  if ((tk_gl_context_ != old_tk_gl_context_))
  {
    old_tk_gl_context_ = tk_gl_context_;
    gui_->lock();
    tk_gl_context_->make_current();
    gui_->unlock();
  }

  // Setup the view...
  View view(view_window_->gui_view_.get());
  viewer_->geomlock_.readLock();

  // Compute znear and zfar.
  double znear;
  double zfar;
  if (compute_depth(view, znear, zfar))
  {
    // Setup picking.
    gui_->lock();

    GLuint pick_buffer[pick_buffer_size];
    glSelectBuffer(pick_buffer_size, pick_buffer);
    glRenderMode(GL_SELECT);
    glInitNames();
#ifdef SCI_64BITS
    glPushName(0);
    glPushName(0);
    glPushName(0);
    glPushName(0);
    glPushName(0);
#else
    glPushName(0); //for the pick
    glPushName(0); //for the object
    glPushName(0); //for the object's face index
#endif

    // Picking
    const double aspect = double(xres_)/double(yres_);
    // XXX - UNICam change-- should be '1.0/aspect' not 'aspect' below
    const double fovy = RtoD(2*Atan(1.0/aspect*Tan(DtoR(view.fov()/2.))));
    glViewport(0, 0, xres_, yres_);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluPickMatrix(x, viewport[3]-y, pick_window, pick_window, viewport);
    if (view_window_->gui_ortho_view_.get())
    {
      drawinfo_->view_.set_is_ortho(true);
      const double len = (view.lookat() - view.eyep()).length();
      const double yval = tan(fovy * M_PI / 360.0) * len;
      const double xval = yval * aspect;
      glOrtho(-xval, xval, -yval, yval, znear, zfar);
    }
    else
    {
      drawinfo_->view_.set_is_ortho(false);
      gluPerspective(fovy, aspect, znear, zfar);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Point eyep(view.eyep());
    Point lookat(view.lookat());
    Vector up(view.up());
    gluLookAt(eyep.x(), eyep.y(), eyep.z(),
              lookat.x(), lookat.y(), lookat.z(),
              up.x(), up.y(), up.z());

    drawinfo_->lighting_=0;
    drawinfo_->set_drawtype(DrawInfoOpenGL::Flat);
    drawinfo_->pickmode_=1;

    // Draw it all.
    view_window_->do_for_visible(this,
                                 (ViewWindowVisPMF)&OpenGL::pick_draw_obj);

#ifdef SCI_64BITS
    glPopName();
    glPopName();
    glPopName();
#else
    glPopName();
    glPopName();
#endif

    glFlush();
    int hits = glRenderMode(GL_RENDER);

//    CHECK_OPENGL_ERROR();

    gui_->unlock();
    GLuint min_z;
#ifdef SCI_64BITS
    unsigned long hit_obj=0;
    //    GLuint hit_obj_index = 0x12345678;
    unsigned long hit_pick=0;
    //    GLuint hit_pick_index = 0x12345678;  // need for object indexing
#else
    GLuint hit_obj = 0;
    //GLuint hit_obj_index = 0x12345678;  // need for object indexing
    GLuint hit_pick = 0;
    //GLuint hit_pick_index = 0x12345678;  // need for object indexing
#endif
    
    if (drawinfo_->show_bbox_) hits = 0;
    
    if (hits >= 1)
    {
      int idx = 0;
      min_z = 0;
      bool have_one = false;
      for (int h=0; h<hits; h++)
      {
        int nnames = pick_buffer[idx++];
        GLuint z=pick_buffer[idx++];
        if (nnames > 1 && (!have_one || z < min_z))
        {
          min_z = z;
          have_one = true;
          idx++; // Skip Max Z
#ifdef SCI_64BITS
          idx += nnames - 5; // Skip to the last one.
          const unsigned int ho1 = pick_buffer[idx++];
          const unsigned int ho2 = pick_buffer[idx++];
          hit_pick = ((long)ho1<<32) | ho2;
          //hit_obj_index = pick_buffer[idx++];
          const unsigned int hp1 = pick_buffer[idx++];
          const unsigned int hp2 = pick_buffer[idx++];
          hit_obj = ((long)hp1<<32)|hp2;
          //hit_pick_index = pick_buffer[idx++];
          idx++;
#else
          // hit_obj=pick_buffer[idx++];
          // hit_obj_index=pick_buffer[idx++];
          //for (int i=idx; i<idx+nnames; ++i) cerr << pick_buffer[i] << "\n";
          idx += nnames - 3; // Skip to the last one.
          hit_pick = pick_buffer[idx++];
          hit_obj = pick_buffer[idx++];
          idx++;
          //hit_pick_index=pick_buffer[idx++];
#endif
        }
        else
        {
          idx += nnames + 1;
        }
      }

      if ((hit_obj != 0)&&(hit_pick != 0))
      {
        pick_obj = (GeomObj*)hit_obj;
        pick_pick = (GeomPick*)hit_pick;
        pick_obj->getId(pick_index); //(int)hit_pick_index;
      }
    }
  }
  
  viewer_->geomlock_.readUnlock();
}


// Dump a ppm image.
void
OpenGL::dump_image(const string& fname, const string& ftype)
{
  CHECK_OPENGL_ERROR();

  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);

  CHECK_OPENGL_ERROR();

  const int pix_size = 3;  // for RGB
  const int n = pix_size * vp[2] * vp[3];
  unsigned char* pixels = scinew unsigned char[n];
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //glReadBuffer(GL_FRONT);
  glReadPixels(0, 0, vp[2], vp[3], GL_RGB, GL_UNSIGNED_BYTE, pixels);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  // copy to the framebuffer
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, vp[2], 0.0, vp[3], -10.0, 10.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_DEPTH_TEST);
  glRasterPos2i(0, 0);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_ROW_LENGTH, vp[2]);
  glDrawBuffer(GL_FRONT); //make sure we update what the user sees.
  glDrawPixels(vp[2], vp[3], GL_RGB, GL_UNSIGNED_BYTE, pixels);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

#if defined(HAVE_PNG) && HAVE_PNG
  if (ftype == "png")
  {
    // Create the PNG struct.
    png_structp png =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png == NULL) {
      view_window_->setMovieMessage( "ERROR - Failed to create PNG write struct", true );
      return;
    }

    // Create the PNG info struct.
    png_infop info = png_create_info_struct(png);

    if (info == NULL) {
      view_window_->setMovieMessage( "ERROR - Failed to create PNG info struct", true );
      png_destroy_write_struct(&png, NULL);
      return;
    }

    if (setjmp(png_jmpbuf(png))) {
      view_window_->setMovieMessage( "ERROR - Initializing PNG.", true );
      png_destroy_write_struct(&png, &info);
      return;
    }

    // Initialize the PNG IO.
    FILE *fp = fopen(fname.c_str(), "wb");
    png_init_io(png, fp);
    
    png_set_IHDR(png, info, vp[2], vp[3],
		 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    // Write the PNG header.
    png_write_info(png, info);      

    // Run loop to divide memory into "row" chunks
    png_bytep *rows = (png_bytep*)malloc(sizeof(png_bytep) * vp[3]);
    for (int hi = 0; hi < vp[3]; hi++) {
      rows[hi] = &((png_bytep)pixels)[(vp[3] - hi - 1) * vp[2] * pix_size];
    }

    png_set_rows(png, info, rows);

    png_write_image(png, rows);

    /* end write */
    if (setjmp(png_jmpbuf(png))) {
      view_window_->setMovieMessage( "Error during end of PNG write", true );
      png_destroy_write_struct(&png, &info);
      return;
    }
      
    // Finish writing.
    png_write_end(png, NULL);
      
    // More clean up.
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    free(rows);
  }
  else
#endif
  {
    string fnamecopy = fname;
    if (ftype == "png")
    {
      static bool firsttime = true;
      if (firsttime)
      {
        view_window_->setMovieMessage(" ERROR - No PNG support in this build, saving PPM images.", true);
        firsttime = false;
      }
      // WARNING: I think the below line has the very real potential
      // to break on any file name that happens to have png in it (in
      // addition to the extension).  Probably should do a "rfind"
      // instead of a "find"...  Don't have time to verify this right
      // now though.  FIXME
      fnamecopy = fname.substr(0, fname.find("png")) + "ppm";
    }
    ofstream dumpfile( fnamecopy.c_str(), ios::binary );
    if ( !dumpfile )
    {
      string errorMsg = "ERROR opening file: " + fnamecopy;
      view_window_->setMovieMessage( errorMsg, true );
      cerr << errorMsg << "\n";
      return;
    }

    // Print out the ppm  header.
    dumpfile << "P6\n";
    dumpfile << vp[2] << " " << vp[3] << "\n";
    dumpfile << 255 << "\n";

    // OpenGL renders upside-down to ppm_file writing.
    unsigned char *top_row, *bot_row;     
    unsigned char *tmp_row = scinew unsigned char[ vp[2] * pix_size];
    int top, bot;
    for ( top = vp[3] - 1, bot = 0; bot < vp[3]/2; top --, bot++){
      top_row = pixels + vp[2] * top * pix_size;
      bot_row = pixels + vp[2] * bot * pix_size;
      memcpy(tmp_row, top_row, vp[2] * pix_size);
      memcpy(top_row, bot_row, vp[2] * pix_size);
      memcpy(bot_row, tmp_row, vp[2] * pix_size);
    }
    // Now dump the file.
    dumpfile.write((const char *)pixels, n);
    delete [] tmp_row;
  }

  delete [] pixels;
}



void
OpenGL::put_scanline(int y, int width, Color* scanline, int repeat)
{
  float* pixels = scinew float[width*3];
  float* p = pixels;
  int i;
  for (i=0; i<width; i++)
  {
    *p++ = scanline[i].r();
    *p++ = scanline[i].g();
    *p++ = scanline[i].b();
  }
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslated(-1.0, -1.0, 0.0);
  glScaled(2.0 / xres_, 2.0 / yres_, 1.0);
  glDepthFunc(GL_ALWAYS);
  glDrawBuffer(GL_FRONT);
  for (i=0; i<repeat; i++)
  {
    glRasterPos2i(0, y + i);
    glDrawPixels(width, 1, GL_RGB, GL_FLOAT, pixels);
  }
  glDepthFunc(GL_LEQUAL);
  glDrawBuffer(GL_BACK);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  delete[] pixels;
}



void
OpenGL::pick_draw_obj(ViewScene* viewer, ViewWindow*, GeomHandle obj)
{
#ifdef SCI_64BITS
  unsigned long o = (unsigned long)(obj.get_rep());
  unsigned int o1 = (o>>32)&0xffffffff;
  unsigned int o2 = o&0xffffffff;
  glPopName();
  glPopName();
  glPopName();
  glPushName(o1);
  glPushName(o2);
  glPushName(0x12345678);
#else
  glPopName();
  glPushName((GLuint)(obj.get_rep()));
  glPushName(0x12345678);
#endif
  obj->draw(drawinfo_, viewer->default_material_.get_rep(), current_time_);
}



void
OpenGL::redraw_obj(ViewScene* viewer, ViewWindow* viewwindow, GeomHandle obj)
{
  GeomViewerItem *gvi  = dynamic_cast<GeomViewerItem *>(obj.get_rep());
  ASSERT(gvi)
  viewwindow->setDI(drawinfo_, gvi->getString());
  obj->draw(drawinfo_, viewer->default_material_.get_rep(), current_time_);
}






void
OpenGL::deriveFrustum()
{
  double pmat[16];
  glGetDoublev(GL_PROJECTION_MATRIX, pmat);
  const double G = (pmat[10]-1)/(pmat[10]+1);
  frustum_.znear = -(pmat[14]*(G-1))/(2*G);
  frustum_.zfar = frustum_.znear*G;

  if (view_window_->gui_ortho_view_.get())
  {
    drawinfo_->view_.set_is_ortho(true);
    frustum_.left = (pmat[8]-1)/pmat[0];
    frustum_.right = (pmat[8]+1)/pmat[0];
    frustum_.bottom = (pmat[9]-1)/pmat[5];
    frustum_.top = (pmat[9]+1)/pmat[5];
    frustum_.width = frustum_.right - frustum_.left;
    frustum_.height = frustum_.top - frustum_.bottom;
  }
  else
  {
    drawinfo_->view_.set_is_ortho(false);
    frustum_.left = frustum_.znear*(pmat[8]-1)/pmat[0];
    frustum_.right = frustum_.znear*(pmat[8]+1)/pmat[0];
    frustum_.bottom = frustum_.znear*(pmat[9]-1)/pmat[5];
    frustum_.top = frustum_.znear*(pmat[9]+1)/pmat[5];
    frustum_.width = frustum_.right - frustum_.left;
    frustum_.height = frustum_.top - frustum_.bottom;
  }
}



void
OpenGL::setFrustumToWindowPortion()
{
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  if (view_window_->gui_ortho_view_.get())
  {
    drawinfo_->view_.set_is_ortho(true);
    glOrtho(frustum_.left + frustum_.width / hi_res_.ncols * hi_res_.col,
            frustum_.left + frustum_.width / hi_res_.ncols * (hi_res_.col+1),
            frustum_.bottom + frustum_.height / hi_res_.nrows * hi_res_.row,
            frustum_.bottom + frustum_.height / hi_res_.nrows *(hi_res_.row+1),
            znear_, zfar_);
  }
  else
  {
    drawinfo_->view_.set_is_ortho(false);
    glFrustum(frustum_.left + frustum_.width / hi_res_.ncols * hi_res_.col,
              frustum_.left + frustum_.width / hi_res_.ncols * (hi_res_.col+1),
              frustum_.bottom + frustum_.height / hi_res_.nrows* hi_res_.row,
              frustum_.bottom + frustum_.height /hi_res_.nrows*(hi_res_.row+1),
              frustum_.znear, frustum_.zfar);
  }
}



void
OpenGL::saveImage(const string& fname,
                  const string& type,
                  int x, int y)
{
  RenderWindowMsgHandle rwm = scinew RenderWindowMsg(DO_IMAGE,fname,type,x,y);
  mailbox_.send(rwm);
}

void
OpenGL::scheduleSyncFrame()
{
  RenderWindowMsgHandle rwm = scinew RenderWindowMsg(DO_SYNC_FRAME);
  mailbox_.send(rwm);
}

void
OpenGL::getData(int datamask, FutureValue<GeometryData*>* result)
{
  RenderWindowMsgHandle rwm = scinew RenderWindowMsg(DO_GETDATA,datamask,result);
  mailbox_.send(rwm);
}


void
OpenGL::real_getData(int datamask, FutureValue<GeometryData*>* result)
{
  GeometryData* res = new GeometryData;
  if (datamask&GEOM_VIEW)
  {
    res->view=new View(cached_view_);
    res->xres=xres_;
    res->yres=yres_;
    res->znear=znear_;
    res->zfar=zfar_;
  }
  if (datamask&(GEOM_COLORBUFFER|GEOM_DEPTHBUFFER/*CollabVis*/|GEOM_MATRICES))
  {
    gui_->lock();
  }
  if (datamask&GEOM_COLORBUFFER)
  {
    ColorImage* img = res->colorbuffer = new ColorImage(xres_, yres_);
    float* data=new float[xres_*yres_*3];
    WallClockTimer timer;
    timer.start();

    glReadPixels(0, 0, xres_, yres_, GL_RGB, GL_FLOAT, data);
    timer.stop();
    float* p = data;
    for (int y=0; y<yres_; y++)
    {
      for (int x=0; x<xres_; x++)
      {
        img->put_pixel(x, y, Color(p[0], p[1], p[2]));
        p += 3;
      }
    }
    delete[] data;
  }
  if (datamask&GEOM_DEPTHBUFFER)
  {
    unsigned int* data=new unsigned int[xres_*yres_*3];
    WallClockTimer timer;
    timer.start();
    glReadPixels(0, 0,xres_,yres_, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, data);
    timer.stop();
  }

  if (datamask&(GEOM_COLORBUFFER|GEOM_DEPTHBUFFER/*CollabVis*/|GEOM_MATRICES))
  {
//    CHECK_OPENGL_ERROR();
    gui_->unlock();
  }

  if (datamask&(GEOM_VIEW_BOUNDS))
  {
    view_window_->get_bounds_all(res->view_bounds_);
  }
  result->send(res);
}


void
OpenGL::StartMpeg(const string& fname)
{
#ifdef HAVE_MPEG
  // Get a file pointer pointing to the output file.
  mpeg_file_ = fopen(fname.c_str(), "w");
  if (!mpeg_file_)
  {
    cerr << "Failed to open file " << fname << " for writing\n";
    return;
  }
  // Get the default options.
  MPEGe_default_options( &mpeg_options_ );
  // Change a couple of the options.
  char *pattern = scinew char[4];
  strncpy(pattern, "II\0", 4);
  mpeg_options_.frame_pattern = pattern;
  mpeg_options_.search_range[1]=0;
  mpeg_options_.gop_size=1;
  mpeg_options_.IQscale=1;
  mpeg_options_.PQscale=1;
  mpeg_options_.BQscale=1;
  mpeg_options_.pixel_search=MPEGe_options::FULL;
  if ( !MPEGe_open(mpeg_file_, &mpeg_options_ ) )
  {
    cerr << "MPEGe library initialisation failure!:" <<
      mpeg_options_.error << "\n";
    return;
  }
#endif // HAVE_MPEG
}


void
OpenGL::AddMpegFrame()
{
#ifdef HAVE_MPEG
  // Looks like we'll blow up if you try to make more than one movie
  // at a time, as the memory per frame is static.
  static ImVfb *image=NULL; /* Only alloc memory for these once. */
  int width, height;
  ImVfbPtr ptr;

  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);

  width = vp[2];
  height = vp[3];

  // Set up the ImVfb used to store the image.
  if ( !image )
  {
    image=MPEGe_ImVfbAlloc( width, height, IMVFBRGB, true );
    if ( !image )
    {
      cerr<<"Couldn't allocate memory for frame buffer\n";
      exit(2);
    }
  }

  // Get to the first pixel in the image.
  ptr = ImVfbQPtr( image, 0, 0 );
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  //glReadBuffer(GL_FRONT);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, ptr);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  // copy to the framebuffer
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, vp[2], 0.0, vp[3], -10.0, 10.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_DEPTH_TEST);
  glRasterPos2i(0, 0);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_ROW_LENGTH, vp[2]);
  glDrawBuffer(GL_FRONT); //make sure we update what the user sees.
  glDrawPixels(vp[2], vp[3], GL_RGB, GL_UNSIGNED_BYTE, ptr);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  const int r = 3 * width;
  unsigned char* row = scinew unsigned char[r];
  unsigned char* p0, *p1;

  int k, j;
  for (k = height -1, j = 0; j < height/2; k--, j++)
  {
    p0 = ptr + r * j;
    p1 = ptr + r * k;
    memcpy( row, p0, r);
    memcpy( p0, p1, r);
    memcpy( p1, row, r);
  }
  delete[] row;

  if ( !MPEGe_image(image, &mpeg_options_) )
  {
    view_window_->setMovieMessage( string("ERROR creating MPEG frame: ") + mpeg_options_.error, true );
  }
#endif // HAVE_MPEG
}



void
OpenGL::EndMpeg()
{
#ifdef HAVE_MPEG
  if ( !MPEGe_close(&mpeg_options_) )
  {
    string errorMsg = string("ERROR closing MPEG file: ") + mpeg_options_.error;
    view_window_->setMovieMessage( errorMsg, true );
  }
  else
  {
    string message = "Ending Mpeg.";
    view_window_->setMovieMessage( message );
  }

  view_window_->setMovieStopped();
#endif // HAVE_MPEG
}



// Return world-space depth to point under pixel (x, y).
bool
OpenGL::pick_scene( int x, int y, Point *p )
{
  // y = 0 is bottom of screen (not top of screen, which is what X
  // events reports)
  y = (yres_ - 1) - y;
  int index = x + (y * xres_);
  double z = depth_buffer_[index];
  if (p)
  {
    // Unproject the window point (x, y, z).
    GLdouble world_x, world_y, world_z;
    gluUnProject(x, y, z,
                 modelview_matrix_, projection_matrix_, viewport_matrix_,
                 &world_x, &world_y, &world_z);

    *p = Point(world_x, world_y, world_z);
  }

  // if z is close to 1, then assume no object was picked
  return (z < .999999);
}



bool
OpenGL::compute_depth(const View& view, double& znear, double& zfar)
{
  znear = DBL_MAX;
  zfar =- DBL_MAX;
  BBox bb;
  view_window_->get_bounds(bb);
  if (bb.valid())
  {
    // We have something to draw.
    Point min(bb.min());
    Point max(bb.max());
    Point eyep(view.eyep());
    Vector dir(view.lookat()-eyep);
    const double dirlen2 = dir.length2();
    if (dirlen2 < 1.0e-6 || dirlen2 != dirlen2)
    {
      return false;
    }
    dir.safe_normalize();
    const double d = -Dot(eyep, dir);
    for (int i=0;i<8;i++)
    {
      const Point p((i&1)?max.x():min.x(),
                    (i&2)?max.y():min.y(),
                    (i&4)?max.z():min.z());
      const double dist = Dot(p, dir) + d;
      znear = Min(znear, dist);
      zfar = Max(zfar, dist);
    }
    znear *= 0.99;
    zfar  *= 1.01;

    if (znear <= 0.0)
    {
      if (zfar <= 0.0)
      {
        // Everything is behind us - it doesn't matter what we do.
        znear = 1.0;
        zfar = 2.0;
      }
      else
      {
        znear = zfar * 0.001;
      }
    }
    return true;
  }
  else
  {
    return false;
  }
}


bool
OpenGL::compute_fog_depth(const View &view, double &znear, double &zfar,
                          bool visible_only)
{
  znear = DBL_MAX;
  zfar = -DBL_MAX;
  BBox bb;
  if (visible_only)
  {
    view_window_->get_bounds(bb);
  }
  else
  {
    view_window_->get_bounds_all(bb);
  }
  if (bb.valid())
  {
    // We have something to draw.
    Point eyep(view.eyep());
    Vector dir(view.lookat()-eyep);
    const double dirlen2 = dir.length2();
    if (dirlen2 < 1.0e-6 || dirlen2 != dirlen2)
    {
      return false;
    }
    dir.safe_normalize();
    const double d = -Dot(eyep, dir);

    // Compute distance to center of bbox.
    const double dist = Dot(bb.center(), dir);
    // Compute bbox view radius.
    const double radius = bb.diagonal().length() * dir.length2() * 0.5;

    znear = d + dist - radius;
    zfar = d + dist + radius;

    return true;
  }
  else
  {
    return false;
  }
}


// i is the frame number, usually refers to left or right when do_stereo
// is set.

void
OpenGL::render_rotation_axis(const View &view,
                             bool do_stereo, int i, const Vector &eyesep)
{
  static GeomHandle axis_obj = 0;
  if (axis_obj.get_rep() == 0) axis_obj = view_window_->createGenAxes();

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  const int xysize = Min(viewport[2], viewport[3]) / 4;
  glViewport(viewport[2] - xysize, viewport[3] - xysize, xysize, xysize);
  const double aspect = 1.0;

  // fovy 16 eyedist 10 is approximately the default axis view.
  // fovy 32 eyedist 5 gives an exagerated perspective.
  const double fovy = 32.0;
  const double eyedist = 5.0;
  const double znear = eyedist - 2.0;
  const double zfar = eyedist + 2.0;

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPerspective(fovy, aspect, znear, zfar);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  Vector oldeye(view.eyep().asVector() - view.lookat().asVector());
  oldeye.safe_normalize();
  Point eyep((oldeye * eyedist).asPoint());
  Point lookat(0.0, 0.0, 0.0);
  if (do_stereo)
  {
    if (i == 0)
    {
      eyep -= eyesep;
      if (!view_window_->gui_sr_.get())
      {
        lookat -= eyesep;
      }
    }
    else
    {
      eyep += eyesep;
      if (!view_window_->gui_sr_.get())
      {
        lookat += eyesep;
      }
    }
  }

  Vector up(view.up());
  gluLookAt(eyep.x(), eyep.y(), eyep.z(),
            lookat.x(), lookat.y(), lookat.z(),
            up.x(), up.y(), up.z());
  if (do_hi_res_)
  {
    // Draw in upper right hand corner of total image, not viewport image.
    const int xysize = Min(hi_res_.resx, hi_res_.resy) / 4;
    const int xoff = hi_res_.resx - hi_res_.col * viewport[2];
    const int yoff = hi_res_.resy - hi_res_.row * viewport[3];
    glViewport(xoff - xysize, yoff - xysize, xysize, xysize);
  }

  view_window_->setState(drawinfo_, "global");

  // Disable fog for the orientation axis.
  const bool fog = drawinfo_->fog_;
  if (fog) { glDisable(GL_FOG); }
  drawinfo_->fog_ = false;

  // Set up Lighting
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  const Lighting& l = viewer_->lighting_;
  int idx=0;
  int ii;
  for (ii=0;ii<l.lights.size();ii++)
  {
    LightHandle light=l.lights[ii];
    light->opengl_setup(view, drawinfo_, idx);
  }
  for (ii=0;ii<idx && ii<max_gl_lights_;ii++)
    glEnable((GLenum)(GL_LIGHT0+ii));
  for (;ii<max_gl_lights_;ii++)
    glDisable((GLenum)(GL_LIGHT0+ii));

  // Disable clipping planes for the orientation icon.
  vector<bool> cliplist(6, false);
  for (ii = 0; ii < 6; ii++)
  {
    if (glIsEnabled((GLenum)(GL_CLIP_PLANE0+ii)))
    {
      glDisable((GLenum)(GL_CLIP_PLANE0+ii));
      cliplist[ii] = true;
    }
  }

  // Use depthrange to force the icon to move forward.
  // Ideally the rest of the scene should be drawn at 0.05 1.0,
  // so there was no overlap at all, but that would require
  // mucking about in the picking code.
  glDepthRange(0.0, 0.05);
  axis_obj->draw(drawinfo_, 0, current_time_);
  glDepthRange(0.0, 1.0);

  drawinfo_->fog_ = fog;  // Restore fog state.
  if (fog) { glEnable(GL_FOG); }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

  // Reenable clipping planes.
  for (ii = 0; ii < 6; ii++)
  {
    if (cliplist[ii])
    {
      glEnable((GLenum)(GL_CLIP_PLANE0+ii));
    }
  }
}

void
OpenGL::render_clip_frames()
{

  unsigned int i,ii;

  // first check to see if their are any frames on..
  bool frame_on = false;
  for(i = 0; i < 6; i++){
    if(view_window_->viewwindow_clip_frames_draw_[i]){
      frame_on = true;
      break;
    }
  }
  
  if(!frame_on) return;

  // a frame is on, let's draw it.
  view_window_->setState(drawinfo_, "global");

  GeomGroup *frames = scinew GeomGroup;
  Material *frame_color = scinew Material(Color(0.0, 0.0, 0.0),
                                          Color(.44, .50, .86),
                                          Color(.5,.5,.5), 20);
  GeomTranspQuads *quads = scinew GeomTranspQuads;
  const Color black(0,0,0), gray(0.3,0.3,0.3);
  Material *screen = scinew Material(black, gray, gray, 5);
  screen->transparency = 0.8;

  for(i = 0; i < view_window_->viewwindow_clip_frames_.size(); i++){
    if( view_window_->viewwindow_clip_frames_draw_[i] ){
      ViewWindowClipFrame *cf =
        view_window_->viewwindow_clip_frames_[i];
      
      for(ii = 0; ii < cf->edges_.size(); ii++){
        frames->add(scinew GeomSphere(*(cf->corners_[ii])));
        frames->add(scinew GeomCylinder(*(cf->edges_[ii])));

      }

      // keep all quads in one list so that they are sorted properly.
      quads->add(cf->verts_[0],  screen,
                 cf->verts_[1],  screen,
                 cf->verts_[2],  screen,
                 cf->verts_[3],  screen);
      
    }
  }
  // add the quads to the frame
  frames->add(quads);
  
  // turn clip planes off
  vector<bool> cliplist(6, false);
  for (ii = 0; ii < 6; ii++)
  {
    if (glIsEnabled((GLenum)(GL_CLIP_PLANE0+ii)))
    {
      glDisable((GLenum)(GL_CLIP_PLANE0+ii));
      cliplist[ii] = true;
    }
  }

  // Put everything in a GeometryHandle.  This should
  // clean up all of the GeomObjects we just made.
  GeomHandle objs = scinew GeomMaterial(frames, frame_color);
  // now draw it.
  objs->draw(drawinfo_,viewer_->default_material_.get_rep(), current_time_);


  // turn clip planes back on
  for (ii = 0; ii < 6; ii++)
  {
    if (cliplist[ii])
    {
      glEnable((GLenum)(GL_CLIP_PLANE0+ii));
    }
  }
}

} // End namespace SCIRun

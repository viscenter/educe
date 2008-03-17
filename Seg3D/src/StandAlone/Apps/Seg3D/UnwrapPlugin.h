#ifndef SEG3D_UnwrapPlugin_h
#define SEG3D_UnwrapPlugin_h

#include <StandAlone/Apps/Seg3D/GenericPlugin.h>
#include <StandAlone/Apps/Seg3D/Painter.h>
#include <Core/Events/BaseTool.h>

#include <wx/filedlg.h>
#include <wx/colordlg.h>
#include <wx/aboutdlg.h>
#include <wx/tokenzr.h>
#include <wx/fs_zip.h>

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

namespace SCIRun {

	struct Unwrapping  {
		CvMat * point_lookup;
		unsigned int num_unwraps;
		unsigned int num_slices;
		Nrrd * referenced_volume;
	};

	class UnwrappedView : public wxScrolledWindow {
		public:	
			wxBitmap bmap;
			Unwrapping * unwrapped;
			CvMat * vol_lookup;
			int unwrap_pos;
			NrrdRange *range;

			UnwrappedView(wxWindow * parent) : wxScrolledWindow(parent) {
				range = NULL;
			}
			void OnDraw(wxDC& dc) {
				dc.DrawBitmap(bmap, 0, 0, false);
			}
			void update_image(void) {
				SetScrollbars(1,1,bmap.GetWidth(),bmap.GetHeight(),0,0,true);
				Refresh();
			}
			void set_image(Unwrapping * uwrap, int position = -1) {
				if(uwrap != NULL) {
					unwrapped = uwrap;
				}
				if(position == -1) {
					position = unwrapped->num_unwraps/2;
				}
				unwrap_pos = (position >= 0) && (position < unwrapped->num_unwraps) ? position : 0;
				CvMat * cvIm = unwrapped->point_lookup; 
				vol_lookup = cvIm;

				IplImage * myIm = cvCreateImage(cvSize(cvIm->width,unwrapped->num_slices),8,3);

				if(range == NULL) {
					range = nrrdRangeNewSet(unwrapped->referenced_volume,0);
					printf("min: %g\tmax: %g\n", range->min, range->max);
				}
			
				printf("Ref vol head: %x\n",*(unwrapped->referenced_volume));
			
				int width = unwrapped->referenced_volume->axis[1].size,
				height = unwrapped->referenced_volume->axis[2].size,
				slices = unwrapped->referenced_volume->axis[3].size;

				double (*nrrdlup)(const void *, size_t I) = nrrdDLookup[unwrapped->referenced_volume->type];

				IplImage *cvcast = cvCreateImageHeader(cvSize(width, height),
					IPL_DEPTH_8U, 1);

				double min = range->min, max = range->max;

				printf("Trying %d\n",unwrap_pos);
				for(int y = 0; y < unwrapped->num_slices /*cvIm->height*/; y++) {
					for(int x = 0; x < cvIm->width; x++) {
						int sx = cvIm->data.i[(unwrap_pos*3*cvIm->width*unwrapped->num_slices)+y*3*cvIm->width+x*3+0];
						int sy = cvIm->data.i[(unwrap_pos*3*cvIm->width*unwrapped->num_slices)+y*3*cvIm->width+x*3+1];
						int sz = cvIm->data.i[(unwrap_pos*3*cvIm->width*unwrapped->num_slices)+y*3*cvIm->width+x*3+2];

						// taken from airIndex, from nrrdQuantize
						unsigned int idx = 
								(unsigned int)((double)(1<<8)*
										(nrrdlup(unwrapped->referenced_volume->data,
											sz*(width*height)+sy*width+sx)-min)/(max-min)
										);
						idx -= (idx == (1<<8));

						cvSet2D(myIm,y,x,
							cvScalarAll(idx)
						);
					}
				}
				
				// nrrdRangeNix(range);

				cvReleaseImageHeader(&cvcast);

				cvSaveImage("unwrapped-shown.png",myIm);

				unsigned char * rawData;
				int step = 0;
				CvSize roiSize;
				cvGetRawData( myIm, &rawData, &step, &roiSize );

				wxImage wxi = wxImage(myIm->width, myIm->height, rawData, true);
				bmap = wxBitmap(wxi.Scale(myIm->width,myIm->height));

				cvReleaseImage(&myIm);

				update_image();
			}
			void OnMouse(wxMouseEvent& event) {
				if(event.LeftIsDown()) {
					int xx = event.GetX();
					int yy = event.GetY();
					int x,y;
					CalcUnscrolledPosition(xx,yy,&x,&y);
					if((x >= 0) && (y >= 0) && 
							(x < vol_lookup->width) && (y < vol_lookup->height)) {
						// printf("Location: %d, %d\n",x,y);
						//CvScalar lup = cvGet2D(vol_lookup,event.GetY(),event.GetX());
						int xx = vol_lookup->data.i[(unwrap_pos*3*vol_lookup->width*unwrapped->num_slices)+y*3*vol_lookup->width+x*3+0];
						int yy = vol_lookup->data.i[(unwrap_pos*3*vol_lookup->width*unwrapped->num_slices)+y*3*vol_lookup->width+x*3+1];
						int zz = vol_lookup->data.i[(unwrap_pos*3*vol_lookup->width*unwrapped->num_slices)+y*3*vol_lookup->width+x*3+2];

						/*
						printf("LUP: %d, %d, %d\n",
								xx,
								yy,
								zz
								);
						*/

						SCIRun::ThrowSkinnerSignalEvent *tsse =
							new SCIRun::ThrowSkinnerSignalEvent("Painter::UnwrapProbe");
						tsse->add_var("Painter::UnwrapProbe::x", to_string(xx));
						tsse->add_var("Painter::UnwrapProbe::y", to_string(yy));
						tsse->add_var("Painter::UnwrapProbe::z", to_string(zz));
						SCIRun::Painter::ThrowSkinnerSignal(tsse);
					}

					//Painter::ThrowSkinnerSignal("Painter::UnwrapProbe");
					event.Skip();
				}
			}
		private:
			DECLARE_EVENT_TABLE();
	};

	BEGIN_EVENT_TABLE(UnwrappedView, wxScrolledWindow)
		EVT_LEFT_DOWN(UnwrappedView::OnMouse)
		EVT_MOTION(UnwrappedView::OnMouse)
	END_EVENT_TABLE()
	
	class UnwrappedSlider : public wxSlider {
		public:
			UnwrappedView * scroll;

			UnwrappedSlider(wxWindow * parent) : wxSlider(parent,-1,0,0,1, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL | wxSL_LEFT | wxSL_LABELS) {}
			void set_scroll(UnwrappedView * scr) {
				scroll = scr;
				SetRange(0,(scroll->unwrapped->num_unwraps)-1);
				SetValue((scroll->unwrapped->num_unwraps)/2);
			}
			void OnChange(wxScrollEvent& event) {
				scroll->set_image(NULL, GetValue());
			}
		private:
			DECLARE_EVENT_TABLE();
	};

	BEGIN_EVENT_TABLE(UnwrappedSlider, wxSlider)
		EVT_SCROLL_CHANGED(UnwrappedSlider::OnChange)
	END_EVENT_TABLE()

	class UnwrapPluginWindow : public wxFrame {
		public:
			UnwrappedView * scroll;
			UnwrappedSlider * slide;

			UnwrapPluginWindow(const wxString& title, wxFrame *frame,
             const wxPoint& pos,
             const wxSize& size, long style = wxDEFAULT_FRAME_STYLE) :
				wxFrame(frame, wxID_ANY, title, pos, size, style) {
					printf("UnwrapPluginWindow shown\n");
		
					wxSplitterWindow *split = new wxSplitterWindow(this,-1);

					slide = new UnwrappedSlider(split);
					scroll = new UnwrappedView(split);

					split->SplitVertically(slide,scroll,30);
				}
	};

	class UnwrapPlugin : public virtual BaseTool, public virtual GenericPlugin {
		protected:
			Painter *	painter_;
			UnwrapPluginWindow * unwrap_win_;
			void init_window(Unwrapping * unwrapped) {
				wxCommandEvent wxevent(wxEVT_COMMAND_UNWRAP_WINDOW);
				wxevent.SetClientData(unwrapped);
				wxPostEvent(painter_->global_seg3dframe_pointer_, wxevent);
			}
		public:
			UnwrapPlugin(Painter * ip = NULL) : painter_(ip) {}
			virtual BaseTool::propagation_state_e process_event(event_handle_t) { printf("Processing event inside UnwrapPlugin\n"); return CONTINUE_E; }
			virtual int get_type() const { return PLUGIN_FILTER; }
			virtual void set_painter(Painter * ip) { painter_ = ip; }
			virtual std::string get_menu_string() const { return "UnwrapPlugin"; }
			virtual void menu_event() const { printf("TPME\n"); }
		private:
			virtual void run_filter() const { printf("Running UnwrapPlugin\n"); }
	};

}

#endif // SEG3D_UnwrapPlugin_h

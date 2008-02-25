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
		Nrrd * referenced_volume;
	};

	class UnwrappedView : public wxScrolledWindow {
		public:	
			wxBitmap bmap;
			CvMat * vol_lookup;
			UnwrappedView(wxWindow * parent) : wxScrolledWindow(parent) {}	
			void OnDraw(wxDC& dc) {
				dc.DrawBitmap(bmap, 0, 0, false);
			}
			void update_image(void) {
				SetScrollbars(1,1,bmap.GetWidth(),bmap.GetHeight(),0,0,true);
				Refresh();
			}
			void set_image(Unwrapping * unwrapped) {
				CvMat * cvIm = unwrapped->point_lookup; 
				vol_lookup = cvIm;
				/*
				IplImage * test = cvCreateImage(cvSize(1000,1000), 8, 3);

				cvSet(test, cvScalar(128));
				*/

				IplImage * myIm = cvCreateImage(cvSize(cvIm->width,cvIm->height),8,3);

				/*
				if(cvIm->nChannels == 3) {
					myIm = cvIm;
				}
				else {
					printf("Size: %d %d\n",cvIm->width,cvIm->height);
					myIm = cvCreateImage(cvSize(cvIm->width,cvIm->height),8,3);
					cvCvtColor(cvIm, myIm, CV_GRAY2BGR);
				}
				*/

				NrrdRange *range = nrrdRangeNewSet(unwrapped->referenced_volume,0);
				printf("min: %g\tmax: %g\n", range->min, range->max);
				Nrrd *curnrrd = nrrdNew();
			
				nrrdQuantize(curnrrd,unwrapped->referenced_volume,range,8);

				nrrdRangeNix(range);

				int width = curnrrd->axis[1].size,
				height = curnrrd->axis[2].size,
				slices = curnrrd->axis[3].size;

				int (*nrrdlup)(const void *, size_t I) = nrrdILookup[curnrrd->type];

				for(int y = 0; y < cvIm->height; y++) {
					for(int x = 0; x < cvIm->width; x++) {

						CvScalar lup = cvGet2D(cvIm,y,x);
						int sx = (int)lup.val[0];
						int sy = (int)lup.val[1];
						int sz = (int)lup.val[2];
						cvSet2D(myIm,y,x,cvScalarAll(
							nrrdlup(curnrrd->data,sz*(width*height)+sy*width+sx)
									));
					}
				}

				nrrdNuke(curnrrd);

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
						printf("Location: %d, %d\n",x,y);
						//CvScalar lup = cvGet2D(vol_lookup,event.GetY(),event.GetX());
						int xx = vol_lookup->data.i[y*3*vol_lookup->width+x*3+0];
						int yy = vol_lookup->data.i[y*3*vol_lookup->width+x*3+1];
						int zz = vol_lookup->data.i[y*3*vol_lookup->width+x*3+2];

						printf("LUP: %d, %d, %d\n",
								xx,
								yy,
								zz
								);

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

	class UnwrapPluginWindow : public wxFrame {
		public:
			UnwrappedView * scroll;
			UnwrapPluginWindow(const wxString& title, wxFrame *frame,
             const wxPoint& pos,
             const wxSize& size, long style = wxDEFAULT_FRAME_STYLE) :
				wxFrame(frame, wxID_ANY, title, pos, size, style) {
					printf("UnwrapPluginWindow shown\n");
					scroll = new UnwrappedView(this);
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

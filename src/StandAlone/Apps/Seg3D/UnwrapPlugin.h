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

	class UnwrappedView : public wxScrolledWindow {
		public:	
			wxBitmap bmap;
			UnwrappedView(wxWindow * parent) : wxScrolledWindow(parent) {}	
			void OnDraw(wxDC& dc) {
				dc.DrawBitmap(bmap, 0, 0, false);
			}
			void update_image(void) {
				SetScrollbars(1,1,bmap.GetWidth(),bmap.GetHeight(),0,0,true);
				Refresh();
			}
			void set_image(IplImage * cvIm) {
				/*
				IplImage * test = cvCreateImage(cvSize(1000,1000), 8, 3);

				cvSet(test, cvScalar(128));
				*/

				IplImage * myIm;

				if(cvIm->nChannels == 3) {
					myIm = cvIm;
				}
				else {
					printf("Size: %d %d\n",cvIm->width,cvIm->height);
					myIm = cvCreateImage(cvSize(cvIm->width,cvIm->height),8,3);
					cvCvtColor(cvIm, myIm, CV_GRAY2BGR);
				}

				unsigned char * rawData;
				int step = 0;
				CvSize roiSize;
				cvGetRawData( myIm, &rawData, &step, &roiSize );

				wxImage wxi = wxImage(myIm->width, myIm->height, rawData, true);
				bmap = wxBitmap(wxi.Scale(myIm->width,myIm->height));

				if(cvIm->nChannels != 3) {
					cvReleaseImage(&myIm);
				}

				update_image();
			}
			void OnMouse(wxMouseEvent& event) {
				printf("Location: %d, %d\n",event.GetX(),event.GetY());
				Painter::ThrowSkinnerSignal("Painter::UnwrapProbe");
				event.Skip();
			}
		private:
			DECLARE_EVENT_TABLE();
	};

	BEGIN_EVENT_TABLE(UnwrappedView, wxScrolledWindow)
		EVT_LEFT_DOWN(UnwrappedView::OnMouse)
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
			void init_window(IplImage * im) {
				wxCommandEvent wxevent(wxEVT_COMMAND_UNWRAP_WINDOW);
				wxevent.SetClientData(im);
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

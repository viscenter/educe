#include <StandAlone/Apps/Seg3D/UnwrapPlugin.h>
 
#include <sci_defs/insight_defs.h>

#include <string>
#include <Core/Events/BaseTool.h>
#include <Core/Datatypes/ITKDatatype.h>
#include <StandAlone/Apps/Seg3D/VolumeFilter.h>
#include <itkImageToImageFilter.h>
#include <itkCommand.h>
#include <itkOtsuThresholdImageFilter.h>

#include "highgui.h"

SET_PLUGIN_VERSION;

#define PI 3.14159
#define TORAD 1.0/PI

#define RADIAL_SAMPLES (720*2)

#define SOBEL_THRESH 100
#define MAX_LAYERS 1

#define THICKNESS 9

#define NUMUNWRAPS THICKNESS

namespace SCIRun {

class ScrollUnwrapPlugin : public UnwrapPlugin {
	public:
		virtual std::string get_menu_string() const {
			return "Scroll Unwrap Multiple";
		}
		virtual void menu_event() const {
			// printf("Hello from ExampleUnwrapPlugin\n");
		}
		BaseTool::propagation_state_e process_event(event_handle_t event) /*const*/ {
			// printf("Attempting to handle event inside plugin\n");
			// check for the FinishEvent signal generated by the start button
			if (dynamic_cast<FinishEvent *>(event.get_rep())) {
				if (!painter_->check_for_active_data_volume("Scroll Unwrapping filter"))
				{
					return STOP_E;
				}
				else if(!(painter_->mask_volume_.get_rep())) {
					painter_->set_status("Scroll Unwrapping filter requires a mask to be selected.");
					return STOP_E;
				}
			
				run_filter();
			}

			return CONTINUE_E;
		}
	private:
		CvPoint calc_ray_outer(float theta, CvPoint center)
		{
			CvPoint result;
			float hyp = (float)center.x;
			float dx = hyp * cos(theta);
			float dy = hyp * sin(theta);

			result.x = center.x - (int)dx;
			result.y = center.y - (int)dy;

			return result;
		}

		CvPoint get_coordinate_from_position(IplImage *image, CvLineIterator li, int p)
		{
			CvPoint result;
			int offset;

			for(int i = 0; i < p; i++) {
				CV_NEXT_LINE_POINT(li);
			}

			offset = li.ptr - (uchar*)(image->imageData);
			result.y = offset/image->widthStep;
			result.x = (offset - result.y * image->widthStep)/(sizeof(uchar));

			return result;
		}

		void radial_sample(int width, int height, int slices, label_type* data, float* ddata, CvMat *plookup, int slice)
		{
			IplImage *cvcast = cvCreateImage(cvSize(width, height),
					IPL_DEPTH_8U, 1);

			for(int y = 0; y < height; y++) {
				for(int x = 0; x < width; x++) {
					cvSet2D(cvcast,y,x,cvScalarAll(data[y*width+x]));
				}
			}
			
			//IplImage *cvcastd = cvCreateImage(cvSize(width, height),
			//		IPL_DEPTH_8U, 1);
			
			CvMat *cvcastd = cvCreateMat(height,width,CV_32FC1);
			for(int y = 0; y < height; y++) {
				for(int x = 0; x < width; x++) {
					cvSet2D(cvcastd,y,x,cvScalarAll(ddata[y*width+x]));
				}
			}

			if(slice == 0) {
			cvSaveImage("slicem.png",cvcast);
			// cvSaveImage("sliced.png",cvcastd);
			}
			int cx = width/2;
			int cy = height/2;

			CvPoint center = cvPoint(cx,cy);

			unsigned char* linebuf;
			float* dlinebuf;
			for(int sample = 0; sample < RADIAL_SAMPLES; sample++) {
				float theta = 0.785398163+((float)sample)*((2.0*PI)/(float)RADIAL_SAMPLES);
				CvPoint outer = calc_ray_outer(theta, center);

				// printf("%g:\t%d,%d\n", theta*(180.0/PI), outer.x, outer.y);
				cvClipLine(cvSize(width, height), &outer, &center);
				int linesize = abs(center.x-outer.x)+abs(center.y-outer.y)+1;
				
				linebuf = (unsigned char*)malloc(linesize);
				dlinebuf = (float*)malloc(sizeof(float)*linesize);
				
				cvSampleLine(cvcast,outer,center,linebuf,4);
				
				CvLineIterator curline;
				
				cvSampleLine(cvcastd,outer,center,dlinebuf,4);
				
				IplImage *castline = cvCreateImageHeader(cvSize(linesize,1), IPL_DEPTH_8U, 1);
				castline->imageData = (char*)linebuf;

				int layer = 0;
				for(int i = 0; (i < linesize) && (layer < MAX_LAYERS); i++) {
					if((int)cvGetReal1D(castline,i) > 0) {
						float max = 0;
						int	max_i = 0;
						int min = 255, min_i = 0;
						int j = i;
						for(; (i < linesize) && ((i-j) < THICKNESS); i++) {
							int maskval = (int)cvGetReal1D(castline,i);
							float curval = dlinebuf[i];
							if(maskval == 0) break;
							if(curval > max) {
								max = curval;
								max_i = i;
							}
							if(curval < min) {
								min = (int)curval;
								min_i = i;
							}
						}
						int sampledpos = j; // j+((i-j)/2);
						for(int unwrap_pos = 0; unwrap_pos < NUMUNWRAPS; unwrap_pos++, sampledpos++) {
							cvInitLineIterator(cvcast,outer,center,&curline,4);
							CvPoint sampledpoint = get_coordinate_from_position(cvcast,curline,sampledpos);
							
							//printf("T: %g, d: %d - %d,%d\n",theta,max_i,sampledpoint.x,sampledpoint.y);
							
							plookup->data.i[(unwrap_pos*3*plookup->width*slices)+slice*3*plookup->width+((layer*RADIAL_SAMPLES)+sample)*3+0] = sampledpoint.x;
							plookup->data.i[(unwrap_pos*3*plookup->width*slices)+slice*3*plookup->width+((layer*RADIAL_SAMPLES)+sample)*3+1] = sampledpoint.y;
							plookup->data.i[(unwrap_pos*3*plookup->width*slices)+slice*3*plookup->width+((layer*RADIAL_SAMPLES)+sample)*3+2] = slice;
						}
						
						//cvSet2D(plookup,slice,(layer*RADIAL_SAMPLES)+sample,cvScalar(0,0,slice));
						
						layer++;
						// i = j + THICKNESS;
						
						while((i < linesize) && ((int)cvGetReal1D(castline,i)>0))
							i++;
					}
				}
				cvReleaseImageHeader(&castline);

				free(linebuf);
				free(dlinebuf);
			}
			cvReleaseImage(&cvcast);
			cvReleaseMat(&cvcastd);
		}
		virtual void run_filter() {	
			printf("Scroll Unwrapping run\n");

			painter_->set_status("Running scroll unwrapping filter.");

			NrrdDataHandle source_label = painter_->mask_volume_->nrrd_handle_;
			NrrdDataHandle source_data = painter_->current_volume_->nrrd_handle_;


			const label_type mlabel = painter_->mask_volume_->label_;

			const size_t labelsize = VolumeOps::nrrd_elem_count(source_label);
			const size_t datasize = VolumeOps::nrrd_elem_count(source_data);

			ASSERT(labelsize == datasize);
			ASSERT(source_data->nrrd_->type == nrrdTypeFloat);
			ASSERT(source_label->nrrd_->type == LabelNrrdType);

			label_type *labeld = (label_type*)source_label->nrrd_->data;
			float *srcd = (float*)source_data->nrrd_->data;
			
			int width = source_data->nrrd_->axis[1].size,
				height = source_data->nrrd_->axis[2].size,
				slices = source_data->nrrd_->axis[3].size;

			CvMat *plookup = cvCreateMat(slices*NUMUNWRAPS,RADIAL_SAMPLES*MAX_LAYERS,CV_32SC3);
			cvSet(plookup,cvScalarAll(0));
	
			for(int i = 0; i < slices; i++) {
				radial_sample(width, height, slices, labeld+(width * height * i), srcd+(width * height * i), plookup, i);
				painter_->update_progress((int)(((float)i/(float)slices)*100));
			}

			struct Unwrapping *unwrapping = (struct Unwrapping*)malloc(sizeof(struct Unwrapping));
			unwrapping->point_lookup = plookup;
			unwrapping->num_unwraps = NUMUNWRAPS;
			unwrapping->num_slices = slices;
			unwrapping->referenced_volume = source_data->nrrd_;

			printf("Ref vol head: %x\n",*(unwrapping->referenced_volume));

			init_window(unwrapping);
			printf("Init'd win\n");
			
			painter_->update_progress(100);
			painter_->extract_all_window_slices();
			painter_->redraw_all();
			
			painter_->hide_tool_panel();
		}
};

EXPORT_CREATE_FUNCTION(UnwrapPlugin, ScrollUnwrapPlugin)
EXPORT_DELETE_FUNCTION(UnwrapPlugin, ScrollUnwrapPlugin)

}
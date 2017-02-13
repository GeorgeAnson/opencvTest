
/*
*稀疏光流
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>
static const double pi = 3.14159265358979323846;

inline static double square(int a)
{
	return a * a;
}

/*这仅仅是一个行内分配图像。我这样做是在实际的计算机视觉算法的代码，
以减少混乱。基本上它分配请求的图像，除非该图像已经非NULL。为正的，
即使该图像的大小，深度和/或信道比的要求是不同的，它总是会留下一个非NULL图像。*/

inline static void allocateOnDemand(IplImage **img, CvSize size, int depth, int channels)
{
	if (*img != NULL) return;
	*img = cvCreateImage(size, depth, channels);
	if (*img == NULL)
	{
		fprintf(stderr, "Error: Couldn't allocate image. Out of memory?\n");
		exit(-1);
	}
}

int main(void)
{
	/*创建一个对象，解码输入的视频数据流。*/
	CvCapture *input_video = cvCaptureFromFile("C:\\Users\\Administrator.SC-201604111342\\Desktop\\pe.mp4");
	if (input_video == NULL)
	{
		fprintf(stderr, "Error: Can't open video.\n");
		return -1;
	}

	cvQueryFrame(input_video);

	CvSize frame_size;
	frame_size.height = (int)cvGetCaptureProperty(input_video, CV_CAP_PROP_FRAME_HEIGHT);
	frame_size.width = (int)cvGetCaptureProperty(input_video, CV_CAP_PROP_FRAME_WIDTH);

	long number_of_frames;

	cvSetCaptureProperty(input_video, CV_CAP_PROP_POS_AVI_RATIO, 1.);

	number_of_frames = (int)cvGetCaptureProperty(input_video, CV_CAP_PROP_POS_FRAMES);

	cvSetCaptureProperty(input_video, CV_CAP_PROP_POS_FRAMES, 0.);

	cvNamedWindow("Optical Flow", CV_WINDOW_AUTOSIZE);
	long current_frame = 0;
	while (true)
	{
		static IplImage *frame = NULL, *frame1 = NULL, *frame1_1C = NULL, *frame2_1C =
			NULL, *eig_image = NULL, *temp_image = NULL, *pyramid1 = NULL, *pyramid2 = NULL;

		cvSetCaptureProperty(input_video, CV_CAP_PROP_POS_FRAMES, current_frame);

		frame = cvQueryFrame(input_video);
		if (frame == NULL)
		{

			fprintf(stderr, "Error: Hmm. The end came sooner than we thought.\n");
			return -1;
		}


		allocateOnDemand(&frame1_1C, frame_size, IPL_DEPTH_8U, 1);


		cvConvertImage(frame, frame1_1C, CV_CVTIMG_FLIP);


		allocateOnDemand(&frame1, frame_size, IPL_DEPTH_8U, 3);
		cvConvertImage(frame, frame1, CV_CVTIMG_FLIP);


		frame = cvQueryFrame(input_video);
		if (frame == NULL)
		{
			fprintf(stderr, "Error: Hmm. The end came sooner than we thought.\n");
			return -1;
		}
		allocateOnDemand(&frame2_1C, frame_size, IPL_DEPTH_8U, 1);
		cvConvertImage(frame, frame2_1C, CV_CVTIMG_FLIP);

		allocateOnDemand(&eig_image, frame_size, IPL_DEPTH_32F, 1);
		allocateOnDemand(&temp_image, frame_size, IPL_DEPTH_32F, 1);


		CvPoint2D32f frame1_features[400];



		int number_of_features;

		number_of_features = 400;


		cvGoodFeaturesToTrack(frame1_1C, eig_image, temp_image, frame1_features, &number_of_features, .01, .01, NULL);


		CvPoint2D32f frame2_features[400];


		char optical_flow_found_feature[400];


		float optical_flow_feature_error[400];


		CvSize optical_flow_window = cvSize(3, 3);


		CvTermCriteria optical_flow_termination_criteria = cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3);


		allocateOnDemand(&pyramid1, frame_size, IPL_DEPTH_8U, 1);
		allocateOnDemand(&pyramid2, frame_size, IPL_DEPTH_8U, 1);


		cvCalcOpticalFlowPyrLK(frame1_1C, frame2_1C, pyramid1, pyramid2, frame1_features,
			frame2_features, number_of_features, optical_flow_window, 5,
			optical_flow_found_feature, optical_flow_feature_error,
			optical_flow_termination_criteria, 0);


		for (int i = 0; i < number_of_features; i++)
		{

			if (optical_flow_found_feature[i] == 0) continue;
			int line_thickness; line_thickness = 1;

			CvScalar line_color; line_color = CV_RGB(255, 0, 0);

			CvPoint p, q;
			p.x = (int)frame1_features[i].x;
			p.y = (int)frame1_features[i].y;
			q.x = (int)frame2_features[i].x;
			q.y = (int)frame2_features[i].y;
			double angle; angle = atan2((double)p.y - q.y, (double)p.x - q.x);
			double hypotenuse; hypotenuse = sqrt(square(p.y - q.y) + square(p.x - q.x));

			q.x = (int)(p.x - 3 * hypotenuse * cos(angle));
			q.y = (int)(p.y - 3 * hypotenuse * sin(angle));

			cvLine(frame1, p, q, line_color, line_thickness, CV_AA, 0);

			p.x = (int)(q.x + 9 * cos(angle + pi / 4));
			p.y = (int)(q.y + 9 * sin(angle + pi / 4));
			cvLine(frame1, p, q, line_color, line_thickness, CV_AA, 0);
			p.x = (int)(q.x + 9 * cos(angle - pi / 4));
			p.y = (int)(q.y + 9 * sin(angle - pi / 4));
			cvLine(frame1, p, q, line_color, line_thickness, CV_AA, 0);
		}

		cvShowImage("Optical Flow", frame1);

		int key_pressed;
		key_pressed = cvWaitKey(33);

		if (key_pressed == 'b' || key_pressed == 'B') current_frame--;
		else current_frame++;

		if (current_frame < 0) current_frame = 0;
		if (current_frame >= number_of_frames - 1) current_frame = number_of_frames - 2;
	}
}




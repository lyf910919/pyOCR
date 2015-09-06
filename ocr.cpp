#include "ocr.h"
using namespace std;
using namespace cv;

const double OCR::RESIZERATIO = 5;
const int OCR::THRESH = 180;
const int OCR::AREA_THRESH = 50;
const int OCR::MARGIN_THRESH = 10;
const int OCR::GAP_THRESH = 15;

OCR::OCR()
{
	///initiate tesseract engine
	api = new tesseract::TessBaseAPI();
	char *configs[] = {"myconfig"};
	int configs_size = 1;
	if (!api -> SetVariable("use_definite_ambigs_for_classifier", "1"))
	{
		cerr << "Unable to set use_definite_ambigs_for_adaption" <<endl;
		exit(-1);
	}
	if (!api -> SetVariable("use_ambigs_for_adaption", "1"))
	{
		cerr << "Unable to set use_ambigs_for_adaption" <<endl;
		exit(-1);
	}

	if (api -> Init(NULL, "eng2+fra2+deu2+ita2", tesseract::OEM_DEFAULT, configs, configs_size, NULL, NULL, false))
	{
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(-1);
	}
}

OCR::~OCR()
{
	api -> End();
}

void OCR::preprocess(Mat src, Mat & dst)
{
	if (!src.data)
	{
		cerr << "Preprocess: No image data!" << endl;
		return;
	}

	resize(src, dst, Size(0,0), RESIZERATIO, RESIZERATIO, CV_INTER_CUBIC);
	// imshow("resize", dst);
	// waitKey();

	////threshold each channel seperately, better removing noise
	vector<Mat> chans(3), dest(3);
	split(dst, chans);
	for (int i = 0; i < 3; ++i)
	{
		threshold(chans[i], dest[i], THRESH, 255, THRESH_BINARY_INV);
		// imshow(to_string(i)+"thresh", dest[i]);
		// waitKey();
	}
	Mat comb;
	dest[0].copyTo(comb);
	for (int i = 1; i < chans.size(); ++i)
	{
		bitwise_and(comb, dest[i], comb);
	}
	// imshow("comb", comb);
	// waitKey(0);
	comb.copyTo(dst);

	// cvtColor(dst, dst, COLOR_BGR2GRAY);
	// threshold(dst, dst, THRESH, 255, THRESH_BINARY_INV);
	// imshow("thresh", dst);
	// waitKey();

	// dst = dst(Rect(0, 3, dst.cols, dst.rows-3));
	// threshold(dst, dst, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	////get horizontal projection, and remove margin noise 
	Mat projection;
	reduce(dst, projection, 0, CV_REDUCE_MAX);
	int cnt = 0, leftMargin = 0, rightMargin = projection.cols-1;
	// cout << projection << endl;
	for (int i = 0; i < projection.cols / 6; ++i)
	{
		// cout << (int)projection.at<uchar>(0, i) << endl;
		if (projection.at<uchar>(0, i) == 0)
		{
			++cnt;
		}
		else
		{
			if (cnt > GAP_THRESH)
			{
				leftMargin = i;
			}
			cnt = 0;
		}
	}
	cnt = 0;
	for (int i = projection.cols - 1; i >= projection.cols * 5 / 6; --i)
	{
		if (projection.at<uchar>(0, i) == 0)
		{
			++cnt;
		}
		else
		{
			if (cnt > GAP_THRESH)
			{
				rightMargin = i;
			}
			cnt = 0;
		}
	}
	// cout << leftMargin << " " << rightMargin << endl;
	rectangle(dst, Rect(0, 0, leftMargin-1, dst.rows), Scalar::all(0), CV_FILLED);
	rectangle(dst, Rect(rightMargin+1, 0, dst.cols-1, dst.rows), Scalar::all(0), CV_FILLED);
	// imshow("filled", dst);
	// waitKey(0);

	////only fill small regions near margin
	// Mat cp;
	// dst.copyTo(cp);
	// vector<vector<Point> > contours;
	// vector<Vec4i> hierarchy;
	// Mat canvas = Mat::zeros(dst.size(), CV_8UC3);
	// findContours(cp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	// for (int i = 0; i < contours.size(); ++i)
	// {
	// 	int area = contourArea(contours[i]);
	// 	cout << area << endl;
	// 	// if (area < AREA_THRESH)
	// 	// {
	// 		int l = contours[i][0].x, r = contours[i][0].x, 
	// 		u = contours[i][0].y, d = contours[i][0].y;
	// 		for (int j = 1; j < contours[i].size(); ++j)
	// 		{
	// 			if (contours[i][j].x < l) l = contours[i][j].x;
	// 			if (contours[i][j].x > r) r = contours[i][j].x;
	// 			if (contours[i][j].y < u) u = contours[i][j].y;
	// 			if (contours[i][j].y > d) d = contours[i][j].y;
	// 		}
	// 		cout << "height: " << d - u << endl;
	// 		// if (l < MARGIN_THRESH || src.cols - r < MARGIN_THRESH ||
	// 		// 	u < MARGIN_THRESH || src.rows - d < MARGIN_THRESH)
	// 		// {
	// 			drawContours(canvas, contours, i, Scalar::all(255), CV_FILLED);
	// 			imshow("contours", canvas);
	// 			waitKey();
	// 			// drawContours(dst, contours, i, Scalar::all(0), CV_FILLED);
	// 			// drawContours(dst, contours, i, Scalar::all(0), 2);
	// 		// }
	// 	// }
		
	// }

	////floodFill margin white area
	// for (int i = 0; i < dst.rows; ++i)
	// {
	// 	if (dst.at<uchar>(i, 0) != 0)
	// 		floodFill(dst, Mat(), Point(0,i), Scalar::all(0));
	// 	if (dst.at<uchar>(i, dst.cols-1) != 0)
	// 		floodFill(dst, Mat(), Point(dst.cols-1, i), Scalar::all(0));
	// }
	// for (int i = 0; i < dst.cols; ++i)
	// {
	// 	if (dst.at<uchar>(0, i) != 0)
	// 		floodFill(dst, Mat(), Point(i,0), Scalar::all(0));
	// 	if (dst.at<uchar>(dst.rows-1, i) != 0)
	// 		floodFill(dst, Mat(), Point(i,dst.rows-1), Scalar::all(0));
	// }
	

	
	
	// imshow("bin", dst);
	// waitKey(0);

	//erode binary image
	// erode(dst, dst, Mat());
	// imshow("dilate", dst);
	// waitKey(0);
}

void OCR::preprocess(Mat src, Mat & dst, Rect mask)
{
	if (!src.data)
	{
		cerr << "Preprocess: No image data!" << endl;
		return;
	}

	resize(src, dst, Size(0,0), RESIZERATIO, RESIZERATIO);
	cvtColor(dst, dst, COLOR_BGR2GRAY);
	threshold(dst, dst, THRESH, 255, THRESH_BINARY_INV);

	///delete poi label in binary image
	Rect resizedRect(mask.x * RESIZERATIO, mask.y * RESIZERATIO,
		mask.width * RESIZERATIO, mask.height * RESIZERATIO);
	rectangle(dst, resizedRect, Scalar::all(0), CV_FILLED);

	// imshow("bin", dst);
	// waitKey(0);
}

string OCR::getText(Mat src)
{
	if (!src.data)
	{
		cerr << "getText: No image data!" << endl;
		return string("");
	}

	// tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	// char *configs[] = {"myconfig"};
	// int configs_size = 1;
	// if (!api -> SetVariable("use_definite_ambigs_for_classifier", "1"))
	// {
	// 	cerr << "Unable to set use_definite_ambigs_for_adaption" <<endl;
	// 	exit(-1);
	// }
	// if (!api -> SetVariable("use_ambigs_for_adaption", "1"))
	// {
	// 	cerr << "Unable to set use_ambigs_for_adaption" <<endl;
	// 	exit(-1);
	// }

	// if (api -> Init(NULL, "eng2+fra2+deu2+ita2", tesseract::OEM_DEFAULT, configs, configs_size, NULL, NULL, false))
	// {
	// 	fprintf(stderr, "Could not initialize tesseract.\n");
	// 	exit(-1);
	// }
	



	Mat dst;
	preprocess(src, dst);

	api -> SetImage((uchar*)dst.data, dst.size().width, dst.size().height, 
		dst.channels(), dst.step1());
	
	//Get OCR result
	char *outText = api -> GetUTF8Text();
	string text = outText;

	// api -> End();
	delete [] outText;
	//pixDestroy(&image);
	return text;
}

string OCR::getText(Mat src, Rect mask)
{
	if (!src.data)
	{
		cerr << "getText: No image data!" << endl;
		return string("");
	}

	// tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();

	// if (api -> Init(NULL, "eng2+fra+ita+deu"))
	// {
	// 	fprintf(stderr, "Could not initialize tesseract.\n");
	// 	exit(-1);
	// }

	Mat dst;
	preprocess(src, dst, mask);

	api -> SetImage((uchar*)dst.data, dst.size().width, dst.size().height, 
		dst.channels(), dst.step1());
	
	//Get OCR result
	char *outText = api -> GetUTF8Text();
	string text = outText;

	// api -> End();
	delete [] outText;
	//pixDestroy(&image);
	return text;
}
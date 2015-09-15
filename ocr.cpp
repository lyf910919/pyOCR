#include "ocr.h"
#include <cmath>
#define LOG 1
using namespace std;
using namespace cv;

const double OCR::RESIZERATIO = 5;
const int OCR::THRESH = 190;
const int OCR::AREA_THRESH = 50;
const int OCR::MARGIN_THRESH = 10;
const int OCR::GAP_THRESH = 15;
const int OCR::COLOR_THRESH = 65;

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
	api -> SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);

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
	Mat resized;
	dst.copyTo(resized);
	if (LOG)
	{
		imshow("resize", dst);
		waitKey();
	}
	

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
	if (LOG)
	{
		imshow("comb", comb);
		waitKey(0);
	}
	
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
				break;
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
				break;
			}
			cnt = 0;
		}
	}
	// cout << leftMargin << " " << rightMargin << endl;
	rectangle(dst, Rect(0, 0, leftMargin-1, dst.rows), Scalar::all(0), CV_FILLED);
	rectangle(dst, Rect(0, 0, leftMargin-1, dst.rows), Scalar::all(0), 2);
	rectangle(dst, Rect(rightMargin+1, 0, dst.cols-1, dst.rows), Scalar::all(0), CV_FILLED);
	rectangle(dst, Rect(rightMargin+1, 0, dst.cols-1, dst.rows), Scalar::all(0), 2);
	
	// imshow("filled", dst);
	// waitKey(0);

	////get average color of white area
	Scalar avgColor = getMaskColor(resized, dst);
	if (LOG)
	{
		cout << "avgColor: " << avgColor << endl;
	}


	//remove connected components whose colors are far from average
	Mat cp;
	dst.copyTo(cp);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(cp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < contours.size(); ++i)
	{
		Mat canvas = Mat::zeros(dst.size(), CV_8UC1);
		int area = contourArea(contours[i]);
		if (LOG)
		{
			cout << "contour area: " << area << endl;
		}
		if (hierarchy[i][3] < 0)
		{
			drawContours(canvas, contours, i, Scalar::all(255), CV_FILLED);
			if (hierarchy[i][2] >= 0)
			{
				int holeInd = hierarchy[i][2];
				do
				{
					drawContours(canvas, contours, holeInd, Scalar::all(0), CV_FILLED);
					holeInd = hierarchy[holeInd][0];
				}while(holeInd >= 0);
			}
			Scalar contourColor = getMaskColor(resized, canvas);
			Scalar distVec = contourColor - avgColor;
			double dist = 0;
			for (int i = 0; i < 3; ++i)
			{
				dist += distVec[i] * distVec[i];
			}
			dist = sqrt(dist);
			if (dist > COLOR_THRESH)
			{
				drawContours(dst, contours, i, Scalar::all(0), CV_FILLED);
				drawContours(dst, contours, i, Scalar::all(0), 2);
			}
			if (LOG)
			{
				Moments mu = moments(contours[i], true);
				float center_x = mu.m10 / MAX(1, mu.m00);
				float center_y = mu.m01 / MAX(1, mu.m00);
				cout << "contour color: " << contourColor << endl;
				cout << "dist: " << dist << endl;
				cout << "center: " << center_x << " " << center_y << endl;
				cout << "m00: " << mu.m00 << endl;
				imshow("contours", canvas);
				waitKey();
			}

		}		
	}

	////remove connected components whose mass center in margin
	for (int i = 0; i < contours.size(); ++i)
	{
		Moments mu = moments(contours[i], true);
		float center_x = mu.m10 / MAX(1, mu.m00);
		float center_y = mu.m01 / MAX(1, mu.m00);
		if (center_x < MARGIN_THRESH || dst.cols - center_x < MARGIN_THRESH ||
			center_y < MARGIN_THRESH || dst.rows - center_y < MARGIN_THRESH)
		{
			drawContours(dst, contours, i, Scalar::all(0), CV_FILLED);
			drawContours(dst, contours, i, Scalar::all(0), 2);
		}
	}

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
	

	
	if (LOG)
	{
		imshow("bin", dst);
		waitKey(0);
	}
	

	//erode binary image
	// erode(dst, dst, Mat());
	// imshow("dilate", dst);
	// waitKey(0);
}

void OCR::preprocess(Mat src, Mat & dst, Rect mask)
{
	preprocess(src, dst);

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

//src is resize src image, mask is binary image with type CV_8UC1
Scalar OCR::getMaskColor(cv::Mat src, cv::Mat mask)
{
	Mat product(src.rows, src.cols, CV_32FC3);
	vector<Mat> bin3vec;
	for (int i = 0; i < 3; ++i)
	{
		bin3vec.push_back(mask);
	}
	Mat bin3;
	merge(bin3vec, bin3);
	bin3.convertTo(bin3, CV_32FC3);
	divide(bin3, 255, bin3);
	src.convertTo(src, CV_32FC3);
	multiply(src, bin3, product);
	Scalar avgColor;
	divide(sum(product), sum(bin3), avgColor);
	return avgColor;
}
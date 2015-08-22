#include "ocr.h"
using namespace std;
using namespace cv;

const double OCR::RESIZERATIO = 5;
const int OCR::THRESH = 180;

OCR::OCR(Mat _src)
{
	if (!_src.data)
	{
		cerr << "Constructor: No image data!" << endl;
	}
	else
	{
		_src.copyTo(src);
	}
	text = "";
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
	cvtColor(dst, dst, COLOR_BGR2GRAY);
	threshold(dst, dst, THRESH, 255, THRESH_BINARY_INV);

	// dst = dst(Rect(0, 3, dst.cols, dst.rows-3));
	// threshold(dst, dst, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	///floodFill margin white area
	for (int i = 0; i < dst.rows; ++i)
	{
		if (dst.at<uchar>(i, 0) != 0)
			floodFill(dst, Mat(), Point(0,i), Scalar::all(0));
		if (dst.at<uchar>(i, dst.cols-1) != 0)
			floodFill(dst, Mat(), Point(dst.cols-1, i), Scalar::all(0));
	}
	for (int i = 0; i < dst.cols; ++i)
	{
		if (dst.at<uchar>(0, i) != 0)
			floodFill(dst, Mat(), Point(i,0), Scalar::all(0));
		if (dst.at<uchar>(dst.rows-1, i) != 0)
			floodFill(dst, Mat(), Point(i,dst.rows-1), Scalar::all(0));
	}
	

	
	
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

string OCR::getText()
{
	if (!src.data)
	{
		cerr << "getText: No image data!" << endl;
		return string("");
	}

	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	char *configs[] = {"myconfig"};
	int configs_size = 1;

	if (api -> Init(NULL, "eng2+fra2+ita2+deu2", tesseract::OEM_DEFAULT, configs, configs_size, NULL, NULL, false))
	{
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(-1);
	}
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



	Mat dst;
	preprocess(src, dst);

	api -> SetImage((uchar*)dst.data, dst.size().width, dst.size().height, 
		dst.channels(), dst.step1());
	
	//Get OCR result
	char *outText = api -> GetUTF8Text();
	this -> text = outText;

	api -> End();
	delete [] outText;
	//pixDestroy(&image);
	return this -> text;
}

string OCR::getText(Rect mask)
{
	if (!src.data)
	{
		cerr << "getText: No image data!" << endl;
		return string("");
	}

	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();

	if (api -> Init(NULL, "eng2+fra+ita+deu"))
	{
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(-1);
	}

	Mat dst;
	preprocess(src, dst, mask);

	api -> SetImage((uchar*)dst.data, dst.size().width, dst.size().height, 
		dst.channels(), dst.step1());
	
	//Get OCR result
	char *outText = api -> GetUTF8Text();
	this -> text = outText;

	api -> End();
	delete [] outText;
	//pixDestroy(&image);
	return this -> text;
}
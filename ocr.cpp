#include "ocr.h"
using namespace std;
using namespace cv;

const double OCR::RESIZERATIO = 3;
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

	resize(src, dst, Size(0,0), RESIZERATIO, RESIZERATIO);
	cvtColor(dst, dst, COLOR_BGR2GRAY);
	threshold(dst, dst, THRESH, 255, THRESH_BINARY_INV);
	
	// imshow("bin", dst);
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

	if (api -> Init(NULL, "eng"))
	{
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(-1);
	}

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

	if (api -> Init(NULL, "eng"))
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
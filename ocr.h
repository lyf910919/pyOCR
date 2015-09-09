#ifndef OCR_H
#define OCR_H
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <iostream>

class OCR
{
public:
	OCR();
	std::string getText(cv::Mat src);
	std::string getText(cv::Mat src, cv::Rect mask);
	~OCR();

private:
	tesseract::TessBaseAPI *api;
	void preprocess(cv::Mat src, cv::Mat & dst);
	void preprocess(cv::Mat src, cv::Mat & dst, cv::Rect mask);
	cv::Scalar getMaskColor(cv::Mat src, cv::Mat mask);
	static const double RESIZERATIO;
	static const int THRESH;
	static const int AREA_THRESH;
	static const int MARGIN_THRESH;
	static const int GAP_THRESH;
	static const int COLOR_THRESH;
};

#endif
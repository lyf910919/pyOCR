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
	OCR(cv::Mat src);
	std::string getText();
	std::string getText(cv::Rect mask);
	~OCR(){};

private:
	cv::Mat src;
	std::string text;
	void preprocess(cv::Mat src, cv::Mat & dst);
	void preprocess(cv::Mat src, cv::Mat & dst, cv::Rect mask);
	static const double RESIZERATIO;
	static const int THRESH;
};

#endif
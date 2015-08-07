import sys
sys.path.append('/usr/local/lib/python2.7/site-packages')
import pytesseract
from PIL import Image
import cv2

def ocr(img, language):
	print pytesseract.image_to_string(img, lang=language)

def prepro(src, resize_ratio, bin_thresh):
	src = cv2.resize(src, (0,0), fx=resize_ratio, fy=resize_ratio)
	cv2.imshow("resize", src)
	cv2.waitKey(0)
	src = cv2.cvtColor(src, cv2.COLOR_BGR2GRAY)
	a, dst = cv2.threshold(src, bin_thresh, 255, cv2.THRESH_BINARY_INV)
	cv2.namedWindow("thresh", cv2.WINDOW_NORMAL)
	cv2.imshow("thresh", dst)
	cv2.waitKey(0)
	return dst


img = cv2.imread(sys.argv[1])
dst = prepro(img, 3, 180)
dst = Image.fromarray(dst)
# img = Image.open(sys.argv[1])
language = "eng"
if len(sys.argv) > 2:
	language = str(sys.argv[2])
ocr(dst, language)

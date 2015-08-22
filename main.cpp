#include "ocr.h"
#include <iostream>
#include <fstream>
#include <locale>
using namespace cv;
using namespace std;

vector<string> split(string s);
double getScore(vector<string> a, vector<string> b);
double getWordScore(string a, string b);
string join(vector<string> sVec);
string tailRemove(string a);

int amain(int argc, char** argv)
{
	Mat src = imread(argv[1], IMREAD_COLOR);
	OCR ocr(src);
	string ocrText = ocr.getText();
	cout << ocrText << endl;
	cout << tailRemove(ocrText) << endl;
	return 0;
}

int main(int argc, char** argv)
{
	double allScore = 0;
	for (int i = 1; i <= 200; ++i)
	{
		Mat src = imread(string("textImg/")+to_string(i)+".png", IMREAD_COLOR);
		OCR ocr(src);
		string ocrText = ocr.getText();
		ocrText = tailRemove(ocrText);
		//cout << ocrText << endl;
		vector<string> ocrVec = split(ocrText);
		ocrText = join(ocrVec);
		//cout << ocrText << endl;
		// for (int i = 0; i < ocrVec.size(); ++i)
		// 	cout << ocrVec[i] << " ";
		// cout << endl;
		
		ifstream infile(string("text/")+to_string(i)+".txt");
		string read(""), temp;
		while (getline(infile, temp))
		{
			read += temp;
		}
		infile.close();
		//cout << read << endl;
		vector<string> readVec = split(read);
		read = join(readVec);
		//cout << read << endl;
		// for (int i = 0; i < readVec.size(); ++i)
		// 	cout << readVec[i] << " ";
		// cout << endl;
		//double score = getScore(ocrVec, readVec);
		double score = getWordScore(ocrText, read);
		if (score < 1)
		{
			cout << i << endl;
			cout << ocrText << endl;
			cout << read << endl;
			cout << "score: " << score << endl;
		}
		
		allScore += score; 
	}
	allScore /= 200;
	cout << "avg score: " << allScore << endl;
	return 0;
}

vector<string> split(string s)
{
	s = string(" ") + s + " ";
	int i = 0, j = 0;
	bool inword = false;
	vector<string> ret;
	for (i = 0; i < s.length(); ++i)
	{
		if (s[i] == ' ' || s[i] == '\n')
		{
			if (inword)
			{
				ret.push_back(s.substr(j, i - j));
				inword = false;
			}
		}
		else
		{
			if (!inword)
			{
				j = i;
				inword = true;
			}
		}
	}
	return ret;
}

string join(vector<string> sVec)
{
	string ret("");
	for (int i = 0; i < sVec.size(); ++i)
	{
			ret += sVec[i] + " ";
	}
	ret = ret.substr(0, ret.length() - 1);
	return ret;
}

double getScore(vector<string> a, vector<string> b)
{
	int m = a.size(), n = b.size();
	double ** score = new double*[m];
	for (int i = 0; i < m; ++i)
		score[i] = new double[n];

	for (int i = 0; i < n; ++i)
	{
		// score[0][i] = !a[0].compare(b[i]);
		score[0][i] = getWordScore(a[0], b[i]);
	}
	for (int i = 0; i < m; ++i)
	{
		// score[i][0] = !a[i].compare(b[0]);
		score[i][0] = getWordScore(a[i], b[0]);
	}

	for (int i = 1; i < m; ++i)
	{
		for (int j = 1; j < n; ++j)
		{
			// int curScore = !a[i].compare(b[j]);
			double curScore = getWordScore(a[i], b[j]);
			score[i][j] = MAX(MAX(score[i-1][j], score[i][j-1]), 
				score[i-1][j-1]+curScore);
		}
	}
	double ret = 1.0 * score[m-1][n-1] / MAX(m,n);

	for (int i = 0; i < m; ++i)
		delete [] score[i];
	delete [] score;
	return ret;
}

double getWordScore(string a, string b)
{
	int m = a.length(), n = b.length();
	int ** score = new int*[m];
	for (int i = 0; i < m; ++i)
		score[i] = new int[n];

	double ret = 0;
	for (int i = 0; i < m; ++i)
		score[i][0] = (a[i] == b[0] ? 1 : 0);
	for (int j = 0; j < n; ++j)
		score[0][j] = (a[0] == b[j] ? 1 : 0);
	for (int i = 1; i < m; ++i)
	{
		for (int j = 1; j < n; ++j)
		{
			score[i][j] = MAX(MAX(score[i-1][j], score[i][j-1]), 
				score[i-1][j-1] + (a[i] == b[j] ? 1 : 0));
		}
	}
	ret = 1.0 * score[m-1][n-1] / MAX(m, n);

	for (int i = 0; i < m; ++i)
		delete [] score[i];
	delete [] score;

	return ret;
}

string tailRemove(string s)
{
	locale loc("en_US.UTF-8");
	vector<string> ocrVec = split(s);
	vector<string>::iterator it;
	for (it = ocrVec.begin(); it != ocrVec.end(); ++it)
	{
		bool valid = false;
		for (int j = 0; j < it->size(); ++j)
		{
			if (!ispunct((*it)[j], loc))
			{
				valid = true;
				break;
			}
		}
		if (valid) 
			break;
		else
		{
			it = ocrVec.erase(it);
		}
	}
	for (it = ocrVec.end() - 1; it != ocrVec.begin() - 1; --it)
	{
		bool valid = false;
		for (int j = 0; j < it->size(); ++j)
		{
			if (!ispunct((*it)[j], loc))
			{
				valid = true;
				break;
			}
		}
		if (valid) 
			break;
		else
		{
			it = ocrVec.erase(it);
		}
	}
	string ret = join(ocrVec);
	return ret;
}
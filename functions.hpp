#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <opencv2\opencv.hpp>
#include <iostream>
#include <fstream>
#include <array>

using namespace std;
using namespace cv;

#define AVERAGE 0
#define MODE 1
#define MEDIAN 2

const int ROWS = 240;
const int COLUMNS = 320;

// Parameters
const int ITERATIONS = 100;
// const array<int, 13> INDEXES = { 0, 3, 28, 53, 78, 103, 128, 153, 178, 203, 228, 253, 255 };
const array<int, 28> INDEXES = { 0, 3, 13, 23, 33, 43, 53, 63, 73, 83, 93, 103, 113, 123, 133, 143, 153, 163, 173, 183, 193, 203, 213, 223, 233, 243, 253, 255 };

const float THRESHOLD = 20;
const float ALPHA = 0.2;

const array<Vec3b, 6> COLORS = { Vec3b(255, 0, 0), Vec3b(0, 255, 0), Vec3b(0, 0, 255), Vec3b(255, 255, 0), Vec3b(255, 0, 255), Vec3b(0, 255, 255) };
// ---

// Data structures
class Interval {
	public:
		int start;
		int end;

		int count;
};

class Pixel {
	public:
		vector<int> values;
		vector<Interval> intervals;
};

extern Pixel pixels[ROWS][COLUMNS];

class Object {
	public:
		int label;

		int area;
		int perimeter;
		string classification;
	
		Vec3b color;
};
// ---

Mat toGrayScale(Mat currentFrame);

Mat processBackground(VideoCapture stream, int mode);
Mat averageBackground(VideoCapture stream);
Mat modeBackground(VideoCapture stream);
Mat medianBackground(VideoCapture stream);

Mat twoFrameDifference(Mat previousFrame, Mat currentFrame);
Mat updateBackground(Mat background, Mat currentFrame, Mat mask);

Mat open(Mat mask, int size);
Mat close(Mat mask, int size);
Mat contour(Mat mask, int size);

Mat blobFiltering(Mat mask);
Mat blobDetection(Mat mask, Mat currentFrame, int frameIndex);

#endif
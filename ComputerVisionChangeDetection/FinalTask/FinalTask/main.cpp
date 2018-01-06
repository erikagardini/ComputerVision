#include "functions.hpp"

int main() {
	string file = "video.avi";
	VideoCapture stream(file);

	Mat background = Mat::zeros(ROWS, COLUMNS, CV_8UC1);
	background = processBackground(stream, MODE);

	imshow("background", background);
	waitKey(1000);

	Mat currentFrame = Mat::zeros(ROWS, COLUMNS, CV_8UC1);
	int frameIndex = 0;

	while ( stream.read(currentFrame) ) {
		currentFrame = toGrayScale(currentFrame);
		Mat mask = twoFrameDifference(background, currentFrame);

		mask = blobFiltering(mask);
		mask = close(mask, 10);
		mask = open(mask, 5);

		Mat output = blobDetection(mask, currentFrame, frameIndex);
		imshow("output", output);		
		waitKey(1);

		background = updateBackground(background, currentFrame, mask);
		frameIndex = frameIndex + 1;
	}
	
	stream.release();
	destroyAllWindows();
	return (0);
}
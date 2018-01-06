#include "functions.hpp"

Pixel pixels[ROWS][COLUMNS];

Mat toGrayScale(Mat currentFrame) {
	Mat newFrame = Mat::zeros(ROWS, COLUMNS, CV_8UC1);

	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLUMNS; j++) {
			Vec3b rgb = currentFrame.at<Vec3b>(i, j);
			int grey = (rgb.val[0] + rgb.val[1] + rgb.val[2]) / 3;
			newFrame.at<unsigned char>(i, j) = grey;
		}
	}

	return (newFrame);
}

Mat processBackground(VideoCapture stream, int mode) {
	if ( mode == AVERAGE )
		return ( averageBackground(stream) );
	if ( mode == MODE )
		return ( modeBackground(stream) );
	if ( mode == MEDIAN )
		return ( medianBackground(stream) );

	Mat null = Mat::zeros(ROWS, COLUMNS, CV_8UC1);
	return (null);
}

Mat averageBackground(VideoCapture stream) {
	Mat currentFrame, background;
	currentFrame = Mat::zeros(ROWS, COLUMNS, CV_8UC1);
	background = Mat::zeros(ROWS, COLUMNS, CV_8UC1);

	for (int iteration = 0; iteration < ITERATIONS; iteration++) {
		stream.read(currentFrame);
		currentFrame = toGrayScale(currentFrame);

		for (int row = 0; row < ROWS; row++)
			for (int column = 0; column < COLUMNS; column++) {
				int color = currentFrame.at<unsigned char>(row, column);
				pixels[row][column].values.push_back(color);
			}
	}

	for (int row = 0; row < ROWS; row++)
		for (int column = 0; column < COLUMNS; column++) {
			int sum = 0;
			for ( int index = 0; index < ITERATIONS; index++ ) {
				int color = pixels[row][column].values.at(index);
				sum = sum + color;
			}
			background.at<unsigned char>(row, column) = sum / ITERATIONS;
		}

	return (background);
}

Mat modeBackground(VideoCapture stream) {
	Mat currentFrame, background;
	currentFrame = Mat::zeros(ROWS, COLUMNS, CV_8UC1);
	background = Mat::zeros(ROWS, COLUMNS, CV_8UC1);

	for ( int row = 0; row < ROWS; row++ )
		for ( int column = 0; column < COLUMNS; column++ )
			for ( int index = 0; index < (INDEXES.size() - 1); index++ ) {
				Interval interval = Interval();
				interval.start = INDEXES[index];
				interval.end = INDEXES[index + 1];
				interval.count = 0;

				pixels[row][column].intervals.push_back(interval);
			}

	for (int iteration = 0; iteration < ITERATIONS; iteration++) {
		stream.read(currentFrame);
		currentFrame = toGrayScale(currentFrame);

		for (int row = 0; row < ROWS; row++)
			for (int column = 0; column < COLUMNS; column++) {
				int color = currentFrame.at<unsigned char>(row, column);
				pixels[row][column].values.push_back(color);
				for ( int index = 0; index < (INDEXES.size() - 1); index++ ) {
					Interval interval = pixels[row][column].intervals.at(index);
					if ( interval.start < color && color < interval.end )
						pixels[row][column].intervals.at(index).count = interval.count + 1;
				}
			}
	}

	for (int row = 0; row < ROWS; row++)
		for (int column = 0; column < COLUMNS; column++) {
			int count = 0; int color = 0;
			for ( int index = 0; index < (INDEXES.size() - 1); index++ ) {
				Interval interval = pixels[row][column].intervals.at(index);
				if ( interval.count > count ) {
					count = interval.count;
					color = (interval.start + interval.end) / 2;
				}
			}
			background.at<unsigned char>(row, column) = color;
		}

	return (background);
}

Mat medianBackground(VideoCapture stream) {
	Mat currentFrame, background;
	currentFrame = Mat::zeros(ROWS, COLUMNS, CV_8UC1);
	background = Mat::zeros(ROWS, COLUMNS, CV_8UC1);

	for (int iteration = 0; iteration < ITERATIONS; iteration++) {
		stream.read(currentFrame);
		currentFrame = toGrayScale(currentFrame);

		for (int row = 0; row < ROWS; row++)
			for (int column = 0; column < COLUMNS; column++) {
				int color = currentFrame.at<unsigned char>(row, column);
				pixels[row][column].values.push_back(color);
			}
	}

	for (int row = 0; row < ROWS; row++)
		for (int column = 0; column < COLUMNS; column++) {
			vector<int> values = pixels[row][column].values;

			bool swap = true;
			while (swap) {
				swap = false;
				for ( int i = 0; i < (ITERATIONS - 1); i++ )
					if ( values.at(i) > values.at(i + 1) ) {
						int t = values.at(i);
						values.at(i) = values.at(i + 1);
						values.at(i + 1) = t;

						swap = true;
					}
			}

			if ( (ITERATIONS % 2) == 0 ) {
				int color1 = values.at( (ITERATIONS / 2) - 1 );
				int color2 = values.at(ITERATIONS / 2);
				int color = (color1 + color2) / 2;
				background.at<unsigned char>(row, column) = color;
			} else if ( (ITERATIONS % 2) == 1 ) {
				int color = values.at(ITERATIONS / 2);
				background.at<unsigned char>(row, column) = color;
			}
		}

	return (background);
}

Mat twoFrameDifference(Mat frame1, Mat frame2) {
	Mat difference = Mat::zeros(ROWS, COLUMNS, CV_8UC1);
	for (int row = 0; row < ROWS; row++)
		for (int column = 0; column < COLUMNS; column++) {
			int colorDifference = abs( frame1.at<unsigned char>(row, column) - frame2.at<unsigned char>(row, column) );
			if ( colorDifference < THRESHOLD )
				difference.at<unsigned char>(row, column) = 0;
			else
				difference.at<unsigned char>(row, column) = 255;
		}

	return (difference);
}

Mat updateBackground(Mat background, Mat currentFrame, Mat mask) {
	Mat newBackground = Mat::zeros(ROWS, COLUMNS, CV_8UC1);

	for (int row = 0; row < ROWS; row++)
		for (int column = 0; column < COLUMNS; column++) {
			if ( mask.at<unsigned char>(row, column) == 0 ) {
				int backgroundColor = background.at<unsigned char>(row, column);
				int frameColor = currentFrame.at<unsigned char>(row, column);
				int color = backgroundColor + ALPHA * (frameColor - backgroundColor);
				newBackground.at<unsigned char>(row, column) = color;
			} else
				newBackground.at<unsigned char>(row, column) = background.at<unsigned char>(row, column);
		}

	return (newBackground);
}

Mat open(Mat mask, int size) {
	Mat output = Mat::zeros(ROWS, COLUMNS, CV_8UC1);
    Mat element = getStructuringElement(0, Size(2 * size + 1, 2 * size + 1), Point(size, size));
    morphologyEx(mask, output, 2, element);
	return (output);
}

Mat close(Mat mask, int size) {
	Mat output = Mat::zeros(ROWS, COLUMNS, CV_8UC1);
    Mat element = getStructuringElement(0, Size(2 * size + 1, 2 * size + 1), Point(size, size));
    morphologyEx(mask, output, 3, element);
	return (output);
}

Mat contour(Mat mask, int size) {
	Mat output = Mat::zeros(ROWS, COLUMNS, CV_8UC1);
    Mat element = getStructuringElement(0, Size(2 * size + 1, 2 * size + 1), Point(size, size));
    morphologyEx(mask, output, 4, element);
	return (output);
}

Mat blobFiltering(Mat mask) {
	Mat connectedComponents = Mat::zeros(ROWS, COLUMNS, CV_32S);
	Mat statistics, centroids;
	int components = connectedComponentsWithStats(mask, connectedComponents, statistics, centroids);

	vector<Object> objects;
	
	for (int label = 1; label < components; label++) {
		int area = statistics.at<int>(label, CC_STAT_AREA);
		if ( area > 50 ) {
			Object object;
			object.label = label;
			object.color = Vec3b(255, 255, 255);
			
			objects.push_back(object);
		}
	}

	Mat output = Mat::zeros(ROWS, COLUMNS, CV_8UC3);
	for (int row = 0; row < ROWS; row++)
		for (int column = 0; column < COLUMNS; column++) {
			int label = connectedComponents.at<int>(row, column);

			Vec3b color = Vec3b(0, 0, 0);
			for (int i = 0; i < objects.size(); i++) {
				Object object = objects.at(i);
				if ( object.label == label )
					color = object.color;
			}

			output.at<Vec3b>(row, column) = color;
		}

	output = toGrayScale(output);

	return (output);
}

void checkIfIsReal(Mat mask, Mat currentFrame, Object object) {
	Mat cannyMask; Mat cannyFrame;
	Canny(mask, cannyMask, 100, 200);
	Canny(currentFrame, cannyFrame, 100, 200);

	vector<Point> points;

	Mat edgesDifferences = Mat::zeros(ROWS, COLUMNS, CV_8UC1);
	for (int row = 0; row < ROWS; row++)
		for (int column = 0; column < COLUMNS; column++) {
			edgesDifferences.at<unsigned char>(row, column) = cannyMask.at<unsigned char>(row, column) - cannyFrame.at<unsigned char>(row, column);
			if ( mask.at<Vec3b>(row, column) == object.color )
				points.push_back( Point(row, column) );
		}

	int differences = 0;
	for (int i = 0; i < points.size(); i++) {
		Point point = points.at(i);
		int row = point.x; int column = point.y;
		if ( cannyMask.at<unsigned char>(row, column) == edgesDifferences.at<unsigned char>(row, column) )
			differences++;
	}

	float differencesPercentage = (float) differences / (float) points.size();

	if ( differencesPercentage > 0.9 )
		for (int i = 0; i < points.size(); i++) {
			Point point = points.at(i);
			int row = point.x; int column = point.y;
			int color = currentFrame.at<unsigned char>(row, column);
			mask.at<Vec3b>(row, column) = Vec3b(color, color, color);
		}
}

Mat blobDetection(Mat mask, Mat currentFrame, int frameIndex) {
	ofstream f;
	if ( frameIndex == 0 )
		f.open("output.txt");
	else
		f.open("output.txt", ios::app);
	
	Mat connectedComponents = Mat::zeros(ROWS, COLUMNS, CV_32S);
	Mat statistics, centroids;
	int components = connectedComponentsWithStats(mask, connectedComponents, statistics, centroids);

	vector<Object> objects;
	int colorIndex = 0;
	
	for (int label = 1; label < components; label++) {
		int area = statistics.at<int>(label, CC_STAT_AREA);
		if ( area > 500 ) {
			Object object;
			object.label = label;
			object.area = area;

			int width = statistics.at<int>(label, CC_STAT_WIDTH);
			int height = statistics.at<int>(label, CC_STAT_HEIGHT);
			int perimeter = (width * 2) + (height * 2);
			object.perimeter = perimeter;

			string classification;
			Vec3b color;
			if ( area > 1500 )
				classification = "person";
			else
				classification = "other";

			object.classification = classification;
			color = COLORS[colorIndex];
			object.color = color;
			colorIndex = colorIndex + 1;
			
			objects.push_back(object);
		}
	}

	int objectsCount = objects.size();
	f << frameIndex << " " << objectsCount << endl;
	for (int i = 0; i < objectsCount; i++) {
		Object object = objects.at(i);
		f << object.label << " " << object.area << " " << object.perimeter << " " << object.classification << endl;
	}
	
	Mat output = Mat::zeros(ROWS, COLUMNS, CV_8UC3);
	for (int row = 0; row < ROWS; row++)
		for (int column = 0; column < COLUMNS; column++) {
			int label = connectedComponents.at<int>(row, column);
			int frameColor = currentFrame.at<unsigned char>(row, column);

			Vec3b color = Vec3b(frameColor, frameColor, frameColor);
			for (int i = 0; i < objects.size(); i++) {
				Object object = objects.at(i);
				if ( object.label == label )
					color = object.color;
			}

			output.at<Vec3b>(row, column) = color;
		}
	
	for (int i = 0; i < objectsCount; i++) {
		Object object = objects.at(i);
		if ( object.classification == "other" )
			checkIfIsReal(output, currentFrame, object);
	}

	f.close();
	return (output);
}


#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

namespace {
	const char* about = "DroneMapper.com geoBit GCP Target Detector";
	const char* keys =
		"{d        |       | Dictionary: DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2,"
		"DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7, "
		"DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12,"
		"DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
		"{c        |       | Camera intrinsic configuration in .xml. Needed for camera pose }"
		"{l        | 0.161 | Marker side length (in meters). Needed for correct scale in camera pose }"
		"{i        |       | Input JPG image }"
		"{dp       |       | File of marker detector parameters }"
		"{r        |       | Show rejected targets }";
}

template <class T>
static bool compareVectors(vector<T> a, vector<T> b)
{
	std::sort(a.begin(), a.end());
	std::sort(b.begin(), b.end());
	return (a == b);
}

/**
*/
static bool readCameraParameters(string filename, Mat &camMatrix, Mat &distCoeffs) {
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
		return false;
	fs["camera_matrix"] >> camMatrix;
	fs["distortion_coefficients"] >> distCoeffs;
	return true;
}

/**
*/
static bool readDetectorParameters(string filename, Ptr<aruco::DetectorParameters> &params) {
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
		return false;
	fs["adaptiveThreshWinSizeMin"] >> params->adaptiveThreshWinSizeMin;
	fs["adaptiveThreshWinSizeMax"] >> params->adaptiveThreshWinSizeMax;
	fs["adaptiveThreshWinSizeStep"] >> params->adaptiveThreshWinSizeStep;
	fs["adaptiveThreshConstant"] >> params->adaptiveThreshConstant;
	fs["minMarkerPerimeterRate"] >> params->minMarkerPerimeterRate;
	fs["maxMarkerPerimeterRate"] >> params->maxMarkerPerimeterRate;
	fs["polygonalApproxAccuracyRate"] >> params->polygonalApproxAccuracyRate;
	fs["minCornerDistanceRate"] >> params->minCornerDistanceRate;
	fs["minDistanceToBorder"] >> params->minDistanceToBorder;
	fs["minMarkerDistanceRate"] >> params->minMarkerDistanceRate;
	fs["doCornerRefinement"] >> params->cornerRefinementMethod;
	fs["cornerRefinementWinSize"] >> params->cornerRefinementWinSize;
	fs["cornerRefinementMaxIterations"] >> params->cornerRefinementMaxIterations;
	fs["cornerRefinementMinAccuracy"] >> params->cornerRefinementMinAccuracy;
	fs["markerBorderBits"] >> params->markerBorderBits;
	fs["perspectiveRemovePixelPerCell"] >> params->perspectiveRemovePixelPerCell;
	fs["perspectiveRemoveIgnoredMarginPerCell"] >> params->perspectiveRemoveIgnoredMarginPerCell;
	fs["maxErroneousBitsInBorderRate"] >> params->maxErroneousBitsInBorderRate;
	fs["minOtsuStdDev"] >> params->minOtsuStdDev;
	fs["errorCorrectionRate"] >> params->errorCorrectionRate;
	return true;
}

// main
int main(int argc, char *argv[]) {
	CommandLineParser parser(argc, argv, keys);
	parser.about(about);

	if (argc < 2) {
		parser.printMessage();
		return 0;
	}

	int dictionaryId = parser.get<int>("d");
	bool showRejected = parser.has("r");
	bool estimatePose = parser.has("c");
	float markerLength = parser.get<float>("l");
	string imageName = parser.get<string>("i");

	Ptr<aruco::DetectorParameters> detectorParams = aruco::DetectorParameters::create();
	if (parser.has("dp")) {
		bool readOk = readDetectorParameters(parser.get<string>("dp"), detectorParams);
		if (!readOk) {
			cerr << "Detector configuration .yml NOT OK" << endl;
			return 0;
		}
	}
	// do cornerRefinement
	detectorParams->cornerRefinementMethod = true;

	if (!parser.check()) {
		parser.printErrors();
		return 0;
	}

	//Ptr<aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

	cv::FileStorage fsr("detector_dictionary.yml", cv::FileStorage::READ);
	int mSize, mCBits;
	cv::Mat bits;
	fsr["MarkerSize"] >> mSize;
	fsr["maxCorrectionBits"] >> mCBits;
	fsr["ByteList"] >> bits;
	fsr.release();

	Ptr<aruco::Dictionary> dictionary = makePtr<cv::aruco::Dictionary>(bits, mSize, mCBits);

	Mat camMatrix, distCoeffs;
	if (estimatePose) {
		bool readOk = readCameraParameters(parser.get<string>("c"), camMatrix, distCoeffs);
		if (!readOk) {
			cerr << "Camera configuration .xml NOT OK" << endl;
			return 0;
		}
	}

	// do it
	Mat image, imageCopy;
	image = imread(imageName.c_str(), IMREAD_COLOR);

	if (image.empty()) {
		cout << "Could not open JPG image" << std::endl;
		return -1;
	}

	vector< int > ids;
	vector< vector< Point2f > > corners, rejected;
	vector< Vec3d > rvecs, tvecs;

	// detect markers and estimate pose
	aruco::detectMarkers(image, dictionary, corners, ids, detectorParams, rejected);

	// find target center geoBit
	for (int i = 0; i < ids.size(); ++i) {
		cv::Point2f center(0, 0);
		for (auto& corner : corners[i]) {
			center += corner;
		}
		center /= 4;
		cout << "DM-" << i << " " << imageName << " " << center.x << " " << center.y << "\n";
	}


	// pose
	if (estimatePose && ids.size() > 0)
		aruco::estimatePoseSingleMarkers(corners, markerLength, camMatrix, distCoeffs, rvecs, tvecs);

	// draw results
	image.copyTo(imageCopy);

	if (ids.size() > 0) {
		aruco::drawDetectedMarkers(imageCopy, corners, ids);
		if (estimatePose) {
			for (unsigned int i = 0; i < ids.size(); i++) {
				aruco::drawAxis(imageCopy, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);
			}
		}
	}

	if (showRejected && rejected.size() > 0)
		aruco::drawDetectedMarkers(imageCopy, rejected, noArray(), Scalar(100, 0, 255));

	imwrite("result.JPG", imageCopy);
}


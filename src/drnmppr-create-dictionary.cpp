
#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>

using namespace cv;

namespace {
	const char* about = "DroneMapper.com geoBit GCP Target Detector";
	const char* keys =
		"{@outfile |<none> | Output image }"
		"{d        |       | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2,"
		"DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7, "
		"DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12,"
		"DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
		"{id       |       | Marker id in the dictionary }"
		"{ms       | 200   | Marker size in pixels }"
		"{bb       | 1     | Number of bits in marker borders }"
		"{si       | false | show generated image }";
}

int main(int argc, char *argv[]) {
	int number = 16, dimension = 3;
	cv::aruco::Dictionary dictionary = cv::aruco::generateCustomDictionary(number, dimension);
	cv::Mat store = dictionary.bytesList;
	cv::FileStorage fs("detector_dictionary.yml", cv::FileStorage::WRITE);
	fs << "MarkerSize" << dictionary.markerSize;
	fs << "MaxCorrectionBits" << dictionary.maxCorrectionBits;
	fs << "ByteList" << dictionary.bytesList;
	fs.release();
	return 0;
}


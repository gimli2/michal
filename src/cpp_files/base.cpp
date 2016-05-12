#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WIN32 //_WIN32
	#include <direct.h>
	#define getCurrentDir _getcwd
#else
	#include <unistd.h>
	#define getCurrentDir getcwd
#endif

#include <iostream>
#include <fstream>
#include <iomanip>

#include "../header_files/base.h"
#include "../header_files/hardness.h"

using namespace cv;
using namespace std;

map<int, pair<int, int> > boarders = {
	{ kernel_shape, make_pair(0, 2) },
	{ kernel_size, make_pair(10, 26) },
	{ center_fill_size, make_pair(1, 21) },
	{ hough_threshold, make_pair(45, 71) },
	{ probabilistic, make_pair(0, 2) },
	{ groups_min_distance, make_pair(0, 41) },
	{ max_gab_tolerance, make_pair(0, 51) },
	{ resultCompare_tolerance, make_pair(0, 31) },
	{ symmetrize_tolerance, make_pair(0, 31) }
};

Result::Result(){
	top = 0;
	bottom = 0;
	left = 0;
	right = 0;
	center = Point2f(0., 0.);
}
Result::Result(float _top, float _bottom, float _left, float _right)
	:top(_top), bottom(_bottom), left(_left), right(_right) {
	center = Point2f(_left + (_right - _left) / 2, _top + (_bottom - _top) / 2);
}
void Result::setPosition(float _top, float _bottom, float _left, float _right){
	top = _top;
	bottom = _bottom;
	left = _left;
	right = _right;
	center.x = left + (right - left) / 2;
	center.y = top + (bottom - top) / 2;
}

Group::Group(){
	points = 0;
	position = new Result();
}
Group::Group(float _top, float _bottom, float _left, float _right){
	points = 0;
	position = new Result(_top, _bottom, _left, _right);
}
Group::~Group(){
	if (position != NULL)
		delete[] position;
}

bool readInt(int & value, const char* str, string error_msg){
	istringstream ss(str);
	if (!(ss >> value)){
		cout << error_msg << ": error reading parameter, integer value required." << endl;
		return false;
	}
	return true;
}
bool readFloat(float & value, const char* str, string error_msg){
	istringstream ss(str);
	if (!(ss >> value)){
		cout << error_msg << ": error reading parameter, float value required." << endl;
		return false;
	}
	return true;
}
int borderize(int value, individuals_p parameter){
	if (value < boarders[parameter].first)
		value = boarders[parameter].first;
	if (value > boarders[parameter].second)
		value = boarders[parameter].second;
	return value;
}

string previous_path = "";
void SetPath(string path){
	previous_path = path;
}
void SaveImg(Mat & image, string path, string addition, bool show){
	if (path == "" && previous_path != "")
		path = previous_path;
	else if (previous_path == "")
		return;

	string name;
	int i = path.length() - 1;
	while (i >= 0 && path[i] != '.')		//remove file type
		i--;
	i--;
	while (i >= 0 && path[i] != '\\' && path[i] != '/')		//get file name
		name = path[i--] + name;

	char current_path[FILENAME_MAX];
	getCurrentDir(current_path, sizeof(current_path));
	string pos = current_path;
	pos += cfg.dir_out + name + addition + ".jpg";	//save as jpg
	imwrite(pos, image);
	
	if (cfg.debug_level > 0)
		cout << "Saving image " << addition << " to: " << pos << endl;

	if (show){
		imshow(addition, image);
		waitKey();
		destroyWindow(addition);
	}
}


void DrawResult(Mat & image, Result result, Scalar color){
	if (image.channels() == 1)
		cvtColor(image, image, CV_GRAY2BGR);

	int thickness = 1;
	int lineType = 8;
	int shift = 0;
	rectangle(image, Point2f(result.left, result.top), Point2f(result.right, result.bottom),
		color, thickness, lineType, shift);
}

void DrawHardness(Mat & image, Result result){
	HardnessResult hardness_result = computeHardness(result);
	
	stringstream ss;
	string text;
	int fontFace = FONT_HERSHEY_SIMPLEX;//CV_FONT_HERSHEY_DUPLEX;
	double fontScale = 1.0;
	int thickness = 2;

	ss << std::setprecision(2) << fixed << "Diagonal1:  " << hardness_result.w << " um";
	text = ss.str();
	cout << text;
	if (cfg.writef_open) cfg.write_file << text;
	putText(image, text, Point(10, 30), fontFace, fontScale, Scalar::all(0), thickness);

	ss.str(string());
	ss << "Diagonal2:  " << hardness_result.w << " um";
	text = ss.str();
	cout << " " << text << endl;
	if (cfg.writef_open) cfg.write_file << " " << text << "\r\n";
	putText(image, text, Point(10, 70), fontFace, fontScale, Scalar::all(0), thickness);

	ss.str(string());
	ss << "Hardness: " << hardness_result.hardness << " HV";
	text = ss.str();
	cout << text << endl;
	if (cfg.writef_open) cfg.write_file << text << "\r\n";
	putText(image, text, Point(10, 110), fontFace, fontScale, Scalar::all(0), thickness);
}

bool GetTemplateImg(Mat & temp){
	char current_path[FILENAME_MAX];
	getCurrentDir(current_path, sizeof(current_path));
	string filename = current_path;
	filename += "/template_image/puncture_template.jpg";
	
	cout << filename << endl;

	temp = imread(filename.c_str(), 0);
	if (!temp.data)
		return false;
	else
		return true;
}

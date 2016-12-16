#ifndef BASE_H
#define BASE_H

#include <fstream>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

enum individuals_p
{
	kernel_shape = 0,
	kernel_size = 1,
	// ------- Detect the puncture ---------
	//match_method = 2,
	// ------- fill center --------
	center_fill_size = 2,
	// ------- Hough transform -----------
	hough_threshold = 3,
	probabilistic = 4,
	// filter and sort the intersections groups
	groups_min_distance = 5,
	max_gab_tolerance = 6,
	// ------ finalize ------------
	resultCompare_tolerance = 7,
	symmetrize_tolerance = 8,

	genes_length = 9,
	mutable_genes = 7
};

struct config{
	string fin;

	bool writef_open;
	ofstream write_file;
	string dir_out;
	string puncture_tpl_file;
	bool one_line;

	vector<string> images;
	vector<double> image_hardness;
	
	// ----- parameters search -------
	int parameters_search;
	int population_size;
	int generations;
	int min_eval_percentage;

	int debug_level;

	// ------- preproccess -----------
	int kernel_shape;
	int kernel_size;
	// ------- Detect the puncture ---------
	int match_method;
	// ------- fill center --------
	int center_fill_size;
	// ------- Hough transform -----------
	int hough_threshold;
	bool probabilistic;
	// filter and sort the intersections groups
	float groups_min_distance;
	float max_gab_tolerance;
	// ------ finalize ------------
	float resultCompare_tolerance;
	float symmetrize_tolerance;
};

struct Pos{
	int x;
	int y;
	Pos(int x, int y) : x(x), y(y) {};
	Pos() { x = 0; y = 0; };
};

class Result{
public:
	Result();
	Result(float _top, float _bottom, float _left, float _right);
	void setPosition(float _top, float _bottom, float _left, float _right);
	float top, bottom, left, right;
	Point2f center;
private:
};

class Group{
public:
	Group();
	Group(float _top, float _bottom, float _left, float _right);
	~Group();
	Result* position;
	unsigned int points;
private:
};

extern config cfg;

int borderize(int value, individuals_p parameter);
bool readInt(int & value, const char* str, string error_msg);
bool readFloat(float & value, const char* str, string error_msg);

void SetPath(string path);
void SaveImg(Mat &img, string path, string addition = "", bool show = false);
void DrawResult(Mat & image, Result result, Scalar color);
void DrawHardness(Mat & image, Result result);
bool GetTemplateImg(Mat & temp);

template < typename T > std::string to_string( const T& n )
{
    std::ostringstream stm ;
    stm << n ;
    return stm.str() ;
}

#endif

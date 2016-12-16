#include <iostream>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#ifdef WIN32 //_WIN32
	#define END_ERROR system("pause"); return -1;
	#define END system("pause"); return 0;
#else
	#define END_ERROR return -1;
	#define END return 0;
#endif

#include "header_files/preprocess.h"
#include "header_files/convexHull.h"
#include "header_files/parameters.h"
#include "header_files/hardness.h"
#include "header_files/evaluate.h"
#include "header_files/detect.h"
#include "header_files/hough.h"
#include "header_files/base.h"

using namespace cv;
using namespace std;

config cfg;

extern map<int, pair<int, int> > boarders;
bool readImagesPaths(int i, int argc, char* const* argv, config* cfg);

/**
\brief Print help and usage informations.
*/
void printUsage(int exit_code, char* const* argv) {
	cout << "MICHAL - MICro Hardness AnaLysis v.2." << endl << endl;
	cout << "Aplication for analysis of micro hardness measurements." << endl << endl;
	//cout << "Usage: " << argv[0] << " [options] -i INPUTFILEs" << endl << endl;
	cout << "Usage: " << argv[0] << " [Common_options] [-x|Process_options] {[-i][INPUTFILEs]}" << endl;
	cout << "INPUTFILEs:" << endl;
	cout << "       Image filenames to analyse." << endl;
	cout << "Common options:" << endl;
	cout << "  -h" << endl;
	cout << "       Show this help." << endl;
	cout << "  -d" << endl;
	cout << "       0 = result output only. (default)" << endl;
	cout << "       1 = + all information during detection." << endl;
	cout << "       2 = + saving steps images." << endl;
	cout << "       3 = + show main steps images." << endl;
	cout << "       4 = + show all steps images." << endl;
	cout << "  -i" << endl;
	cout << "       Textfile of image filenames to analyse." << endl;
	cout << "  -o" << endl;
	cout << "       Evaluaded images directory." << endl;
	cout << "       Default directory is ./out" << endl;
	cout << "  -m" << endl;
	cout << "       Filename of puncture template in JPG format." << endl;
	cout << "       Default: ./templates/puncture_template_smooth.jpg" << endl; 
	cout << "  -t" << endl;
	cout << "       Filename to write text results." << endl;
	cout << "  -j" << endl;
	cout << "       Write results information text in one line." << endl;
	cout << "  -z" << endl;
	cout << "       Lense zoom." << endl;
	cout << "       Default is 40x with size of one pixel = 0.20432*10^-3 m." << endl;
	cout << "" << endl;
	cout << "Parameter search options:" << endl;
	cout << "		INPUTFILEs required format: IMAGE HARDNESS" << endl;
	cout << "  -e	Evolution algorithm search" << endl;
	cout << "		population_size = size of a population per generation" << endl;
	cout << "		number_of_generations = generations count" << endl;
	cout << "		minimum_evaluation_percentage <0,100>" << endl;
	cout << "  -b	Brute force search" << endl;
	cout << "" << endl;
	cout << "Process options:" << endl;
	cout << "  -k	Noice remove kernel shape:" << endl;
	cout << "		0 = MORPH_RECT, 1 = MORPH_ELLIPSE (default = 1)" << endl;
	cout << "  -n	Noice remove kernel size:" << endl;
	cout << "		<" << boarders[kernel_size].first << "," << boarders[kernel_size].second << ">, default = 17" << endl;
	cout << "  -f	Center fill size:" << endl;
	cout << "		<" << boarders[center_fill_size].first << "," << boarders[center_fill_size].second << ">, default = 6" << endl;
	cout << "  -p	Probabilistic Hough transform" << endl;
	cout << "  -h	Hough transform accumulator threshold parameter:" << endl;
	cout << "		<" << boarders[hough_threshold].first << "," << boarders[hough_threshold].second << ">, default = 50" << endl;
	cout << "  -l	Line intersections sorting distance:" << endl;
	cout << "		<" << boarders[groups_min_distance].first << "," << boarders[groups_min_distance].second << ">, default = 5" << endl;
	cout << "  -g	Maximal gab tolerance:" << endl;
	cout << "		<" << boarders[max_gab_tolerance].first << "," << boarders[max_gab_tolerance].second << ">, default = 100" << endl;
	cout << "  -c	Hough transform and convex hull comparison tolerance:" << endl;
	cout << "		<" << boarders[resultCompare_tolerance].first << "," << boarders[resultCompare_tolerance].second << ">, default = 10" << endl;
	cout << "  -s	Center symmetrize tolerance:" << endl;
	cout << "		<" << boarders[symmetrize_tolerance].first << "," << boarders[symmetrize_tolerance].second << ">, default = 10" << endl;
	exit(exit_code);
}


bool readParams(int argc, char* const* argv, config* cfg) {
	
	// default values
	// ------ i/o ---------
	cfg->dir_out = "/out/";
	cfg->puncture_tpl_file = "templates/puncture_template_smooth.jpg";
	string fout;
	cfg->writef_open = false;
	cfg->one_line = false;
	// ------ debug ----------
	cfg->debug_level = 0;
	// ------- preproccess -----------
	cfg->kernel_shape = MORPH_CROSS;
	cfg->kernel_size = 17;
	// ------- Detect the puncture ---------
	cfg->match_method = CV_TM_SQDIFF_NORMED;
	// ------- fill center --------
	cfg->center_fill_size = 6;
	// ------- Hough transform -----------
	cfg->hough_threshold = 50;
	cfg->probabilistic = false;
	// filter and sort the intersections groups
	cfg->groups_min_distance = 5.;
	cfg->max_gab_tolerance = 100;
	// ------ finalize ------------
	cfg->resultCompare_tolerance = 10.;
	cfg->symmetrize_tolerance = 10.;
	// ------ parameters search ------
	cfg->parameters_search = 0;
	cfg->population_size = 0;
	cfg->generations = 0;
	cfg->min_eval_percentage = 90;


	// parse command line
	int i = 1, tmp;
	int next_option;
	bool parse_end = false;
	while (!parse_end && i < argc){
		if (argv[i][0] != '-'){
			//end of options arguments
			parse_end = true;
			break;
		}

		// option parameter
		next_option = argv[i++][1];
		
		// true/false arguments
		if (next_option == 'h' || next_option == '?'){
			printUsage(0, argv);
		}
		if (next_option == 'p'){
			cfg->probabilistic = true;
			continue;
		}
		if (next_option == 'b'){
			cfg->parameters_search = 2;
			continue;
		}
		if (next_option == 'j'){
			cfg->one_line = true;
			continue;
		}

		// parameter required argumets
		if (i >= argc){
			cout << "Missing parameter for -" << (char)next_option << endl;
			return false;
		}
		switch (next_option) {
		case 'z':
			int magnification;
			if (!readInt(magnification, argv[i++], "Magnification"))
				return false;
			setLens(magnification);
			break;
		case 'd':
			if (!readInt(cfg->debug_level, argv[i++], "Debug level"))
				return false;
			break;
		case 'i':
			cfg->fin = argv[i++];
			break;
		case 'm':
			cfg->puncture_tpl_file = argv[i++];
			break;
		case 't':
			fout = argv[i++];
			break;
		case 'o':
			struct stat info;
			if( stat( argv[i], &info ) != 0 ){
				cout << "Cannot access " << argv[i] << endl;
				return false;
			}
			else if( info.st_mode & S_IFDIR ){
				// argv[i] is a directory
				cfg->dir_out = argv[i++];
				cfg->dir_out = "/" + cfg->dir_out + "/";
			}
			else{
				cout << argv[i] << "is not a directory." << endl;
				return false;
			}
			break;
		case 'k':
			if (!readInt(cfg->kernel_shape, argv[i++], "Kernel shape"))
				return false;
			if (cfg->kernel_shape != 0)
				cfg->kernel_shape = 1;
			break;
		case 'n':
			if (!readInt(cfg->kernel_size, argv[i++], "Kernel size"))
				return false;
			cfg->kernel_size = borderize(cfg->kernel_size, kernel_size);
			break;
		case 'f':
			if (!readInt(cfg->center_fill_size, argv[i++], "Center fill size"))
				return false;
			cfg->center_fill_size = borderize(cfg->center_fill_size, center_fill_size);
			break;
		case 'h':
			if (!readInt(cfg->hough_threshold, argv[i++], "Hough threshold"))
				return false;
			cfg->hough_threshold = borderize(cfg->hough_threshold, hough_threshold);
			break;
		case 'l':
			if (!readInt(tmp, argv[i++], "Line intersections sorting distance"))
				return false;
			cfg->groups_min_distance = (float)borderize(tmp, groups_min_distance);
			break;
		case 'g':
			if (!readInt(tmp, argv[i++], "Maximal gab tolerance"))
				return false;
			cfg->max_gab_tolerance = (float)borderize(tmp, max_gab_tolerance);
			break;
		case 'c':
			if (!readInt(tmp, argv[i++], "Comparison tolerance"))
				return false;
			cfg->resultCompare_tolerance = (float)borderize(tmp, resultCompare_tolerance);
			break;
		case 's':
			if (!readInt(tmp, argv[i++], "Symmetrize tolerance"))
				return false;
			cfg->symmetrize_tolerance = (float)borderize(tmp, symmetrize_tolerance);
			break;

		// parameter search
		case 'e':
			cfg->parameters_search = 1;
			if (!readInt(cfg->population_size, argv[i++], "Population size"))
				return false;
			if (i >= argc){
				cout << "Missing parameter - number of generations" << endl;
				return false;
			}
			if (!readInt(cfg->generations, argv[i++], "Number of generations"))
				return false;
			if (i >= argc){
				cout << "Missing parameter - minimum evaluation percentage" << endl;
				return false;
			}
			if (!readInt(cfg->min_eval_percentage, argv[i++], "Minimum evaluation percentage"))
				return false;
			if (cfg->min_eval_percentage > 100)
				cfg->min_eval_percentage = 100;
			break;

		// options end
		case -1: /* Done with options.  */
			break;
		default: /* Something else: unexpected.  */
			return false;
		}
	}

	if (!readImagesPaths(i, argc, argv, cfg))
		return false;
	

	if (!fout.empty()){
		cfg->write_file.open(fout);
		if (cfg->write_file.is_open())
			cfg->writef_open = true;
		else{
			cout << "Unable to open file " << fout << endl;
			return false;
		}
	}

	return true;
}
bool readImagesPaths(int i, int argc, char* const* argv, config* cfg){
	if (i == argc && cfg->fin.empty()){
		cout << "Missing input file." << endl;
		return false;
	}
	while (i < argc){
		// read images form command line
		if (cfg->parameters_search > 0){
			// get (image, hardness) for parameter search
			cfg->images.push_back(argv[i++]);
			if (i == argc){
				cout << "Missing hardness result for " << cfg->images.back() << endl;
				return false;
			}
			float val;
			if (!readFloat(val, argv[i++], "Image hardness value"))
				return false;
			cfg->image_hardness.push_back(val);
		}
		else{
			// get images for evaluation
			cfg->images.push_back(argv[i++]);
		}
	}
	if (!cfg->fin.empty()){
		// read images form file
		ifstream read_file(cfg->fin);
		if (read_file.is_open()){
			double image_hardness;
			string image_name;

			while (!read_file.eof()){
				read_file >> image_name;
				cfg->images.push_back(image_name);
				if (cfg->parameters_search){
					if (read_file.eof()){
						cout << "Missing hardness result for " << cfg->images.back() << endl;
						return false;
					}
					read_file >> image_hardness;
					cfg->image_hardness.push_back(image_hardness);
				}
			}
			read_file.close();
		}
		else{
			cout << "Unable to open file " << cfg->fin << endl;
			return false;
		}
	}
	return true;
}

bool detectPuncture(Mat & image_working, Mat & temp, Result & result){
	// preproccess
	removeNoise(image_working, MORPH_OPEN, cfg.kernel_shape, cfg.kernel_size);

	// Detect the puncture center
	Result center_result = DetectByTemplate(image_working, temp, cfg.match_method);
	Point2f center = center_result.center;
	Result result_hough, result_convex;

	// fill the center star
	removeNoise(image_working, MORPH_CLOSE, cfg.kernel_shape, cfg.kernel_size);
	Mat kernel = getStructuringElement(cfg.kernel_shape, Size(cfg.center_fill_size, cfg.center_fill_size));
	erode(image_working, image_working, kernel);
	floodFill(image_working, center, Scalar::all(0));

	// fill small areas (remove possible inner lines)
	removeNoise(image_working, MORPH_CLOSE, cfg.kernel_shape, cfg.kernel_size);

	// Hough + convex hull method
	result_hough = HoughTransf(image_working, center, cfg.hough_threshold, cfg.probabilistic, cfg.groups_min_distance, cfg.max_gab_tolerance);
	result_convex = ConvexHullMethod(image_working.clone(), center);

	if (!resultCompare(center, result_hough, result_convex, result, cfg.resultCompare_tolerance)){
		// The puncture not detected (too much deformation, noise or missing)
		cout << "Hardness cannot be calculated." << endl;
		if (cfg.writef_open) cfg.write_file << "Hardness cannot be calculated." << "\r\n";
		return false;
	}
	symmetrize(center, result, cfg.symmetrize_tolerance);
	return true;
}

int main(int argc, char* argv[])
{
	if (argc < 2){
		printUsage(-1, argv);
		END_ERROR
	}
	
	if (!readParams(argc, argv, &cfg)){
		END_ERROR
	}

	Mat temp;
	if (!GetTemplateImg(temp)){
		cout << "Error reading template image." << endl;
		END_ERROR
	}


	if (cfg.parameters_search > 0){
		try{
			float min_eval_poercent = (float)cfg.min_eval_percentage / 100;
			if (cfg.parameters_search == 1){
				int population_size = cfg.population_size;
				int generations = cfg.generations;
				
				evolutionSearch(population_size, generations, min_eval_poercent, temp);
			}
			else{
				bruteForceSearch(min_eval_poercent, temp);
			}
		}
		catch (int e){
			if (e){}
			END_ERROR
		}


		END
	}


	for (string image_path : cfg.images)
	{
		Mat image;
		SetPath(image_path);

		image = imread(image_path, 0);
		if (!image.data){
			cout << "Error reading image " << image_path << endl;
			END_ERROR
		}
		else{
			if (!cfg.one_line && cfg.debug_level == 0) cout << endl << "Proccessing image " << image_path << endl;
			if (cfg.writef_open && !cfg.one_line) cfg.write_file << "\r\n" << "Proccessing image " << image_path << "\r\n";
		}


		// Convert to gray-style image
		Mat image_working;
		threshold(image, image_working, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);


		Result result;
		if (detectPuncture(image_working, temp, result)){

			DrawResult(image, result, Scalar(0, 0, 0));
			DrawHardness(image, result);
	
		}
		SaveImg(image, image_path, "_myeval", (cfg.debug_level >= 2));
		
	}


	END
}


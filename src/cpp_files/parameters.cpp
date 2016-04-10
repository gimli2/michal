#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <map>
#include <stdio.h>
#include <stdlib.h> 

#include <time.h>


#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "../header_files/base.h"
#include "../header_files/hardness.h"
#include "../header_files/parameters.h"

#include "../header_files/preprocess.h"
#include "../header_files/convexHull.h"
#include "../header_files/parameters.h"
#include "../header_files/hardness.h"
#include "../header_files/evaluate.h"
#include "../header_files/detect.h"
#include "../header_files/hough.h"
#include "../header_files/base.h"

using namespace cv;
using namespace std;

extern map<int, pair<int, int> > boarders;

// ----------- INDIVIDUAL ---------------
Individual::Individual(){
	genotype = new int[genes_length];
	for (size_t i = 0; i < genes_length; i++)
		genotype[i] = rand() % (boarders[i].second - boarders[i].first) + boarders[i].first;
}
Individual::Individual(Individual* other){
	genotype = new int[genes_length];
	for (size_t i = 0; i < genes_length; i++)
		genotype[i] = other->genotype[i];
	fitness = other->getFitness();
	hardness_results = other->getHardnessResults();
}
Individual::~Individual(){
	delete[] genotype;
}

void Individual::computeFitness(vector<Mat> images, Mat & temp, float min_percent_evaluation) {
	fitness = 0;
	hardness_results.clear();
	unsigned int min_eval_count = (unsigned int)(images.size() * min_percent_evaluation);

	cout << "Fitness calculation for genotype: ";
	for (size_t i = 0; i < mutable_genes; i++){
		cout << genotype[i] << " ";
	}
	cout << endl;


	vector<Mat> working_imgs;
	// ------- noice remove -----------
	for (Mat image : images)
	{
		Mat noice_rem;
		image.copyTo(noice_rem);
		removeNoise(noice_rem, MORPH_OPEN, genotype[kernel_shape], genotype[kernel_size]);
		working_imgs.push_back(noice_rem);
	}
	// -------- Detect the puncture ---------
	vector<Point2f> centers;
	for (Mat image : working_imgs)
	{
		Result center_result = DetectByTemplate(image, temp, CV_TM_SQDIFF_NORMED);//genotype[match_method]);
		centers.push_back(center_result.center);
	}
	// -------- center fill ------------------
	for (size_t index = 0; index < working_imgs.size(); index++)
	{
		removeNoise(working_imgs[index], MORPH_CLOSE, genotype[kernel_shape], genotype[kernel_size]);
		Mat kernel = getStructuringElement(genotype[kernel_shape], Size(genotype[center_fill_size], genotype[center_fill_size]));
		erode(working_imgs[index], working_imgs[index], kernel);
		floodFill(working_imgs[index], centers[index], Scalar::all(0));
		removeNoise(working_imgs[index], MORPH_CLOSE, genotype[kernel_shape], genotype[kernel_size]);
	}
	// -------- hough + convex --------------
	vector<Result> hough_results;
	vector<Result> convex_results;
	for (size_t index = 0; index < working_imgs.size(); index++)
	{
		Result result_hough = HoughTransf(working_imgs[index], centers[index], genotype[hough_threshold],
			(genotype[probabilistic] == 1), (float)genotype[groups_min_distance], (float)genotype[max_gab_tolerance]);
		hough_results.push_back(result_hough);

		Result result_convex = ConvexHullMethod(working_imgs[index].clone(), centers[index]);
		convex_results.push_back(result_convex);
	}
	// ----- result compare -----------------
	// Brute force search for tolerance parameters (time efficiency)
	double difference = 0;
	unsigned int evaluaded_count = 0;
	vector<double> hardness_tmp;
	for (float resultCompare_tolerance_val = (float)boarders[resultCompare_tolerance].first; resultCompare_tolerance_val < (float)boarders[resultCompare_tolerance].second; resultCompare_tolerance_val += 1.){
		for (float symmetrize_tolerance_val = (float)boarders[symmetrize_tolerance].first; symmetrize_tolerance_val < (float)boarders[symmetrize_tolerance].second; symmetrize_tolerance_val += 1.)
		{
			difference = 0;
			hardness_tmp.clear();
			evaluaded_count = 0;
			for (size_t index = 0; index < centers.size(); index++)
			{
				Result result;
				if (!resultCompare(centers[index], hough_results[index], convex_results[index], result, resultCompare_tolerance_val)){
					hardness_tmp.push_back(0);
					continue;
				}
				symmetrize(centers[index], result, symmetrize_tolerance_val);

				HardnessResult hardness_res = computeHardness(result);
				hardness_tmp.push_back(hardness_res.hardness);
				difference += (hardness_tmp[index] / cfg.image_hardness[index] > 1) ? (100 - ((hardness_tmp[index] / cfg.image_hardness[index]) * 100 - 100)) : (hardness_tmp[index] / cfg.image_hardness[index]) * 100;
				evaluaded_count++;
			}
			difference /= evaluaded_count;
			if (difference > fitness && evaluaded_count >= min_eval_count){
				fitness = difference;
				hardness_results = hardness_tmp;
				genotype[resultCompare_tolerance] = (int)resultCompare_tolerance_val;
				genotype[symmetrize_tolerance] = (int)symmetrize_tolerance_val;
			}
		}
	}


	cout << "Fitness = " << fitness << endl;
}

bool Individual::mutate(int mutation_probability) {
	// Individuals genotype mutation
	bool mutated = false;
	for (unsigned int i = 0; i < mutable_genes; i++)
	{
		if (mutation_probability >= rand() % 100) {
			mutated = true;
			int tmp = rand() % (boarders[i].second - boarders[i].first) + boarders[i].first;
			if (tmp == genotype[i]){
				tmp = (tmp + 1) % boarders[i].second;
				if (tmp < boarders[i].first)
					tmp = boarders[i].first;
			}
			genotype[i] = tmp;
		}
	}
	return mutated;
}

pair<Individual*, Individual*> Individual::crossover(Individual* other) {
	pair<Individual*, Individual*> result;

	// Two-point crossover
	unsigned int border1 = rand() % mutable_genes;
	unsigned int border2 = rand() % mutable_genes;
	if (border2 < border1){
		border1 += border2;
		border2 = border1 - border2;
		border1 -= border2;
	}

	Individual* individum1 = new Individual(this);
	Individual* individum2 = new Individual(other);

	for (unsigned int i = 0; i < mutable_genes; i++)
	{
		if (i <= border1 || i >= border2){
			individum1->genotype[i] = other->genotype[i];
			individum2->genotype[i] = this->genotype[i];
		}
	}

	result = make_pair(individum1, individum2);
	return result;
}

double Individual::getFitness(){
	return fitness;
}
vector<double> Individual::getHardnessResults(){
	vector<double> newHardness;
	for (double d : hardness_results)
		newHardness.push_back(d);
	return newHardness;
}
void Individual::printGenotype(ostream & stream){
	stream << genotype[kernel_shape] << " kernel_shape" << endl;
	stream << genotype[kernel_size] << " kernel_size" << endl;
	stream << genotype[center_fill_size] << " center_fill_size" << endl;
	stream << genotype[hough_threshold] << " hough_threshold" << endl;
	stream << genotype[probabilistic] << " probabilistic" << endl;
	stream << genotype[groups_min_distance] << " groups_min_distance" << endl;
	stream << genotype[max_gab_tolerance] << " max_gab_tolerance" << endl;
	stream << genotype[resultCompare_tolerance] << " resultCompare_tolerance" << endl;
	stream << genotype[symmetrize_tolerance] << " symmetrize_tolerance" << endl;
	/*
	cout << "kernel_shape " << genotype[kernel_shape] << endl;
	cout << "kernel_size " << genotype[kernel_size] << endl;
	cout << "center_fill_size " << genotype[center_fill_size] << endl;
	cout << "hough_threshold " << genotype[hough_threshold] << endl;
	cout << "probabilistic " << genotype[probabilistic] << endl;
	cout << "groups_min_distance " << genotype[groups_min_distance] << endl;
	cout << "max_gab_tolerance " << genotype[max_gab_tolerance] << endl;
	cout << "resultCompare_tolerance " << genotype[resultCompare_tolerance] << endl;
	cout << "symmetrize_tolerance " << genotype[symmetrize_tolerance] << endl;
	*/
}

// ----------- POPULATION ---------------
Population::Population(Mat & temp, float _min_percent_evaluation){
	cout << "Inicializing population" << endl;
	min_percent_evaluation = _min_percent_evaluation;
	for (string image_path : cfg.images){
		Mat image;
		image = imread(image_path, 0);
		if (!image.data){
			cout << "Error reading image " << image_path << endl;
			throw 1;
		}
		// Convert to gray-style image
		threshold(image, image, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		images.push_back(image);
	}

	individuals = new Individual*[cfg.images.size()];
	population_size = cfg.images.size();
	for (size_t i = 0; i < population_size; i++){
		individuals[i] = new Individual();
		individuals[i]->computeFitness(images, temp, min_percent_evaluation);
	}
}

Population::~Population(){
	clearPopulation();
	delete[] individuals;
}
	
vector<Individual*> Population::selectIndividuals(unsigned int count) {
	vector<Individual*> selected;
	int tournament_size = 4;

	Individual* fittest;
	Individual* challenger;

	while (selected.size() < count) {
		fittest = individuals[rand() % population_size];
		for (int i = 1; i < tournament_size; i++)
		{
			challenger = individuals[rand() % population_size];
			if (challenger->getFitness() > fittest->getFitness())
				fittest = challenger;
		}
		selected.push_back(fittest);
	}
	return selected;
}

Individual* Population::getBestIndividual(){
	Individual* best = individuals[0];
	for (size_t i = 1; i < population_size; i++){
		if (individuals[i]->getFitness() > best->getFitness())
			best = individuals[i];
	}
	return best;
}

double Population::getAvgFitness(){
	double sum = 0;
	int evaluaded = 0;
	for (size_t i = 0; i < population_size; i++){
		sum += individuals[i]->getFitness();
		if (individuals[i]->getFitness() != 0)
			evaluaded++;
	}
	sum /= evaluaded;
	return sum;
}

void Population::addIndividual(Individual* individum){
	individuals[population_size++] = individum;
}
void Population::clearPopulation(){
	for (size_t i = 0; i < population_size; i++){
		delete individuals[i];
	}
	population_size = 0;
}


// --------------- FUNCTIONS ------------------

void evolutionSearch(unsigned int population_size, int generations, float min_percent_evaluation, Mat & temp){
	cout << "population size = " << population_size << endl;
	cout << "generations = " << generations << endl;
	cout << "minimum evaluate percentage = " << (int)(min_percent_evaluation * 100) << endl;
	
	int crossover_probability = 2; // percent
	int mutation_probability = 10;

	Population* population = new Population(temp, min_percent_evaluation);

	Individual** new_pop = new Individual*[population_size];
	unsigned int new_pop_size;
	for (int g = 0; g < generations; g++) {
		cout << "generation " << g+1 << "/" << generations << endl;
		new_pop[0] = new Individual(population->getBestIndividual());
		new_pop_size = 1;

		// fill the generation
		while (new_pop_size < population_size) {
			// select 2 parents
			vector<Individual*> parents = population->selectIndividuals(2);

			// perform crossover
			pair<Individual*, Individual*> offspring;
			bool crossed = false;
			if (crossover_probability >= rand() % 100) {
				crossed = true;
				offspring = parents[0]->crossover(parents[1]);
			}
			else {
				offspring = make_pair(new Individual(parents[0]), new Individual(parents[1]));
			}

			// mutate first offspring
			if (offspring.first->mutate(mutation_probability) || crossed){
				offspring.first->computeFitness(population->images, temp, population->min_percent_evaluation);
			}
			new_pop[new_pop_size++] = offspring.first;

			// if there is still space mutate and add second offspring
			if (new_pop_size < population_size) {
				if (offspring.second->mutate(mutation_probability) || crossed){
					offspring.second->computeFitness(population->images, temp, population->min_percent_evaluation);
				}
				new_pop[new_pop_size++] = offspring.second;
			}
			else{
				delete offspring.second;
			}
		}

		// replace the current population with the new one
		population->clearPopulation();
		for (unsigned int i = 0; i<new_pop_size; i++) {
			population->addIndividual(new_pop[i]);
		}

		if (g+1 < generations){
			cout << "generation = " << g + 1 << endl;
			cout << "Best fitness = " << population->getBestIndividual()->getFitness() << endl;
			cout << "Average fitness = " << population->getAvgFitness() << endl;
		}
	}

	cout << "Evolution complete" << endl;
	cout << "Best fitness = " << population->getBestIndividual()->getFitness() << endl;
	cout << "Average fitness = " << population->getAvgFitness() << endl;
	population->getBestIndividual()->printGenotype(cout);
	cout << "Best hardness results:" << endl;
	vector<double> hardness = population->getBestIndividual()->getHardnessResults();
	for (size_t i = 0; i < hardness.size(); i++)
		cout << hardness[i] << "(" << cfg.image_hardness[i] << ")  diff = " << abs(cfg.image_hardness[i] - hardness[i]) << endl;
	if (cfg.writef_open){
		cfg.write_file << "Best fitness = " << population->getBestIndividual()->getFitness() << endl;
		cfg.write_file << "Average fitness = " << population->getAvgFitness() << endl;
		population->getBestIndividual()->printGenotype(cfg.write_file);
		cfg.write_file << "Best hardness results:" << endl;
		vector<double> hardness = population->getBestIndividual()->getHardnessResults();
		for (size_t i = 0; i < hardness.size(); i++)
			cfg.write_file << hardness[i] << "(" << cfg.image_hardness[i] << ")  diff = " << abs(cfg.image_hardness[i] - hardness[i]) << endl;
	}

	delete population;
}








void bruteForceSearch(float min_percent_evaluation, Mat & temp){
	vector<Mat> images;
	for (string image_path : cfg.images){
		Mat image;
		image = imread(image_path, 0);
		if (!image.data){
			cout << "Error reading image " << image_path << endl;
			throw 1;
		}
		// Convert to gray-style image
		threshold(image, image, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		images.push_back(image);
	}


	config best;
	double difference, difference_best = 0;
	vector<double> hardness_results, hardness_results_best;

	// ------- preproccess -----------
	for (int kernel_shape_val = boarders[kernel_shape].first; kernel_shape_val < boarders[kernel_shape].second; kernel_shape_val++){
		for (int kernel_size_val = boarders[kernel_size].first; kernel_size_val < boarders[kernel_size].second; kernel_size_val += 5)
		{
			// ------- noice remove -----------
			vector<Mat> noice_removed;
			for (Mat image : images)
			{
				Mat noice_rem;
				image.copyTo(noice_rem);
				removeNoise(noice_rem, MORPH_OPEN, kernel_shape_val, kernel_size_val);
				noice_removed.push_back(noice_rem);
			}

			// -------- Detect the puncture ---------
			vector<Point2f> centers;
			for (Mat image : noice_removed)
			{
				Result center_result = DetectByTemplate(image, temp, cfg.match_method);
				centers.push_back(center_result.center);
			}

			// -------- fill the center star --------
			for (int center_fill_val = boarders[center_fill_size].first; center_fill_val < boarders[center_fill_size].second; center_fill_val += 2)
			{
				vector<Mat> center_filled;
				for (size_t index = 0; index < noice_removed.size(); index++)
				{
					Mat image_working;
					noice_removed[index].copyTo(image_working);
					removeNoise(image_working, MORPH_CLOSE, kernel_shape_val, kernel_size_val);
					Mat kernel = getStructuringElement(kernel_shape_val, Size(center_fill_val, center_fill_val));
					erode(image_working, image_working, kernel);
					floodFill(image_working, centers[index], Scalar::all(0));
					removeNoise(image_working, MORPH_CLOSE, kernel_shape_val, kernel_size_val);
					center_filled.push_back(image_working);
				}


				vector<Result> convex_results;
				for (size_t index = 0; index < center_filled.size(); index++){
					Result result_convex = ConvexHullMethod(center_filled[index].clone(), centers[index]);
					convex_results.push_back(result_convex);
				}

				// -------- hough + convex ---------
				for (int prob = 0; prob <= 1; prob++){
					bool probabilistic_val = (prob == 1);
					for (int hough_threshold_val = boarders[hough_threshold].first; hough_threshold_val < boarders[hough_threshold].second; hough_threshold_val += 5){
						for (float groups_min_distance_val = (float)boarders[groups_min_distance].first; groups_min_distance_val < (float)boarders[groups_min_distance].second; groups_min_distance_val += 5.){
							for (float max_gab_tolerance_val = (float)boarders[max_gab_tolerance].first; max_gab_tolerance_val < (float)boarders[max_gab_tolerance].second; max_gab_tolerance_val += 5.){
								vector<Result> hough_results;
								for (size_t index = 0; index < center_filled.size(); index++){
									Result result_hough = HoughTransf(center_filled[index], centers[index], hough_threshold_val, probabilistic_val, groups_min_distance_val, max_gab_tolerance_val);
									hough_results.push_back(result_hough);
								}

								// --------- result compare --------
								for (float resultCompare_tolerance_val = (float)boarders[resultCompare_tolerance].first; resultCompare_tolerance_val < (float)boarders[resultCompare_tolerance].second; resultCompare_tolerance_val += 1.){
									for (float symmetrize_tolerance_val = (float)boarders[symmetrize_tolerance].first; symmetrize_tolerance_val < (float)boarders[symmetrize_tolerance].second; symmetrize_tolerance_val += 1.)
									{
										difference = 0;
										hardness_results.clear();
										unsigned int evaluaded_count = 0;
										for (size_t index = 0; index < centers.size(); index++)
										{
											Result result;
											if (!resultCompare(centers[index], hough_results[index], convex_results[index], result, resultCompare_tolerance_val)){

												hardness_results.push_back(0);
												continue;
											}
											symmetrize(centers[index], result, symmetrize_tolerance_val);

											HardnessResult hardness_res = computeHardness(result);
											hardness_results.push_back(hardness_res.hardness);
											difference += (hardness_results[index] / cfg.image_hardness[index] > 1) ? (100 - ((hardness_results[index] / cfg.image_hardness[index]) * 100 - 100)) : (hardness_results[index] / cfg.image_hardness[index]) * 100;
											evaluaded_count++;
										}
										difference /= evaluaded_count;
										if (difference > difference_best && evaluaded_count >= cfg.image_hardness.size() / 2){
											//cfg_best.copy(cfg);
											best.kernel_shape = kernel_shape_val;
											best.kernel_size = kernel_size_val;
											best.match_method = cfg.match_method;
											best.center_fill_size = center_fill_val;
											best.hough_threshold = cfg.hough_threshold;
											best.probabilistic = cfg.probabilistic;
											best.groups_min_distance = cfg.groups_min_distance;
											best.max_gab_tolerance = cfg.max_gab_tolerance;
											best.resultCompare_tolerance = resultCompare_tolerance_val;
											best.symmetrize_tolerance = symmetrize_tolerance_val;
											// copy end


											cout << "new best  : " << difference_best << " from " << difference << endl;

											difference_best = difference;
											hardness_results_best = hardness_results;
										}
									}
								}
							}
						}
					}
				}
				// hough
			}
		}
	}
	

	cout << "----- BEST RESULTS ----- " << difference_best << endl;
	for (size_t index = 0; index < hardness_results_best.size(); index++)
		cout << hardness_results_best[index] << "(" << cfg.image_hardness[index] << ")  diff = " << abs(cfg.image_hardness[index] - hardness_results_best[index]) << endl;
	cout << endl;
	cout << "----- CFG -----" << endl;
	cout << best.kernel_shape << " cfg.kernel_shape" << endl;
	cout << best.kernel_size << " cfg.kernel_size" << endl;
	cout << best.match_method << " cfg.match_method" << endl;
	cout << best.center_fill_size << " cfg.center_fill_size" << endl;
	cout << best.hough_threshold << " cfg.hough_threshold" << endl;
	cout << best.probabilistic << " cfg.probabilistic" << endl;
	cout << best.groups_min_distance << " cfg.groups_min_distance" << endl;
	cout << best.max_gab_tolerance << " cfg.max_gab_tolerance" << endl;
	cout << best.resultCompare_tolerance << " cfg.resultCompare_tolerance" << endl;
	cout << best.symmetrize_tolerance << " symmetrize_tolerance" << endl;
}
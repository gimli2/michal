#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <fstream>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void bruteForceSearch(float min_percent_evaluation, Mat & temp);
void evolutionSearch(unsigned int population_size, int generations, float min_percent_evaluation, Mat & temp);

class Individual{
public:
	Individual();
	Individual(Individual* other);
	~Individual();
	void computeFitness(vector<Mat> images, Mat & temp, float min_percent_evaluation);
	bool mutate(int mutation_probability);
	pair<Individual*, Individual*> crossover(Individual* other);
	double getFitness();
	vector<double> getHardnessResults();
	void printGenotype(ostream & stream);
private:
	int* genotype;
	double fitness = 0;
	vector<double> hardness_results;
};

class Population{
public:
	Population(Mat & temp, float _min_percent_evaluation);
	~Population();
	vector<Individual*> selectIndividuals(unsigned int count);
	Individual* getBestIndividual();
	double getAvgFitness();
	void addIndividual(Individual* individum);
	void clearPopulation();

	vector<Mat> images;
	float min_percent_evaluation;
private:
	Individual** individuals;
	unsigned int population_size;
};

#endif
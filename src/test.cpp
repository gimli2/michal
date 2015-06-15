#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>


using namespace std;


std::string getFileName(const std::string& filepath, const std::string& delim) {
  size_t pos = filepath.rfind(delim);
  if(pos == std::string::npos)
    pos = -1;
  return std::string(filepath.begin() + pos + 1, filepath.end());
}

std::string extractExtension(std::string fileName) {
  size_t position = fileName.find(".");
  return (string::npos == position)? fileName : fileName.substr(0, position);
}

std::string get_working_path() {
  const int MAXPATHLEN = 1000;

  char temp[MAXPATHLEN];
  return ( getcwd(temp, MAXPATHLEN) ? std::string( temp ) : std::string("") );
}

bool fileExists(const char* dirname) {
  struct stat st;
  if(stat(dirname,&st) == 0)
    return true;
  return false;
}

struct Result {
  double diameter;
  double hardness;
  string inpfile;
};

std::string myExec(const char* cmd) {
  FILE* pipe = popen(cmd, "r");
  if (!pipe) return "ERROR";
  char buffer[128];
  std::string result = "";
  while(!feof(pipe)) {
    if(fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
}

vector<Result> findPicsToEval(ifstream& config,const char* inpdir) {
  vector<Result> toparse;

  string line;
  for ( int rowno = 0; getline (config,line); rowno++ ) {
    //headers
    if(rowno < 7)
      continue;

    char delim = ';';
    stringstream ss(line);
    string item;


    Result res;
//	string picpath = "";

    int matches = 0;
    for (int col = 0; std::getline(ss, item, delim); col++) {
      if(col == 14) {
        matches++;
//		cout << "Diameter:\t"<<item;
        res.diameter = atof(item.c_str());
      }
      if(col == 30) {
        matches++;
//		cout << "AbsX:\t"<<item;
      }
      if(col == 31) {
        matches++;
//		cout << "AbsY:\t"<<item;
      }
      if(col == 42) {
        matches++;
//		cout << "Path:\t"<<getFileName(item);

        res.inpfile = inpdir;
        res.inpfile += "/";
        res.inpfile += item;
//		res.inpfile += getFileName(item, "\\");
      }
      if(col == 3) {
        matches++;
        res.hardness = atof(item.c_str());
//\		cout << "Hardness:\t"<<item;
      }

//	    string //	    elems.push_back(item);
//	    cout << "\t"<<item;
    }

    if(matches != 5) {
      cerr << "Structure error in csv on line "<<(rowno+1)<<endl;
      continue;
    }
    if(!fileExists(res.inpfile.c_str())) {
      cerr << "Picture file "<<res.inpfile.c_str()<< " does not exists"<<endl;
      continue;
    }
    toparse.push_back(res);
  }

  config.close();

  return toparse;
}

Result parseOutput(std::string output) {
  Result res;

  stringstream ss(output);


  res.diameter = 0.;
//    int row = 0;
  string line;
  for(int row = 0; getline(ss, line); row++) {
    switch(row) {
    case 0:
      res.hardness = atof(line.c_str());
      break;
    case 1:
      res.diameter += atof(line.c_str());
      break;
    case 2:
      res.diameter += atof(line.c_str());
      break;
    }
  }
  res.diameter /= 2.;

  return res;
}

vector<Result> myEvaluate(const char* outdir, const vector<Result>& res) {
  vector<Result> ret;

  int cnt = 0;
  for(vector<Result>::const_iterator it = res.begin(); it != res.end(); ++it) {
    const Result& curres = *it;
    string command = get_working_path();
    command += "/main ";
    command += curres.inpfile;

    string outfilename = outdir;
    outfilename += "/";
//	cout << "INPFILE "<<curres.inpfile<<endl;
//	cout << "INPFILE "<<extractExtension(getFileName(curres.inpfile, "/"))<<endl;
    outfilename += extractExtension(getFileName(curres.inpfile, "/"));
    outfilename += "_myeval.jpg";

    command += " ";
    command += outfilename;


    cout << "Parsing image "<<(cnt++)<<" of "<<res.size() <<endl;
    cout << " --------- "<<command.c_str()<<endl;
    string output = myExec(command.c_str());

    ret.push_back(parseOutput(output));
  }

  return ret;
}

void printStatistics(vector<Result> orig, vector<Result> my) {
  vector<Result>::iterator orig_it;
  vector<Result>::iterator my_it;

  int cnt_995 = 0;
  int cnt_99 = 0;
  int cnt_98 = 0;
  int cnt_95 = 0;
  int cnt_90 = 0;
  int cnt_80 = 0;
  int cnt_75 = 0;
  int cnt_50 = 0;

  vector<float> accuracies;

  int orig_match = 0;
  for(orig_it = orig.begin(), my_it = my.begin(); orig_it != orig.end() && my_it != my.end(); ++my_it,++orig_it) {

    const Result& orig_res = *orig_it;
    const Result& my_res   = *my_it;

    if(orig_res.hardness <= 0.0000000001) {
      accuracies.push_back(-1);
      continue;
    }


    double hardness1 = orig_res.hardness;
    double hardness2 = my_res.hardness;

    if(hardness2 > hardness1) {
      //swap em
      double pom = hardness1;
      hardness1 = hardness2;
      hardness2 = pom;
    }

    float part = hardness2/hardness1;
    if(part >= 0.995)
      cnt_995++;
    if(part >= 0.99)
      cnt_99++;
    if(part >= 0.98)
      cnt_98++;
    if(part >= 0.95)
      cnt_95++;
    if(part >= 0.90)
      cnt_90++;
    if(part >= 0.80)
      cnt_80++;
    if(part >= 0.75)
      cnt_75++;
    if(part >= 0.5)
      cnt_50++;

    accuracies.push_back(part);

    orig_match++;
  }

  cout << "<<<<<<<<<<Computed values>>>>>>>>>"<<endl;
  cout << "<<filename>>\t\t<<difference>>\t\t<<HV - orig>>\t\t<<HV - computed>>"<<endl;
  int i = 0;
  vector<float>::iterator it;
  for(
    orig_it = orig.begin(),
    my_it = my.begin(),
    it = accuracies.begin()
         ;
    it != accuracies.end()
    &&
    orig_it != orig.end()
    &&
    my_it != my.end()
    ;
    ++it,
    ++my_it,
    ++orig_it
  ) {
    const Result& orig_res = *orig_it;
    const Result& my_res   = *my_it;

    cout << orig_res.inpfile.c_str() <<"\t\t";
    float accuracy = *it;
    if(accuracy < 0)
      cout << "not matched";
    else
      cout << accuracy*100. << "%";

    cout << "\t\t";
    cout << orig_res.hardness;
    cout << "\t\t";
    cout << my_res.hardness;

    cout << endl;
    i++;
  }

  cout << "<<<<<<<<<<Statistical results>>>>>>>>>"<<endl;
  cout << "Original match - "<<orig_match<<" results ("<<std::setprecision(6)<<(float(orig.size())*100./orig_match) <<"%)"<<endl;


  cout << "99.5% accuracy - "<<cnt_995<<" results ("<<std::setprecision(6)<<(float(cnt_995)*100./orig_match) <<"%)"<<endl;
  cout << "99%   accuracy - "<<cnt_99<<" results ("<<std::setprecision(6)<<(float(cnt_99)*100./orig_match) <<"%)"<<endl;
  cout << "98%   accuracy - "<<cnt_98<<" results ("<<std::setprecision(6)<<(float(cnt_98)*100./orig_match) <<"%)"<<endl;
  cout << "95%   accuracy - "<<cnt_95<<" results ("<<std::setprecision(6)<<(float(cnt_95)*100./orig_match) <<"%)"<<endl;
  cout << "90%   accuracy - "<<cnt_90<<" results ("<<std::setprecision(6)<<(float(cnt_90)*100./orig_match) <<"%)"<<endl;
  cout << "80%   accuracy - "<<cnt_80<<" results ("<<std::setprecision(6)<<(float(cnt_80)*100./orig_match) <<"%)"<<endl;
  cout << "75%   accuracy - "<<cnt_75<<" results ("<<std::setprecision(6)<<(float(cnt_75)*100./orig_match) <<"%)"<<endl;
  cout << "50%   accuracy - "<<cnt_50<<" results ("<<std::setprecision(6)<<(float(cnt_50)*100./orig_match) <<"%)"<<endl;
}

int main(int argc, char* argv[]) {
  if( argc < 3) {
    cout <<" Usage: ./text csv_config_file input_directory output_directory " << endl;
    return -1;
  }

  ifstream config(argv[1]);
  if(!config.is_open()) {
    cerr << "Config file "<<argv[1]<<" does not exists"<<endl;
    return 1;
  }

  const char* inpdir = argv[2];
  if(!fileExists(inpdir)) {
    cerr << "Input directory "<<inpdir<<" must exists"<<endl;
    return 1;
  }

  const char* outdir = argv[3];

  vector<Result> res = findPicsToEval(config,inpdir);

  cout << "evaluating "<<res.size() << " images "<<endl;

  vector<Result> myres = myEvaluate(outdir, res);

  printStatistics(res, myres);


  return 0;
}

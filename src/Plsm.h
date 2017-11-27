#ifndef __PLSM_H
#define __PLSM_H

#include "Database.h"

#include <vector>
#include <map>
#include <string>
#include <random>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

class PLSM {
  public:
    PLSM();
    ~PLSM();

    void Initialize();
    void SetDatabase(Database *db);
    void RunEM(const int &iters, const int &topic);
    void LoadPwz(const std::string &file_name);
    void LoadPdz(const std::string &file_name);
    void LoadPz(const std::string &file_name);
    void LoadModel(const std::string &file_name);
    void OutputModel(const std::string &file_name);
    void OutputDocuments(double length_mean, double length_sd, int n, const std::string &file_name);
    double GetLogLikelihood();

  private:
    void RunEStep(double beta);
    void RunMStep();

    void GenerateRandomParameters();
    void GenerateRandomPwz();
    void GenerateRandomPdz();
    void GenerateRandomPz();

    void CalculateDenominators();
    void CalculatePwz();
    void CalculatePdz();
    void CalculatePz();

    void InvertMaps();
    int SelectRandom(const std::vector<double>& v);
    std::mt19937 rng;

    // P(z|d,w)
    std::vector<std::vector<std::map<int,double> > > Pzdw;
    std::vector<std::vector<double> > Pwz, Pdz, Pwz_I, PdzI;
    std::vector<double> Pz;
    std::vector<double> denominators;
    Database *mDb;
    int mNumWords, mNumDocs, mNumTopics, mR;
};

#endif

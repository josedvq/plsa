#include "Plsm.h"

template<class T, class V>
bool isEmpty(std::map<T,V> myMap) {
  return myMap.empty();
}

PLSM::PLSM() {
}

PLSM::~PLSM() {
}

void PLSM::Initialize() {
  std::random_device rd;
  rng = std::mt19937(rd());
}

void PLSM::SetDatabase(Database *db)
{
  mDb = db;
  mNumWords = mDb->GetNumWords();
  mNumDocs = mDb->GetNumDocs();
  mR = mDb->GetTotalWordCount();
}

void PLSM::LoadModel(const std::string &file_name) {
  std::ifstream inputfile(file_name.c_str());
  int i,j;

  if (inputfile.is_open())
  {
    inputfile >> mNumTopics >> mNumWords >> mNumDocs;
    
    // Read the P(z)
    for(i = 0; i < mNumTopics; ++i)
    {
      inputfile >> Pz[i];
    }
    
    // Read the P(w|z)
    for(i = 0; i < mNumTopics; ++i)
    {
      for(j = 0; j < mNumWords; ++j)
      {
        inputfile >> Pwz[j][i];
      }
    }

    // Read the P(d|z)
    for(i = 0; i < mNumTopics; ++i)
    {
      for(j = 0; j < mNumDocs; ++j)
      {
        inputfile >> Pdz[j][i];
      }
    }

    inputfile.close();
  } else {
    std::cout << "Unable to open file"; 
  }
}

double PLSM::GetLogLikelihood() {
  double Pdw, LL = 0.0;
  int z_counter, d_counter = 0;
  auto docs_words = mDb->GetDocsWords();
  for(std::vector<std::map<int,int> >::iterator d_iterator = docs_words.begin(); d_iterator != docs_words.end(); ++d_iterator)
  {
    for(std::map<int,int>::iterator w_iterator = d_iterator->begin(); w_iterator != d_iterator->end(); ++w_iterator)
    {
      // Calculate P(d,w)
      Pdw = 0.0;
      z_counter = 0;
      for(std::vector<double>::iterator z_iterator = Pz.begin(); z_iterator != Pz.end(); ++z_iterator)
      {
        // P(d,w) = sum_{P(z)P(w|z)P(d|z)}
        Pdw += (*z_iterator)*Pwz[w_iterator->first][z_counter]*Pdz[d_counter][z_counter];
        ++z_counter;
      }
      LL += w_iterator->second*std::log(Pdw);
    }
    ++d_counter;
  }

  return LL;
}

void PLSM::InvertMaps() {
  Pwz_I = std::vector<std::vector<double> >(mNumTopics,std::vector<double>(mNumWords,0.0));
  
  int z_counter, w_counter = 0;
  for(std::vector<std::vector<double> >::iterator w_iterator = Pwz.begin(); w_iterator != Pwz.end(); ++w_iterator)
  {
    z_counter = 0;
    for(std::vector<double>::iterator z_iterator = w_iterator->begin(); z_iterator != w_iterator->end(); ++z_iterator)
    {
      Pwz_I[z_counter][w_counter] = *z_iterator;
      ++z_counter;
    }

    ++w_counter;
  }
}

int PLSM::SelectRandom(const std::vector<double> & v) {
  std::uniform_real_distribution<double> dis(0.0, 1.0);

  int cnt = 0;
  double num = dis(rng);
  double sum = 0;
  for(std::vector<double>::const_iterator it = v.begin(); it != v.end(); ++it)
  {
    sum += *it;
    if(num < sum)
    {
      return cnt;
    }

    ++cnt;
  }
  //throw "Loss of precision errors in SelectRandom.";
}

//
void PLSM::RunEM(const int &iters, const int &topics) {
  mNumTopics = topics;

  // Initialize Pzdw
  Pzdw = std::vector<std::vector<std::map<int,double> > >(mNumTopics,std::vector<std::map<int,double> >(mNumDocs,std::map<int,double>()));
  
  // Initialize other vectors
  Pz = std::vector<double>(mNumTopics);
  denominators = std::vector<double>(mNumTopics);
  Pwz = std::vector<std::vector<double> >(mNumWords,std::vector<double>(mNumTopics,0.0));
  Pdz = std::vector<std::vector<double> >(mNumDocs,std::vector<double>(mNumTopics,0.0));

  GenerateRandomParameters();
  std::cout << "Random Parameters Finished!!" << std::endl;
  for (int i = 0; i < iters; ++ i) {
    RunEStep(1.0);
    RunMStep();
    std::cout << "Log likelihood: " << GetLogLikelihood() << std::endl;
  }
}

void PLSM::RunEStep(double beta) {

  int d_counter, w_counter, z_counter;
  double denominator;
  auto docs_words = mDb->GetDocsWords();

  d_counter = 0;
  for(std::vector<std::map<int,int> >::iterator d_iterator = docs_words.begin(); d_iterator != docs_words.end(); ++d_iterator)
  {
    for(std::map<int,int>::iterator w_iterator = d_iterator->begin(); w_iterator != d_iterator->end(); ++w_iterator)
    {
      // Calculate the denominator
      denominator = 0;
      z_counter = 0;
      for(std::vector<std::vector<std::map<int,double> > >::iterator z_iterator = Pzdw.begin(); z_iterator != Pzdw.end(); ++z_iterator)
      {
        denominator += pow(Pz[z_counter]*Pdz[d_counter][z_counter]*Pwz[w_iterator->first][z_counter],beta);
        ++z_counter;
      }

      z_counter = 0;
      for(std::vector<std::vector<std::map<int,double> > >::iterator z_iterator = Pzdw.begin(); z_iterator != Pzdw.end(); ++z_iterator)
      {
        
        // Calculate the numerator
        Pzdw[z_counter][d_counter][w_iterator->first] = pow(Pz[z_counter]*Pdz[d_counter][z_counter]*Pwz[w_iterator->first][z_counter],beta) / denominator;
        ++z_counter;
      }
    }
    ++d_counter;
  }
}

void PLSM::RunMStep() {
  CalculateDenominators();
  CalculatePwz();
  CalculatePdz();
  CalculatePz();
}

void PLSM::GenerateRandomPwz() {
  double val, sum;
  const int k_random_size = 10000;
  for(int topic = 0; topic < mNumTopics; ++topic)
  {
    sum = 0;
    for(std::vector<std::vector<double> >::iterator w_iterator = Pwz.begin(); w_iterator != Pwz.end(); ++w_iterator)
    {
      val = rand() % k_random_size * 1.0;
      (*w_iterator)[topic] = val;
      sum += val;
    }

    for(std::vector<std::vector<double> >::iterator w_iterator = Pwz.begin(); w_iterator != Pwz.end(); ++w_iterator)
    {
      (*w_iterator)[topic] /= sum;
    }
  }
}

void PLSM::GenerateRandomPdz() {
  double val, sum;
  const int k_random_size = 10000;
  for(int topic = 0; topic < mNumTopics; ++topic)
  {
    sum = 0;
    for(std::vector<std::vector<double> >::iterator d_iterator = Pdz.begin(); d_iterator != Pdz.end(); ++d_iterator)
    {
      val = rand() % k_random_size * 1.0;
      (*d_iterator)[topic] = val;
      sum += val;
    }

    for(std::vector<std::vector<double> >::iterator d_iterator = Pdz.begin(); d_iterator != Pdz.end(); ++d_iterator)
    {
      (*d_iterator)[topic] /= sum;
    }
  }
}

void PLSM::GenerateRandomPz() {
  double val, sum;
  const int k_random_size = 10000;
  for(std::vector<double>::iterator z_iterator = Pz.begin(); z_iterator != Pz.end(); ++z_iterator)
  {
    val = rand() % k_random_size * 1.0;
    *z_iterator = val;
    sum += val;
  }

  for(std::vector<double>::iterator z_iterator = Pz.begin(); z_iterator != Pz.end(); ++z_iterator)
  {
    *z_iterator /= sum;
  }
}

void PLSM::GenerateRandomParameters() {
  GenerateRandomPwz();
  GenerateRandomPdz();
  GenerateRandomPz();
}

void PLSM::CalculateDenominators() {
  int d_counter, w_counter, z_counter = 0;
  double numerator, denominator;
  auto docs_words = mDb->GetDocsWords();
  for(std::vector<double>::iterator z_iterator = Pwz.front().begin(); z_iterator != Pwz.front().end(); ++z_iterator)
  {
    // Calculate denominator
    denominator = 0;
    d_counter = 0;
    for(std::vector<std::map<int,int> >::iterator d_iterator = docs_words.begin(); d_iterator != docs_words.end(); ++d_iterator)
    {
      for(std::map<int,int>::iterator w_iterator = d_iterator->begin(); w_iterator != d_iterator->end(); ++w_iterator)
      {
        denominator += w_iterator->second * Pzdw[z_counter][d_counter][w_iterator->first];
      }
      ++d_counter;
    }

    denominators[z_counter] = denominator;
    ++z_counter;
  }
}

void PLSM::CalculatePwz() {

  int w_counter, z_counter = 0;
  double numerator;
  auto words_docs = mDb->GetWordsDocs();
  for(std::vector<double>::iterator z_iterator = Pwz.front().begin(); z_iterator != Pwz.front().end(); ++z_iterator)
  {
    // For each w
    w_counter = 0;
    for(std::vector<std::map<int,int> >::iterator w_iterator = words_docs.begin(); w_iterator != words_docs.end(); ++w_iterator)
    {

      // Calculate numerator
      numerator = 0;
      for(std::map<int,int>::iterator d_iterator = w_iterator->begin(); d_iterator != w_iterator->end(); ++d_iterator)
      {
        numerator += d_iterator->second * Pzdw[z_counter][d_iterator->first][w_counter];
      }

      // Write result
      Pwz[w_counter][z_counter] = numerator / denominators[z_counter];

      ++w_counter;
    }
    ++z_counter;
  }
}

void PLSM::CalculatePdz() {

  int d_counter, z_counter = 0;
  double numerator;
  auto docs_words = mDb->GetDocsWords();
  for(std::vector<double>::iterator z_iterator = Pdz.front().begin(); z_iterator != Pdz.front().end(); ++z_iterator)
  {
    // For each w
    d_counter = 0;
    for(std::vector<std::map<int,int> >::iterator d_iterator = docs_words.begin(); d_iterator != docs_words.end(); ++d_iterator)
    {

      // Calculate numerator
      numerator = 0;
      for(std::map<int,int>::iterator w_iterator = d_iterator->begin(); w_iterator != d_iterator->end(); ++w_iterator)
      {
        numerator += w_iterator->second * Pzdw[z_counter][d_counter][w_iterator->first];
      }

      // Write result
      Pdz[d_counter][z_counter] = numerator / denominators[z_counter];

      ++d_counter;
    }
    ++z_counter;
  }
}

void PLSM::CalculatePz() {
  
  int z_counter = 0;
  for(std::vector<double>::iterator z_iterator = Pz.begin(); z_iterator != Pz.end(); ++z_iterator)
  {
    *z_iterator = denominators[z_counter]/mR;
    ++z_counter;
  }

}

void PLSM::OutputModel(const std::string &file_name) {
  // First output the number of z, d, and w
  std::ofstream fout(file_name.c_str());
  int i,j;
  fout << mNumTopics << " " << mNumWords << " " << mNumDocs << std::endl;

  if (fout.is_open())
  {
    // Now output the probabilities of every topic
    for(i = 0; i < mNumTopics; ++i) {
      fout << Pz[i] << " ";
    }
    fout << std::endl;

    // Now output the P(w|z), one topic per line
    for(i = 0; i < mNumTopics; ++i) {
      for(j = 0; j < mNumWords; ++j) {
        fout << Pwz[j][i] << " ";
      }
      fout << std::endl;
    }

    // Now output the P(d|z), one topic per line
    for(i = 0; i < mNumTopics; ++i) {
      for(j = 0; j < mNumDocs; ++j) {
        fout << Pdz[j][i] << " ";
      }
      fout << std::endl;
    }

    fout.close();
  } else {
    std::cout << "Unable to open output file";
  }
}

void PLSM::OutputDocuments(double length_mean, double length_sd, int n, const std::string &file_name) {
  // First output the number of z, d, and w
  std::ofstream fout(file_name.c_str());
  int i,j,doc_length,z,w;

  // Normal distribution of lengths
  
  std::normal_distribution<> doc_length_distribution(length_mean, length_sd);

  if (fout.is_open())
  {
    InvertMaps();
    // Generate n documents
    for(i = 0; i < n; ++i)
    {
      doc_length = std::max(1.0,doc_length_distribution(rng));
      // Generate doc_length words
      for(j = 0; j < doc_length; ++j)
      {
        // Select a topic with prob P(z)
        z = SelectRandom(Pz);

        // Select a word with prob P(w|z)
        w = SelectRandom(Pwz_I[z]);
        fout << w << " ";
      }
      fout << std::endl;
    }

    fout.close();
  } else {
    std::cout << "Unable to open output file";
  }
}

#include "Database.h"

template<class T, class V>
bool isEmpty(std::map<T,V> myMap) {
  return myMap.empty();
}

Database::Database() {

}

Database::~Database() {
	
}

void Database::LoadWordCount(const std::string &file_name) {
  std::ifstream fin(file_name.c_str());

  if (fin.is_open())
  {
    int number_words_per_document, document, word_max;
    std::map<int,int> doc;
    words_docs.push_back(std::map<int,int>());

    mTotalWordCount = 0;
    word_max = 0;
    document = 0;
    while (fin >> number_words_per_document) {

      doc = std::map<int,int>();
      for (int i = 0; i < number_words_per_document; ++ i) {
        int word, frequency;
        char quotation;
        fin >> word >> quotation >> frequency;
        doc[word] += frequency;

        if(word > word_max)
        {
          for(int j = word_max+1; j <= word; ++j)
          {
            words_docs.push_back(std::map<int,int>());
          }
        }

        words_docs[word][document] += frequency;

        word_max = std::max(word_max,word);
        mTotalWordCount += frequency;

      }
      docs_words.push_back(doc);
      ++document;
    }

    // Check that every document has at least one word.
    std::vector<std::map<int,int> >::iterator docs_it = std::find_if(docs_words.begin(), docs_words.end(), isEmpty<int,int>);
    if(docs_it != docs_words.end())
    {
      throw "Some documents do not contain any words.";
    }

    // Check that every word is in at least one document
    std::vector<std::map<int,int> >::iterator words_it = std::find_if(words_docs.begin(), words_docs.end(), isEmpty<int,int>);
    if(words_it != words_docs.end())
    {
      throw "Some words are not contained in any document.";
    }

    fin.close();
  } else {
    throw "Unable to open file"; 
  }
}

void Database::LoadItemsets(const std::string &file_name) {
  std::ifstream inputfile(file_name.c_str());

  if (inputfile.is_open())
  {
    int word, document, word_max;
    std::map<int,int> doc;

    // Initialize structures
    docs_words = std::vector<std::map<int,int> >();
    words_docs = std::vector<std::map<int,int> >();

    std::string line;

    word_max = 0;
    document = 0;
    while(getline(inputfile, line)) {

      doc = std::map<int,int>();

      std::istringstream iss(line);
      while(iss >> word) {
      	doc[word] += 1;

        if(word > word_max)
        {
          for(int j = word_max+1; j <= word; ++j)
          {
            words_docs.push_back(std::map<int,int>());
          }
        }
        words_docs[word][document] += 1;
      }
      docs_words.push_back(doc);
      ++document;
    }

    
    inputfile.close();
  } else {
    throw "Unable to open file"; 
  }
}

void Database::WriteItemsets(const std::string &file_name, bool include_repeated) {
  std::ofstream fout(file_name.c_str());

  if (fout.is_open())
  {
    for(std::vector<std::map<int,int> >::iterator d_iterator = docs_words.begin(); d_iterator != docs_words.end(); ++d_iterator)
    {
    	for(std::map<int,int>::iterator w_iterator = d_iterator->begin(); w_iterator != d_iterator->end(); ++w_iterator)
        {
        	if(include_repeated)
        	{
        		for(int i = 0; i < w_iterator->second; ++i)
        		{
        			fout << w_iterator->first << " ";
        		}
        	}
        	else
        	{
        		fout << w_iterator->first << " ";
        	}
        }
        fout << "\n";
    }

    fout.close();
  } else {
    throw "Unable to open output file";
  }
}

void Database::WriteWordCount(const std::string &file_name, bool include_repeated) {
  std::ofstream fout(file_name.c_str());

  if (fout.is_open())
  {
    for(std::vector<std::map<int,int> >::iterator d_iterator = docs_words.begin(); d_iterator != docs_words.end(); ++d_iterator)
    {
    	for(std::map<int,int>::iterator w_iterator = d_iterator->begin(); w_iterator != d_iterator->end(); ++w_iterator)
        {
        	if(include_repeated)
        	{
        		fout << w_iterator->first << ":" << w_iterator->second << " ";
        	}
        	else
        	{
        		fout << w_iterator->first << ":1 ";
        	}
        }
        fout << "\n";
    }

    fout.close();
  } else {
    throw "Unable to open output file";
  }
}
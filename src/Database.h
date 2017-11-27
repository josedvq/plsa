#ifndef __DATABASE_H
#define __DATABASE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>

class Database {
	public:
		Database();
		~Database();

		void LoadWordCount(const std::string &file_name);
		void LoadItemsets(const std::string &file_name);
		void WriteItemsets(const std::string &file_name, bool include_repeated = true);
		void WriteWordCount(const std::string &file_name, bool include_repeated = true);

		const std::vector<std::map<int,int> >& GetDocsWords() const { return docs_words; }
		const std::vector<std::map<int,int> >& GetWordsDocs() const { return words_docs; }

		int GetNumDocs(void) {return docs_words.size();}
		int GetNumWords(void) {return words_docs.size();}
		int GetTotalWordCount(void) {return mTotalWordCount;}

	private:
		// for each document, a map of word > count
		std::vector<std::map<int,int> > docs_words;
		// for each word, a map of doc > count
	    std::vector<std::map<int,int> > words_docs;
	    int mTotalWordCount;
};

#endif
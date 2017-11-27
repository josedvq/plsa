#include "Database.h"
#include "InputParser.h"
#include "Plsm.h"

#include <stdlib.h>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  InputParser input(argc, argv);

  if(input.cmdOptionExists("itemize"))
  {
        
    const std::string &input_file  = input.getCmdOption("-i");
    const std::string &output_file = input.getCmdOption("-o");

      
    Database db = Database();
    db.LoadWordCount(input_file);
    db.WriteItemsets(output_file);
    std::cout << "File written." << std::endl;
      
  } 
  else if(input.cmdOptionExists("plsa")) 
  {
    const std::string &input_file  = input.getCmdOption("-i");
    const std::string &output_file = input.getCmdOption("-o");
    int iterations = atoi(input.getCmdOption("-n").c_str());
    int k = atoi(input.getCmdOption("-k").c_str());

    try {
      Database db;
      db.LoadWordCount(std::string(input_file));

      PLSM plsm;
      plsm.Initialize();
      plsm.SetDatabase(&db);
      plsm.RunEM(iterations, k);
      plsm.OutputModel(output_file);

      //plsa.GenerateDocuments(20,2,10000,"docs.dat");
    } catch(char const* e) {
      std::cout << e << "\n";
    }

    std::cout << "C'est fini." << std::endl;
  }
  else
  {
    std::cerr << "usage: ./plsa <input file> <topic number> <iteration number> <output file>";
    std::cerr << std::endl;
  }
  return 0;
}

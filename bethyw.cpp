


/*
  +---------------------------------------+
  | BETH YW? WELSH GOVERNMENT DATA PARSER |
  +---------------------------------------+

  AUTHOR: 976789

  This file contains all the helper functions for initialising and running
  Beth Yw? In languages such as Java, this would be a class, but we really
  don't need a class here. Classes are for modelling data, and so forth, but
  here the code is pretty much a sequential block of code (BethYw::run())
  calling a series of helper functions.
*/

#include <iostream>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "lib_cxxopts.hpp"

#include "areas.h"
#include "datasets.h"
#include "bethyw.h"
#include "input.h"

/*
  Run Beth Yw?, parsing the command line arguments, importing the data,
  and outputting the requested data to the standard output/error.

  Hint: cxxopts.parse() throws exceptions you'll need to catch. Read the cxxopts
  documentation for more information.

  @param argc
    Number of program arguments

  @param argv
    Program arguments

  @return
    Exit code
*/
int BethYw::run(int argc, char *argv[]) {

    auto cxxopts = BethYw::cxxoptsSetup();
  auto args = cxxopts.parse(argc, argv);

  // Print the help usage if requested
  if (args.count("help")) {
    std::cerr << cxxopts.help() << std::endl;
    return 0;
  }

  // Parse data directory argument
  std::string dir = args["dir"].as<std::string>() + DIR_SEP;

  // Parse other arguments and import data

   auto datasetsToImport = BethYw::parseDatasetsArg(args);
   auto areasFilter      = BethYw::parseAreasArg(args);
   auto measuresFilter   = BethYw::parseMeasuresArg(args);
   auto yearsFilter      = BethYw::parseYearsArg(args);

  Areas data = Areas();

  BethYw::loadAreas(data, dir, areasFilter);

  BethYw::loadDatasets(data,
                        dir,
                        datasetsToImport,
                        areasFilter,
                        measuresFilter,
                        yearsFilter);

  if (args.count("json")) {
    // The output as JSON
    std::cout << data.toJSON() << std::endl;
  } else {
    // The output as tables
    std::cout << data << std::endl;
  }
  return 0;
}

/*
  This function sets up and returns a valid cxxopts object. You do not need to
  modify this function.

  @return
     A constructed cxxopts object

  @example
    auto cxxopts = BethYw::cxxoptsSetup();
    auto args = cxxopts.parse(argc, argv);
*/
cxxopts::Options BethYw::cxxoptsSetup() {
  cxxopts::Options cxxopts(
        "bethyw",
        "Student ID: " + STUDENT_NUMBER + "\n\n"
        "This program is designed to parse official Welsh Government"
        " statistics data files.\n");
    
  cxxopts.add_options()(
      "dir",
      "Directory for input data passed in as files",
      cxxopts::value<std::string>()->default_value("datasets"))(

      "d,datasets",
      "The dataset(s) to import and analyse as a comma-separated list of codes "
      "(omit or set to 'all' to import and analyse all datasets)",
      cxxopts::value<std::vector<std::string>>())(

      "a,areas",
      "The areas(s) to import and analyse as a comma-separated list of "
      "authority codes (omit or set to 'all' to import and analyse all areas)",
      cxxopts::value<std::vector<std::string>>())(

      "m,measures",
      "Select a subset of measures from the dataset(s) "
      "(omit or set to 'all' to import and analyse all measures)",
      cxxopts::value<std::vector<std::string>>())(

      "y,years",
      "Focus on a particular year (YYYY) or "
      "inclusive range of years (YYYY-ZZZZ)",
      cxxopts::value<std::string>()->default_value("0"))(

      "j,json",
      "Print the output as JSON instead of tables.")(

      "h,help",
      "Print usage.");

  return cxxopts;
}

/*
 *

  Parse the datasets argument passed into the command line. 

  The datasets argument is optional, and if it is not included, all datasets 
  should be imported. If it is included, it should be a comma-separated list of 
  datasets to import. If the argument contains the value "all"


  This function validates the passed in dataset names against the codes in
  DATASETS array in the InputFiles namespace in datasets.h. If an invalid code
  is entered, throw a std::invalid_argument with the message:
  No dataset matches key: <input code>
  where <input name> is the name supplied by the user through the argument.

  @param args
    Parsed program arguments

  @return
    A std::vector of BethYw::InputFileSource instances to import

  @throws
    std::invalid_argument if the argument contains an invalid dataset with
    message: No dataset matches key <input code>

  @example
    auto cxxopts = BethYw::cxxoptsSetup();
    auto args = cxxopts.parse(argc, argv);

    auto datasetsToImport = BethYw::parseDatasetsArg(args);
 */

std::vector<BethYw::InputFileSource> BethYw::parseDatasetsArg(
    cxxopts::ParseResult& args) {
  // This function is incomplete, but to get you started...
  // You may want to delete much of these // comments too!

  // Retrieve all valid datasets, see datasets.h
  size_t numDatasets = InputFiles::NUM_DATASETS;
  auto &allDatasets = InputFiles::DATASETS;

  // Create the container for the return type
  std::vector<InputFileSource> datasetsToImport;

  // You can get the std::vector of arguments from cxxopts like this.
  // Note that this function will throw an exception if datasets is not set as 
  // an argument. Check the documentation! Read it and understand it.
  if(args.count("datasets") == 0){
      for(unsigned int i = 0; i < numDatasets; i++){
          datasetsToImport.push_back(allDatasets[i]);
      }
      return datasetsToImport;
  }
  auto inputDatasets = args["datasets"].as<std::vector<std::string>>();
  for(auto input = inputDatasets.begin(); input != inputDatasets.end(); input++) {

      std::string &inputString = *input;

      if(BethYw::insensitiveEquals(inputString, "all")) {
          datasetsToImport.clear();
          for(unsigned int i = 0; i < numDatasets; i++){
              datasetsToImport.push_back(allDatasets[i]);
          }
          return datasetsToImport;
      }
      bool found = false;
      for (unsigned int x = 0; x < numDatasets; x++) {
          if (inputString == allDatasets[x].CODE) {
              datasetsToImport.push_back(allDatasets[x]);
              found = true;
          }
      }

      if(!found) {
          std::cerr << "No dataset matches key: " << inputString;
          exit(0);
      }

    }
    return datasetsToImport;
}

/*
  Parses the areas command line argument, which is optional. If it doesn't 
  exist or exists and contains "all" as value (any case), all areas should be
  imported, i.e., the filter should be an empty set.

  Unlike datasets we can't check the validity of the values as it depends
  on each individual file imported (which hasn't happened until runtime).
  Therefore, we simply fetch the list of areas and later pass it to the
  Areas::populate() function.

  The filtering of inputs should be case insensitive.

  @param args
    Parsed program arguments

  @return 
    An std::unordered_set of std::strings corresponding to specific areas
    to import, or an empty set if all areas should be imported.

  @throws
    std::invalid_argument if the argument contains an invalid areas value with
    message: Invalid input for area argument
*/
std::unordered_set<std::string> BethYw::parseAreasArg(
    cxxopts::ParseResult& args) {
  // The unordered set you will return
  std::unordered_set<std::string> areas;
  // Retrieve the areas argument like so:
  if(args.count("areas") == 0)
      return areas;

  auto areaArgs = args["areas"].as<std::vector<std::string>>();

  for(unsigned int i = 0; i < areaArgs.size(); i++) {
      if(!BethYw::insensitiveEquals(areaArgs[i], "all")){
          areas.insert(areaArgs[i]);
      }else{
          areas.clear();
          return areas;
      }
  }
  return areas;
}

/*
  Parse the measures command line argument, which is optional. If it doesn't 
  exist or exists and contains "all" as value (any case), all measures should
  be imported.

  Unlike datasets we can't check the validity of the values as it depends
  on each individual file imported (which hasn't happened until runtime).
  Therefore, we simply fetch the list of measures and later pass it to the
  Areas::populate() function.

  The filtering of inputs should be case insensitive.

  @param args
    Parsed program arguments

  @return 
    An std::unordered_set of std::strings corresponding to specific measures
    to import, or an empty set if all measures should be imported.

  @throws
    std::invalid_argument if the argument contains an invalid measures value
    with the message: Invalid input for measures argument
*/
std::unordered_set<std::string> BethYw::parseMeasuresArg(cxxopts::ParseResult& args){
    std::unordered_set<std::string> measures;
    // Retrieve the areas argument like so:
    if(args.count("measures") == 0)
        return measures;
    auto measuresArgs = args["measures"].as<std::vector<std::string>>();

    for(unsigned int i = 0; i < measuresArgs.size(); i++) {
        if(!BethYw::insensitiveEquals(measuresArgs[i], "all")){
            measures.insert(measuresArgs[i]);
        }else{
            measures.clear();
        }
    }

    return measures;

}

/*
 *TODO::
  Parse the years command line argument. Years is either a four digit year 
  value, or two four digit year values separated by a hyphen (i.e. either 
  YYYY or YYYY-ZZZZ).

  This should be parsed as two integers and inserted into a std::tuple,
  representing the start and end year (inclusive). If one or both values are 0,
  then there is no filter to be applied. If no year argument is given return
  <0,0> (i.e. to import all years). You will have to search
  the web for how to construct std::tuple objects! 

  @param args
    Parsed program arguments

  @return
    A std::tuple containing two unsigned ints


  @throws
    std::invalid_argument if the argument contains an invalid years value with
    the message: Invalid input for years argument
*/

std::tuple<unsigned int, unsigned int> BethYw::parseYearsArg(cxxopts::ParseResult& args){


    if(args.count("years") == 0)
        return std::tuple<unsigned int, unsigned int> {0,0};

    auto yearsArgs = args["years"].as<std::string>();
    if(yearsArgs.empty()){
        return std::make_tuple(0,0);
    }
    if(yearsArgs.find("-") == std::string::npos){
        unsigned int year = BethYw::validateYear(yearsArgs);
        return std::make_tuple(year,year);
    }
    std::size_t pos = yearsArgs.find("-");
    std::string first = yearsArgs.substr(0,pos);
    std::string second = yearsArgs.substr(pos+1,yearsArgs.size());
    //setting the 2 given years
    unsigned int firstYear = BethYw::validateYear(first);
    unsigned int secondYear = BethYw::validateYear(second);
    return std::make_tuple(firstYear,secondYear);
}

/*
  Load the areas.csv file from the directory `dir`. Parse the file and
  create the appropriate Area objects inside the Areas object passed to
  the function in the `areas` argument.

  areas.csv is guaranteed to be formatted as:
    Local authority code,Name (eng),Name (cym)

  Hint: To implement this function. First you will need create an InputFile 
  object with the filename of the areas file, open it, and then pass reference 
  to the stream to the Areas::populate() function.

  Hint 2: you can retrieve the specific filename for a dataset, e.g. for the 
  areas.csv file, from the InputFileSource's FILE member variable

  @param areas
    An Areas instance that should be modified (i.e. the populate() function
    in the instance should be called)

  @param dir
    Directory where the areas.csv file is

  @param areasFilter
    An unordered set of areas to filter, or empty to import all areas

  @return
    void

  @example
    Areas areas();

    BethYw::loadAreas(areas, "data", BethYw::parseAreasArg(args));
*/
void BethYw::loadAreas(Areas &areas, std::string dir, std::unordered_set<std::string> areasFilter){
    InputFile areasFile(dir + InputFiles::AREAS.FILE);
    auto fileNameCSV = InputFiles::AREAS.FILE;
    auto cols = InputFiles::AREAS.COLS;
    auto type = InputFiles::AREAS.PARSER;
    try {
        areas.populate(areasFile.open(), type, cols, &areasFilter, nullptr, nullptr);
    }catch(const std::runtime_error& error){
        std::cerr << "Error importing dataset: " << std::endl << error.what();
        exit(0);
    }
}

/*
  Import datasets from `datasetsToImport` as files in `dir` into areas, and
  filtering them with the `areasFilter`, `measuresFilter`, and `yearsFilter`.

  The actual filtering will be done by the Areas::populate() function, thus 
  you need to merely pass pointers on to these flters.

  This function should promise not to throw an exception. If there is an
  error/exception thrown in any function called by thus function, catch it and
  output 'Error importing dataset:', followed by a new line and then the output
  of the what() function on the exception.

  @param areas
    An Areas instance that should be modified (i.e. datasets loaded into it)

  @param dir
    The directory where the datasets are

  @param datasetsToImport
    A vector of InputFileSource objects

  @param areasFilter
    An unordered set of areas (as authority codes encoded in std::strings)
    to filter, or empty to import all areas

  @param measuresFilter
    An unordered set of measures (as measure codes encoded in std::strings)
    to filter, or empty to import all measures

  @param yearsFilter
    An two-pair tuple of unsigned ints corresponding to the range of years 
    to import, which should both be 0 to import all years.

  @return
    void

  @example
    Areas areas();

    BethYw::loadDatasets(
      areas,
      "data",
      BethYw::parseDatasetsArgument(args),
      BethYw::parseAreasArg(args),
      BethYw::parseMeasuresArg(args),
      BethYw::parseYearsArg(args));
*/
void BethYw::loadDatasets(Areas &areas,
                        std::string dir,
                        std::vector<InputFileSource>  datasetsToImport,
                          const StringFilterSet areasFilter,
                          const StringFilterSet measuresFilter,
                          const YearFilterTuple yearsFilter){

        for(auto const& dataset : datasetsToImport) {
            InputFile areasFile(dir + dataset.FILE);
            try{
                areas.populate(areasFile.open(), dataset.PARSER, dataset.COLS, &areasFilter, &measuresFilter, &yearsFilter);
            }catch(const std::runtime_error & error) {
                std::cerr << "Error importing dataset: " << std::endl << error.what();
                exit(0);
            }
        }
}

/*
 * Compares two string cap insensitively.

  @param a
    std::string

  @param b
    std::string

  @return
    bool

  @example
    std::string a;
    std::string b;
    bool = BethYw::insensitiveEquals(a,b);
    */
bool BethYw::insensitiveEquals(std::string const a, std::string const b){
    return convertToLower(a) == convertToLower(b);
}

/*
 * Turns a year as a string into a unsigned int
 * and check if it is valid (if it in the future or has letter)
 * SEPICAL CASE: "0" returns 0;

  @param a
    std::string

  @param b
    std::string

  @return
    bool

  @throw
    std::invalid_argument if year is before 1000 AD

  @throw
    std::invalid_argument if yearString has a no digits char

  @throw
    std::invalid_argument if dates is in the future

  @example
    std::string a;
    std::string b;
    bool = BethYw::insensitiveEquals(a,b)
    */
unsigned int BethYw::validateYear(std::string yearSting){

    if(yearSting == "0")
        return 0;

    if(yearSting.size() != 4)
        throw (std::invalid_argument("Invalid input for years argument"));

    for(char & ch : yearSting){
        if (!isdigit(ch))
            throw (std::invalid_argument("Invalid input for years argument"));
    }

    unsigned int year = std::stoi(yearSting);

    if ( year >= 2021)
        throw (std::invalid_argument("Invalid input for years argument"));

    return year;
}

/*
    Covert string into a string with only lower case letters

  @param string
    std::string

  @return
    std::string

  @example
    std::string message = "YoU WaNT TO gIVe me FuLL MaRKs";
    std::string newMessage = BethYw::convertToLower(message);
    (newMessage == "you want to give me full marks) == ture;
 */
std::string BethYw::convertToLower(const std::string string) {
    std::string lower = "";
    for (unsigned i = 0; i < string.size(); i++)
        lower += std::tolower(string[i]);
    return lower;
}

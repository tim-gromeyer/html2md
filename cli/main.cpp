#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "html2md.h"

using namespace std;

constexpr const char *description =
        " [Options]\n\n"
        "Simple and fast HTML to Markdown converter with table support.\n\n"
        "Options:\n"
        "  -h, --help\tDisplays this help information.\n"
        "  -v, --version\tPrint html2md's version.\n"
        "  -o, --output\tSets the output file.\n"
        "  -i, --input\tSets the input file or text.\n"
        "  -p, --print\tPrint Markdown(overrides -o).\n";


int main(int argc, const char* argv[])
{
  string input;
  string inFile;
  string outFile = "Converted.md";

  bool print = false;

  for (int i = 0; i < argc; ++i) {
    string item = argv[i];

    if (item == "-h" || item == "--help" || argc == 1) {
      cout << argv[0] << description;
      return 0;
    }
    else if (item == "-v" || item == "--version") {
        cout << "Version " << VERSION << endl;
        return 0;
    }
    else if (item == "-p" || item == "--print") {
      print = true;
      continue;
    }

    if (i +1 == argc && i != 0) {
      cout << "Nothing defined for option " << item << ". Leaving now." << endl;
      return 0;
    }
    else if (item == "-o" || item == "--output")
      outFile = argv[++i];
    else if (item == "-i" || item == "--input")
      inFile = argv[++i];
    else if (i != 0)
      cout << "Invalid argument: " << item << endl;
  }

  if (inFile.empty()) {
      cout << "Input empty! See --help for more info." << endl;
      return 0;
  }
  else if (filesystem::exists(inFile)) {
    ifstream in(inFile);
    stringstream buffer;
    buffer << in.rdbuf();
    input = move(buffer.str());

    if (in.fail()) {
      cerr << "Error: " << strerror(errno) << ": " << inFile << endl;
      return 0;
    }

    in.close();
  }
  else input = move(inFile);

  auto html = html2md::Convert(input);

  if (print) {
    cout << html << endl;
    return 0;
  }


  if (filesystem::exists(outFile)) {
    string override;
    cout << outFile << " already exists, override? [y/n] ";
    cin >> override;
    if (override == "n") return 0;
    else if (override != "y") {
      cout << endl << "Invalid input." << endl;
      return 0;
    }
  }

  ofstream out(outFile);
  out.open(outFile);
  if (!out.is_open()) {
    cout << html << endl;
    return 0;
  }

  out << html;
  out.close();

  if (out.fail()) {
    cerr << "Error: " << strerror(errno) << ": " << outFile << endl;
    return -1;
  }

  cout << "Markdown written to " << outFile << endl;

  return 0;
}

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "html2md.h"

using namespace std;

namespace file {
bool exists(const std::string& name) {
  ifstream f(name.c_str());
  return f.good();
}
}

constexpr const char * const description =
        " [Options]\n\n"
        "Simple and fast HTML to Markdown converter with table support.\n\n"
        "Options:\n"
        "  -h, --help\tDisplays this help information.\n"
        "  -v, --version\tDisplay version information and exit.\n"
        "  -o, --output\tSets the output file.\n"
        "  -i, --input\tSets the input file or text.\n"
        "  -p, --print\tPrint Markdown(overrides -o).\n"
        "  -r, --replace\tOverwrite the output file (if it already exists) without asking.\n";


int main(int argc, const char* argv[]) {
  string input;
  string inFile;
  string outFile = "Converted.md";

  bool print = false;
  bool replace = false;

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
    else if (item == "-r" || item == "--replace" || item == "--override") {
      replace = true;
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
  } else if (file::exists(inFile)) {
    ifstream in(inFile);
    stringstream buffer;
    buffer << in.rdbuf();
    input = std::move(buffer.str());

    if (in.bad()) {
      cerr << "Error: " << strerror(errno) << ": " << inFile << endl;
      return 0;
    }

    in.close();
  }
  else input = std::move(inFile);

  auto md = html2md::Convert(input);

  if (print) {
    cout << md << endl;
    return 0;
  }


  if (file::exists(outFile) && !replace) {
    string override;

    while (true) {
      cout << outFile << " already exists, override? [y/n] ";
      cin >> override;
      if (override == "n" || override == "N") return 0;
      else if (override == "y" || override == "Y") break;
      else override.clear();
    }
  }

  fstream out;
  out.open(outFile, ios::out);
  if (!out.is_open()) {
    cout << md << endl;
    return 0;
  }

  out << md;
  out.close();

  if (out.bad()) {
    cerr << "Error: " << strerror(errno) << ": " << outFile << endl;
    return 1;
  }

  cout << "Markdown written to " << outFile << endl;

  return 0;
}

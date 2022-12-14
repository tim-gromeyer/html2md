#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "html2md.h"

using std::string;
using std::ifstream;
using std::fstream;
using std::cerr;
using std::cin;
using std::ios;
using std::cout;
using std::endl;
using std::stringstream;

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

  if (argc == 1) {
      cout << argv[0] << description;
      return 0;
  }

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      cout << argv[0] << description; // Show help
      return 0;
    }
    else if (arg == "-v" || arg == "--version") {
      cout << "Version " << VERSION << endl;
      return 0;
    }
    else if (arg == "-p" || arg == "--print") {
      print = true;
    }
    else if (arg == "-r" || arg == "--replace") {
      replace = true;
    }
    else if (arg == "-o" || arg == "--output") {
      if (i + 1 < argc) {
        outFile = argv[i + 1];
        i++;
      }
      else {
        std::cerr << "The" << arg <<
            "option requires a file name!\n'Converted.md' is used." << std::endl;
      }
    }
    else if (arg == "-i" || arg == "--input") {
      if (i + 1 < argc) {
        inFile = argv[i + 1];
        i++;
      }
      else {
        std::cerr << "The" << arg <<
            "option requires a filename or HTML text!" << std::endl;
        return 1;
      }
    }
  }

  if (file::exists(inFile)) {
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
      getline(cin, override);
      if (override.empty()) continue;

      stringstream ss(override);
      string in;
      ss >> in;
      if (in.empty() || in.length() > 1) {
          cout << "Invalid input" << endl;
          continue;
      }

      char c = (char)tolower(in[0]);
      if (c == 'n') return 0;
      else if (c == 'y') break;
      else {
          cout << "Invalid input" << endl;
          continue;
      }
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

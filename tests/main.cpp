#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "html2md.h"
#include "md4c-html.h"
#include "table.h"

using std::cerr;
using std::cout;
using std::ifstream;
using std::string;
using std::stringstream;
using std::vector;
using std::chrono::duration;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
namespace fs = std::filesystem;

namespace markdown {
void captureHtmlFragment(const MD_CHAR *data, const MD_SIZE data_size,
                         void *userData) {
  auto *str = static_cast<stringstream *>(userData);

  str->write(data, data_size);
}

string toHTML(const string &md) {
  stringstream html;

  static MD_TOC_OPTIONS options;

  md_html(md.c_str(), md.size(), &captureHtmlFragment, &html, MD_DIALECT_GITHUB,
          MD_HTML_FLAG_SKIP_UTF8_BOM, &options);

  return html.str();
};

string fromHTML(string &html) {
  static html2md::Options options;
  options.splitLines = false;

  html2md::Converter c(html, &options);
  return c.convert();
}
} // namespace markdown

namespace file {
string readAll(const string &name) {
  ifstream in(name);
  stringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
};
} // namespace file

// Log the error
void log(const string &file, const string &origMd, const string &generatedMd) {
  cerr << "Task " << fs::path(file).filename() << " failed:\nOriginal Md:\n"
       << origMd << "\nGenerated Markdown:\n"
       << generatedMd << '\n';
}

// Print "Running " + filename
void running(const string &file) {
  cout << "Running test " << fs::path(file).filename() << "...\t";
}

// Print "Passed!" in green
void passed() { cout << "\x1B[32mPassed!\033[0m\n"; }

// Print "Failed!" in red
void error() { cout << "\x1B[31mFailed!\033[0m\n"; }

void runTest(const string &file, short *errorCount) {
  // Read the markdown file
  const string md = file::readAll(file);

  running(file);

  // Convert the Md to HTML
  string html = markdown::toHTML(md);

  // Generate Md from the HTML
  string convertedMd = markdown::fromHTML(html);

  // Convert it back to HTML
  string testHTML = markdown::toHTML(convertedMd);

  // Compare original and result HTML
  if (html == testHTML)
    passed();
  else {
    error();
    log(file, md, convertedMd);
    ++*errorCount;
  }
}

void testOption(const char *name) {
  cout << "Test option \"" << name << "\"...\t";
}

bool testUnorderedList() {
  testOption("unorderedList");

  string html = "<ul><li>List</li></ul>";

  html2md::Options o;
  o.unorderedList = '*';

  html2md::Converter c(html, &o);

  auto md = c.convert();

  return md.find("* List\n") != string::npos;
}

bool testOrderedList() {
  testOption("orderedList");

  string html = "<ol><li>List</li></ol>";

  html2md::Options o;
  o.orderedList = ')';

  html2md::Converter c(html, &o);

  auto md = c.convert();

  return md.find("1) List\n") != string::npos;
}

bool testDisableTitle() {
  testOption("includeTitle");

  string html = "<title>HTML title</title>";

  html2md::Options o;
  o.includeTitle = false;

  html2md::Converter c(html, &o);

  auto md = c.convert();

  return md.empty() &&
         html2md::Convert(html).find("HTML title") != string::npos;
}

bool testFormatTable() {
  testOption("formatTable");

  constexpr const char *inputTable = "| 1 | 2 | 3 |\n"
                                     "| :-- | :-: | --: |\n"
                                     "| Hello | World | ! |\n"
                                     "| foo | bar | buzz |\n";

  constexpr const char *expectedOutput = "| 1     | 2     | 3    |\n"
                                         "|:------|:-----:|-----:|\n"
                                         "| Hello | World | !    |\n"
                                         "| foo   | bar   | buzz |\n";

  string formattedTable = formatMarkdownTable(inputTable);

  return formattedTable == expectedOutput;
}

bool testAttributeWhitespace() {
  testOption("attributeWhitespace");

  // Test different variations of whitespace around equals sign
  vector<string> testCases = {
      "<a href=\"http://example.com/\">no space</a>",
      "<a href =\"http://example.com/\">space before</a>",
      "<a href= \"http://example.com/\">space after</a>",
      "<a href = \"http://example.com/\">space both sides</a>"};

  for (const auto &html : testCases) {
    html2md::Converter c(html);
    auto md = c.convert();

    // Basic check that the conversion worked
    if (md.empty()) {
      cerr << "Failed to convert: " << html << "\n";
      return false;
    }

    // For anchor tags, check if URL was properly extracted
    if (html.find("<a") != string::npos) {
      if (md.find("http://example.com/") == string::npos) {
        cerr << "Failed to extract URL from: " << html << "\n";
        return false;
      }
    }
  }

  return true;
}

bool testUppercaseTags() {
  testOption("uppercaseTags");

  vector<string> testCases = {"<DIV>Uppercase div</DIV>",
                              "<P>Uppercase paragraph</P>",
                              "<STRONG>Uppercase strong</STRONG>",
                              "<EM>Uppercase em</EM>",
                              "<H1>Uppercase h1</H1>",
                              "<BLOCKQUOTE>Uppercase blockquote</BLOCKQUOTE>"};

  for (const auto &html : testCases) {
    html2md::Converter c(html);
    auto md = c.convert();

    if (md.empty()) {
      cerr << "Failed to convert uppercase tag: " << html << "\n";
      return false;
    }

    // Check that content was properly converted
    if (md.find("Uppercase") == string::npos) {
      cerr << "Content missing from uppercase tag conversion: " << html << "\n";
      return false;
    }
  }

  return true;
}

bool testUppercaseAttributes() {
  testOption("uppercaseAttributes");

  vector<string> testCases = {
      "<a HREF=\"http://example.com\" TITLE=\"Example\">link</a>",
      "<img SRC=\"image.png\" ALT=\"Image\">",
      "<div CLASS=\"container\" STYLE=\"color:red\">content</div>"};

  for (const auto &html : testCases) {
    html2md::Converter c(html);
    auto md = c.convert();

    if (md.empty()) {
      cerr << "Failed to convert uppercase attributes: " << html << "\n";
      return false;
    }

    // For anchor tags, check if URL was properly extracted
    if (html.find("<a") != string::npos) {
      if (md.find("http://example.com") == string::npos) {
        cerr << "Failed to extract URL from uppercase attributes: " << html
             << "\n";
        return false;
      }
    }

    // For images, check if src was extracted
    if (html.find("<img") != string::npos) {
      if (md.find("image.png") == string::npos) {
        cerr << "Failed to extract SRC from uppercase attributes: " << html
             << "\n";
        return false;
      }
    }
  }

  return true;
}

bool testMixedCaseTags() {
  testOption("mixedCaseTags");

  vector<string> testCases = {"<DiV>Mixed case div</DiV>",
                              "<p>Mixed case paragraph</p>",
                              "<StRoNg>Mixed case strong</StRoNg>",
                              "<eM>Mixed case em</eM>",
                              "<h1>Mixed case h1</h1>",
                              "<BlockQuote>Mixed case blockquote</BlockQuote>"};

  for (const auto &html : testCases) {
    html2md::Converter c(html);
    auto md = c.convert();

    if (md.empty()) {
      cerr << "Failed to convert mixed case tag: " << html << "\n";
      return false;
    }

    // Check that content was properly converted
    if (md.find("Mixed case") == string::npos) {
      cerr << "Content missing from mixed case tag conversion: " << html
           << "\n";
      return false;
    }
  }

  return true;
}

bool testSelfClosingUppercaseTags() {
  testOption("selfClosingUppercaseTags");

  vector<string> testCases = {"<BR/>", "<HR/>", "<IMG SRC=\"image.png\"/>",
                              "<INPUT TYPE=\"text\"/>"};

  for (const auto &html : testCases) {
    html2md::Converter c(html);
    auto md = c.convert();

    if (html.find("<IMG") != string::npos) {
      // For images, we expect some output
      if (md.empty()) {
        cerr << "Failed to convert self-closing uppercase tag: " << html
             << "\n";
        return false;
      }
    } else {
      // For other self-closing tags, empty output is acceptable
      continue;
    }
  }

  return true;
}

bool testWhitespaceTags() {
  testOption("whitespaceTags");

  // Test cases with various tags containing whitespace
  vector<std::pair<string, string>> testCases = {
      // { HTML input, Expected Markdown output }
      {"< p >Hello</ p >", "Hello\n"},
      {"< p>Text</  p >", "Text\n"},
      {"<p >Text</p  >", "Text\n"}
  };

  for (const auto &[html, expectedMd] : testCases) {
    html2md::Converter c(html);
    auto md = c.convert();

    if (md != expectedMd) {
      cout << "Failed to convert whitespace tag: " << html << "\n"
           << "Expected Markdown: " << expectedMd << "\n"
           << "Generated Markdown: " << md << "\n";
      return false;
    }
  }

  return true;
}

// Test self closing tags <a href=\"http://example1.com/\">First</a>  <br/> then <a href=\"http://example2.com\">second</a>
bool testSelfClosingTags() {
  testOption("selfClosingTags");

  string html = "<a href=\"http://example1.com/\">First</a>  <br/> then <a href=\"http://example2.com\">second</a>";

  html2md::Converter c(html);
  auto md = c.convert();

  return md.find("[First](http://example1.com/)") != string::npos &&
         md.find("[second](http://example2.com)") != string::npos &&
         md.find("  \n") != string::npos;
}

bool testZeroWidthSpaceWithBlockquote() {
  testOption("zeroWidthSpaceWithBlockquote");

  std::vector<std::pair<std::string, std::string>> testCases = {
      // { HTML input, Expected Markdown output }
      {"<html><body>Text<span>\xe2\x80\x8b</span><blockquote>a</blockquote></"
       "body></html>",
       "Text\u200b\n> a\n"},
      {"<html><body>Text<span> </span><blockquote>a</blockquote></body></html>",
       "Text\n> a\n"},
      {"<html><body>Text<blockquote>a\nb</blockquote></body></html>",
       "Text\n> a\n> b\n"}};

  for (const auto &[html, expectedMd] : testCases) {
    html2md::Converter c(html);
    auto md = c.convert();

    if (md != expectedMd) {
      cout << "Failed to convert HTML with zero-width space and blockquote: "
           << html << "\n"
           << "Expected Markdown: " << expectedMd << "\n"
           << "Generated Markdown: " << md << "\n";
      return false;
    }
  }

  return true;
}

bool testInvalidTags() {
  testOption("invalidTags");

  // Test cases with various invalid tags
  vector<string>
      testCases =
          {
              "<p>Valid <invalid>tag</invalid></p>",
              "<p>Self-closing <invalid/></p>",
              "<p>Nested "
              "<invalid><moreinvalid>tags</moreinvalid></invalid></p>",
              "<p>V<sub>i</sub> <a "
              "href=\"http://example.com/\">example</a></p>", // The specific
                                                              // test case from
                                                              // the issue
              "<p>Text with <123invalid>tag</123invalid></p>",
              "<p>Text with <invalid@tag>content</invalid@tag></p>"};

  vector<string> expectedOutputs = {
      "Valid tag\n",
      "Self-closing\n",
      "Nested tags\n",
      "Vi [example](http://example.com/)\n", // Expected output for
                                                        // the specific test
                                                        // case
      "Text with tag\n",
      "Text with content\n"};

  for (size_t i = 0; i < testCases.size(); i++) {
    html2md::Converter c(testCases[i]);
    auto md = c.convert();

    if (md != expectedOutputs[i]) {
      cout << "Failed to handle invalid tags:\n"
           << "Input: " << testCases[i] << "\n"
           << "Expected: " << expectedOutputs[i] << "\n"
           << "Got: " << md << "\n";
      return false;
    }
  }

  return true;
}

int main(int argc, const char **argv) {
  // List to store all markdown files in this dir
  vector<string> files;

  static vector<string> markdownExtensions = {".md", ".markdown", ".mkd"};

  // Find the files
  for (const auto &p : fs::recursive_directory_iterator(DIR)) {
    if (std::find(markdownExtensions.begin(), markdownExtensions.end(),
                  p.path().extension()) != markdownExtensions.end() &&
        p.path().parent_path() == DIR)
      files.emplace_back(p.path().string());
  }

  // Test files passed as argument
  for (int i = 1; i < argc; i++) {
    // Check if the argument is a valid file path and ends with ".md"
    string file = argv[i];
    if (fs::is_regular_file(file) && file.find(".md") == file.size() - 3) {
      files.emplace_back(file);
    }
  }

  // Sort file names
  sort(files.begin(), files.end());

  // File name
  const char *errorFileName = DIR "/error.log";

  // Redirect errors to error.log
  FILE *errorFile = freopen(errorFileName, "w", stderr);
  if (!errorFile)
    cerr << "Failed to open " << errorFileName
         << " for whatever reason!\n"
            "Errors will be printed to the terminal instead of written to the "
            "mentioned file above.";

  // For measuring time.
  auto t1 = high_resolution_clock::now();

  // Count the errors
  short errorCount = 0;

  // Run the tests
  for (auto &file : files)
    runTest(file, &errorCount);

  // Test the options
  auto tests = {&testDisableTitle,
                &testUnorderedList,
                &testOrderedList,
                &testFormatTable,
                &testAttributeWhitespace,
                &testUppercaseTags,
                &testUppercaseAttributes,
                &testMixedCaseTags,
                &testSelfClosingUppercaseTags,
                &testWhitespaceTags,
                &testSelfClosingTags,
                &testZeroWidthSpaceWithBlockquote,
                &testInvalidTags,
              };

  for (const auto &test : tests)
    if (!test()) {
      ++errorCount;
      error();
    } else
      passed();

  auto t2 = high_resolution_clock::now();

  /* Getting number of milliseconds as a double. */
  duration<double, std::milli> ms_double = t2 - t1;

  cout << files.size() + tests.size() << " tests executed in "
       << ms_double.count() << "ms. " << errorCount << " failed.\n";

  return 0;
}

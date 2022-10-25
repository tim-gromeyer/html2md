#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "html2md.h"
#include "md4c-html.h"


using namespace std;
namespace fs = std::filesystem;

short test_index = 0;

namespace markdown {
void captureHtmlFragment(const MD_CHAR* data, const MD_SIZE data_size, void* userData) {
    auto *str = static_cast<string*>(userData);

    str->append(data, data_size);
}

string toHTML(const string &md) {
    string html;

    md_html(md.c_str(), md.size(), &captureHtmlFragment, &html,
            MD_DIALECT_GITHUB, MD_HTML_FLAG_SKIP_UTF8_BOM);

    return html;
};

string fromHTML(string &html) {
    return html2md::Convert(html);
}
}

namespace file {
string readAll(const string &name) {
    ifstream in(name);
    stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
};
}

void log(const string &file, const string &html, const string &htmlOut) {
    cerr << "Task " << fs::path(file).filename() << " failed:\nOriginal HTML:\n" << html << "\nHTML generated from generated Markdown:\n" << htmlOut << '\n';
}

void running(const string &file) {
    cout << "Running test " << fs::path(file).filename() << "... ";
}

void passed(int number, const char *description = nullptr) {
    cout << "\x1B[32mPassed!\033[0m\n";
}

void error(int number, const char *description = nullptr) {
    cout << "\x1B[31mFailed :(\033[0m\n";
}

void runTest(const string& file) {
    const string md = file::readAll(file);

    ++test_index;

    short curr = test_index;

    running(file);

    string html = markdown::toHTML(md);

    string convertedMd = markdown::fromHTML(html);

    string testHTML = markdown::toHTML(convertedMd);

    if (html == testHTML)
        passed(curr);
    else {
        error(curr);
        log(file, html, testHTML);
    }
}

int main(int argc, char ** argv){
    vector<string> files;

    string ext(".md");
    for (const auto &p : fs::recursive_directory_iterator(DIR))
    {
        if (p.path().extension() == ext && p.path().parent_path() == DIR)
            files.emplace_back(p.path().c_str());
    }

    // Redirect errors to error.log
    freopen("./error.log", "w", stderr);

    for (auto &str : files)
        runTest(str);

    return 0;
}

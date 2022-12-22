#include <emscripten/bind.h>
#include "html2md.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(html2md) {
  class_<html2md::options>("Options")
    .constructor<>()
    .property("splitLines", &html2md::options::splitLines, &html2md::options::splitLines)
    .property("unorderedList", &html2md::options::unorderedList, &html2md::options::unorderedList)
    .property("orderedList", &html2md::options::orderedList, &html2md::options::orderedList)
    .property("includeTitle", &html2md::options::includeTitle, &html2md::options::includeTitle);

  class_<html2md::Converter>("Converter")
    .constructor<std::string, html2md::options*>()
    .function("convert2Md", &html2md::Converter::Convert2Md)
    .function("ok", &html2md::Converter::ok);

  function("convert", &html2md::Convert);
}


#include <html2md.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

PYBIND11_MODULE(pyhtml2md, m) {
  m.doc() = "Python bindings for html2md"; // optional module docstring

  // Options class bindings
  py::class_<html2md::Options>(m, "Options")
      .def(py::init<>())
      .def_readwrite(
          "splitLines", &html2md::Options::splitLines,
          "Add new line when a certain number of characters is reached")
      .def_readwrite("softBreak", &html2md::Options::softBreak,
                     "Wrap after ... characters when the next space is reached")
      .def_readwrite("hardBreak", &html2md::Options::hardBreak,
                     "Force a break after ... characters in a line")
      .def_readwrite("unorderedList", &html2md::Options::unorderedList,
                     "The char used for unordered lists")
      .def_readwrite("orderedList", &html2md::Options::orderedList,
                     "The char used after the number of the item")
      .def_readwrite("includeTitle", &html2md::Options::includeTitle,
                     "Whether title is added as h1 heading at the very "
                     "beginning of the markdown")
      .def_readwrite("formatTable", &html2md::Options::formatTable,
                     "Whether to format Markdown Tables")
      .def("__eq__", &html2md::Options::operator==);

  py::class_<html2md::Converter>(m, "Converter")
      .def(py::init<std::string &, html2md::Options *>(),
           "Class for converting HTML to Markdown", py::arg("html"),
           py::arg("options") = py::none())
      .def("convert", &html2md::Converter::convert,
           "This function actually converts the HTML into Markdown.")
      .def("ok", &html2md::Converter::ok,
           "Checks if everything was closed properly(in the HTML).")
      .def("__call__", &html2md::Converter::operator bool);

  m.def("convert", &html2md::Convert,
        "Static wrapper around the Converter class", py::arg("html"),
        py::arg("ok") = py::none());
}

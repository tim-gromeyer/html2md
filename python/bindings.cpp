#include <html2md.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

PYBIND11_MODULE(pyhtml2md, m) {
    m.doc() = "Python bindings for html2md"; // optional module docstring

    py::class_<html2md::options>(m, "Options")
        .def(py::init<>())
        .def_readwrite("splitLines", &html2md::options::splitLines, "Add new line when a certain number of characters is reached")
        .def_readwrite("unorderedList", &html2md::options::unorderedList, "The char used for unordered lists")
        .def_readwrite("orderedList", &html2md::options::orderedList, "The char used after the number of the item")
        .def_readwrite("includeTitle", &html2md::options::includeTitle, "Whether title is added as h1 heading at the very beginning of the markdown");

    py::class_<html2md::Converter>(m, "Converter")
        .def(py::init<std::string&, html2md::options*>(), "Class for converting HTML to Markdown",
             py::arg("html"), py::arg("options") = py::none())
        .def("convert", &html2md::Converter::Convert2Md, "This function actually converts the HTML into Markdown.")
        .def("ok", &html2md::Converter::ok, "Checks if everything was closed properly(in the HTML).");

    m.def("convert", &html2md::Convert, "Static wrapper around the Converter class",
          py::arg("html"), py::arg("ok") = py::none());
}

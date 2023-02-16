/*********************************************************************
** Copyright (c) 2022 Roger Lee.
** Computational and Interpretation Group (CIG),
** University of Science and Technology of China (USTC).
**
** @File: PySegy.cpp
** @Time: 2022/12/05 14:36:48
** @Version: 1.0
** @Description :
*********************************************************************/

#include "segy.h"
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <utility>

namespace py = pybind11;

class Pysegy : public segy::SegyIO {
public:
  using segy::SegyIO::create;
  using segy::SegyIO::read;
  using segy::SegyIO::read_cross_slice;
  using segy::SegyIO::read_inline_slice;
  using segy::SegyIO::read_time_slice;
  using segy::SegyIO::read_trace;
  using segy::SegyIO::SegyIO;

  py::array_t<float> read(int startZ, int endZ, int startY, int endY,
                          int startX, int endX);
  py::array_t<float> read();
  py::array_t<float> read_inline_slice(int iZ);
  py::array_t<float> read_cross_slice(int iY);
  py::array_t<float> read_time_slice(int iX);
  py::array_t<float> read_trace(int iZ, int iY);

  void create(const std::string &segy_out_name, const py::array_t<float> &src);
};

// Be careful the order of the dimensions
// In segy, X (time) is the first, but when you read into python,
// Z (inline) is the first. If need change it to X first,
// use data.transpose() in python
py::array_t<float> Pysegy::read(int startZ, int endZ, int startY, int endY,
                                int startX, int endX) {

  if (startX >= endX || startY >= endY || startZ >= endZ) {
    throw std::runtime_error("Index 'end' must large than 'start'");
  }
  if (startX < 0 || endX > shape(0) || startY < 0 || endY > shape(1) ||
      startZ < 0 || endZ > shape(2)) {
    throw std::runtime_error("Index out of range");
  }

  int sizeX = endX - startX;
  int sizeY = endY - startY;
  int sizeZ = endZ - startZ;
  py::array_t<float> out({sizeZ, sizeY, sizeX});
  auto buff = out.request();
  float *ptr = static_cast<float *>(buff.ptr);
  read(ptr, startX, endX, startY, endY, startZ, endZ);
  return out;
}

py::array_t<float> Pysegy::read() {
  py::array_t<float> out({shape(2), shape(1), shape(0)});
  auto buff = out.request();
  float *ptr = static_cast<float *>(buff.ptr);
  read(ptr);
  return out;
}
py::array_t<float> Pysegy::read_inline_slice(int iZ) {
  py::array_t<float> out({shape(1), shape(0)});
  auto buff = out.request();
  float *ptr = static_cast<float *>(buff.ptr);
  read_inline_slice(ptr, iZ);
  return out;
}
py::array_t<float> Pysegy::read_cross_slice(int iY) {
  py::array_t<float> out({shape(2), shape(0)});
  auto buff = out.request();
  float *ptr = static_cast<float *>(buff.ptr);
  read_cross_slice(ptr, iY);
  return out;
}
py::array_t<float> Pysegy::read_time_slice(int iX) {
  py::array_t<float> out({shape(2), shape(1)});
  auto buff = out.request();
  float *ptr = static_cast<float *>(buff.ptr);
  read_time_slice(ptr, iX);
  return out;
}

py::array_t<float> Pysegy::read_trace(int iZ, int iY) {
  py::array_t<float> out(shape(0));
  auto buff = out.request();
  float *ptr = static_cast<float *>(buff.ptr);
  read_trace(ptr, iY, iZ);
  return out;
}

void Pysegy::create(const std::string &segy_out_name,
                    const py::array_t<float> &src) {
  auto buff = src.request();
  if (buff.ndim != 3) {
    throw std::runtime_error("Input data must be a 3D data.");
  }
  auto r = src.unchecked<3>();
  set_size(r.shape(2), r.shape(1), r.shape(0));

  float *ptr = static_cast<float *>(buff.ptr);
  create(segy_out_name, ptr);
}

void create_by_sharing_header(const std::string &segy_name,
                              const std::string &header_segy,
                              const py::array_t<float> &src, int iline = 189,
                              int xline = 193) {
  auto buff = src.request();
  if (buff.ndim != 3) {
    throw std::runtime_error("Input data must be a 3D data.");
  }

  auto r = src.unchecked<3>();
  float *ptr = static_cast<float *>(buff.ptr);

  segy::create_by_sharing_header(segy_name, header_segy, ptr, r.shape(2),
                                 r.shape(1), r.shape(0), iline, xline);
}

py::array_t<float> fromfile_ignore_header(const std::string &segy_name,
                                          int sizeZ, int sizeY, int sizeX,
                                          int format = 5) {
  py::array_t<float> out({sizeZ, sizeY, sizeX});
  auto buff = out.request();
  float *ptr = static_cast<float *>(buff.ptr);
  segy::read_ignore_header(segy_name, ptr, sizeX, sizeY, sizeZ, format);
  return out;
}

py::array_t<float> fromfile(const std::string &segy_name, int iline = 189,
                            int xline = 193) {
  Pysegy segy_data(segy_name);
  segy_data.setInlineLocation(iline);
  segy_data.setCrosslineLocation(xline);
  segy_data.scan();
  py::array_t<float> out = segy_data.read();

  return out;
}

std::pair<py::array_t<float>, py::array_t<int>>
collect(const std::string &segy_in, int iline = 189, int xline = 193,
        int xfield = 73, int yfield = 77) {
  Pysegy segy(segy_in);
  segy.setInlineLocation(iline);
  segy.setCrosslineLocation(xline);
  segy.setXLocation(xfield);
  segy.setYLocation(yfield);

  auto data =
      py::array_t<float>({static_cast<int>(segy.trace_count()), segy.shape(0)});
  auto buff = data.request();
  float *ptr = static_cast<float *>(buff.ptr);

  auto header = py::array_t<int>({static_cast<int>(segy.trace_count()), 4});
  auto buffh = header.request();
  int *ptrh = static_cast<int *>(buffh.ptr);

  segy.collect(ptr, ptrh);

  std::pair<py::array_t<float>, py::array_t<int>> out(data, header);
  return out;
}

template <typename... Args>
using overload_cast_ = pybind11::detail::overload_cast_impl<Args...>;

PYBIND11_MODULE(cigsegy, m) {
  py::class_<Pysegy>(m, "Pysegy")
      .def(py::init<std::string>())
      .def(py::init<int, int, int>())
      .def(py::init<std::string, int, int, int>())
      .def("setInlineLocation", &Pysegy::setInlineLocation, py::arg("iline"))
      .def("setCrosslineLocation", &Pysegy::setCrosslineLocation,
           py::arg("xline"))
      .def("setXLocation", &Pysegy::setXLocation, py::arg("xfield"))
      .def("setYLocation", &Pysegy::setYLocation, py::arg("yfield"))
      .def("setFillNoValue", &Pysegy::setFillNoValue, py::arg("fills"))
      .def("scan", &Pysegy::scan)
      .def("tofile", &Pysegy::tofile, py::arg("binary_out_name"))
      .def("read", overload_cast_<>()(&Pysegy::read), "read hole volume")
      .def("read",
           overload_cast_<int, int, int, int, int, int>()(&Pysegy::read),
           "read with index", py::arg("startZ"), py::arg("endZ"),
           py::arg("startY"), py::arg("endY"), py::arg("startX"),
           py::arg("endX"))
      .def("read_inline_slice",
           overload_cast_<int>()(&Pysegy::read_inline_slice),
           "read inline slice", py::arg("iZ"))
      .def("read_cross_slice", overload_cast_<int>()(&Pysegy::read_cross_slice),
           "read crossline slice", py::arg("iY"))
      .def("read_time_slice", overload_cast_<int>()(&Pysegy::read_time_slice),
           "read time slice", py::arg("iX"))
      .def("read_trace", overload_cast_<int, int>()(&Pysegy::read_trace),
           "read trace", py::arg("iZ"), py::arg("iY"))
      .def("setSampleInterval", &Pysegy::setSampleInterval, py::arg("dt"))
      .def("setDataFormatCode", &Pysegy::setDataFormatCode, py::arg("format"))
      .def("setStartTime", &Pysegy::setStartTime, py::arg("start_time"))
      .def("setXInterval", &Pysegy::setXInterval, py::arg("dx"))
      .def("setYInterval", &Pysegy::setYInterval, py::arg("dy"))
      .def("setMinInline", &Pysegy::setMinInline, py::arg("minInline"))
      .def("setMinCrossline", &Pysegy::setMinCrossline, py::arg("minXline"))
      .def("textual_header", &Pysegy::textual_header)
      .def("metaInfo", &Pysegy::metaInfo)
      .def("create", overload_cast_<const std::string &>()(&Pysegy::create),
           "create a segy", py::arg("segy_out_name"))
      .def("create",
           overload_cast_<const std::string &,
                          const pybind11::array_t<float> &>()(&Pysegy::create),
           "create a segy from memory", py::arg("segy_out_name"),
           py::arg("src"))
      .def("set_size", &Pysegy::set_size, py::arg("sizeX"), py::arg("sizeY"),
           py::arg("sizeZ"))
      .def("close_file", &Pysegy::close_file);

  m.def("fromfile_ignore_header", &fromfile_ignore_header,
        "read by ignoring header and specify shape", py::arg("segy_name"),
        py::arg("sizeZ"), py::arg("sizeY"), py::arg("sizeX"),
        py::arg("format") = 5);
  m.def("fromfile", &fromfile, "read from a file", py::arg("segy_name"),
        py::arg("iline") = 189, py::arg("xline") = 193);
  m.def("tofile_ignore_header", &segy::tofile_ignore_header,
        "convert to binary file by ignoring header and specify shape",
        py::arg("segy_name"), py::arg("out_name"), py::arg("sizeX"),
        py::arg("sizeY"), py::arg("sizeZ"), py::arg("format") = 5);
  m.def("tofile", &segy::tofile, "convert to binary file", py::arg("segy_name"),
        py::arg("out_name"), py::arg("iline") = 189, py::arg("xline") = 193);
  m.def("collect", &collect, "colloct all trace (data and location)",
        py::arg("segy_in"), py::arg("iline") = 189, py::arg("xline") = 193,
        py::arg("xfield") = 73, py::arg("yfield") = 77);
  m.def("create_by_sharing_header", &create_by_sharing_header,
        "create a segy file using a existed segy header", py::arg("segy_name"),
        py::arg("header_segy"), py::arg("src"), py::arg("iline") = 189,
        py::arg("xline") = 193);
}

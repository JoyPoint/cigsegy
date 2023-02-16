/*********************************************************************
** Copyright (c) 2022 Roger Lee.
** Computational and Interpretation Group (CIG),
** University of Science and Technology of China (USTC).
**
** @File: segy.cpp
** @Time: 2022/11/18 16:28:37
** @Version: 1.0
** @Description :
*********************************************************************/

#include "segy.h"
#include <chrono>
#include <cstring>
#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <stdexcept>

#include "mio.hpp"
#include "progressbar.hpp"
#include "utils.h"

namespace segy {

SegyIO::SegyIO(const std::string &segyname) {
  this->isReadSegy = true;
  memset(&this->m_metaInfo, 0, sizeof(MetaInfo));
  std::error_code error;
  this->m_source.map(segyname, error);
  if (error) {
    throw std::runtime_error("Cannot mmap segy file");
  }
  scanBinaryHeader();
}

SegyIO::SegyIO(int sizeX, int sizeY, int sizeZ) {
  this->isReadSegy = false;
  this->m_metaInfo.sizeX = sizeX;
  this->m_metaInfo.sizeY = sizeY;
  this->m_metaInfo.sizeZ = sizeZ;
  this->m_metaInfo.trace_count = sizeY * sizeZ;
  this->initMetaInfo();
}

SegyIO::SegyIO(const std::string &binaryname, int sizeX, int sizeY, int sizeZ) {
  this->isReadSegy = false;
  std::error_code error;
  this->m_source.map(binaryname, error);
  if (error) {
    throw std::runtime_error("Cannot mmap segy file");
  }
  this->m_metaInfo.sizeX = sizeX;
  this->m_metaInfo.sizeY = sizeY;
  this->m_metaInfo.sizeZ = sizeZ;
  this->m_metaInfo.trace_count = sizeY * sizeZ;
  this->initMetaInfo();
}

SegyIO::~SegyIO() {
  if (m_source.is_mapped()) {
    m_source.unmap();
  }
}

void SegyIO::setInlineLocation(int loc) {
  if (loc != 5 && loc != 9 && loc != 189) {
    fmt::print("[Warning]: You set a unusual inline field: {}, the best choice "
               "is setting it as 189 or 5 or 9.\n",
               loc);
  }
  if (loc <= 0) {
    throw std::runtime_error("Invalid location (must > 0)");
  }
  m_metaInfo.inline_field = loc;
  isScan = false;
}

void SegyIO::setCrosslineLocation(int loc) {
  if (loc != 193 && loc != 17 && loc != 21) {
    fmt::print(
        "[Warning]: You set a unusual crossline field: {}, the best choice "
        "is set it as 193 or 17 or 21.\n",
        loc);
  }
  if (loc <= 0) {
    throw std::runtime_error("Invalid location (must > 0)");
  }
  m_metaInfo.crossline_field = loc;
  isScan = false;
}

void SegyIO::setXLocation(int loc) {
  if (loc != 73 && loc != 181) {
    fmt::print("[Warning]: You set a unusual X field: {}, the best choice "
               "is set it as 73 or 181.\n",
               loc);
  }
  if (loc <= 0) {
    throw std::runtime_error("Invalid location (must > 0)");
  }
  m_metaInfo.X_field = loc;
  isScan = false;
}

void SegyIO::setYLocation(int loc) {
  if (loc != 77 && loc != 185) {
    fmt::print("[Warning]: You set a unusual Y field: {}, the best choice "
               "is set it as 77 or 185.\n",
               loc);
  }
  if (loc <= 0) {
    throw std::runtime_error("Invalid location (must > 0)");
  }
  m_metaInfo.Y_field = loc;
  isScan = false;
}

void SegyIO::setFillNoValue(float noValue) {
  m_metaInfo.fillNoValue = noValue;
  isScan = false;
}

void SegyIO::setSampleInterval(int dt) {
  if (dt <= 0) {
    throw std::runtime_error("Invalid Interval (must > 0)");
  }
  m_metaInfo.sample_interval = dt;
  isScan = false;
}

void SegyIO::setDataFormatCode(int dformat) {
  if (dformat != 1 && dformat != 5) {
    throw std::runtime_error("Don't support this data format now.");
  }
  m_metaInfo.data_format = dformat;
  isScan = false;
}

void SegyIO::setStartTime(int start_time) {
  if (start_time < 0) {
    throw std::runtime_error("Invalid start time (must >= 0)");
  }
  m_metaInfo.start_time = start_time;
  isScan = false;
}

void SegyIO::setXInterval(float dz) {
  if (dz <= 0) {
    throw std::runtime_error("Invalid interval (must > 0)");
  }
  m_metaInfo.Z_interval = dz * static_cast<float>(-m_metaInfo.scalar);
  isScan = false;
}

void SegyIO::setYInterval(float dy) {
  if (dy <= 0) {
    throw std::runtime_error("Invalid interval (must > 0)");
  }
  m_metaInfo.Y_interval = dy * static_cast<float>(-m_metaInfo.scalar);
  isScan = false;
}

void SegyIO::setMinInline(int in) {
  if (in <= 0) {
    throw std::runtime_error("Invalid line number (must > 0)");
  }
  m_metaInfo.min_inline = in;
  m_metaInfo.max_inline = in + m_metaInfo.sizeZ - 1;
  isScan = false;
}

void SegyIO::setMinCrossline(int cross) {
  if (cross <= 0) {
    throw std::runtime_error("Invalid crossline number (must > 0)");
  }
  m_metaInfo.min_crossline = cross;
  m_metaInfo.max_crossline = cross + m_metaInfo.sizeY - 1;
  isScan = false;
}

void SegyIO::scanBinaryHeader() {
  const auto *binary_header = reinterpret_cast<const BinaryHeader *>(
      m_source.data() + kTextualHeaderSize);
  m_metaInfo.data_format = swap_endian(binary_header->data_format);
  m_metaInfo.sizeX = swap_endian(binary_header->trace_length);
  m_metaInfo.sample_interval = swap_endian(binary_header->sample_interval);
  m_metaInfo.trace_count =
      (m_source.size() - kTextualHeaderSize - kBinaryHeaderSize) /
      (kTraceHeaderSize + m_metaInfo.sizeX * sizeof(float));
}

void SegyIO::scan() {
  if (!isReadSegy) {
    throw std::runtime_error(
        "'scan()' function only used in reading segy mode.");
  }

  isScan = true;
  if (m_metaInfo.inline_field == 0) {
    m_metaInfo.inline_field = kDefaultInlineField;
  }

  if (m_metaInfo.crossline_field == 0) {
    m_metaInfo.crossline_field = kDefaultCrosslineField;
  }

  if (m_metaInfo.X_field == 0) {
    m_metaInfo.X_field = kDefaultXField;
  }
  if (m_metaInfo.Y_field == 0) {
    m_metaInfo.Y_field = kDefaultYField;
  }

  // get sizeZ, i.e. line_count
  int trace_size = m_metaInfo.sizeX * sizeof(float) + kTraceHeaderSize;
  const char *start = m_source.data() + kTextualHeaderSize + kBinaryHeaderSize;
  m_metaInfo.start_time = swap_endian(
      *reinterpret_cast<const int16_t *>(start + kTStartTimeField - 1));
  m_metaInfo.scalar = swap_endian(
      *reinterpret_cast<const int16_t *>(start + kTScalarField - 1));

  // line x: ... trace1
  // line x+1: trace2 ...
  TraceInfo trace1{}, trace2{};
  get_TraceInfo(start, trace1);
  get_TraceInfo(start + static_cast<uint64_t>(m_metaInfo.trace_count - 1) *
                            trace_size,
                trace2);
  m_metaInfo.sizeZ = trace2.inline_num - trace1.inline_num + 1;
  m_metaInfo.min_inline = trace1.inline_num;
  m_metaInfo.max_inline = trace2.inline_num;

  // init this two field
  m_metaInfo.min_crossline = trace1.crossline_num;
  m_metaInfo.max_crossline = trace2.crossline_num;

  if (m_metaInfo.sizeZ > kMaxSizeOneDimemsion ||
      m_metaInfo.trace_count / m_metaInfo.sizeZ == 0 || m_metaInfo.sizeZ < 2) {
    throw std::runtime_error(
        "Size Z (inline number) is invalid, don't support. Maybe the "
        "inline location is wrong, use 'setInlineLocation(loc)' to set.");
  }

  m_metaInfo.isNormalSegy =
      m_metaInfo.trace_count % m_metaInfo.sizeZ == 0 ? true : false;
  m_lineInfo.resize(m_metaInfo.sizeZ);

  // fill m_lineInfo
  int jump = m_metaInfo.trace_count / m_metaInfo.sizeZ;
  m_metaInfo.sizeY = jump;
  int64_t itrace = 0;
  get_TraceInfo(start, trace2);
  for (int i = 0; i < m_metaInfo.sizeZ - 1; i++) {
    m_lineInfo[i].trace_start = itrace;
    m_lineInfo[i].line_num = trace2.inline_num;
    m_lineInfo[i].count = 1;

    if (trace2.crossline_num < m_metaInfo.min_crossline) {
      m_metaInfo.min_crossline = trace2.crossline_num;
    }

    itrace += jump;
    get_TraceInfo(start + itrace * trace_size, trace2);
    get_TraceInfo(start + (itrace - 1) * trace_size, trace1);

    if (trace2.inline_num == m_lineInfo[i].line_num) {
      m_metaInfo.isNormalSegy = false;
      while (trace2.inline_num != m_lineInfo[i].line_num + 1 &&
             itrace < m_metaInfo.trace_count &&
             m_metaInfo.sizeY <= kMaxSizeOneDimemsion) {
        itrace++;
        jump++;
        if (jump > kMaxSizeOneDimemsion || itrace >= m_metaInfo.trace_count) {
          throw std::runtime_error(
              "inline/crossline location is wrong, use "
              "'setInlineLocation(loc)'/'setCrosslineLocation(loc)' to set");
        }
        get_TraceInfo(start + itrace * trace_size, trace2);
      }
      if (jump > m_metaInfo.sizeY) {
        m_metaInfo.sizeY = jump;
      }
    } else if (trace1.inline_num > m_lineInfo[i].line_num) {
      m_metaInfo.isNormalSegy = false;
      while (trace1.inline_num != m_lineInfo[i].line_num && itrace > 0 &&
             jump > 0) {
        itrace--;
        jump--;
        if (jump <= 0 || itrace <= 0) {
          throw std::runtime_error(
              "inline/crossline location is wrong, use "
              "'setInlineLocation(loc)'/'setCrosslineLocation(loc)' to set");
        }
        trace2 = trace1;
        get_TraceInfo(start + (itrace - 1) * trace_size, trace1);
      }
    }

    if (trace2.inline_num == m_lineInfo[i].line_num + 1 &&
        trace1.inline_num == m_lineInfo[i].line_num) {
      if (trace1.crossline_num > m_metaInfo.max_crossline) {
        m_metaInfo.max_crossline = trace1.crossline_num;
      }
      m_lineInfo[i].trace_end = itrace - 1;
      m_lineInfo[i].count = itrace - m_lineInfo[i].trace_start;
    } else {
      throw std::runtime_error("Cannot analysis this segy file");
    }
  }

  // the last line
  m_lineInfo[m_metaInfo.sizeZ - 1].trace_start = itrace;
  m_lineInfo[m_metaInfo.sizeZ - 1].line_num = trace2.inline_num;
  m_lineInfo[m_metaInfo.sizeZ - 1].trace_end = m_metaInfo.trace_count - 1;
  m_lineInfo[m_metaInfo.sizeZ - 1].count = m_metaInfo.trace_count - itrace;

  // cal x, y interval
  get_TraceInfo(start, trace1);
  get_TraceInfo(start + m_lineInfo[0].trace_end * trace_size, trace2);
  m_metaInfo.Y_interval = std::sqrt(std::pow(trace2.X - trace1.X, 2) +
                                    std::pow(trace2.Y - trace1.Y, 2)) /
                          m_lineInfo[0].count;
  int num = m_metaInfo.trace_count > 10 ? 10 : m_metaInfo.trace_count - 1;
  get_TraceInfo(start + m_lineInfo[num].trace_start * trace_size, trace2);
  m_metaInfo.Z_interval =
      std::sqrt(std::pow(trace2.X - trace1.X, 2) +
                std::pow(trace2.Y - trace1.Y, 2) -
                std::pow((trace2.crossline_num - trace1.crossline_num) *
                             m_metaInfo.Y_interval,
                         2)) /
      (trace2.inline_num - trace1.inline_num);
}

static inline int32_t getCrossline(const char *source, int field) {
  return swap_endian(*(int32_t *)(source + field - 1));
}

void SegyIO::read(float *dst, int startX, int endX, int startY, int endY,
                  int startZ, int endZ) {
  if (!isReadSegy) {
    throw std::runtime_error(
        "'read()' function used only in reading segy mode");
  }
  if (startX >= endX || startY >= endY || startZ >= endZ) {
    throw std::runtime_error("Index 'end' must large than 'start'");
  }
  if (startX < 0 || endX > m_metaInfo.sizeX || startY < 0 ||
      endY > m_metaInfo.sizeY || startZ < 0 || endZ > m_metaInfo.sizeZ) {
    throw std::runtime_error("Index out of range");
  }

  const char *source = m_source.data() + kTextualHeaderSize + kBinaryHeaderSize;
  int trace_size = m_metaInfo.sizeX * sizeof(float) + kTraceHeaderSize;

  int sizeX = endX - startX;
  int sizeY = endY - startY;
  int sizeZ = endZ - startZ;
  int offset = startX * sizeof(float) + kTraceHeaderSize;

  progressbar bar(sizeZ);
  auto time_start = std::chrono::high_resolution_clock::now();

  // #pragma omp parallel for
  for (int iZ = startZ; iZ < endZ; iZ++) {
    int istart = startY;
    float *dstline = dst + static_cast<uint64_t>(iZ - startZ) * sizeX * sizeY;
    uint64_t trace_start = m_metaInfo.isNormalSegy
                               ? static_cast<uint64_t>(iZ) * m_metaInfo.sizeY
                               : m_lineInfo[iZ].trace_start;
    const char *sourceline = source + trace_start * trace_size;
    bool normal = true;
    if (!m_metaInfo.isNormalSegy) {
      normal = m_lineInfo[iZ].count == m_metaInfo.sizeY ? true : false;
      if (!normal) {
        int dst_crossline = m_metaInfo.min_crossline + startY;
        while (getCrossline(sourceline + istart * trace_size,
                            m_metaInfo.crossline_field) > dst_crossline &&
               istart > 0) {
          istart--;
        }
      }
    }

    for (int iY = startY; iY < endY; iY++) {
      float *dsttrace = dstline + (iY - startY) * sizeX;
      if (normal || getCrossline(sourceline + istart * trace_size,
                                 m_metaInfo.crossline_field) ==
                        (m_metaInfo.min_crossline + iY)) {
        memcpy(dsttrace, sourceline + istart * trace_size + offset,
               sizeX * sizeof(float));
        for (int iX = 0; iX < sizeX; iX++) {
          if (m_metaInfo.data_format == 1) {
            dsttrace[iX] = ibm_to_ieee(dsttrace[iX], true);
          } else if (m_metaInfo.data_format == 5) {
            dsttrace[iX] = swap_endian(dsttrace[iX]);
          } else {
            throw std::runtime_error("Unsuport sample format");
          }
        }
        istart++;
      } else {
        std::fill(dsttrace, dstline + (iY - startY + 1) * sizeX,
                  m_metaInfo.fillNoValue);
      }
    }
    // #pragma omp critical
    bar.update();
  }
  fmt::print("\n");

  auto time_end = std::chrono::high_resolution_clock::now();

  fmt::print("need time: {}s\n",
             std::chrono::duration_cast<std::chrono::nanoseconds>(time_end -
                                                                  time_start)
                     .count() *
                 1e-9);
}

void SegyIO::read(float *dst) {
  if (!isScan) {
    scan();
  }
  read(dst, 0, m_metaInfo.sizeX, 0, m_metaInfo.sizeY, 0, m_metaInfo.sizeZ);
}

void SegyIO::read_inline_slice(float *dst, int iZ) {
  if (!isScan) {
    scan();
  }
  read(dst, 0, m_metaInfo.sizeX, 0, m_metaInfo.sizeY, iZ, iZ + 1);
}

void SegyIO::read_cross_slice(float *dst, int iY) {
  if (!isScan) {
    scan();
  }
  read(dst, 0, m_metaInfo.sizeX, iY, iY + 1, 0, m_metaInfo.sizeZ);
}

void SegyIO::read_time_slice(float *dst, int iX) {
  if (!isScan) {
    scan();
  }
  read(dst, iX, iX + 1, 0, m_metaInfo.sizeY, 0, m_metaInfo.sizeZ);
}

void SegyIO::read_trace(float *dst, int iY, int iZ) {
  if (!isScan) {
    scan();
  }
  read(dst, 0, m_metaInfo.sizeX, iY, iY + 1, iZ, iZ + 1);
}

void SegyIO::tofile(const std::string &binary_out_name) {
  if (!isScan) {
    scan();
  }
  uint64_t need_size = static_cast<uint64_t>(m_metaInfo.sizeX) *
                       m_metaInfo.sizeY * m_metaInfo.sizeZ * sizeof(float);
  int fd = open(binary_out_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 00644);
  for (int i = 0; i < int(need_size / kMaxLSeekSize) + 1; i++) {
    uint64_t move_point = need_size > kMaxLSeekSize ? kMaxLSeekSize : need_size;
    if (lseek(fd, move_point - 1, SEEK_END) < 0) {
      throw std::runtime_error("create file failed");
    }
    if (write(fd, "", 1) < 0) {
      throw std::runtime_error("create file failed");
    }
    if (need_size > kMaxLSeekSize) {
      need_size -= kMaxLSeekSize;
    }
  }

  std::error_code error;
  mio::mmap_sink rw_mmap = mio::make_mmap_sink(fd, error);
  if (error) {
    throw std::runtime_error("mmap fail when write data");
  }
  close(fd);
  // or need split into serveral chunks?
  read(reinterpret_cast<float *>(rw_mmap.data()));
  rw_mmap.unmap();
}

std::string SegyIO::metaInfo() {
  if (!isScan && isReadSegy) {
    scan();
  }

  float Y_interval = 0;
  float Z_interval = 0;

  if (m_metaInfo.scalar != 0) {

    Y_interval = m_metaInfo.scalar > 0
                     ? m_metaInfo.Y_interval * m_metaInfo.scalar
                     : m_metaInfo.Y_interval / -m_metaInfo.scalar;
    Z_interval = m_metaInfo.scalar > 0
                     ? m_metaInfo.Z_interval * m_metaInfo.scalar
                     : m_metaInfo.Z_interval / -m_metaInfo.scalar;
  }

  std::string dformat = m_metaInfo.data_format == 1
                            ? "4-bytes IBM floating-point"
                            : "4-bytes IEEE floating-point";
  return fmt::format(
      "shape: (n-time, n-crossline, n-inline) = ({}, {}, {})\nsample interval: "
      "{}, data format code: {}\ninline "
      "start: {}, crossline start: {}\nX interval: {:.1f}, Y interval: {:.1f}, "
      "time "
      "start: {}\nIs regular file (no missing traces): {}",
      m_metaInfo.sizeX, m_metaInfo.sizeY, m_metaInfo.sizeZ,
      m_metaInfo.sample_interval, dformat, m_metaInfo.min_inline,
      m_metaInfo.min_crossline, Y_interval, Z_interval, m_metaInfo.start_time,
      m_metaInfo.isNormalSegy);
}

std::string SegyIO::textual_header() {
  if (!isReadSegy && m_sink.size() < kTextualHeaderSize) {
    throw std::runtime_error(
        "No textual header, because this is not a segy "
        "file (read mode) or you don't create textual header (create mode)");
  }
  const char *textual_header = nullptr;
  if (isReadSegy) {
    textual_header = m_source.data();
  } else {
    textual_header = m_sink.data();
  }
  char out[kTextualHeaderSize + kTextualRows];
  bool isEBCDIC = isTextInEBCDICFormat(textual_header, kTextualHeaderSize);
  for (int iRow = 0; iRow < kTextualRows; iRow++) {
    int offset = iRow * kTextualColumns;
    for (int iCol = 0; iCol < kTextualColumns; iCol++) {
      if (isEBCDIC) {
        out[iCol + offset + iRow] =
            getASCIIfromEBCDIC(textual_header[iCol + offset]);
      } else {
        out[iCol + offset + iRow] = textual_header[iCol + offset];
      }
    }
    if (iRow < kTextualRows - 1) {
      out[(iRow + 1) * (kTextualColumns + 1) - 1] = '\n';
    }
  }

  return std::string(out);
}

void SegyIO::create(const std::string &segy_out_name, const float *src) {
  if (isReadSegy) {
    throw std::runtime_error(
        "'create() function only can be used for creating segy file.'");
  }
  uint64_t need_size =
      kTextualHeaderSize + kBinaryHeaderSize +
      static_cast<uint64_t>(m_metaInfo.sizeY) * m_metaInfo.sizeZ *
          (m_metaInfo.sizeX * sizeof(float) + kTraceHeaderSize);
  int fd = open(segy_out_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 00644);
  // lseek(int, long, ), check whether need size > max number of long
  for (int i = 0; i < int(need_size / kMaxLSeekSize) + 1; i++) {
    uint64_t move_point = need_size > kMaxLSeekSize ? kMaxLSeekSize : need_size;
    if (lseek(fd, move_point - 1, SEEK_END) < 0) {
      throw std::runtime_error("create file failed");
    }
    if (write(fd, "", 1) < 0) {
      throw std::runtime_error("create file failed");
    }
    if (need_size > kMaxLSeekSize) {
      need_size -= kMaxLSeekSize;
    }
  }

  std::error_code error;
  mio::mmap_sink rw_mmap = mio::make_mmap_sink(fd, error);
  if (error) {
    throw std::runtime_error("mmap fail when write data");
  }
  close(fd);

  write_textual_header(rw_mmap.data(), segy_out_name);
  write_binary_header(rw_mmap.data() + kTextualHeaderSize);
  TraceHeader trace_header{};
  initTraceHeader(&trace_header);
  char *dst = rw_mmap.data() + kTextualHeaderSize + kBinaryHeaderSize;
  char *dstline = dst;

  progressbar bar(m_metaInfo.sizeZ);

  int trace_size = m_metaInfo.sizeX * sizeof(float) + kTraceHeaderSize;
  // #pragma omp parallel for
  for (int iZ = 0; iZ < m_metaInfo.sizeZ; iZ++) {
    for (int iY = 0; iY < m_metaInfo.sizeY; iY++) {
      // write header
      int64_t x = iY * m_metaInfo.Y_interval + 5200;
      int64_t y = iZ * m_metaInfo.Z_interval + 5200;
      write_trace_header(dstline, &trace_header, iY + m_metaInfo.min_crossline,
                         iZ + m_metaInfo.min_inline, x, y);

      // copy data
      float *dstdata = reinterpret_cast<float *>(dstline + kTraceHeaderSize);
      const float *srcline =
          src + static_cast<uint64_t>(iY) * m_metaInfo.sizeX +
          static_cast<uint64_t>(iZ) * m_metaInfo.sizeX * m_metaInfo.sizeY;
      memcpy(dstdata, srcline, m_metaInfo.sizeX * sizeof(float));
      for (int iX = 0; iX < m_metaInfo.sizeX; iX++) {
        if (m_metaInfo.data_format == 1) {
          dstdata[iX] = ieee_to_ibm(dstdata[iX], true);
        }
        dstdata[iX] = swap_endian(dstdata[iX]);
      }

      dstline += trace_size;
    }
    // #pragma omp critical
    bar.update();
  }
  fmt::print("\n");
  rw_mmap.unmap();
}

void SegyIO::create(const std::string &segy_out_name) {
  if (isReadSegy) {
    throw std::runtime_error("Now is read segy mode, cannot create a segy");
  }
  if (!m_source.is_mapped()) {
    throw std::runtime_error("You need to read a binary file before create, or "
                             "you can create from memory");
  }
  create(segy_out_name, (float *)m_source.data());
}

void SegyIO::initMetaInfo() {
  m_metaInfo.isNormalSegy = true;
  m_metaInfo.crossline_field = kDefaultCrosslineField;
  m_metaInfo.inline_field = kDefaultInlineField;
  m_metaInfo.min_inline = 1;
  m_metaInfo.max_inline = m_metaInfo.min_inline + m_metaInfo.sizeZ - 1;
  m_metaInfo.min_crossline = 1;
  m_metaInfo.max_crossline = m_metaInfo.min_crossline + m_metaInfo.sizeY - 1;
  m_metaInfo.data_format = 5;
  m_metaInfo.sample_interval = 2000;
  m_metaInfo.Y_interval = 25 * 100;
  m_metaInfo.Z_interval = 25 * 100;
  m_metaInfo.scalar = -100;
  m_metaInfo.start_time = 0;
  m_metaInfo.X_field = kDefaultXField;
  m_metaInfo.Y_field = kDefaultYField;
}

static inline std::string field_str(int field, int len = 2) {
  return fmt::format("{}-{}", field, field + len - 1);
}

void SegyIO::write_textual_header(char *dst, const std::string &segy_out_name) {
  auto now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::string time_string(25, ' ');
  std::strftime(&time_string[0], time_string.size(), "%Y-%m-%d %H:%M:%S",
                std::localtime(&now));
  // std::string time_string = fmt::format("{:<30}", time_string0);

  std::string dformat = m_metaInfo.data_format == 1
                            ? "4-bytes IBM floating-point"
                            : "4-bytes IEEE floating-point";
  std::string textual_header = fmt::format(
      "C01 Create By CIGSEGY software (CIG, USTC, 2022), see:              "
      "            "
      "C02 github: https://github.com/JintaoLee-Roger/cigsegy              "
      "            "
      "C03                                                                 "
      "            "
      "C04 Name: {:<70}"                           // 10 + 70
      "C05 Type: 3D seismic  Created Time: {:<44}" // 36 + 44
      "C06                                                                 "
      "            "
      "C07 First inline: {:<10}  Last inline: {:<37}" // 36 + 10 + 34
      "C08 First xline:  {:<10}  Last xline:  {:<37}"
      "C09                                                                 "
      "            "
      "C10 nt, nx, ni = {:>6}, {:>6}, {:<38}         " // 30 + 6 + 6 + 38
      "C11 Number of samples per data trace: {:<42}"   // 38 + 42
      "C12 Sample interval: {:<10}   microsecond                           "
      "        " // 70 + 10
      "C13 X interval: {:<10} meters, Y interval: {:<10} meters            "
      "    " // 60 + 10 + 10
      "C14 Byte endian: BIG_ENDIAN                                         "
      "            "
      "C15 Data sample format: {:<56}" // 24 + 56
      "C16                                                                 "
      "            "
      "C17                                                                 "
      "            "
      "C18 Binary header locations:                                        "
      "            "
      "C19 Sample interval             : bytes {:<40}" // 40 + 40
      "C20 Number of samples per trace : bytes {:<40}" // 40 + 40
      "C21 Data sample format code     : bytes {:<40}" // 40 + 40
      "C22                                                                 "
      "            "
      "C23 Trace header locations:                                         "
      "            "
      "C24 Inline number               : bytes {:<40}" // 40 + 40
      "C25 Xline number                : bytes {:<40}"
      "C26 X coordinate                : bytes {:<40}"
      "C27 Y coordinate                : bytes {:<40}"
      "C28 Trace start time/depth      : bytes {:<40}"
      "C29 Number of samples per trace : bytes {:<40}"
      "C30 Sample interval             : bytes {:<40}"
      "C31                                                                 "
      "            "
      "C32                                                                 "
      "            "
      "C33                                                                 "
      "            "
      "C34                                                                 "
      "            "
      "C35                                                                 "
      "            "
      "C36                                                                 "
      "            "
      "C37                                                                 "
      "            "
      "C38                                                                 "
      "            "
      "C39                                                                 "
      "            "
      "C40 END EBCDIC                                                      "
      "            ",
      segy_out_name, time_string.substr(0, 19), m_metaInfo.min_inline,
      m_metaInfo.max_inline, m_metaInfo.min_crossline, m_metaInfo.max_crossline,
      m_metaInfo.sizeX, m_metaInfo.sizeY, m_metaInfo.sizeZ, m_metaInfo.sizeX,
      m_metaInfo.sample_interval, m_metaInfo.Z_interval / 100,
      m_metaInfo.Y_interval / 100, dformat, field_str(kBSampleIntervalField),
      field_str(kBSampleCountField), field_str(kBSampleFormatField),
      field_str(m_metaInfo.inline_field, 4),
      field_str(m_metaInfo.crossline_field, 4),
      field_str(m_metaInfo.X_field, 4), field_str(m_metaInfo.Y_field, 4),
      field_str(kTStartTimeField), field_str(kTSampleCountField),
      field_str(kTSampleIntervalField));

  for (auto &s : textual_header) {
    s = getEBCIDfromASCII(s);
  }

  memcpy(dst, textual_header.c_str(), kTextualHeaderSize);
}

void SegyIO::write_binary_header(char *dst) {
  memset(dst, 0, kBinaryHeaderSize);
  BinaryHeader binary_header{};
  // auto &binary_header = (BinaryHeader &)dst;
  binary_header.jobID = swap_endian(int32_t(1));
  binary_header.line_number = swap_endian(m_metaInfo.min_inline);
  binary_header.num_traces_per_ensemble =
      swap_endian(int16_t(m_metaInfo.min_crossline));
  binary_header.sample_interval = swap_endian(m_metaInfo.sample_interval);
  binary_header.trace_length = swap_endian(int16_t(m_metaInfo.sizeX));
  binary_header.data_format = swap_endian(m_metaInfo.data_format);
  binary_header.ensemble_fold = swap_endian(int16_t(1));
  binary_header.trace_sorting_code = swap_endian(int16_t(4));
  binary_header.measurement_system = swap_endian(int16_t(1));
  binary_header.fixed_length_trace = swap_endian(int16_t(1));
  memcpy(dst, &binary_header, kBinaryHeaderSize);
}

void SegyIO::initTraceHeader(TraceHeader *trace_header) {
  memset(trace_header, 0, kTraceHeaderSize);
  trace_header->trace_sequence_number_in_line = swap_endian(int32_t(1));
  trace_header->trace_num_in_orig = swap_endian(int32_t(1));
  trace_header->trace_num_in_ensemble = swap_endian(int32_t(1));
  trace_header->trace_ID_code = swap_endian(int16_t(1));
  trace_header->data_used_for = swap_endian(int16_t(1));
  trace_header->scalar_for_elev_and_depth = swap_endian(int16_t(1));
  trace_header->scalar_for_coord = swap_endian(m_metaInfo.scalar);
  trace_header->coord_units = swap_endian(int16_t(1));
  trace_header->mute_time_start = swap_endian(m_metaInfo.start_time);
  trace_header->num_sample = swap_endian(int16_t(m_metaInfo.sizeX));
  trace_header->sample_interval = swap_endian(m_metaInfo.sample_interval);
  trace_header->correlated = swap_endian(int16_t(1));
  trace_header->sweep_type_code = swap_endian(int16_t(1));
  trace_header->taper_type = swap_endian(int16_t(1));
}

void SegyIO::write_trace_header(char *dst, TraceHeader *trace_header,
                                int32_t iY, int32_t iZ, int32_t x, int32_t y) {
  trace_header->trace_sequence_number_in_file = swap_endian(iZ);
  trace_header->orig_field_num = swap_endian(iZ);
  trace_header->source_point_num = swap_endian(iY);
  trace_header->ensemble_num = swap_endian(iY);
  trace_header->source_coord_X = swap_endian(x);
  trace_header->source_coord_Y = swap_endian(y);
  trace_header->X = swap_endian(x);
  trace_header->Y = swap_endian(y);
  trace_header->inline_num = swap_endian(iZ);
  trace_header->crossline_num = swap_endian(iY);
  memcpy(dst, trace_header, kTraceHeaderSize);
}

void SegyIO::close_file() {
  if (m_source.is_mapped()) {
    m_source.unmap();
  }
}

void SegyIO::collect(float *data, int *header) {
  const char *source = m_source.data() + kTextualHeaderSize + kBinaryHeaderSize;
  int32_t trace_size = m_metaInfo.sizeX * sizeof(float) + kTraceHeaderSize;
  progressbar bar(100);
  for (int i = 0; i < m_metaInfo.trace_count; i++) {
    if (i % (m_metaInfo.trace_count / 100) == 0) {
      bar.update();
    }
    get_TraceInfo(source + i * trace_size,
                  *reinterpret_cast<TraceInfo *>(header));
    memcpy(data, source + kTraceHeaderSize, m_metaInfo.sizeX * sizeof(float));
    for (int j = 0; j < m_metaInfo.sizeX; j++) {
      if (m_metaInfo.data_format == 1) {
        data[j] = ibm_to_ieee(data[j], true);
      } else {
        data[j] = swap_endian(data[j]);
      }
    }
    data += m_metaInfo.sizeX;
    header += 4;
  }
}

void read(const std::string &segy_name, float *dst, int iline, int xline) {
  SegyIO segy_data(segy_name);
  segy_data.setInlineLocation(iline);
  segy_data.setCrosslineLocation(xline);
  segy_data.scan();
  segy_data.read(dst);
  segy_data.close_file();
}

void tofile(const std::string &segy_name, const std::string &out_name,
            int iline, int xline) {
  SegyIO segy_data(segy_name);
  segy_data.setInlineLocation(iline);
  segy_data.setCrosslineLocation(xline);
  segy_data.scan();
  segy_data.tofile(out_name);
  segy_data.close_file();
}

void read_ignore_header(const std::string &segy_name, float *dst, int sizeX,
                        int sizeY, int sizeZ, int format) {
  SegyIO segy_data(segy_name);
  segy_data.setDataFormatCode(format);
  segy_data.set_size(sizeX, sizeY, sizeZ);
  segy_data.read(dst, 0, sizeX, 0, sizeY, 0, sizeZ);
  segy_data.close_file();
}

void tofile_ignore_header(const std::string &segy_name,
                          const std::string &out_name, int sizeX, int sizeY,
                          int sizeZ, int format) {
  SegyIO segy_data(segy_name);
  segy_data.setDataFormatCode(format);
  segy_data.set_size(sizeX, sizeY, sizeZ);
  segy_data.tofile(out_name);
  segy_data.close_file();
}

void create_by_sharing_header(const std::string &segy_name,
                              const std::string &header_segy, const float *src,
                              int sizeX, int sizeY, int sizeZ, int iline,
                              int xline) {
  SegyIO header(header_segy);
  header.setInlineLocation(iline);
  header.setCrosslineLocation(xline);
  header.scan();
  auto line_info = header.line_info();
  auto meta_info = header.get_metaInfo();
  auto trace_count = header.trace_count();
  header.close_file();

  if (meta_info.sizeY != sizeY || meta_info.sizeZ != sizeZ ||
      meta_info.sizeX != sizeX) {
    throw std::runtime_error(fmt::format(
        "shape of header: {} x {} x {}, but shape of source: {} x {} x {}",
        meta_info.sizeZ, meta_info.sizeY, meta_info.sizeX, sizeZ, sizeY,
        sizeX));
  }

  uint64_t need_size = kTextualHeaderSize + kBinaryHeaderSize +
                       trace_count * (sizeX * sizeof(float) + kTraceHeaderSize);
  int fd = open(segy_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 00644);
  // lseek(int, long, ), check whether need size > max number of long
  for (int i = 0; i < int(need_size / kMaxLSeekSize) + 1; i++) {
    uint64_t move_point = need_size > kMaxLSeekSize ? kMaxLSeekSize : need_size;
    if (lseek(fd, move_point - 1, SEEK_END) < 0) {
      throw std::runtime_error("create file failed");
    }
    if (write(fd, "", 1) < 0) {
      throw std::runtime_error("create file failed");
    }
    if (need_size > kMaxLSeekSize) {
      need_size -= kMaxLSeekSize;
    }
  }

  std::error_code error;
  mio::mmap_sink rw_mmap = mio::make_mmap_sink(fd, error);
  if (error) {
    throw std::runtime_error("mmap fail when write data");
  }
  close(fd);

  mio::mmap_source m_source;
  m_source.map(header_segy, error);
  if (error) {
    throw std::runtime_error("Cannot mmap segy file");
  }

  // copy textual header and binary header
  std::copy(m_source.begin(),
            m_source.begin() + kTextualHeaderSize + kBinaryHeaderSize,
            rw_mmap.begin());

  // trace header and data
  progressbar bar(sizeZ);
  int64_t trace_size = sizeX + kTraceHeaderSize / 4;
  for (int iz = 0; iz < sizeZ; iz++) {
    bar.update();
    int64_t trace_loc = kTextualHeaderSize + kBinaryHeaderSize +
                        trace_size * 4 * line_info[iz].trace_start;

    const float *srcopy = src + iz * sizeY * sizeX;

    const float *m_src =
        reinterpret_cast<const float *>(m_source.data() + trace_loc);
    float *m_dst = reinterpret_cast<float *>(rw_mmap.data() + trace_loc);

    for (int iy = 0; iy < line_info[iz].count; iy++) {
      int srct = iy;
      if (line_info[iz].count != sizeY) {
        const int *tmp = reinterpret_cast<const int *>(m_src);
        srct = swap_endian(
                   tmp[iy * trace_size + (meta_info.crossline_field - 1) / 4]) -
               meta_info.min_crossline;
      }

      // copy trace header
      std::copy(m_src + iy * trace_size,
                m_src + iy * trace_size + kTraceHeaderSize / 4,
                m_dst + iy * trace_size);

      // copy data
      float *t_dst = m_dst + iy * trace_size + kTraceHeaderSize / 4;
      std::copy(srcopy + srct * sizeX, srcopy + srct * sizeX + sizeX, t_dst);
      // convert each value to big endian and its format
      for (int ix = 0; ix < sizeX; ix++) {
        if (meta_info.data_format == 1) {
          t_dst[ix] = ieee_to_ibm(t_dst[ix], true);
        }
        t_dst[ix] = swap_endian(t_dst[ix]);
      }
    }
  }
  fmt::print("\n");

  m_source.unmap();
  rw_mmap.unmap();
}

} // namespace segy

/*********************************************************************
** Copyright (c) 2022 Roger Lee.
** Computational and Interpretation Group (CIG),
** University of Science and Technology of China (USTC).
**
** @File: segy.h
** @Time: 2022/11/16 11:30:42
** @Version: 1.0
** @Description :
*********************************************************************/

#ifndef CIG_SEGY_H
#define CIG_SEGY_H

#include <stdexcept>
#include <vector>
// #include <omp.h>

#include "mio.hpp"
#include "utils.h"

namespace segy {

// const size
const int kTextualHeaderSize = 3200;
const int kBinaryHeaderSize = 400;
const int kTraceHeaderSize = 240;
const int kTextualColumns = 80;
const int kTextualRows = 40;

const int kMaxSizeOneDimemsion = 10000;

// const binary header field
const int kBSampleIntervalField = 17;
const int kBSampleCountField = 21;
const int kBSampleFormatField = 25;

// const trace header field
const int kTStartTimeField = 111; // TODO(Jintao): check
const int kTScalarField = 71;
const int kTSampleCountField = 115;
const int kTSampleIntervalField = 117;

const int kDefaultInlineField = 189;
const int kDefaultCrosslineField = 193;
const int kDefaultXField = 73;
const int kDefaultYField = 77;

// const int kMaxTempSize = 512 * 512 * 512 * 4;

// const int kMaxThreadsNum = 8;

struct BinaryHeader {
  int32_t jobID;
  int32_t line_number;
  int32_t reel_number;
  int16_t num_traces_per_ensemble;
  int16_t num_aux_traces_per_ensemble;
  int16_t sample_interval;
  int16_t sample_interval_orig;
  int16_t trace_length;
  int16_t trace_length_orig;
  int16_t data_format;
  int16_t ensemble_fold;
  int16_t trace_sorting_code;
  int16_t v_sum_code;
  int16_t sweep_freq_start;
  int16_t sweep_freq_end;
  int16_t sweep_length;
  int16_t sweep_type_code;
  int16_t trace_num_sweep_channel;
  int16_t sweep_trace_taper_start;
  int16_t sweep_trace_taper_end;
  int16_t taper_type;
  int16_t correlated_data_trace;
  int16_t bin_gain_recover;
  int16_t amplitude_recover_method;
  int16_t measurement_system;
  int16_t impulse_signal_polarity;
  int16_t vibratory_polarity_code;
  int32_t extend_num_data_tarces;
  int32_t extend_num_aux_data_tarces;
  int32_t extend_trace_length;
  double extend_sample_intervel;
  double extend_sample_intervel_orig;
  int32_t extend_trace_length_orig;
  int32_t extend_ensement_fold;
  char dummy[204];
  char major_version;
  char minor_version;
  int16_t fixed_length_trace;
  int16_t extend_textual_header;
  int16_t max_extend_trace_header;
  int16_t time_bias_code;
  uint64_t num_traces;
  uint64_t byte_offset;
  int32_t num_trailer_stanza;
  char dummy2[68];
};

struct TraceHeader {
  int32_t trace_sequence_number_in_line; // 1-4
  int32_t trace_sequence_number_in_file; // 5-8
  int32_t orig_field_num;                // 9-12
  int32_t trace_num_in_orig;             // 13-16
  int32_t source_point_num;              // 17-20
  int32_t ensemble_num;                  // 21-24
  int32_t trace_num_in_ensemble;         // 25-28
  int16_t trace_ID_code;                 // 29-30
  int16_t num_v_summed_traces;           // 31-32
  int16_t num_h_stacked_tarces;          // 33-34
  int16_t data_used_for;                 // 35-36
  int32_t distance_from_center;          // 37-40
  int32_t elevation_rev;                 // 41-44
  int32_t surface_elevation_source;      // 45-48
  int32_t source_depth;                  // 49-52
  int32_t seis_datum_elevation_rev;      // 53-56
  int32_t seis_datum_elevation_source;   // 57-60
  int32_t water_col_height_source;       // 61-64
  int32_t water_col_height_rev;          // 65-68
  int16_t scalar_for_elev_and_depth;     // 69-70
  int16_t scalar_for_coord;              // 71-72
  int32_t source_coord_X;                // 73-76
  int32_t source_coord_Y;                // 77-80
  int32_t group_coord_X;                 // 81-84
  int32_t group_coord_Y;                 // 85-88
  int16_t coord_units;                   // 89-90
  int16_t weather_vel;                   // 91-92
  int16_t subweather_vel;                // 93-94
  int16_t uphole_time_source;            // 95-96
  int16_t uphole_time_rev;               // 97-98
  int16_t source_static_corr;            // 99-100
  int16_t group_static_corr;             // 101-102
  int16_t total_static;                  // 103-104
  int16_t lag_time_A;                    // 105-106
  int16_t lag_time_B;                    // 107-108
  int16_t delay_record_time;             // 109-110
  int16_t mute_time_start;               // 111-112
  int16_t mute_time_end;                 // 113-114
  int16_t num_sample;
  int16_t sample_interval;
  int16_t gain_type;
  int16_t instrument_gain_constant;
  int16_t instrument_early;
  int16_t correlated;
  int16_t sweep_freq_start;
  int16_t sweep_freq_end;
  int16_t sweep_length;
  int16_t sweep_type_code;
  int16_t sweep_trace_taper_start;
  int16_t sweep_trace_taper_end;
  int16_t taper_type;
  int16_t alias_filter_freq;
  int16_t alias_filter_slope;
  int16_t notch_filter_freq;
  int16_t notch_filter_slope;
  int16_t lowcut_freq;
  int16_t highcut_freq;
  int16_t lowcut_scope;
  int16_t highcut_scope;
  int16_t years;
  int16_t day;
  int16_t hour;
  int16_t minute;
  int16_t secend;
  int16_t time_basis;
  int16_t trace_weight_factor;
  int16_t geophone[3];
  int16_t gap_size;
  int16_t down_or_up;
  int32_t X;
  int32_t Y;
  int32_t inline_num;
  int32_t crossline_num;
  int32_t shotpoint_num;
  int16_t scalar_for_shotpoint;
  int16_t trace_value_measurement_uint;
  char transduction[6];
  int16_t transduction_unit;
  int16_t traceID;
  int16_t scalar_for_95;
  int16_t source_type;
  char source_energy_direction[6];
  char source_measurement[6];
  int16_t source_measu_unit;
  int64_t dummy;
};

struct MetaInfo {
  // count information
  int32_t sizeX; // same as time
  int32_t sizeY; // same as crossline
  int32_t sizeZ; // same as inline
  int64_t trace_count;
  int16_t sample_interval; // dt
  int16_t data_format;     // 1 or 5
  float Y_interval;        // crossline interval
  float Z_interval;        // inline interval
  int16_t start_time;
  int16_t scalar;

  int min_inline;
  int max_inline;
  int min_crossline;
  int max_crossline;

  bool isNormalSegy;

  float fillNoValue;

  // field information
  int inline_field;
  int crossline_field;
  int X_field;
  int Y_field;
};

struct LineInfo {
  int line_num;
  uint64_t trace_start;
  uint64_t trace_end;
  int count;
};

struct TraceInfo {
  int inline_num;
  int crossline_num;
  int X;
  int Y;
};

class SegyIO {
public:
  // read segy mode
  explicit SegyIO(const std::string &segyname);
  // create segy from memory
  SegyIO(int sizeX, int sizeY, int sizeZ);
  // create segy file from binary file
  SegyIO(const std::string &binaryname, int sizeX, int sizeY, int sizeZ);

  ~SegyIO();

  inline int shape(int dimension) {
    if (dimension == 0) {
      return m_metaInfo.sizeX;
    } else if (dimension == 1) {
      return m_metaInfo.sizeY;
    } else if (dimension == 2) {
      return m_metaInfo.sizeZ;
    } else {
      throw std::runtime_error("shape(dim), dim can be only {0, 1, 2}");
    }
  }

  inline int64_t trace_count() { return m_metaInfo.trace_count; }

  inline void set_size(int x, int y, int z) {
    m_metaInfo.sizeX = x;
    m_metaInfo.sizeY = y;
    m_metaInfo.sizeZ = z;
    if (isReadSegy) {
      m_metaInfo.isNormalSegy = true;
      isScan = true;
      int64_t trace_count =
          (m_source.size() - kTextualHeaderSize - kBinaryHeaderSize) /
          (kTraceHeaderSize + x * sizeof(float));
      if (y * z != (trace_count)) {
        throw std::runtime_error("invalid shape. inline * crossline != "
                                 "total_trace_count");
      }
    }
  }

  void collect(float *data, int *header);

  std::string textual_header();
  std::string metaInfo();
  std::string binary_header_string();
  inline std::vector<LineInfo> line_info() { return m_lineInfo; }
  inline MetaInfo get_metaInfo() { return m_metaInfo; }

  void setInlineLocation(int loc);
  void setCrosslineLocation(int loc);
  void setXLocation(int loc);
  void setYLocation(int loc);

  // read segy
  void setFillNoValue(float noValue);
  void scan();
  void tofile(const std::string &binary_out_name);
  void read(float *dst, int startX, int endX, int startY, int endY, int startZ,
            int endZ);
  void read(float *dst);
  void read_inline_slice(float *dst, int iZ);
  void read_cross_slice(float *dst, int iY);
  void read_time_slice(float *dst, int iX);
  void read_trace(float *dst, int iY, int iZ);

  // create segy
  void setSampleInterval(int interval);
  void setDataFormatCode(int fdormat);
  void setStartTime(int start_time);
  void setXInterval(float dz);
  void setYInterval(float dy);
  void setMinInline(int in);
  void setMinCrossline(int cross);

  void create(const std::string &segy_out_name, const float *src);
  void create(const std::string &segy_out_name);

  void close_file();

private:
  bool isReadSegy{};
  bool isScan = false;
  mio::mmap_source m_source;
  mio::mmap_sink m_sink;
  std::vector<LineInfo> m_lineInfo;
  MetaInfo m_metaInfo{};

  void scanBinaryHeader();
  void initMetaInfo();
  void initTraceHeader(TraceHeader *trace_header);
  void write_textual_header(char *dst, const std::string &segy_out_name);
  void write_binary_header(char *dst);
  void write_trace_header(char *dst, TraceHeader *trace_header, int32_t iY,
                          int32_t iZ, int32_t x, int32_t y);

  inline void get_TraceInfo(const char *field, TraceInfo &tmetaInfo) {
    tmetaInfo.inline_num =
        swap_endian(*(int32_t *)(field + m_metaInfo.inline_field - 1));
    tmetaInfo.crossline_num =
        swap_endian(*(int32_t *)(field + m_metaInfo.crossline_field - 1));
    tmetaInfo.X = swap_endian(*(int32_t *)(field + m_metaInfo.X_field - 1));
    tmetaInfo.Y = swap_endian(*(int32_t *)(field + m_metaInfo.Y_field - 1));
  }
};

void read_ignore_header(const std::string &segy_name, float *dst, int sizeX,
                        int sizeY, int sizeZ, int format = 5);
void tofile_ignore_header(const std::string &segy_name,
                          const std::string &out_name, int sizeX, int sizeY,
                          int sizeZ, int format = 5);

void tofile(const std::string &segy_name, const std::string &out_name,
            int iline = kDefaultInlineField,
            int xline = kDefaultCrosslineField);
void read(const std::string &segy_name, float *dst,
          int iline = kDefaultInlineField, int xline = kDefaultCrosslineField);

void create_by_sharing_header(const std::string &segy_name,
                              const std::string &header_segy, const float *src,
                              int sizeX, int sizeY, int sizeZ, int iline = 189,
                              int xline = 193);
} // namespace segy

#endif
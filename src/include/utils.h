/*********************************************************************
** Copyright (c) 2022 Roger Lee.
** Computational and Interpretation Group (CIG),
** University of Science and Technology of China (USTC).
**
** @File: utils.h
** @Time: 2022/11/16 11:12:32
** @Version: 1.0
** @Description :
*********************************************************************/
#ifndef CIG_UTILS_H
#define CIG_UTILS_H

#include <cctype>
#include <climits>
#include <limits>
#include <map>
#include <set>

namespace segy {

// A key map that convert EBCDIC to ASCII format
const std::map<unsigned char, char> kEBCDICtoASCIImap = {
    {64, ' '},   {75, '.'},  {76, '<'},   {77, '('},  {78, '+'},  {79, '|'},
    {80, '&'},   {90, '!'},  {91, '$'},   {92, '*'},  {93, ')'},  {94, ';'},
    {96, '-'},   {97, '/'},  {106, '|'},  {107, ','}, {108, '%'}, {109, '_'},
    {110, '>'},  {111, '?'}, {121, '`'},  {122, ':'}, {123, '#'}, {124, '@'},
    {125, '\''}, {126, '='}, {127, '"'},  {129, 'a'}, {130, 'b'}, {131, 'c'},
    {132, 'd'},  {133, 'e'}, {134, 'f'},  {135, 'g'}, {136, 'h'}, {137, 'i'},
    {145, 'j'},  {146, 'k'}, {147, 'l'},  {148, 'm'}, {149, 'n'}, {150, 'o'},
    {151, 'p'},  {152, 'q'}, {153, 'r'},  {161, '~'}, {162, 's'}, {163, 't'},
    {164, 'u'},  {165, 'v'}, {166, 'w'},  {167, 'x'}, {168, 'y'}, {169, 'z'},
    {192, '{'},  {193, 'A'}, {194, 'B'},  {195, 'C'}, {196, 'D'}, {197, 'E'},
    {198, 'F'},  {199, 'G'}, {200, 'H'},  {201, 'I'}, {208, '}'}, {209, 'J'},
    {210, 'K'},  {211, 'L'}, {212, 'M'},  {213, 'N'}, {214, 'O'}, {215, 'P'},
    {216, 'Q'},  {217, 'R'}, {224, '\\'}, {226, 'S'}, {227, 'T'}, {228, 'U'},
    {229, 'V'},  {230, 'W'}, {231, 'X'},  {232, 'Y'}, {233, 'Z'}, {240, '0'},
    {241, '1'},  {242, '2'}, {243, '3'},  {244, '4'}, {245, '5'}, {246, '6'},
    {247, '7'},  {248, '8'}, {249, '9'}};

const std::map<char, unsigned char> kASCIItoEBCDICmap = {
    {' ', 64},   {'.', 75},  {'<', 76},   {'(', 77},  {'+', 78},  {'|', 79},
    {'&', 80},   {'!', 90},  {'$', 91},   {'*', 92},  {')', 93},  {';', 94},
    {'-', 96},   {'/', 97},  {'|', 106},  {',', 107}, {'%', 108}, {'_', 109},
    {'>', 110},  {'?', 111}, {'`', 121},  {':', 122}, {'#', 123}, {'@', 124},
    {'\'', 125}, {'=', 126}, {'"', 127},  {'a', 129}, {'b', 130}, {'c', 131},
    {'d', 132},  {'e', 133}, {'f', 134},  {'g', 135}, {'h', 136}, {'i', 137},
    {'j', 145},  {'k', 146}, {'l', 147},  {'m', 148}, {'n', 149}, {'o', 150},
    {'p', 151},  {'q', 152}, {'r', 153},  {'~', 161}, {'s', 162}, {'t', 163},
    {'u', 164},  {'v', 165}, {'w', 166},  {'x', 167}, {'y', 168}, {'z', 169},
    {'{', 192},  {'A', 193}, {'B', 194},  {'C', 195}, {'D', 196}, {'E', 197},
    {'F', 198},  {'G', 199}, {'H', 200},  {'I', 201}, {'}', 208}, {'J', 209},
    {'K', 210},  {'L', 211}, {'M', 212},  {'N', 213}, {'O', 214}, {'P', 215},
    {'Q', 216},  {'R', 217}, {'\\', 224}, {'S', 226}, {'T', 227}, {'U', 228},
    {'V', 229},  {'W', 230}, {'X', 231},  {'Y', 232}, {'Z', 233}, {'0', 240},
    {'1', 241},  {'2', 242}, {'3', 243},  {'4', 244}, {'5', 245}, {'6', 246},
    {'7', 247},  {'8', 248}, {'9', 249}};

const std::map<int, const char *> kBinaryHeaderHelp = {
    {1, "Job ID"},
    {5, "Line number"},
    {9, "Reel Number"},
    {13, "N traces per ensembel"},
    {15, "N auxiliary traces per ensembel"},
    {17, "Sample interval(dt)"},
    {19, "dt of original"},
    {21, "N samples per traces(ns)"},
    {23, "ns of orignal"},
    {25, "Data sample format code (1-IBM, 5-IEEE)"},
    {27, "Ensemble fold"},
    {29, "Trace sorting code"},
    {31, "vertical sum code"},
    {33, "Sweep freq at start(Hz)"},
    {35, "Sweep freq at end(HZ)"},
    {37, "Sweep length(ms)"},
    {39, "Sweep type code"},
    {41, "Trace number of sweep channel"},
    {43, "Sweep trace taper length in ms at strat"},
    {45, "Sweep trace taper length in ms at end"},
    {47, "Taper type"},
    {49, "Correlated data traces"},
    {51, "Binary gain recovered"},
    {53, "Amplitude recovery method"},
    {55, "Measurement system (units)"},
    {57, "Impulse signal polarity"},
    {59, "Vibratory polariy code"},
    {301, "SEGY format revision number"},
    {303, "Fixed length trace flag"},
    {305, "Number of 3200-byte, Extended textual File Header"},
    {307, "Max number of additional 240-byte trace header"},
    {311, "Time basis code"},
    {313, "number of trace header in this file"},
    {321, "Byte offset of first trace"},
    {329, "Number of 3200-byte data trailer stanza"}};

const std::map<int, const char *> kTraceHeaderHelp = {
    {1, "Trace sequence number within line"},
    {5, "Trace sequence number within SEG-Y file"},
    {9, "Original field record number"},
    {13, "Trace number within the original field record"},
    {17, "Energy source point number"},
    {21, "Ensemble number"},
    {25, "Trace number within the ensemble"},
    {29, "Trace identification code"},
    {31, "Number of vertically summed traces yielding this trace"},
    {33, "Number of horizontally stacked traces yielding this trace"},
    {35, "Data use"},
    {37,
     "Distance from center of the source point to the center of the receiver "
     "group"},
    {41, "Elevation of receiver group"},
    {45, "Surface elevation at source location"},
    {49, "Source depth below surface"},
    {53, "Seismic Datum elevation at receiver group"},
    {57, "Seismic Datum elevation at source"},
    {61, "Water column height at source location"},
    {65, "Water column height at receiver group location"},
    {69, "Scalar to be applied to all elevations and depths"},
    {71, "Scalar to be applied to all coordinates"},
    {73, "Source coordinate - X"},
    {77, "Source coordinate - Y"},
    {81, "Group coordinate - X"},
    {85, "Group coordinate - Y"},
    {89, "Coordinate units"},
    {91, "Weathering velocity"},
    {93, "Subweathering velocity"},
    {95, "Uphole time at source in ms"},
    {97, "Uphole time at group in ms"},
    {99, "Source static correction in ms"},
    {101, "Group static correction in ms"},
    {103, "Total static applied in ms"},
    {105, "Lag time A"},
    {107, "Lag Time B"},
    {109, "Delay recording time"},
    {111, "Mute time — Start time in ms"},
    {113, "Mute time — End time in ms"},
    {115, "Number of samples in this trace"},
    {117, "Sample interval for this trace"},
    {119, "Gain type of field instruments"},
    {121, "Instrument gain constant"},
    {123, "Instrument gain constant"},
    {125, "Correlated"},
    {127, "Sweep frequency at start"},
    {129, "Sweep frequency at end"},
    {131, "Sweep length in ms"},
    {133, "Sweep type"},
    {135, "Sweep trace taper length at start in ms"},
    {137, "Sweep trace taper length at end in ms"},
    {139, "Taper type"},
    {141, "Alias filter frequency (Hz),"},
    {143, "Alias filter slope (dB/octave)"},
    {145, "Notch filter frequency (Hz)"},
    {147, "Notch filter slope (dB/octave)"},
    {149, "Low-cut frequency (Hz)"},
    {151, "High-cut frequency (Hz)"},
    {153, "Low-cut slope (dB/octave)"},
    {155, "High-cut slope (dB/octave)"},
    {157, "Year data recorded"},
    {159, "Day of year"},
    {161, "Hour of day"},
    {163, "Minute of hour"},
    {165, "Second of minute"},
    {167, "Time basis code"},
    {169, "Trace weighting factor"},
    {171, "Geophone group number of roll switch position one"},
    {173,
     "Geophone group number of trace number one within original field record"},
    {175, "Geophone group number of last trace within original field record"},
    {177, "Gap size (total number of groups dropped)"},
    {179, "Over travel associated with taper at beginning or end of line"},
    {181, "X coordinate"},
    {185, "Y coordinate"},
    {189, "The in-line number"},
    {193, "The cross-line number"},
    {197, "Shotpoint number"},
    {201, "Scalar to be applied to the shotpoint number"},
    {203, "Trace value measurement unit"},
    {211, "Transduction Units"},
    {213, "Device/Trace Identifier"},
    {215, "Scalar to be applied to bytes 95-114"},
    {217, "Source Type/Orientation"},
    {231, "Source Measurement Unit"}};

const std::set<int> kBinaryHeader4Bytes = {1, 5, 9, 307, 329};
const std::set<int> kBinaryHeader8Bytes = {313, 321};
const std::set<int> kTraceHeader4Bytes = {1,  5,  9,  13,  17,  21,  25,  37,
                                          41, 45, 49, 53,  57,  61,  65,  73,
                                          77, 81, 85, 181, 185, 189, 193, 197};

const uint64_t kMaxLSeekSize = std::numeric_limits<long>::max();

template <typename T> T swap_endian(T u) {
  static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");

  union {
    T u;
    unsigned char u8[sizeof(T)];
  } source, dest;

  source.u = u;

  for (size_t k = 0; k < sizeof(T); k++)
    dest.u8[k] = source.u8[sizeof(T) - k - 1];

  return dest.u;
}

inline float ieee_to_ibm(float value, bool is_litte_endian_input) {
  if (!is_litte_endian_input)
    value = swap_endian<float>(value);

  int32_t *addr = reinterpret_cast<int32_t *>(&value);
  int32_t int_val = *addr;

  int32_t sign = (int_val >> 31) & 1;
  int32_t exponent = ((int_val & 0x7f800000) >> 23) - 127;
  int32_t fraction = int_val & 0x007fffff;

  if ((int_val & 0x7fffffff) == 0) {
    return sign ? -0.0f : 0.0f;
  }

  fraction <<= 1; // 24 bits

  fraction |= 0x01000000; // add 1, 25 bits

  // convert 2-base to 16-base
  fraction <<= (exponent & 3); // 28 bits
  exponent >>= 2;

  if (fraction & 0x0f000000) { // 24 bits
    fraction >>= 4;
    exponent += 1;
  }

  exponent += 64;

  int32_t ibm_value;
  if (exponent > 127) {
    return (sign ? -std::numeric_limits<float>::max()
                 : std::numeric_limits<float>::max());
  } else if (exponent <= 0) {
    ibm_value = (sign << 31) | fraction;
  } else {
    ibm_value = (sign << 31) | (exponent << 24) | fraction;
  }

  float *float_addr = reinterpret_cast<float *>(&ibm_value);

  return *float_addr;
}

inline float ibm_to_ieee(float value, bool is_big_endian_input) {
  if (is_big_endian_input) {
    value = swap_endian<float>(value);
  }

  int32_t *int_addr = reinterpret_cast<int32_t *>(&value);
  int32_t int_val = *int_addr;

  int32_t sign = (int_val >> 31) & 1;
  int32_t fraction = int_val & 0x00ffffff;

  if (fraction == 0) {
    return sign ? -0.0f : 0.0f;
  }

  // Convert exponent to be of base 2 and remove IBM exponent bias.
  int32_t exponent = ((int_val & 0x7f000000) >> 22) - 256;

  // Drop the last bit since we can store only 23 bits in IEEE.
  fraction >>= 1;

  // Normalize such that the implicit leading bit of the fraction is 1.
  while (fraction && (fraction & 0x00800000) == 0) {
    fraction <<= 1;
    --exponent;
  }

  // Drop the implicit leading bit.
  fraction &= 0x007fffff;

  // Add IEEE bias to the exponent.
  exponent += 127;

  // Handle overflow.
  if (exponent >= 255) {
    return (sign ? -std::numeric_limits<float>::max()
                 : std::numeric_limits<float>::max());
  }

  int32_t ieee_value;

  // Handle underflow.
  if (exponent <= 0)
    ieee_value = (sign << 31) | fraction;
  else
    ieee_value = (sign << 31) | (exponent << 23) | fraction;

  float *float_addr = reinterpret_cast<float *>(&ieee_value);
  return *float_addr;
}

inline char getASCIIfromEBCDIC(char c) {
  if (kEBCDICtoASCIImap.find(c) != kEBCDICtoASCIImap.end())
    return kEBCDICtoASCIImap.at(c);
  return ' ';
}

inline char getEBCIDfromASCII(char c) {
  if (kASCIItoEBCDICmap.find(c) != kASCIItoEBCDICmap.end()) {
    return kASCIItoEBCDICmap.at(c);
  }
  return ' ';
}

inline bool isTextInEBCDICFormat(const char *text, size_t size) {
  int alnumASCII = 0;
  for (size_t i = 0; i < size; i++) {
    if (std::isalnum(text[i]))
      alnumASCII++;
  }

  int alnumEBCDIC = 0;
  for (size_t i = 0; i < size; i++) {
    if (std::isalnum(getASCIIfromEBCDIC(text[i])))
      alnumEBCDIC++;
  }

  if (alnumASCII > alnumEBCDIC)
    return false;
  return true;
}

} // namespace segy

#endif

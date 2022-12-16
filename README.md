# cigsegy

A tool for segy-format file reading and segy-format creating from binary file in **Python** and **c++**.

### Third part dependencies

- `src/include/mio.hpp` is from [mandreyel/mio](https://github.com/mandreyel/mio): Cross-platform C++11 header-only library for memory mapped file IO

- `src/include/progressbar.hpp` is from [gipert/progressbar](https://github.com/gipert/progressbar): A very simple progress bar for C++ loops

- `tools/cxxopts.hpp` is from [jarro2783/cxxopts](https://github.com/jarro2783/cxxopts): Lightweight C++ command line option parser

- [pybind/pybind11](https://github.com/pybind/pybind11): Seamless operability between C++11 and Python

- [fmtlib/fmt](https://github.com/fmtlib/fmt): A modern formatting library

### Building and Installing

- install via `pip`

```
pip install cigsegy
```

- install locally
First, you need to install `fmt` and `pybind11`.
```bash
# linux
sudo apt-get install python3-pybind11 libfmt-dev

# mac
brew install pybind11 fmt
```
You can also install `fmt` and `pybind11` manually.
```bash
# install fmt
mkdir thridPart && cd thridPart/
git clone https://github.com/fmtlib/fmt.git
# Now fmt is installed into /xxx/cigsegy/thridPart/fmt

# install pybind11 using pypi
pip install pybind11
# Now pybind11 is installed into /xxx/lib/python3.8/site-packages/pybind11/
```

If you need the python package only, run:
```bash
pip install -U pip
pip install .

# if fmt is not in the env path
pip install . --install-option="--fmt_root=/xxx/fmt"

# if you need to build a wheel
# pip wheel . --build-option="--fmt_root=/xxx/fmt"
```

If you need the two executables files:
```bash
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=your-install-dir
make
make install

# if install fmt manually and pybind11 via pypi, using this
cmake .. -DCMAKE_INSTALL_PREFIX=your-install-dir \
  -Dpybind11_ROOT=/xxx/lib/python3.8/site-packages/pybind11/ \
  -Dfmt_ROOT=/xxx/cigsegy/thridPart/fmt/build/fmt/

# selecting other python version, add -DPYTHON_EXECUTABL
cmake .. -DCMAKE_INSTALL_PREFIX=your-install-dir -DPYTHON_EXECUTABLE=/xxx/bin/python -DPYTHON_LIBRARIES=/xxx/lib/
```

### python binding
Using functions for reading
```python
>>> import cigsegy
>>> cigsegy.textual_header('f3.segy')
C01 SEGY OUTPUT                    
C02                     
C03                                                                             
C04 Name: f3.segy  
...
>>> cigsegy.metaInfo('f3.segy', iline=5, xline=17)
shape: (n-time, n-crossline, n-inline) = (462, 951, 651)
sample interval: 4000, data format code: 4-bytes IEEE floating-point
inline start: 1, crossline start: 1
X interval: 25.0, Y interval: 25, time start: 0

>>> d = cigsegy.fromfile('f3.segy')
>>> d.shape
(90, 180, 43)

>>> d = cigsegy.fromfile('f3.segy', iline=5, xline=17)

# format=5 means 4 bytes IEEE floating point,
# format=1 means 4 bytes IBM floating point
>>> d = cigsegy.fromfile_ignore_header('f3.segy', 90, 180, 43, format=5)
>>> d.shape
(90, 180, 43)

>>> cigsegy.tofile('f3.segy', 'f3.dat')
>>> cigsegy.tofile('f3.segy', 'f3.dat', iline=5, xline=17)
>>> cigsegy.tofile_ignore_header('f3.segy', 'f3.dat', 90, 180, 43, format=5)

>> cigsegy.create('out.segy', 'in.dat', 100, 329, 190, format=5)
>> cigsegy.create('out.segy', d, 90, 180, 43, format=5)
```

Reading segy file
```python
>>> from cigsegy import Pysegy
>>> d = Pysegy('f3.segy')
>>> print(d.textual_header())
C01 SEGY OUTPUT                    
C02                     
C03                                                                             
C04 Name: f3.segy                                                               
C05 Type: 3D seismic  Created Time: 2022/11/25T10:16                            
......
C40 END EBCDIC

>>> print(d.metaInfo())
shape: (n-time, n-crossline, n-inline) = (462, 951, 651)
sample interval: 4000, data format code: 4-bytes IEEE floating-point
inline start: 1, crossline start: 1
X interval: 25.0, Y interval: 25.0, time start: 0

>>> f = d.read()
[##################################################] 100%
>>> f.shape
(651, 951, 462) # Note, in python, inline-number is the first, not n-time
>>> d.tofile('f3.dat')
[##################################################] 100%
>>> f = d.read(10, 100, 20, 200, 56, 99)
>>> f.shape
(90, 180, 43)
```

Creating segy file
```python
>>> import numpy as np
>>> from cigsegy import Pysegy
>>> d = Pysegy('f3.dat', 462, 951, 651)
>>> d.setTimeInterval(5000)
>>> d.create('ff.segy')
[##################################################] 100%
>>> data = np.random.rand(100, 97, 125)
>>> data.shape
(100, 97, 125) # Note, in python (100, 97, 125) == (inline, crossline, time)
>>> d = Pysegy(125, 97, 100) # time is the first argument
>>> d.create('kks.segy', data)
```

All binding functions can be seen in `python/PySegy.cpp` file.


### tools
There are two executables in `your-install-dir/bin/`: `SEGYRead` and `SEGYCreate`.

`SEGYRead`
```bash
./SEGYRead
# ./SEGYRead - a tool for segy file reading to binary file
# Usage:
#   ./SEGYRead [OPTION...] positional parameters

#   -o, --out arg               out binary file name
#   -f, --fills arg             the number to fill the miss trace, can be any 
#                               float or nan, or NAN
#   -z, --inline-loc arg        inline field in trace header, default is 189
#   -c, --crossline-loc arg     crossline field in trace header, default is 
#                               193
#   -p, --print_textual_header  print 3200 bytes textual header
#   -m, --meta_info             print meta info
#       --ignore-header         reading segy by ignoring header and specify 
#                               shape
#   -d, --dimensions arg        the dimensions (x, y, z) or (nt, ncrossline, 
#                               ninline), use as '-d 128,128,256' (Required)

# Examples:
#   ./SEGYRead -p f3.segy             : show textual header
#   ./SEGYRead -m f3.segy             : show meta information
#   ./SEGYRead -o f3.dat f3.segy      : convert
#   ./SEGYRead -i f3.segy -o f3.dat   : convert
#   ./SEGYRead -o f3.dat -z 5 f3.segy : convert by specify inline field
#   ./SEGYRead -o f3.dat -f nan f3.segy : convert and fill with nan
#   ./SEGYRead -o f3.dat --ignore-header -d 236,789,890 f3.segy : ignore header and specify shape
```

`SEGYCreate`
```bash
./SEGYCreate
# ./SEGYCreate - a tool for creating a segy file from a binary file
# Usage:
#   ./SEGYCreate [OPTION...] positional parameters

#   -o, --out arg            out segy file name (Required)
#   -d, --dimensions arg     the dimensions (x, y, z) or (nt, ncrossline, 
#                            ninline), use as '-d 128,128,256' (Required)
#   -z, --inline-loc arg     set inline field in trace header, default is 189
#   -c, --crossline-loc arg  set crossline field in trace header, default is 
#                            193
#       --dt arg             set sample interval, default is 4000
#   -f, --data_format arg    data format code, 4 bytes (1 for IBM, 5 for 
#                            IEEE) floating point, defualt is 5
#       --dx arg             set X interval
#       --dy arg             set Y interval
#       --min-inline arg     set start inline number
#       --min-crossline arg  set start crossline number
#       --start-time arg     set start time for each trace

# Examples:
#   ./SEGYCreate -i test.dat -o test.segy -d 128,128,256 : convert
#   ./SEGYCreate -o test.segy -d 128,128,256 test.dat : convert
#   ./SEGYCreate -o test.segy -d 128,128,256 -f 5 test.dat : specify data format
#   ./SEGYCreate -o test.segy -d 128,128,256 --dt 2000 test.dat : specify time interval
```


### Features
- Fast (Implemented in c++)
- python wraping
- dealing with irregular segy volume (missing some traces but sorted)
- ignore headers mode

### Limitations

- Only support post-stack segy file
- Only support 3D segy volume
- Only support 4 bytes IBM floating point and 4 bytes IEEE floating point

### Comparison

Compared with `segysak`, our implementation is much more faster (`segysak` implemented in pure python).
`cigsegy` is a little slower than `segyio` when reading a segy file, but the gap is small. However,
`cigsegy` is faster than `segyio` when creating a segy file.

`segyio` assumes that the file is a regular, sorted 3D volume.
It also supports files (unstrict mode) that are just a collection of traces too,
but lots of features are now disabled and will raise errors in this mode.

However, there are losts of segy files that are sorted, but missing some traces.
`segyio` doesn't support these files although they are easy dealed with.
`cigsegy` supports these files just using the same methods, e.g. `cigsegy.fromfile('miss.segy')`.

For some reasons (confidentiality requirement, mistakes when recording, ...), 
the file headers are broken. If you remember the volume size and sample format (1 for IBM, 5 for IEEE), 
`cigsegy` can read the files too. Just using 
```python
d = cigsegy.fromfile_ignore_header('miss.segy', inline_size, crossline_size, time, dformat)
```

`segymat` is implememted in MATLAB, it runs very slowly.

`shape = (651, 951, 462) 1.3G`
|mode|cigsegy|segyio|segysak|segymat|
|---|---|---|---|---|
|read|1.212s|0.944s|134.8s|151.99s|
|create|3.01s|14.03s|-|-|



Run `read` 3 times for `cigsegy` and `segyio`, 
once for `segysak`, 
`(1062, 2005, 2401) 20G` 
|mode|cigsegy|segyio|segysak|segymat|
|---|---|---|---|---|
|read|45.6s+45.5s+45s|65s+20s+19s|612.45s|>1500s|
|create|41.14s+43.35s|78.74s+82.74s|-|-|

The second reading by `segyio` is faster than the first reading by `segyio`, 
while the time required for reading three times by `cigsegy` is very close.


### special cases handling

- sorted but missing some traces

``` python
d = cigsegy.fromfile('ex.segy')
```

- ignore headers

```python
# shape (n-inline, n-crossline, n-time) = (167, 78, 99)
# format = 5 for IEEE, 1 for IBM
d = cigsegy.fromfile_ignore_header('ex.segy', 167, 78, 99, format=5)
```

- if the segy data is resampled, e.g. segy files from USGS

```python
>>> cigsegy.textual_header('G3D201611-05_lns5090_5458.sgy')
C14 First-Line: 5090     Last-Line: 5458  Line-Spacing: 53.3650m   Inc: 2       
C15 First-CDP:  11800    Last-CDP:  15600 CDP-Spacing: 13.3439m    Inc: 5 
...
C19 Line-Byte-Pos:221-224 32Bit CDP-Byte-Pos:21-24 32Bit
```

We note that the data is resampled, and the inline number interval is 2,
crossline number interval is 5, not 1. So we can use:

```python
d = cigsegy.tools.read_with_step('G3D201611-05_lns5090_5458.sgy', 
      iline=221, xline=21, iline_step=2, xline_step=5)
```

Or 

```python
d = cigsegy.tools.read_unstrict('G3D201611-05_lns5090_5458.sgy', 
      iline=221, xline=21)
```

- just want to collect the traces and their location (iline, xline, x, y)

```python
data, header = cigsegy.collect('G3D201611-05_lns5090_5458.sgy', iline=221, xline=21, xfield=73, yfield=77)
print(data.shape)
# (140785, 1002)
# = (trace_count, n-time)
print(header.shape)
# (140785, 4)
# each raw is [inline, crossline, x, y]
```
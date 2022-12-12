# cigsegy

A tool for segy-format file reading and segy-format creating from binary file.
I also offer the python-binding of the tool.

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
First, you need install `fmt` and `pybind11`.
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

If you need python extension only, run:
```bash
pip install -U pip
pip install . --install-option="--fmt_root=/xxx/fmt"
```

If you need two executables files
install cigsegy
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
cmake .. -DCMAKE_INSTALL_PREFIX=your-install-dir -DPYTHON_EXECUTABL=/xxx/bin/python -DPYTHON_LIBRARIES=/xxx/lib/
```

### python binding
Using `your-install-path/python/cigsegy.cpython-3x-platform-OS-gnu.so` file.

Using functions for reading
```python
>>> import cigsegy
>>> d = cigsegy.fromfile('f3.segy')
>>> d.shape
(90, 180, 43)
>>> d = cigsegy.fromfile('f3.segy', iline=5, xline=17)
>>> d = cigsegy.fromfile_ignore_header('f3.segy', 90, 180, 43, format=5)
>>> d.shape
(90, 180, 43)
>>> cigsegy.tofile('f3.segy', 'f3.dat')
>>> cigsegy.tofile('f3.segy', 'f3.dat', iline=5, xline=17)
>>> cigsegy.tofile_ignore_header('f3.segy', 'f3.dat', 90, 180, 43, format=5)
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
X interval: 24.973711, Y interval: 25, time start: 0
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

### Limitation

- Only support post-stack segy file
- Only support 3D segy volume

### Comparison

Compared with `segysak`, our implementation is much more faster (`segysak` implemented in pure python).
`cigsegy` is a little slower than `segyio` when reading a segy file, but the gap is small. However,
`cigsegy` is faster than `segyio` when creating a segy file.

`segyio` assumes that the file is a regular, sorted 3D volume.
It also supports files (unstrict mode) that are just a collection of traces too,
but lots of features are now disabled and will raise errors in this mode.

However, there are losts of segy files that are sorted, but missing some traces.
`segyio` doesn't support these files although they are easy dealed with.
`cigsegy` supports these files just using the some methods, e.g. `cigsegy.fromfile('miss.segy')`.

For some reasons (confidentiality requirement, mistakes when recording, ...), 
the file headers are broken. If you remember the volume size and data format, 
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
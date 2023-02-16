from __future__ import annotations
import cigsegy
import typing
import numpy

_Shape = typing.Tuple[int, ...]

__all__ = [
    "Pysegy", "fromfile", "fromfile_ignore_header", "tofile",
    "tofile_ignore_header", "collect"
]


class Pysegy():

    @typing.overload
    def __init__(self, segy_name: str) -> None:
        """
        Reading mode

        Parameters:
        - segy_name: the input segy format file
        """

    @typing.overload
    def __init__(self, sizeX: int, sizeY: int, sizeZ: int) -> None:
        """
        Creating mode

        Parameters:
        - sizeX: the number of samples per trace,
        - sizeY: the number of crossline,
        - sizeZ: the number of inline.
        """

    @typing.overload
    def __init__(self, binary_name: str, sizeX: int, sizeY: int,
                 sizeZ: int) -> None:
        """
        Creating mode

        Parameters:
        - binary_name: the input binary file name,
        - sizeX: the number of samples per trace,
        - sizeY: the number of crossline,
        - sizeZ: the number of inline.
        """

    @typing.overload
    def create(self, segy_out_name: str) -> None:
        """
        create a segy

        Parameters:
        - segy_out_name: the output file name to create segy format
        """

    @typing.overload
    def create(self, segy_out_name: str,
               src: numpy.ndarray[numpy.float32]) -> None:
        """
        create a segy from src
        """

    def metaInfo(self) -> str:
        """ 
        the meta info for the whole segy file
        """

    @typing.overload
    def read(self) -> numpy.ndarray[numpy.float32]:
        """
        read whole volume from segyfile
        """

    @typing.overload
    def read(self, startZ: int, endZ: int, startY: int, endY: int, startX: int,
             endX: int) -> numpy.ndarray[numpy.float32]:
        """ 
        read a volume with index, the volume size is 
        [startZ:endZ, startY:endY, startX:endX]

        Return: numpy.ndarray
        """

    def read_cross_slice(self, iY: int) -> numpy.ndarray[numpy.float32]:
        """
        read a crossline slice with index
        """

    def read_inline_slice(self, iZ: int) -> numpy.ndarray[numpy.float32]:
        """
        read an inline slice with index
        """

    def read_time_slice(self, iX: int) -> numpy.ndarray[numpy.float32]:
        """
        read a time slice with index
        """

    def read_trace(self, iZ: int, iY: int) -> numpy.ndarray[numpy.float32]:
        """
        read a trace with index
        """

    def scan(self) -> None:
        """
        scan the whole segy file
        """

    def setCrosslineLocation(self, xline: int) -> None:
        """ 
        set the crossline field of trace headers (for reading segy)
        recommend: 193, 17, 21, default is 193.
        """

    def setDataFormatCode(self, format: int) -> None:
        """
        set data format code (for creating segy).
        Only support: 
        1 for 4 bytes IBM float
        5 for 4 bytes IEEE float
        default is 5.
        """

    def setFillNoValue(self, fills: float) -> None:
        """ 
        set a value for filling the missing trace (for reading segy),
        can be any float number or np.nan
        """

    def setInlineLocation(self, iline: int) -> None:
        """ 
        set the crossline field of trace headers (for reading segy)
        recommend: 189, 5, 9, default is 189.
        """

    def setXLocation(self, xfield: int) -> None:
        """
        set the x field of trace headers (for reading segy)
        recommend: 73, 181
        """

    def setYLocation(self, yfield: int) -> None:
        """
        set the y field of trace headers (for reading segy)
        recommend: 77, 185
        """

    def setMinCrossline(self, minXline: int) -> None:
        """
        set min crossline number (for creating segy)
        """

    def setMinInline(self, minInline: int) -> None:
        """
        set min inline number (for creating segy)
        """

    def setSampleInterval(self, dt: int) -> None:
        """ 
        set sample interval for creating segy,
        the defualt is 4000, i.e. 4 ms
        """

    def setStartTime(self, start_time: int) -> None:
        """
        set start time for creating segy
        the defualt is 0
        """

    def setXInterval(self, dx: float) -> None:
        """ 
        set X interval for creating segy
        """

    def setYInterval(self, dy: float) -> None:
        """ 
        set Y interval for creating segy
        """

    def set_size(self, sizeX: int, sizeY: int, sizeZ: int) -> None:
        """
        set data shape for creating segy or reading by ignoring headers.

        Parameters:
        - sizeX: number of samples per trace
        - sizeY: number of crossline
        - sizeZ: number of inline
        """

    def textual_header(self) -> str:
        """
        obtain the 3200 bytes textual header.

        Return: str
        """

    def tofile(self, binary_out_name: str) -> None:
        """ 
        read a segy file and convert it to a binary file.

        Parameters:
        - binary_out_name: the output binary file name
        """

    def close_file(self) -> None:
        """
        close mmap for reading mode
        """

    pass


def fromfile(segy_name: str,
             iline: int = 189,
             xline: int = 193) -> numpy.ndarray[numpy.float32]:
    """
    reading from a segy file.

    Parameters:
    - segy_name: the input segy file name
    - iline: the inline number field in each trace header
    - xline: the crossline number field in each trace header
    """


def fromfile_ignore_header(segy_name: str,
                           sizeZ: int,
                           sizeY: int,
                           sizeX: int,
                           format: int = 5) -> numpy.ndarray[numpy.float32]:
    """
    reading by ignoring segy headers and specifing the volume shape

    Parameters:
    - segy_name: the input segy file
    - sizeZ: number of inline
    - sizeY: number of crossline
    - sizeX: number of samples per trace
    - format: the data format code, 1 for 4 bytes IBM float, 5 for 4 bytes IEEE float
    """


def tofile(segy_name: str,
           out_name: str,
           iline: int = 189,
           xline: int = 193) -> None:
    """
    convert a segy file to a binary file

    Parameters:
    - segy_name: the input segy file name
    - out_name: the output binary file name
    - iline: the inline number field in each trace header
    - xline: the crossline number field in each trace header
    """


def tofile_ignore_header(segy_name: str,
                         out_name: str,
                         sizeX: int,
                         sizeY: int,
                         sizeZ: int,
                         format: int = 5) -> None:
    """
    convert a segy file to a binary file by ignoring segy header 
    and specifing the volume shape.

    Parameters:
    - segy_name: the input segy file name
    - out_name: the output binary file name
    - sizeZ: number of inline
    - sizeY: number of crossline
    - sizeX: number of samples per trace
    - format: the data format code, 1 for 4 bytes IBM float, 5 for 4 bytes IEEE float
    """


def collect(
    segy_in: str,
    iline: int = 189,
    xline: int = 193,
    xfield: int = 73,
    yfield: int = 77
) -> typing.Tuple[numpy.ndarray[numpy.float32], numpy.ndarray[numpy.int32]]:
    """
    collect all traces data and their location (iline, xline, X, Y) from the `segy_in` file.

    Parameters:
    - segy_in: str, the input segy file
    - iline: int, the inline number field
    - xline: int, the crossline number field
    - xfield: int, the X field
    - yfield: int, the Y field

    Returns:
    a tuple, 
    - the first element is the data (numpy.ndarray), its shape = (trace_cout, n-time)
    - the second element is the header, 
    its shape = (trace_count, 4) = trace_cout * (iline, xline, x, y)
    """


def create_by_sharing_header(segy_name: str,
                             header_segy: str,
                             src: numpy.ndarray[numpy.float32],
                             iline: int = 189,
                             xline: int = 193):
    """
    create a segy and its header is from an existed segy.

    Parameters:
    - segy_name: str, the out segy name
    - header_segy: str, the header segy file
    - src: numpy.ndarray, source data
    - iline: int, the inline number field of header segy
    - xline: int, the crossline number field of header segy
    """

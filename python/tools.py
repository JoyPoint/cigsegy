import numpy
from typing import Tuple
from .cigsegy import (Pysegy, collect)


def create(segy_out: str,
           binary_in: str or numpy.ndarray,
           sizeZ: int,
           sizeY: int,
           sizeX: int,
           format: int = 5,
           dt: int = 2000,
           start_time: int = 0,
           X_interval: float = 25,
           Y_interval: float = 25,
           min_iline: int = 1,
           min_xline: int = 1):
    """
    Create a segy format file from a binary file or numpy.ndarray
    
    Parameters:
        - segy_out: str, out segy format file path
        - binary_in: str or numpy.array, the input binary file or array
        - sizeZ: int, number of inline
        - sizeY: int, number of crossline
        - sizeX: int, number of samples per trace
        - format: int, the data format code, 1 for 4 bytes IBM float, 5 for 4 bytes IEEE float
        - dt: int, data sample interval, 2000 means 2ms
        - start_time: int, start time for each trace
        - X_interval: int
        - Y_interval: int
        - min_iline: int, the start inline number
        - min_xline: int, the start crossline number
    """
    if isinstance(binary_in, str):
        segy_create = Pysegy(binary_in, sizeX, sizeY, sizeZ)
    elif isinstance(binary_in, numpy.ndarray):
        if binary_in.shape == (sizeZ, sizeY, sizeX):
            segy_create = Pysegy(sizeX, sizeY, sizeZ)
        else:
            raise RuntimeError(
                f'the binary_in shape: {binary_in.shape} does not match the input dim: ({sizeZ}, {sizeY}, {sizeX})'
            )
    else:
        raise ValueError(
            f'the input argument: binary_in must be a string or numpy array')
    segy_create.setDataFormatCode(format)
    segy_create.setSampleInterval(dt)
    segy_create.setStartTime(start_time)
    segy_create.setXInterval(X_interval)
    segy_create.setYInterval(Y_interval)
    segy_create.setMinInline(min_iline)
    segy_create.setMinCrossline(min_xline)
    if isinstance(binary_in, str):
        segy_create.create(segy_out)
    else:
        segy_create.create(segy_out, binary_in)


def is_sorted(header: numpy.ndarray) -> bool:
    for i in range(header.shape[0] - 1):
        if (header[i, 0], header[i, 1]) >= (header[i + 1, 0], header[i + 1,
                                                                     1]):
            return False

    return True


def step(header: numpy.ndarray) -> Tuple[int, int]:
    iline = numpy.unique(header[:, 0])
    xline = numpy.unique(header[:, 1])
    step1 = iline[2] - iline[1]
    step2 = xline[2] - xline[1]
    if ((iline + step1)[:-1] == iline[1:]).all() and ((xline + step2)[:-1]
                                                      == xline[1:]).all():
        return (step1, step2)
    else:
        return (-1, -1)


def read_unstrict(segy_name, iline, xline) -> numpy.ndarray:
    data, header = collect(segy_name, iline, xline)
    if not is_sorted(header):
        raise RuntimeError("the segy file is unsorted, don't supprt now")

    step1, step2 = step(header)
    if step1 != -1 and step2 != -1:
        ni = (header[:, 0].max() - header[:, 0].min()) / step1 + 1
        nx = (header[:, 1].max() - header[:, 1].min()) / step2 + 1
        nt = data.shape[1]
        if ni * nx != data.shape[0]:
            raise RuntimeError(
                "n-inline * n-crossline != trace_count in this settings")

        data = data.reshape(int(ni), int(nx), nt)
        return data
    else:
        raise RuntimeError("Please use `cigsegy.fromfile()` function")


def read_with_step(segy_name, iline, xline, iline_step,
                   xline_step) -> numpy.ndarray:
    data, header = collect(segy_name, iline, xline)
    ni = (header[:, 0].max() - header[:, 0].min()) / iline_step + 1
    nx = (header[:, 1].max() - header[:, 1].min()) / xline_step + 1
    nt = data.shape[1]
    if ni * nx != data.shape[0]:
        raise RuntimeError(
            "n-inline * n-crossline != trace_count in this settings")

    data = data.reshape(int(ni), int(nx), nt)
    return data


def textual_header(segy_name: str):
    segy = Pysegy(segy_name)
    print(segy.textual_header())
    segy.close_file()


def metaInfo(segy_name: str, iline: int = 189, xline: int = 193):
    segy = Pysegy(segy_name)
    segy.setInlineLocation(iline)
    segy.setCrosslineLocation(xline)
    print(segy.metaInfo())
    segy.close_file()

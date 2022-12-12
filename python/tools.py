import numpy
from .cigsegy import Pysegy


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
        segy_out: str, out segy format file path
        binary_in: str or numpy.array, the input binary file or array
        sizeZ: int, number of inline
        sizeY: int, number of crossline
        sizeX: int, number of samples per trace
        format: int, the data format code, 1 for 4 bytes IBM float, 5 for 4 bytes IEEE float
        dt: int, data sample interval, 2000 means 2ms
        start_time: int, start time for each trace
        X_interval: int
        Y_interval: int
        min_iline: int, the start inline number
        min_xline: int, the start crossline number
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

# Source code for "Comparison of photo-matching algorithms commonly used for photo capture-recapture studies"

This repository contains the source code used to perform the numeric analysis from the paper *"Comparison of photo-matching algorithms commonly used for photo capture-recapture studies"* which was published in *Ecology and Evolution*.

The data analysis has been done with Python v2.7 ([http://python.org](Python)). Please refer to the Python documentation on how to install and use Python for your operating system. It is also advisable to install a ready-made Python distribution such as [https://www.continuum.io/downloads](Anaconda) or [https://www.enthought.com/products/epd/](Enthought Python).

## Building I3S and APHIS
The I3S and APHIS pattern comparison modules are implemented in C++ and directly taken from the original [http://www.reijns.com/i3s/(I3S source code) and [https://imedea.uib-csic.es/bc/gep/docs/aphis/APHISPROGRAM/APHIS_source/](APHIS source code). However, python bindings were added to make the system easier to automate. The system uses [https://cmake.org/](CMake) as the project file generator. Before building the project, the following prerequisites need to be fulfilled:

- a compiled [http://boost.org](C++ boost library) including Boost.Python to be accessible by CMake
- Python development files accessible by CMake
- [http://opencv.org/](OpenCV) development files to be accessible via CMake

Generic build commands would be
```
$ cd path/to/extracted/archive
$ mkdir build
$ cmake ../src
$ make
```

Please refer to the CMake documentation for specific instructions.

## Building Wild-ID
A Java program named `WildIDBatchCompare` is provided, which compares all patterns in a given directory with the Wild-ID algorithm and creates a corresponding equality score table file. In order to compile the program, you need to obtain the WildID source code from [http://dartmouth.edu/faculty-directory/douglas-thomas-bolger](Douglas Bolger's website). Also, install [http://eclipse.org](Eclipse) and import the source code from WildID. Furthermore, create a project called `WildIDBatchCompare` and import the source files from the archive, which reside in the folder `WildIDBatchCompare`.

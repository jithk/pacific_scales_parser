# Pacific Scales Parser
A Program to parse the pacific scales UART data

## Prerequisites
* Working C++ Compiler that supports **C++17** standard - Tested on GCC 10.2 and Clang-14
* [CMake](https://cmake.org/) 3.12 or above
* [Python3](https://www.python.org/downloads/) version 3.9 or above
* GIT if checking out from Repo
* Make or [Ninja](https://ninja-build.org/) build utility
## Downloading the source code
### Use GIT to checkout the latest source code
  ```bash
  $ git clone https://github.com/jithk/pacific_scales_parser.git
  ```
* OR download the latest zip from [this location](https://github.com/jithk/pacific_scales_parser/archive/refs/heads/main.zip)

### Project Structure
  There are three main folders in this project
  * `include` - Location of all the header files
  * `src`     - Source files (including main and Parser,SerialIO implementations)

## Building PacificScalesParser
  Once the source code is downloaded/cloned, do the following steps inside the source dir to build PacificScalesParser
  ```bash
  $ cd pacific_scales_parser # Move into our source dir
  $ rm -rf build && mkdir build # Delete any existing build dir and create a new one
  $ cd build  # Move into the build dir
  $ cmake ..   # Generate CMake config and buid files
  $ make pacific-parser  # Build executable
  ```
Note: There is a helper-script `build.sh` in the root folder that can build PacificScalesParser
## Running PacificScalesParser
Once built - `pacific-parser` binary will be inside the `build` directory. To run `pacific-parser`, just execute this from command line
```bash
cd pacific_scales_parser # move into the source dir
sudo ./build/pacific-parser  -p /dev/ttyUSB0 -b 115200  # execute the binary with DeviceName and BaudRate
```
Note: `sudo` might not be necessary if the user has sufficient permissions to read the serial device
### Expected Output
```bash
sudo ./pacific-parser -p /dev/ttyUSB0 -b 115200
Opened the device : /dev/ttyUSB0
Latest weight data for: Mon Nov 25 00:02:20 2024

{
  "A" : 5000,
  "B" : 17000,
  "C" : 22000,
  "D" : 15000,
  "TOTAL" : 59000,
  "VALID" : true
}

Latest weight data for: Mon Nov 25 00:02:30 2024

{
  "A" : 5500,
  "B" : 17500,
  "C" : 22500,
  "D" : 15500,
  "TOTAL" : 61000,
  "VALID" : true
}

Latest weight data for: Mon Nov 25 00:02:40 2024

{
  "B" : 18000,
  "A" : 6000,
  "C" : 23000,
  "D" : 16000,
  "TOTAL" : 63000,
  "VALID" : true
}

Latest weight data for: Mon Nov 25 00:02:50 2024

{
  "A" : 5300,
  "B" : 17300,
  "C" : 22300,
  "D" : 15300,
  "TOTAL" : 60200,
  "VALID" : true
}

```
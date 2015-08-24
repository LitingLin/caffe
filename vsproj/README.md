# Visual Studio Projects for Caffe

What works:
- Nearly every thing, include lmdb backend
- Shared library build

What not works (Just because I don't need it currently):
- Python interface
- Matlab interface

Requirements:
- Visual Studio 2013
- Cuda Toolkit 7.0 (with VS integration)
- Intel Math Kernel Library (with VS integration)
- Incredibuild 7.0 or upper (optional, to speed up the build procedure)

Other dependencies:
- Boost
- Cudnn
- Gflags
- Glog
- Gtest
- Hdf5
- LevelDB
- Lmdb
- OpenCV
- Protobuf
- Snappy

You can build these dependencies by yourself, or simply download it from Google Drive (https://drive.google.com/file/d/0B-TUGY6fy2wgSEJYWDFfeHlES1k/view) or Baidu Yun (http://pan.baidu.com/s/1hqev0A4) and decompress it in this directory.
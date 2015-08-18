#ifndef CAFFE_UTIL_IO_H_
#define CAFFE_UTIL_IO_H_

#include <unistd.h>
#include <string>

#include "google/protobuf/message.h"

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/proto/caffe.pb.h"

namespace caffe {

using ::google::protobuf::Message;
const char temp_path_template[] =
#ifdef __unix__
  "/tmp/caffe_test.";
#elif defined _WIN32
  "%TEMP%/caffe_test.";
#endif

inline void MakeTempFilename(string* temp_filename) {
  temp_filename->clear();
  *temp_filename = temp_path_template + string(tmpnam(nullptr));
  FILE* fd = fopen(temp_filename->c_str(), "w");
  CHECK(fd != NULL) << "Failed to open a temporary file at: " << *temp_filename << ". Error code: " << errno;
  fclose(fd);
}

inline void MakeTempDir(string* temp_dirname) {
  temp_dirname->clear();
  *temp_dirname = temp_path_template + string(tmpnam(nullptr));
  int mkdtemp_result = mkdir(temp_dirname->c_str(), 0700);
  CHECK_EQ(mkdtemp_result, 0)
	  << "Failed to create a temporary directory at: " << *temp_dirname << ". Error code: " << errno;
}

CAFFE_EXPORT bool ReadProtoFromTextFile(const char* filename, Message* proto);

inline bool ReadProtoFromTextFile(const string& filename, Message* proto) {
  return ReadProtoFromTextFile(filename.c_str(), proto);
}

inline void ReadProtoFromTextFileOrDie(const char* filename, Message* proto) {
  CHECK(ReadProtoFromTextFile(filename, proto));
}

inline void ReadProtoFromTextFileOrDie(const string& filename, Message* proto) {
  ReadProtoFromTextFileOrDie(filename.c_str(), proto);
}

CAFFE_EXPORT void WriteProtoToTextFile(const Message& proto, const char* filename);
inline void WriteProtoToTextFile(const Message& proto, const string& filename) {
  WriteProtoToTextFile(proto, filename.c_str());
}

CAFFE_EXPORT bool ReadProtoFromBinaryFile(const char* filename, Message* proto);

inline bool ReadProtoFromBinaryFile(const string& filename, Message* proto) {
  return ReadProtoFromBinaryFile(filename.c_str(), proto);
}

inline void ReadProtoFromBinaryFileOrDie(const char* filename, Message* proto) {
  CHECK(ReadProtoFromBinaryFile(filename, proto));
}

inline void ReadProtoFromBinaryFileOrDie(const string& filename,
                                         Message* proto) {
  ReadProtoFromBinaryFileOrDie(filename.c_str(), proto);
}


CAFFE_EXPORT void WriteProtoToBinaryFile(const Message& proto, const char* filename);
inline void WriteProtoToBinaryFile(
    const Message& proto, const string& filename) {
  WriteProtoToBinaryFile(proto, filename.c_str());
}

CAFFE_EXPORT bool ReadFileToDatum(const string& filename, const int label, Datum* datum);

inline bool ReadFileToDatum(const string& filename, Datum* datum) {
  return ReadFileToDatum(filename, -1, datum);
}

CAFFE_EXPORT bool ReadImageToDatum(const string& filename, const int label,
    const int height, const int width, const bool is_color,
    const std::string & encoding, Datum* datum);

inline bool ReadImageToDatum(const string& filename, const int label,
    const int height, const int width, const bool is_color, Datum* datum) {
  return ReadImageToDatum(filename, label, height, width, is_color,
                          "", datum);
}

inline bool ReadImageToDatum(const string& filename, const int label,
    const int height, const int width, Datum* datum) {
  return ReadImageToDatum(filename, label, height, width, true, datum);
}

inline bool ReadImageToDatum(const string& filename, const int label,
    const bool is_color, Datum* datum) {
  return ReadImageToDatum(filename, label, 0, 0, is_color, datum);
}

inline bool ReadImageToDatum(const string& filename, const int label,
    Datum* datum) {
  return ReadImageToDatum(filename, label, 0, 0, true, datum);
}

inline bool ReadImageToDatum(const string& filename, const int label,
    const std::string & encoding, Datum* datum) {
  return ReadImageToDatum(filename, label, 0, 0, true, encoding, datum);
}

CAFFE_EXPORT bool DecodeDatumNative(Datum* datum);
CAFFE_EXPORT bool DecodeDatum(Datum* datum, bool is_color);

CAFFE_EXPORT cv::Mat ReadImageToCVMat(const string& filename,
    const int height, const int width, const bool is_color);

CAFFE_EXPORT cv::Mat ReadImageToCVMat(const string& filename,
    const int height, const int width);

CAFFE_EXPORT cv::Mat ReadImageToCVMat(const string& filename,
    const bool is_color);

CAFFE_EXPORT cv::Mat ReadImageToCVMat(const string& filename);

CAFFE_EXPORT cv::Mat DecodeDatumToCVMatNative(const Datum& datum);
CAFFE_EXPORT cv::Mat DecodeDatumToCVMat(const Datum& datum, bool is_color);

CAFFE_EXPORT void CVMatToDatum(const cv::Mat& cv_img, Datum* datum);

}  // namespace caffe

#endif   // CAFFE_UTIL_IO_H_

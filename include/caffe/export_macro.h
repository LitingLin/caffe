#ifndef CAFFE_EXPORTS
#ifdef _MSC_VER
#ifndef CAFFE_LINK_STATIC
#ifdef CAFFE_EXPORTS_DLL
#define CAFFE_EXPORT __declspec(dllexport)
#else
#define CAFFE_EXPORT __declspec(dllimport)
#endif
#else
#define CAFFE_EXPORT
#endif
#else
#define CAFFE_EXPORT
#endif
#endif

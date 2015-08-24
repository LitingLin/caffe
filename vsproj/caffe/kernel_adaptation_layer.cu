#include "caffe/layer.hpp"
#include "caffe/util/im2col.hpp"
#include "caffe/util/math_functions.hpp"
#include "caffe/vision_layers.hpp"

namespace caffe{
	template <typename Dtype>
	void KernelAdaptationLayer<Dtype>::Forward_gpu(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top)
	{
	}

	template <typename Dtype>
	void KernelAdaptationLayer<Dtype>::Backward_gpu(const vector<Blob<Dtype>*>& top, const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom)
	{
	}
	
	INSTANTIATE_LAYER_GPU_FUNCS(KernelAdaptationLayer);
}
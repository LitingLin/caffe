#include "caffe/common.hpp"
#include "caffe/layer.hpp"
#include "caffe/syncedmem.hpp"
#include "caffe/util/math_functions.hpp"
#include "caffe/vision_layers.hpp"
#include "caffe/util/im2col.hpp"

#include <math.h>

namespace caffe{
	template <typename Dtype>
	void KernelAdaptationLayer<Dtype>::LayerSetUp(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top)
	{
		CHECK_EQ(4, bottom[0]->num_axes()) << "Input must have 4 axes, "
			<< "corresponding to (num, channels, height, width)";
		// Configure the kernel size, padding, stride, and inputs.
		ConvolutionParameter conv_param = this->layer_param_.convolution_param();
		CHECK(!conv_param.has_kernel_size() !=
			!(conv_param.has_kernel_h() && conv_param.has_kernel_w()))
			<< "Filter size is kernel_size OR kernel_h and kernel_w; not both";
		CHECK(conv_param.has_kernel_size() ||
			(conv_param.has_kernel_h() && conv_param.has_kernel_w()))
			<< "For non-square filters both kernel_h and kernel_w are required.";
		CHECK((!conv_param.has_pad() && conv_param.has_pad_h()
			&& conv_param.has_pad_w())
			|| (!conv_param.has_pad_h() && !conv_param.has_pad_w()))
			<< "pad is pad OR pad_h and pad_w are required.";
		CHECK((!conv_param.has_stride() && conv_param.has_stride_h()
			&& conv_param.has_stride_w())
			|| (!conv_param.has_stride_h() && !conv_param.has_stride_w()))
			<< "Stride is stride OR stride_h and stride_w are required.";
		if (conv_param.has_kernel_size()) {
			adaptation_kernel_h_ = adaptation_kernel_w_ = conv_param.kernel_size();
		}
		else {
			adaptation_kernel_h_ = conv_param.kernel_h();
			adaptation_kernel_w_ = conv_param.kernel_w();
		}
		CHECK_GT(adaptation_kernel_h_, 0) << "Filter dimensions cannot be zero.";
		CHECK_GT(adaptation_kernel_w_, 0) << "Filter dimensions cannot be zero.";
		kernel_h_ = kernel_w_ = sqrt(adaptation_kernel_h_);
		if (!conv_param.has_pad_h()) {
			pad_h_ = pad_w_ = conv_param.pad();
		}
		else {
			pad_h_ = conv_param.pad_h();
			pad_w_ = conv_param.pad_w();
		}
		if (!conv_param.has_stride_h()) {
			stride_h_ = stride_w_ = conv_param.stride();
		}
		else {
			stride_h_ = conv_param.stride_h();
			stride_w_ = conv_param.stride_w();
		}
		// Special case: im2col is the identity for 1x1 convolution with stride 1
		// and no padding, so flag for skipping the buffer and transformation.
		is_1x1_ = kernel_w_ == 1 && kernel_h_ == 1
			&& stride_h_ == 1 && stride_w_ == 1 && pad_h_ == 0 && pad_w_ == 0;
		// Configure output channels and groups.
		channels_ = bottom[0]->channels();
		num_output_ = this->layer_param_.convolution_param().num_output();
		CHECK_GT(num_output_, 0);
		group_ = this->layer_param_.convolution_param().group();
		CHECK_EQ(channels_ % group_, 0);
		CHECK_EQ(num_output_ % group_, 0)
			<< "Number of output should be multiples of group.";
		if (reverse_dimensions()) {
			conv_out_channels_ = channels_;
			conv_in_channels_ = num_output_;
		}
		else {
			conv_out_channels_ = num_output_;
			conv_in_channels_ = channels_;
		}
		// Handle the parameters: weights and biases.
		// - blobs_[0] holds the filter weights
		// - blobs_[1] holds the biases (optional)
		bias_term_ = this->layer_param_.convolution_param().bias_term();
		if (this->blobs_.size() > 0) {
			LOG(INFO) << "Skipping parameter initialization";
		}
		else {
			if (bias_term_) {
				this->blobs_.resize(2);
			}
			else {
				this->blobs_.resize(1);
			}
			// Initialize and fill the weights:
			// output channels x input channels per-group x kernel height x kernel width
			this->blobs_[0].reset(new Blob<Dtype>(
				conv_out_channels_, conv_in_channels_ / group_, adaptation_kernel_h_, adaptation_kernel_w_));
			shared_ptr<Filler<Dtype> > weight_filler(GetFiller<Dtype>(
				this->layer_param_.convolution_param().weight_filler()));
			weight_filler->Fill(this->blobs_[0].get());
			// If necessary, initialize and fill the biases.
			if (bias_term_) {
				vector<int> bias_shape(1, num_output_);
				this->blobs_[1].reset(new Blob<Dtype>(bias_shape));
				shared_ptr<Filler<Dtype> > bias_filler(GetFiller<Dtype>(
					this->layer_param_.convolution_param().bias_filler()));
				bias_filler->Fill(this->blobs_[1].get());
			}
		}
		// Propagate gradients to the parameters (as directed by backward pass).
		this->param_propagate_down_.resize(this->blobs_.size(), true);
	}

	template <typename Dtype>
	void KernelAdaptationLayer<Dtype>::Reshape(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top)
	{
		true_kernel_.Reshape(1, channels_, kernel_h_, kernel_w_);

		CHECK_EQ(4, bottom[0]->num_axes()) << "Input must have 4 axes, "
			<< "corresponding to (num, channels, height, width)";
		num_ = bottom[0]->num();
		height_ = bottom[0]->height();
		width_ = bottom[0]->width();
		CHECK_EQ(bottom[0]->channels(), channels_) << "Input size incompatible with"
			" convolution kernel.";
		// TODO: generalize to handle inputs of different shapes.
		/*  for (int bottom_id = 1; bottom_id < bottom.size(); ++bottom_id) {
		CHECK_EQ(num_, bottom[bottom_id]->num()) << "Inputs must have same num.";
		CHECK_EQ(channels_, bottom[bottom_id]->channels())
		<< "Inputs must have same channels.";
		CHECK_EQ(height_, bottom[bottom_id]->height())
		<< "Inputs must have same height.";
		CHECK_EQ(width_, bottom[bottom_id]->width())
		<< "Inputs must have same width.";
		}*/
		// Shape the tops.
		compute_output_shape();
		for (int top_id = 0; top_id < top.size(); ++top_id) {
			top[top_id]->Reshape(num_, num_output_, height_out_, width_out_);
		}
		if (reverse_dimensions()) {
			conv_in_height_ = height_out_;
			conv_in_width_ = width_out_;
			conv_out_spatial_dim_ = height_ * width_;
		}
		else {
			conv_in_height_ = height_;
			conv_in_width_ = width_;
			conv_out_spatial_dim_ = height_out_ * width_out_;
		}
		kernel_dim_ = conv_in_channels_ * kernel_h_ * kernel_w_;
		weight_offset_ = conv_out_channels_ * kernel_dim_ / group_ / group_;
		col_offset_ = kernel_dim_ * conv_out_spatial_dim_ / group_;
		output_offset_ = conv_out_channels_ * conv_out_spatial_dim_ / group_;
		// The im2col result buffer will only hold one image at a time to avoid
		// overly large memory usage. In the special case of 1x1 convolution
		// it goes lazily unused to save memory.
		if (reverse_dimensions()) {
			col_buffer_.Reshape(1, kernel_dim_, height_, width_);
		}
		else {
			col_buffer_.Reshape(1, kernel_dim_, height_out_, width_out_);
		}
		// Set up the all ones "bias multiplier" for adding biases by BLAS
		if (bias_term_) {
			vector<int> bias_multiplier_shape(1, height_out_ * width_out_);
			bias_multiplier_.Reshape(bias_multiplier_shape);
			caffe_set(bias_multiplier_.count(), Dtype(1),
				bias_multiplier_.mutable_cpu_data());
		}
	}

	// Doesn't support group
	template <typename Dtype>
	void KernelAdaptationLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top)
	{
		const Dtype* weight = this->blobs_[0]->cpu_data();
		const Dtype* bottom_data = bottom[0]->cpu_data();
		const Dtype* shape_info = bottom[1]->cpu_data();
		Dtype* true_kernel = true_kernel_.mutable_cpu_data();
		Dtype* top_data = top[0]->mutable_cpu_data();
		for (int n = 0; n < this->num_; ++n) {
			caffe::caffe_cpu_gemm<Dtype>(CblasNoTrans, CblasNoTrans, adaptation_kernel_h_, 1, adaptation_kernel_w_, 1., weight, shape_info, 0., true_kernel);
			this->forward_cpu_gemm(bottom_data + bottom[0]->offset(n), true_kernel,
				top_data + top[0]->offset(n));
			if (this->bias_term_) {
				const Dtype* bias = this->blobs_[1]->cpu_data();
				this->forward_cpu_bias(top_data + top[0]->offset(n), bias);
			}
		}
	}

	template <typename Dtype>
	void KernelAdaptationLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top, const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom)
	{
		const Dtype* weight = this->blobs_[0]->cpu_data();
		Dtype* weight_diff = this->blobs_[0]->mutable_cpu_diff();
		for (int i = 0; i < top.size(); ++i) {
			const Dtype* top_diff = top[i]->cpu_diff();
			const Dtype* bottom_data = bottom[i]->cpu_data();
			Dtype* bottom_diff = bottom[i]->mutable_cpu_diff();
			// Bias gradient, if necessary.
			if (this->bias_term_ && this->param_propagate_down_[1]) {
				Dtype* bias_diff = this->blobs_[1]->mutable_cpu_diff();
				for (int n = 0; n < this->num_; ++n) {
					this->backward_cpu_bias(bias_diff, top_diff + top[i]->offset(n));
				}
			}
			if (this->param_propagate_down_[0] || propagate_down[i]) {
				for (int n = 0; n < this->num_; ++n) {
					// gradient w.r.t. weight. Note that we will accumulate diffs.
					if (this->param_propagate_down_[0]) {
						const Dtype* col_buff = bottom_data + bottom[i]->offset(n);
						const Dtype* output = top_diff + top[i]->offset(n);
						if (!is_1x1_) {
							conv_im2col_cpu(col_buff, col_buffer_.mutable_cpu_data());
							col_buff = col_buffer_.cpu_data();
						}
						for (int g = 0; g < group_; ++g) {
							caffe_cpu_gemm<Dtype>(CblasNoTrans, CblasTrans, conv_out_channels_ / group_,
								kernel_dim_ / group_, conv_out_spatial_dim_,
								(Dtype)1., output + output_offset_ * g, col_buff + col_offset_ * g,
								(Dtype)1., weight_diff + weight_offset_ * g);
						}
						this->weight_cpu_gemm(bottom_data + bottom[i]->offset(n),
							top_diff + top[i]->offset(n), weight_diff);
					}
					// gradient w.r.t. bottom data, if necessary.
					if (propagate_down[i]) {
						this->backward_cpu_gemm(top_diff + top[i]->offset(n), weight,
							bottom_diff + bottom[i]->offset(n));
					}
				}
			}
		}
	}

	template <typename Dtype>
	void KernelAdaptationLayer<Dtype>::compute_output_shape()
	{
		this->height_out_ = (this->height_ + 2 * this->pad_h_ - this->kernel_h_)
			/ this->stride_h_ + 1;
		this->width_out_ = (this->width_ + 2 * this->pad_w_ - this->kernel_w_)
			/ this->stride_w_ + 1;
	}
#ifdef CPU_ONLY
	STUB_GPU(KernelAdaptationLayer);
#endif

	INSTANTIATE_CLASS(KernelAdaptationLayer);
	REGISTER_LAYER_CLASS(KernelAdaptation);
}  // namespace caffe
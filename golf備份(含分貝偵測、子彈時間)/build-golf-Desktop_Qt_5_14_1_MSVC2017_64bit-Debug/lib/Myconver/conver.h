#pragma once
#ifndef CONVER_H
#define CONVER_H

#ifdef CPP_DLL_EXPORT
#define CPP_DLL_API __declspec(dllexport)
#else
#define CPP_DLL_API __declspec(dllimport)
#endif

//Version 2023.09.08

extern "C" {
#include <libavformat/avformat.h>
}

#include <opencv2/core/cuda.hpp>

namespace MyConver {

	struct Kernel_param
	{
		size_t thread_x = 1;
		size_t thread_y = 1;
		size_t thread_z = 1;
		size_t block_x = 1;
		size_t block_y = 1;
		size_t block_z = 1;
	};


	CPP_DLL_API Kernel_param Get_Kernel_Param(int width, int height);

	CPP_DLL_API cv::cuda::GpuMat Avframe_NV12_to_Mat_BGR(AVFrame* NV12_Frame);
	CPP_DLL_API cv::cuda::GpuMat Avframe_NV12_to_Mat_RGB(AVFrame* NV12_Frame);

	CPP_DLL_API cv::cuda::GpuMat Avframe_NV12_to_Mat_BGR(AVFrame* NV12_Frame, Kernel_param K_pram);
	CPP_DLL_API cv::cuda::GpuMat Avframe_NV12_to_Mat_RGB(AVFrame* NV12_Frame, Kernel_param K_pram);

	CPP_DLL_API void Avframe_NV12_to_Mat_BGR(AVFrame* NV12_Frame, cv::cuda::GpuMat* BGR_Frame);
	CPP_DLL_API void Avframe_NV12_to_Mat_RGB(AVFrame* NV12_Frame, cv::cuda::GpuMat* RGB_Frame);

	CPP_DLL_API void Avframe_NV12_to_Mat_BGR(AVFrame* NV12_Frame, cv::cuda::GpuMat* BGR_Frame, MyConver::Kernel_param K_pram);
	CPP_DLL_API void Avframe_NV12_to_Mat_RGB(AVFrame* NV12_Frame, cv::cuda::GpuMat* RGB_Frame, MyConver::Kernel_param K_pram);

	CPP_DLL_API void Avframe_NV12_to_Mat_BGR(AVFrame* NV12_Frame, cv::cuda::GpuMat& BGR_Frame);
	CPP_DLL_API void Avframe_NV12_to_Mat_RGB(AVFrame* NV12_Frame, cv::cuda::GpuMat& RGB_Frame);

	CPP_DLL_API void Avframe_NV12_to_Mat_BGR(AVFrame* NV12_Frame, cv::cuda::GpuMat& BGR_Frame, MyConver::Kernel_param K_pram);
	CPP_DLL_API void Avframe_NV12_to_Mat_RGB(AVFrame* NV12_Frame, cv::cuda::GpuMat& RGB_Frame, MyConver::Kernel_param K_pram);

	//*----------------------------------------------------------------------------

	CPP_DLL_API Kernel_param Get_Kernel_Param_2(int width, int height);

	CPP_DLL_API cv::cuda::GpuMat Avframe_NV12_to_Mat_BGR_2(AVFrame* NV12_Frame);
	CPP_DLL_API cv::cuda::GpuMat Avframe_NV12_to_Mat_RGB_2(AVFrame* NV12_Frame);

	CPP_DLL_API cv::cuda::GpuMat Avframe_NV12_to_Mat_BGR_2(AVFrame* NV12_Frame, Kernel_param K_pram);
	CPP_DLL_API cv::cuda::GpuMat Avframe_NV12_to_Mat_RGB_2(AVFrame* NV12_Frame, Kernel_param K_pram);


	CPP_DLL_API void Avframe_NV12_to_Mat_BGR_2(AVFrame* NV12_Frame,cv::cuda::GpuMat* BGR_Frame);
	CPP_DLL_API void Avframe_NV12_to_Mat_RGB_2(AVFrame* NV12_Frame,cv::cuda::GpuMat* RGB_Frame);


	CPP_DLL_API void Avframe_NV12_to_Mat_BGR_2(AVFrame* NV12_Frame, cv::cuda::GpuMat* BGR_Frame, MyConver::Kernel_param K_pram);
	CPP_DLL_API void Avframe_NV12_to_Mat_RGB_2(AVFrame* NV12_Frame, cv::cuda::GpuMat* RGB_Frame, MyConver::Kernel_param K_pram);

	CPP_DLL_API void Avframe_NV12_to_Mat_BGR_2(AVFrame* NV12_Frame, cv::cuda::GpuMat& BGR_Frame);
	CPP_DLL_API void Avframe_NV12_to_Mat_RGB_2(AVFrame* NV12_Frame, cv::cuda::GpuMat& RGB_Frame);


	CPP_DLL_API void Avframe_NV12_to_Mat_BGR_2(AVFrame* NV12_Frame, cv::cuda::GpuMat& BGR_Frame, MyConver::Kernel_param K_pram);
	CPP_DLL_API void Avframe_NV12_to_Mat_RGB_2(AVFrame* NV12_Frame, cv::cuda::GpuMat& RGB_Frame, MyConver::Kernel_param K_pram);

	//----------------------------------------------------------------------------

	CPP_DLL_API void Mat_BGR_to_uint8_NV12_2(cv::cuda::GpuMat* RGB_Frame, unsigned char* NV12_Frame);
	CPP_DLL_API void Mat_BGR_to_uint8_NV12_3(cv::cuda::GpuMat* RGB_Frame, unsigned char* GPU_data, unsigned char* NV12_Frame);

	CPP_DLL_API uint8_t* Create_GPU_uint8_NV12(cv::cuda::GpuMat* Frame);
	CPP_DLL_API uint8_t* Create_GPU_uint8(size_t size);
	CPP_DLL_API void Free_GPU_uint8(uint8_t*);



}

namespace MyKernel 
{
	CPP_DLL_API void NV12toBGR_Kernel(MyConver::Kernel_param K_param,const unsigned char* nv12Data, unsigned char* bgrData, int width, int height, int nv12_w_linesize, int bgr_w_linesize, int nv12_extra_size);
	CPP_DLL_API void NV12toRGB_Kernel(MyConver::Kernel_param K_param, const unsigned char* nv12Data, unsigned char* rgbData, int width, int height, int nv12_w_linesize, int rgb_w_linesize, int nv12_extra_size);
	CPP_DLL_API void NV12toBGR_Kernel_2(MyConver::Kernel_param K_param, const unsigned char* nv12Data, unsigned char* bgrData, int width, int height, int nv12_w_linesize, int bgr_w_linesize, int nv12_extra_size);
	CPP_DLL_API void NV12toRGB_Kernel_2(MyConver::Kernel_param K_param, const unsigned char* nv12Data, unsigned char* rgbData, int width, int height, int nv12_w_linesize, int rgb_w_linesize, int nv12_extra_size);
	
	//---------------------------------------------------------------------------
	CPP_DLL_API void BGRtoNV12_Kernel(MyConver::Kernel_param K_param, const unsigned char* bgrData, unsigned char* nv12Data, int width, int height, int brg_linesize);
}


#endif // CONVER_H

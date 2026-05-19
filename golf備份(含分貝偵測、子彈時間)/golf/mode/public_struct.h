#ifndef PUBLIC_STRUCT_H
#define PUBLIC_STRUCT_H

#include <opencv2/core/cuda.hpp>
#include <iostream>


using clock_type = std::chrono::high_resolution_clock;
using milli_type = std::chrono::duration<double,std::milli>;
using micro_type = std::chrono::duration<double,std::micro>;

//auto t1 = clock_type::now();
//printf("time3 [%lf]\n",milli_type(t2-t1).count());
typedef std::chrono::time_point<std::chrono::steady_clock,std::chrono::duration<long long ,std::ratio<1,1000000000>>> _time;


struct GpuMat_Struct
{
    cv::cuda::GpuMat gpu_mat;
    uint32_t frame_num;
    uint64_t ntp;
};

#endif // PUBLIC_STRUCT_H

//
// Created by chenyaohuang on 2021/12/27.
//
#pragma once
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "lite/backends/opencl/cl_runtime.h"
#include "paddle_place.h"
namespace paddle{
namespace lite_api{
namespace opencl{

LITE_API bool IsLiteOpenCLBackendValid(bool check_fp16_valid = false);

//UNKNOWN：0
//QUALCOMM_ADRENO：1
//IMAGINATION_POWERVR：2
//ARM_MALI：3
//OTHERS：4
//error：-1；
LITE_API int LiteCLGetGpuType();

LITE_API bool IsOpenCLSupportHalf();

LITE_API void LiteSetOpenclTune(CLTuneMode tune_mode = CL_TUNE_NONE,
                                const std::string& path = "",
                                const std::string& name = "",
                                size_t lws_repeats = 4);


//CL_PRECISION_AUTO：0
//CL_PRECISION_FP16：1
//CL_PRECISION_FP32：2
//error：-1；
LITE_API int LiteSetPrecision(int p=0);
LITE_API int LiteGetPrecision();

LITE_API std::unique_ptr<cl::Program> CreateProgram( const cl::Context& context, std::string file_name);

LITE_API cl::Context& GetContext();

LITE_API  cl::CommandQueue& GetQueue();



LITE_API cl::Kernel & CreateKernel(const std::string &kernel_name,
                                                  const std::string &file_name,
                                                  const std::string &options = "",
                                                  const std::string &time_stamp = "");
class LiteKernelApi{
 public:
  LITE_API void AddKernel(const std::string &kernel_name,
                          const std::string &file_name,
                          const std::string &options = "",
                          const std::string &time_stamp = "");
  LITE_API  cl::Kernel& GetKernel(const int index);
  LITE_API  cl::Kernel& GetKernel(const std::string &name);
 private:
  std::vector<std::shared_ptr<cl::Kernel>> kernels_;
  std::map<std::string, int> kernel_offset_;
};

}// namespace opencl
}//namespace lite_api
}// namespace paddle
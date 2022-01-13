//
// Created by chenyaohuang on 2021/12/27.
//
#include "lite/backends/opencl/cl_runtime.h"
#include "paddle_opencl_api.h"
#include "lite/backends/opencl/cl_utility.h"
#include <utility>



namespace paddle{
namespace lite_api{
namespace opencl {
bool IsLiteOpenCLBackendValid(bool check_fp16_valid) {
#ifdef LITE_WITH_LOG
  LOG(INFO) << "need to check fp16 valid:" << check_fp16_valid;
#endif
  bool opencl_valid = false;

#ifdef LITE_WITH_OPENCL
  bool opencl_lib_found = paddle::lite::CLWrapper::Global()->OpenclLibFound();
#ifdef LITE_WITH_LOG
  LOG(INFO) << "Found opencl library:" << opencl_lib_found;
#endif
  if (opencl_lib_found == false) return false;

  bool dlsym_success = paddle::lite::CLWrapper::Global()->DlsymSuccess();
#ifdef LITE_WITH_LOG
  LOG(INFO) << "dlsym_success:" << dlsym_success;
#endif
  if (dlsym_success == false) return false;
  opencl_valid = paddle::lite::CLRuntime::Global()->OpenCLAvaliableForDevice(
      check_fp16_valid);

#ifdef LITE_WITH_LOG
  LOG(INFO) << "opencl_valid:" << opencl_valid;
#endif
#endif
  return opencl_valid;
}

int LiteCLGetGpuType() {
    if(IsLiteOpenCLBackendValid()==0)
      return -1;
  paddle::lite::CLRuntime::Global()->GetDeviceInfo();
  auto it = paddle::lite::CLRuntime::Global()->GetGpuType();
  int gpu_type = 0;
  switch (it) {
    case GpuType::UNKNOWN:
      LOG(INFO) << "gpu_type:UNKNOWN";
      gpu_type = 0;
      break;
    case GpuType::QUALCOMM_ADRENO:
      LOG(INFO) << "gpu_type:QUALCOMM_ADRENO";
      gpu_type = 1;
      break;
    case GpuType::IMAGINATION_POWERVR:
      LOG(INFO) << "gpu_type:IMAGINATION_POWERVR";
      gpu_type = 3;
      break;
    case GpuType::ARM_MALI:
      LOG(INFO) << "gpu_type:ARM_MALI";
      gpu_type = 2;
      break;
    case GpuType::OTHERS:
      LOG(INFO) << "gpu_type:OTHERS";
      gpu_type = 4;
      break;
    default:
      LOG(INFO) << "gpu_type:it is bug";
      gpu_type = -1;
      break;
  }
  return gpu_type;
}

bool IsOpenCLSupportHalf(){
    if(IsLiteOpenCLBackendValid()){
      return paddle::lite::CLRuntime::Global()->support_half();
    }
    else
      return false;
}

void LiteSetOpenclTune(CLTuneMode tune_mode,
                       const std::string &path,
                       const std::string &name,
                       size_t lws_repeats) {
#ifdef LITE_WITH_OPENCL
  if (IsLiteOpenCLBackendValid()) {
//    opencl_tune_mode_ = tune_mode;
    paddle::lite::CLRuntime::Global()->set_auto_tune(
        tune_mode, path, name, lws_repeats);
#ifdef LITE_WITH_LOG
    LOG(INFO) << "set opencl_tune_mode: "
              << CLTuneModeToStr(lite::CLRuntime::Global()->auto_tune())
              << ", lws_repeats:" << lws_repeats;
    LOG(INFO) << "tuned file path & name:" << path << "/" << name;
#endif
  }
#endif
}


int LiteSetPrecision(int p){
    if(IsLiteOpenCLBackendValid()==0)
        return -1;
  if(p==0) {
    paddle::lite::CLRuntime::Global()->set_precision(lite_api::CLPrecisionType::CL_PRECISION_AUTO);
#ifdef LITE_WITH_LOG
    LOG(INFO) << "set opencl precision : CL_PRECISION_AUTO";
#endif
  } else if(p==1){
    paddle::lite::CLRuntime::Global()->set_precision(lite_api::CLPrecisionType::CL_PRECISION_FP16);
#ifdef LITE_WITH_LOG
    LOG(INFO) << "set opencl precision : CL_PRECISION_FP16";
#endif
  }
  else if(p==2){
    paddle::lite::CLRuntime::Global()->set_precision(lite_api::CLPrecisionType::CL_PRECISION_FP32);
#ifdef LITE_WITH_LOG
    LOG(INFO) << "set opencl precision : CL_PRECISION_FP32";
#endif
  }
  else{
    return -1;
#ifdef LITE_WITH_LOG
    LOG(INFO) << "error: set opencl precision  only is  CL_PRECISION_AUTO：0  CL_PRECISION_FP16：1   CL_PRECISION_FP32：2";
#endif
  }
  return LiteGetPrecision();
}


int LiteGetPrecision(){
  lite_api::CLPrecisionType p_ = paddle::lite::CLRuntime::Global()->get_precision();//default lite_api::CL_PRECISION_AUTO
  if(p_==lite_api::CLPrecisionType::CL_PRECISION_AUTO) {
#ifdef LITE_WITH_LOG
    LOG(INFO) << "opencl precision : CL_PRECISION_AUTO";
#endif
    return 0;
  } else if(p_==lite_api::CLPrecisionType::CL_PRECISION_FP16) {
#ifdef LITE_WITH_LOG
    LOG(INFO) << "opencl precision : CL_PRECISION_FP16";
#endif
    return 1;
  } else if(p_==lite_api::CLPrecisionType::CL_PRECISION_FP32) {
#ifdef LITE_WITH_LOG
    LOG(INFO) << "opencl precision : CL_PRECISION_FP32";
#endif
    return 2;
  }
  else{
    return -1;
  }
}

cl::Context& GetContext(){
  return paddle::lite::CLRuntime::Global()->context();//return pointer is null by default
}

cl::CommandQueue& GetQueue(){
  return paddle::lite::CLRuntime::Global()->command_queue();//return pointer is null by default
}

std::unique_ptr<cl::Program> CreateProgram(
    const cl::Context& context, std::string file_name){
   if(IsLiteOpenCLBackendValid()){
      return paddle::lite::CLRuntime::Global()->CreateProgramFromSource(context,file_name);
   }
   else{
     return nullptr;
   }
}

cl::Kernel & CreateKernel(const std::string &kernel_name,
                                         const std::string &file_name,
                                         const std::string &options,
                                         const std::string &time_stamp){
  cl_int status{CL_SUCCESS};
#ifdef LITE_WITH_LOG
  VLOG(3) << " --- to get program " << file_name << " --- ";
#endif
  auto program = paddle::lite::CLRuntime::Global()->GetProgram(file_name, options);
#ifdef LITE_WITH_LOG
  VLOG(3) << " --- end get program --- ";
  VLOG(3) << " --- to create kernel: " << kernel_name << " --- ";
#endif
  std::shared_ptr<cl::Kernel> kernel(
      new cl::Kernel(program, kernel_name.c_str(), &status));

  {
    using namespace paddle::lite;
    CL_CHECK_FATAL(status);
  }

#ifdef LITE_WITH_LOG
  VLOG(3) << " --- end create kernel --- ";
#endif
  return *kernel;

}

} //opencl
} //namespace lite_api
} // namespace paddle
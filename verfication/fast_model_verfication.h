#ifndef MML_NATIVE_SRC_FAST_FILE_VERIFICATION_MML_FAST_MODEL_VERFICATION_H

#define MML_NATIVE_SRC_FAST_FILE_VERIFICATION_MML_FAST_MODEL_VERFICATION_H

#ifdef ARM64
#include <arm_acle.h>
#include <arm_neon.h>
#endif

#include <fstream>
#include <iostream>
namespace file_verfication {
#define KBYTES 1032
#define SEGMENTBYTES 256
static constexpr const uint32_t kCRC32Xor = static_cast<uint32_t>(0xffffffffU);
uint32_t fast_model_verfication_file(const char *file_path,const bool is_check_code = false);
uint32_t fast_model_verfication_buffer(const char *data,const size_t size,const bool is_check_code = false);
} // namespace file_verfication
#endif // MML_NATIVE_SRC_FAST_FILE_VERIFICATION_MML_FAST_MODEL_VERFICATION_H

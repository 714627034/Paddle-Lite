
#include "fast_model_verfication.h"

namespace file_verfication {

#ifdef ARM64
#define CRC32C32BYTES(P, IND)                                                  \
  do {                                                                         \
    crc1 = __crc32cd(                                                          \
        crc1, *((const uint64_t *)(P) + (SEGMENTBYTES / 8) * 1 + (IND)));      \
    crc2 = __crc32cd(                                                          \
        crc2, *((const uint64_t *)(P) + (SEGMENTBYTES / 8) * 2 + (IND)));      \
    crc3 = __crc32cd(                                                          \
        crc3, *((const uint64_t *)(P) + (SEGMENTBYTES / 8) * 3 + (IND)));      \
    crc0 = __crc32cd(                                                          \
        crc0, *((const uint64_t *)(P) + (SEGMENTBYTES / 8) * 0 + (IND)));      \
  } while (0);

// compute 8*8 bytes for each segment parallelly
#define CRC32C256BYTES(P, IND)                                                 \
  do {                                                                         \
    CRC32C32BYTES((P), (IND)*8 + 0)                                            \
    CRC32C32BYTES((P), (IND)*8 + 1)                                            \
    CRC32C32BYTES((P), (IND)*8 + 2)                                            \
    CRC32C32BYTES((P), (IND)*8 + 3)                                            \
    CRC32C32BYTES((P), (IND)*8 + 4)                                            \
    CRC32C32BYTES((P), (IND)*8 + 5)                                            \
    CRC32C32BYTES((P), (IND)*8 + 6)                                            \
    CRC32C32BYTES((P), (IND)*8 + 7)                                            \
  } while (0);

// compute 4*8*8 bytes for each segment parallelly
#define CRC32C1024BYTES(P)                                                     \
  do {                                                                         \
    CRC32C256BYTES((P), 0)                                                     \
    CRC32C256BYTES((P), 1)                                                     \
    CRC32C256BYTES((P), 2)                                                     \
    CRC32C256BYTES((P), 3)                                                     \
    (P) += 4 * SEGMENTBYTES;                                                   \
  } while (0)

uint32_t verification(uint32_t crc, const uint8_t *data, size_t size) {
    std::cout << "走在armv8" << std::endl;

  int64_t length = size;
  uint32_t crc0, crc1, crc2, crc3;
  uint64_t t0, t1, t2;
  int id = 0;
  // k0=CRC(x^(3*SEGMENTBYTES*8)), k1=CRC(x^(2*SEGMENTBYTES*8)),
  // k2=CRC(x^(SEGMENTBYTES*8))
  const poly64_t k0 = 0x8d96551c, k1 = 0xbd6f81f8, k2 = 0xdcb17aa4;

  crc = crc ^ kCRC32Xor;
  while (length >= KBYTES) {
    crc0 = crc;
    crc1 = 0;
    crc2 = 0;
    crc3 = 0;
    // Process 1024 bytes in parallel.
    CRC32C1024BYTES(data);
    // Merge the 4 partial CRC32C values.
    t2 = (uint64_t)vmull_p64(crc2, k2);
    t1 = (uint64_t)vmull_p64(crc1, k1);
    t0 = (uint64_t)vmull_p64(crc0, k0);
    crc = __crc32cd(crc3, *(uint64_t *)data);
    data += sizeof(uint64_t);
    crc ^= __crc32cd(0, t2);
    crc ^= __crc32cd(0, t1);
    crc ^= __crc32cd(0, t0);

    length -= KBYTES;
  }

  while (length >= 8) {
    crc = __crc32cd(crc, *(uint64_t *)data);
    data += 8;
    length -= 8;
  }
  if (length & 4) {
    crc = __crc32cw(crc, *(uint32_t *)data);
    data += 4;
  }
  if (length & 2) {
    crc = __crc32ch(crc, *(uint16_t *)data);
    data += 2;
  }
  if (length & 1) {
    crc = __crc32cb(crc, *data);
  }
  return crc ^ kCRC32Xor;
}
#else
static const uint32_t crc32c_table[256] = {
    0x00000000L, 0xF26B8303L, 0xE13B70F7L, 0x1350F3F4L, 0xC79A971FL,
    0x35F1141CL, 0x26A1E7E8L, 0xD4CA64EBL, 0x8AD958CFL, 0x78B2DBCCL,
    0x6BE22838L, 0x9989AB3BL, 0x4D43CFD0L, 0xBF284CD3L, 0xAC78BF27L,
    0x5E133C24L, 0x105EC76FL, 0xE235446CL, 0xF165B798L, 0x030E349BL,
    0xD7C45070L, 0x25AFD373L, 0x36FF2087L, 0xC494A384L, 0x9A879FA0L,
    0x68EC1CA3L, 0x7BBCEF57L, 0x89D76C54L, 0x5D1D08BFL, 0xAF768BBCL,
    0xBC267848L, 0x4E4DFB4BL, 0x20BD8EDEL, 0xD2D60DDDL, 0xC186FE29L,
    0x33ED7D2AL, 0xE72719C1L, 0x154C9AC2L, 0x061C6936L, 0xF477EA35L,
    0xAA64D611L, 0x580F5512L, 0x4B5FA6E6L, 0xB93425E5L, 0x6DFE410EL,
    0x9F95C20DL, 0x8CC531F9L, 0x7EAEB2FAL, 0x30E349B1L, 0xC288CAB2L,
    0xD1D83946L, 0x23B3BA45L, 0xF779DEAEL, 0x05125DADL, 0x1642AE59L,
    0xE4292D5AL, 0xBA3A117EL, 0x4851927DL, 0x5B016189L, 0xA96AE28AL,
    0x7DA08661L, 0x8FCB0562L, 0x9C9BF696L, 0x6EF07595L, 0x417B1DBCL,
    0xB3109EBFL, 0xA0406D4BL, 0x522BEE48L, 0x86E18AA3L, 0x748A09A0L,
    0x67DAFA54L, 0x95B17957L, 0xCBA24573L, 0x39C9C670L, 0x2A993584L,
    0xD8F2B687L, 0x0C38D26CL, 0xFE53516FL, 0xED03A29BL, 0x1F682198L,
    0x5125DAD3L, 0xA34E59D0L, 0xB01EAA24L, 0x42752927L, 0x96BF4DCCL,
    0x64D4CECFL, 0x77843D3BL, 0x85EFBE38L, 0xDBFC821CL, 0x2997011FL,
    0x3AC7F2EBL, 0xC8AC71E8L, 0x1C661503L, 0xEE0D9600L, 0xFD5D65F4L,
    0x0F36E6F7L, 0x61C69362L, 0x93AD1061L, 0x80FDE395L, 0x72966096L,
    0xA65C047DL, 0x5437877EL, 0x4767748AL, 0xB50CF789L, 0xEB1FCBADL,
    0x197448AEL, 0x0A24BB5AL, 0xF84F3859L, 0x2C855CB2L, 0xDEEEDFB1L,
    0xCDBE2C45L, 0x3FD5AF46L, 0x7198540DL, 0x83F3D70EL, 0x90A324FAL,
    0x62C8A7F9L, 0xB602C312L, 0x44694011L, 0x5739B3E5L, 0xA55230E6L,
    0xFB410CC2L, 0x092A8FC1L, 0x1A7A7C35L, 0xE811FF36L, 0x3CDB9BDDL,
    0xCEB018DEL, 0xDDE0EB2AL, 0x2F8B6829L, 0x82F63B78L, 0x709DB87BL,
    0x63CD4B8FL, 0x91A6C88CL, 0x456CAC67L, 0xB7072F64L, 0xA457DC90L,
    0x563C5F93L, 0x082F63B7L, 0xFA44E0B4L, 0xE9141340L, 0x1B7F9043L,
    0xCFB5F4A8L, 0x3DDE77ABL, 0x2E8E845FL, 0xDCE5075CL, 0x92A8FC17L,
    0x60C37F14L, 0x73938CE0L, 0x81F80FE3L, 0x55326B08L, 0xA759E80BL,
    0xB4091BFFL, 0x466298FCL, 0x1871A4D8L, 0xEA1A27DBL, 0xF94AD42FL,
    0x0B21572CL, 0xDFEB33C7L, 0x2D80B0C4L, 0x3ED04330L, 0xCCBBC033L,
    0xA24BB5A6L, 0x502036A5L, 0x4370C551L, 0xB11B4652L, 0x65D122B9L,
    0x97BAA1BAL, 0x84EA524EL, 0x7681D14DL, 0x2892ED69L, 0xDAF96E6AL,
    0xC9A99D9EL, 0x3BC21E9DL, 0xEF087A76L, 0x1D63F975L, 0x0E330A81L,
    0xFC588982L, 0xB21572C9L, 0x407EF1CAL, 0x532E023EL, 0xA145813DL,
    0x758FE5D6L, 0x87E466D5L, 0x94B49521L, 0x66DF1622L, 0x38CC2A06L,
    0xCAA7A905L, 0xD9F75AF1L, 0x2B9CD9F2L, 0xFF56BD19L, 0x0D3D3E1AL,
    0x1E6DCDEEL, 0xEC064EEDL, 0xC38D26C4L, 0x31E6A5C7L, 0x22B65633L,
    0xD0DDD530L, 0x0417B1DBL, 0xF67C32D8L, 0xE52CC12CL, 0x1747422FL,
    0x49547E0BL, 0xBB3FFD08L, 0xA86F0EFCL, 0x5A048DFFL, 0x8ECEE914L,
    0x7CA56A17L, 0x6FF599E3L, 0x9D9E1AE0L, 0xD3D3E1ABL, 0x21B862A8L,
    0x32E8915CL, 0xC083125FL, 0x144976B4L, 0xE622F5B7L, 0xF5720643L,
    0x07198540L, 0x590AB964L, 0xAB613A67L, 0xB831C993L, 0x4A5A4A90L,
    0x9E902E7BL, 0x6CFBAD78L, 0x7FAB5E8CL, 0x8DC0DD8FL, 0xE330A81AL,
    0x115B2B19L, 0x020BD8EDL, 0xF0605BEEL, 0x24AA3F05L, 0xD6C1BC06L,
    0xC5914FF2L, 0x37FACCF1L, 0x69E9F0D5L, 0x9B8273D6L, 0x88D28022L,
    0x7AB90321L, 0xAE7367CAL, 0x5C18E4C9L, 0x4F48173DL, 0xBD23943EL,
    0xF36E6F75L, 0x0105EC76L, 0x12551F82L, 0xE03E9C81L, 0x34F4F86AL,
    0xC69F7B69L, 0xD5CF889DL, 0x27A40B9EL, 0x79B737BAL, 0x8BDCB4B9L,
    0x988C474DL, 0x6AE7C44EL, 0xBE2DA0A5L, 0x4C4623A6L, 0x5F16D052L,
    0xAD7D5351L};
uint32_t verification(uint32_t crc, const uint8_t *data, size_t size) {
  std::cout << "走在armv7" << std::endl;
  unsigned int length = size;
  crc = crc ^ kCRC32Xor;
  while (length--) {
    //        std::cout<<((crc ^ *data) & 0xFFL)<<std::endl;
    crc = crc32c_table[(crc ^ *data++) & 0xFFL] ^ (crc >> 8);
  }

  return crc ^ kCRC32Xor;
}

#endif

uint32_t fast_model_verfication_file(const char *file,const bool is_check_code) {
  std::cout << "is_check_code: " <<is_check_code<< std::endl;
  char sign[8]="cyh*mml";
  bool is_crc= true;
  std::ifstream in(file, std::ios::binary);
  if (!in) {
    std::cout << "file open error" << std::endl;
    return 0;
  }
  in.seekg(0, std::ios::end);
  size_t size = in.tellg();
  uint32_t da;
  if(is_check_code == true){
    if (size < sizeof(sign) * 2 + sizeof(uint32_t)) {
      std::cout << "文件长度过短，文件不存在校验码" << std::endl;
      return -1;
    }
    in.seekg(-(sizeof(sign) * 2 + sizeof(uint32_t)), std::ios::cur);
    char *mml_st;
    mml_st = new char[10];
    in.read(mml_st, sizeof(sign));
    std::cout << "mml_st: " << mml_st << std::endl;

    in.read((char *)&da, sizeof(uint32_t));
    std::cout << "da: " << da << std::endl;

    char *mml_en;
    mml_en = new char[10];
    in.read(mml_en, sizeof(sign));
    std::cout << "mml_en: " << mml_en << std::endl;
    for (int i = 0; i < 7; i++) {
      if (mml_en[i] != sign[i] || mml_st[i] != sign[i]) {
        is_crc = false;
        break;
      }
    }
    delete[] mml_en;
    delete[] mml_st;
    mml_en = nullptr;
    mml_st = nullptr;
    if(is_crc) {
      std::cout<<"存在校验码:"<<std::hex<<da<<std::endl;
      size-=(sizeof(sign)*2+sizeof(uint32_t));
    }
    else{
      std::cout<<"文件不存在校验码,或校验码已损坏"<<std::endl;
      return -1;
    }
  }

  in.seekg(0, std::ios::beg);
  uint8_t *data = nullptr;
  data = new uint8_t[size];
  in.read((char *)data, size);
//  std::cout << "file open size: " << size<<std::endl;
  uint32_t crc = 0;
  crc = verification(crc, data, size);
  delete[] data;
  data = nullptr;
  in.close();

  if(!is_check_code){
    std::cout << "文件末尾写入校验码" <<std::hex<<crc<<std::endl;
    std::ofstream out(file, std::ios::binary | std::ios::app);
    if (!out) {
      std::cout << "file open error" << std::endl;
      return 0;
    }
    out.write(sign, sizeof(sign));
    out.write((char *)(&crc), sizeof(uint32_t));
    out.write(sign, sizeof(sign));
    out.close();
    return crc;
  }
  else {
    if(da==crc) {
      std::cout << "校验成功" << std::endl;
      return crc;
    }
    else{
      std::cout << "校验失败" << std::endl;
      return -1;
    }
  }

  //  std::ifstream in_(file, std::ios::binary);
//  if (!in_) {
//    std::cout << "file open error" << std::endl;
//    return 0;
//  }
//  in_.seekg(0, std::ios::end);
//  size_t size_ = in_.tellg();
//  size_t s=(size_-size);
//  std::cout << "cyh :file open size: " <<(size_-size)<<std::endl;
//  in_.seekg(-s, std::ios::cur);
//  char *mml;
//  mml=new char[10];
//  in_.read(mml, sizeof(sign));
//  std::cout << "mml: " << mml<<std::endl;
//  delete[] mml;
//  mml= nullptr;
//
//  uint32_t  da;
//  in_.read((char *)&da,sizeof(uint32_t));
//  std::cout << "da: " << da<<std::endl;
//
//  mml=new char[10];
//  in_.read(mml, sizeof(sign));
//  std::cout << "mml: " << mml<<std::endl;
//  in_.close();
//  return crc;
}

uint32_t fast_model_verfication_buffer(const char *data,const size_t size,const bool is_check_code) {

  char sign[8]="cyh*mml";
  bool is_crc= true;
  uint32_t da;
  size_t size_ = size;
  if(is_check_code== true){
    if (size < sizeof(sign) * 2 + sizeof(uint32_t)) {
      std::cout << "文件长度过短，文件不存在校验码" << std::endl;
      return -1;
    }
    char *mml_st;

    mml_st = (char *)(data + (size - (sizeof(sign) * 2 + sizeof(uint32_t))));

    da = *((uint32_t *)(data + (size - (sizeof(sign) + sizeof(uint32_t)))));

    char *mml_en;
    mml_en = (char *)(data + (size - sizeof(sign)));
    for (int i = 0; i < 7; i++) {
      if (*(mml_en + i) != sign[i] || *(mml_st + i) != sign[i]) {
        is_crc = false;
        break;
      }
    }
    if (is_crc) {
      std::cout << "存在校验码:" << std::hex << da << std::endl;
      size_ -= (sizeof(sign) * 2 + sizeof(uint32_t));
    } else {
      std::cout << "文件不存在校验码,或校验码已损坏" << std::endl;
      return -1;
    }
  }
  else{
    std::cout << "fast_model_verfication_buffer() Only check check codes are supported, and generation and writing of check codes are not supported. Please use fast_model_verfication_file()" << std::endl;
    return -1;
  }
  uint32_t crc = 0;
  std::cout << "file open size: " << size_<<std::endl;
  crc = verification(crc, (uint8_t*)data, size_);
  if(da==crc) {
    std::cout << "校验成功" << std::endl;
    return crc;
  }
  else{
    std::cout << "校验失败" << std::endl;
    return -1;
  }
}

} // namespace file_verfication
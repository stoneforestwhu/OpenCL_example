#ifndef __UTIL_H__
#define __UTIL_H__

#include <assert.h>
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <io.h>
#include <CL/cl.h>
#include <type_traits>


using std::ifstream;
using std::fstream;
using std::ios;
using std::ostringstream;
using std::string;
using std::vector;
using std::shared_ptr;

#define GLenum enum
#define uint64_t unsigned long long

bool exist_opengl32() { return _access("opengl32.dll", 0) != -1; }

char *getHwStr() { 
  if (_access("opengl32.dll", 0) != -1)
    return "nv";
  else
    return "wx";
}

typedef struct __trace_img_header // 8 Byte align
{
  unsigned int magicNum;
  GLenum format;
  GLenum type;
  GLenum target;
  uint64_t width;
  uint64_t height;
  uint64_t depth;
  uint64_t imageSize;
} __trace_img_header;

enum DataType {
  floatType = 0,
  integerType = 1,
};

std::shared_ptr<char> readKernelFile(std::string strSource) {
  std::ifstream myfile(strSource);
  if (myfile.fail()) {
    std::cout << "Can not open it " << std::endl;
    // throw new std::runtime_error("IO stream corrupted");
  }
  std::string shaderStr((std::istreambuf_iterator<char>(myfile)),
                    std::istreambuf_iterator<char>());
  myfile.close();
  size_t len = shaderStr.length();
  std::shared_ptr<char> shaderPtr(new char[len + 1]);
  strcpy_s(shaderPtr.get(), len + 1, shaderStr.c_str());
  std::cout << shaderStr << std::endl;
  return shaderPtr;
}

template<typename T>
void writeDataToFile(void *data, string fileName, size_t dataSize) {
  size_t elementSize = dataSize/4;  //  float or int always contains 4 bytes
  size_t row = elementSize / 4; 
  size_t coloum = 4;
  size_t extraRow = 0;
  size_t extraColumn = (elementSize) % 4;
  if (dataSize % 16 != 0)
    extraRow = 1;

  T* dataToWrite = (T*)data;
  string fmtStr = "%d";
  if (std::is_same<T, float>::value)
    fmtStr = "%f";
  std::fstream outFile(fileName, ios::out);
  for (unsigned int i = 0; i < row; i++) {
    for (unsigned int j = 0; j < coloum; j++) {
      T aVal = dataToWrite[i * coloum + j];
      char aStr[512];
      sprintf_s(aStr, fmtStr.c_str(), aVal);
      outFile << aStr << " ";
    }
    outFile << "\n";
  }

  if (extraRow)  // data may not always contain times of 4 int.
    for (unsigned int i = 0; i < extraColumn; ++i) {
      T aVal = dataToWrite[row * coloum + i];
      char aStr[512];
      sprintf_s(aStr, fmtStr.c_str(), aVal);
      outFile << aStr << " ";
    }
  outFile << "\n";
  outFile << "==========================\n";
  outFile.close();
}

void writeOriginBufDataToFile(std::string bufFileName, size_t bufSize) {
  shared_ptr<char> bufPtr(new char[bufSize]);
  fstream bufFile(bufFileName, ios::in | ios::binary);
  if (!bufFile) {
    std::cout << "can't find " << bufFileName << " \n";
    assert(0);
  }
  bufFile.read(bufPtr.get(), bufSize);
  string bufFpTxtName = bufFileName;
  bufFpTxtName.replace(bufFpTxtName.rfind(".bin"), 4, "_fp.txt");
  string bufIntTxtName = bufFileName;
  bufIntTxtName.replace(bufIntTxtName.rfind(".bin"), 4, "_int.txt");
  writeDataToFile<float>(bufPtr.get(), bufFpTxtName, bufSize);
  writeDataToFile<int>(bufPtr.get(), bufIntTxtName, bufSize);
}

template <typename T>
void writeResultBufDataToFile(T *data, unsigned int data_size) {
  static int counter = 0;
  char *log_subfix = getHwStr();
  char result_file_name[512];
  sprintf_s(result_file_name, "cmp/int_result_%s_%d.log", log_subfix, counter);
  writeDataToFile<T>(data, result_file_name, data_size);
  counter++;
}

std::shared_ptr<char> readBufferFile(std::string bufFileName, size_t bufSize) {
  std::shared_ptr<char> bufPtr(new char[bufSize + 1]);
  std::fstream bufFile(bufFileName, ios::in | ios::binary);
  if (!bufFile) {
    std::cout << "can't find " << bufFileName << " \n";
    assert(0);
  }
  bufFile.read(bufPtr.get(), bufSize);
  return bufPtr;
}

std::shared_ptr<char> readTexFile(std::string texFileName, size_t texSize) {
  __trace_img_header a_header;
  unsigned int header_size = sizeof(a_header);
  std::shared_ptr<char> bufPtr(new char[texSize + 1]);
  std::fstream bufFile(texFileName, ios::in | ios::binary);
  if (!bufFile) {
    std::cout << "can't find " << texFileName << " \n";
    assert(0);
  }
  bufFile.seekg(header_size, ios::beg); //  set offset
  bufFile.read(bufPtr.get(), texSize);
  return bufPtr;
}

template<typename T>
void printData(T *data, unsigned int row, unsigned int coloum) {
  for (unsigned int i = 0; i < row; i++) {
    for (unsigned int j = 0; j < coloum; j++) {
      std::cout << data[i * coloum + j] << " ";
    }
    std::cout << "\n";
  }
}



#endif
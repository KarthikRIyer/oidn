// ======================================================================== //
// Copyright 2009-2018 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "tensor.h"

namespace oidn {

  std::map<std::string, Tensor> parse_tensors(void* buffer)
  {
    char* input = (char*)buffer;

    // Parse the magic value
    const int magic = *(unsigned short*)input;
    if (magic != 0x41D7)
      throw std::runtime_error("invalid tensor archive");
    input += sizeof(unsigned short);

    // Parse the version
    const int major_version = *(unsigned char*)input++;
    const int minor_version = *(unsigned char*)input++;
    UNUSED(minor_version);
    if (major_version > 1)
      throw std::runtime_error("unsupported tensor archive version");

    // Parse the number of tensors
    const int num_tensors = *(int*)input;
    input += sizeof(int);

    // Parse the tensors
    std::map<std::string, Tensor> tensor_map;
    for (int i = 0; i < num_tensors; ++i)
    {
      Tensor tensor;

      // Parse the name
      const int name_len = *(unsigned char*)input++;
      std::string name(input, name_len);
      input += name_len;

      // Parse the number of dimensions
      const int ndims = *(unsigned char*)input++;

      // Parse the shape of the tensor
      tensor.dims = std::vector<int>((int*)input, (int*)input + ndims);
      input += ndims * sizeof(int);

      // Parse the format of the tensor
      tensor.format = std::string(input, input + ndims);
      input += ndims;

      // Parse the data type of the tensor
      const char type = *(unsigned char*)input++;
      if (type != 'f') // only float32 is supported
        throw std::runtime_error("unsupported tensor data type");

      // Skip the data
      tensor.data = (float*)input;
      input += tensor.size() * sizeof(float);

      // Add the tensor to the map
      tensor_map.emplace(name, std::move(tensor));
    }

    return tensor_map;
  }

} // ::oidn
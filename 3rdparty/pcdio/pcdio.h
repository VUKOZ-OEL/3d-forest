/*******************************************************************************
 * Copyright (C) 2022 MINIEYE L4 Department. All Rights Reserved
 *
 ******************************************************************************
 */
/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2010-2011, Willow Garage, Inc.
 *  Copyright (c) 2012-, Open Perception, Inc.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the copyright holder(s) nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id$
 *
 */
/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.

    Modifications to the original BSD-licensed code:
    - Fixed compiler and linker errors.
    - Fixed compiler warnings.
    - Removed BOOL field from PCDReader::read().
    - Removed unused log levels L_ALWAYS and L_VERBOSE.
    - Added log handler.
    - Added PointCloud.
    - Added PointXYZI.
*/

#pragma once

#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <exception>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <cmath>
#include <iterator>
#include <array>
#include <unordered_map>

#include <stdint.h>
#include <climits>
#include <cstdarg>  // for va_list, va_start, va_end
#include <stdio.h>
#include <cstring>
#include <fcntl.h>

#if defined(_WIN32)
    #include <share.h>
    #include <sys/stat.h>
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <io.h>
#include <windows.h>
#ifdef _MSC_VER
// ssize_t is already defined in MinGW and its definition conflicts with that of
// SSIZE_T on a 32-bit target, so do this only for MSVC.
#include <basetsd.h>
using ssize_t = SSIZE_T;
#endif /* _MSC_VER */
#else
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#endif
#include <cstddef>

namespace pcl {
/*******************************************************************************
 * Port
 *******************************************************************************
 */
using std::shared_ptr;
using uindex_t = uint64_t;
using index_t = int64_t;

struct Vector4f : public std::array<float, 4> {
  Vector4f() = default;
  Vector4f(float v0, float v1, float v2, float v3) {
    at(0) = v0;
    at(1) = v1;
    at(2) = v2;
    at(3) = v3;
  }

  static Vector4f Zero() { return Vector4f(0, 0, 0, 0); }
};

class Quaternionf {
 private:
  float x_;
  float y_;
  float z_;
  float w_;

 public:
  Quaternionf() = default;
  Quaternionf(float ix, float iy, float iz, float iw)
      : x_(ix), y_(iy), z_(iz), w_(iw){};

  float x() const { return x_; }
  float y() const { return y_; }
  float z() const { return z_; }
  float w() const { return w_; }

  static Quaternionf Identity() { return Quaternionf(0, 0, 0, 1); }
};

inline bool IsBigEndian() {
  uint32_t var = 0x01020304;
  return *(reinterpret_cast<uint8_t *>(&var)) == 0x01;
}

// replace boost::iequals
inline bool iequals(const std::string &str1, const std::string &str2) {
  if (str1.size() != str2.size()) {
    return false;
  }

  for (size_t i = 0; i < str1.size(); ++i) {
    if (std::tolower(str1[i]) != std::tolower(str2[i])) {
      return false;
    }
  }

  return true;
}

// Helper function to check if a character is a whitespace character
inline bool isWhitespace(char ch) {
  return std::isspace(static_cast<unsigned char>(ch));
}

// Trim from start (in place)
inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  [](char ch) { return !isWhitespace(ch); }));
}

// Trim from end (in place)
inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](char ch) { return !isWhitespace(ch); })
              .base(),
          s.end());
}

// Trim from both ends (in place)  replace boost::trim
inline void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

/*******************************************************************************
 * Log
 *******************************************************************************
 */
enum VERBOSITY_LEVEL { L_ERROR, L_WARN, L_INFO, L_DEBUG };

typedef void (*LogMessageHandler)(VERBOSITY_LEVEL level,
                                  const std::string &message);

inline LogMessageHandler logMessageHandler = nullptr;

inline void consoleLogMessageHandler(VERBOSITY_LEVEL level,
                                     const std::string &message)
{
  FILE *stream = (level == L_WARN || level == L_ERROR) ? stderr : stdout;

  switch (level) {
    case L_DEBUG:
      fprintf(stream, "%s", "\033[0;32m");  // reset, green
      break;
    case L_WARN:
      fprintf(stream, "%s", "\033[1;33m");  // highlight, yellow
      break;
    case L_ERROR:
      fprintf(stream, "%s", "\033[1;31m");  // highlight, red
      break;
    case L_INFO:
    default:
      break;
  }

  fprintf(stream, "%s", message.c_str());

  fprintf(stream, "%s", "\033[0m");  // reset
}

inline void console_print(VERBOSITY_LEVEL level, const char *format, ...)
{
  if (!logMessageHandler)
  {
    return;
  }

  va_list args;

  va_start(args, format);

  va_list args_copy;
  va_copy(args_copy, args);
  int result = vsnprintf(nullptr, 0, format, args_copy);
  va_end(args_copy);

  if (result <= 0)
  {
    va_end(args);
    return;
  }

  size_t size = static_cast<size_t>(result);
  std::vector<char> buffer(size + 1);
  vsnprintf(buffer.data(), buffer.size(), format, args);

  va_end(args);

  logMessageHandler(level, std::string(buffer.data(), size));
}

#define PCL_ERROR(...) console_print(VERBOSITY_LEVEL::L_ERROR, __VA_ARGS__)
#define PCL_WARN(...) console_print(VERBOSITY_LEVEL::L_WARN, __VA_ARGS__)
#define PCL_INFO(...) console_print(VERBOSITY_LEVEL::L_INFO, __VA_ARGS__)
#define PCL_DEBUG(...) console_print(VERBOSITY_LEVEL::L_DEBUG, __VA_ARGS__)


/*******************************************************************************
 * Common & Utils
 *******************************************************************************
 */
/** \brief Lightweight tokenization function
 * This function can be used as a boost::split substitute. When benchmarked
 * against boost, this function will create much less allocations and hence it
 * is much better suited for quick line tokenization.
 *
 * Cool thing is this function will work with SequenceSequenceT =
 * std::vector<std::string> and std::vector<std::string_view>
 */
template <typename SequenceSequenceT>
void split(SequenceSequenceT &result, std::string const &in,
           const char *const delimiters) {
  using StringSizeT = std::string::size_type;

  const auto len = in.length();
  StringSizeT token_start = 0;

  result.clear();
  while (token_start < len) {
    // eat leading whitespace
    token_start = in.find_first_not_of(delimiters, token_start);
    if (token_start == std::string::npos) {
      return;  // nothing left but white space
    }

    // find the end of the token
    const auto token_end = in.find_first_of(delimiters, token_start);

    // push token
    if (token_end == std::string::npos) {
      result.emplace_back(in.data() + token_start, len - token_start);
      return;
    } else {
      result.emplace_back(in.data() + token_start, token_end - token_start);
    }

    // set up for next loop
    token_start = token_end + 1;
  }
}

namespace console {
class TicToc {
 public:
  TicToc() = default;
  void tic() { tictic_ = std::chrono::steady_clock::now(); };

  inline double toc() const {
    auto end_time = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::ratio<1, 1000>>(end_time -
                                                              tictic_)
        .count();
  };

  inline void toc_print() const {
    double milliseconds = toc();
    // int minutes = (int) std::floor ( seconds / 60.0 );
    // seconds -= minutes * 60.0;
    // if (minutes != 0)
    //{
    //  print_value ("%i", minutes);
    //  print_info (" minutes, ");
    //}

    // @brief modified by nick.liao
    PCL_INFO("%g ms\n", milliseconds);
    // print_value("%g", milliseconds);
    // print_info(" ms\n");
  };

 private:
  std::chrono::time_point<std::chrono::steady_clock> tictic_;
};
}  // namespace console

/*******************************************************************************
 * PCLHeader
 *******************************************************************************
 */
struct PCLHeader {
  /** \brief Sequence number */
  std::uint32_t seq = 0;
  /** \brief A timestamp associated with the time when the data was acquired
   *
   * The value represents microseconds since 1970-01-01 00:00:00 (the UNIX
   * epoch).
   */
  std::uint64_t stamp = 0;
  /** \brief Coordinate frame ID */
  std::string frame_id;

  using Ptr = shared_ptr<PCLHeader>;
  using ConstPtr = shared_ptr<const PCLHeader>;
};  // struct PCLHeader

using HeaderPtr = PCLHeader::Ptr;
using HeaderConstPtr = PCLHeader::ConstPtr;

inline std::ostream &operator<<(std::ostream &out, const PCLHeader &h) {
  out << "seq: " << h.seq;
  out << " stamp: " << h.stamp;
  out << " frame_id: " << h.frame_id << std::endl;
  return (out);
}

inline bool operator==(const PCLHeader &lhs, const PCLHeader &rhs) {
  return (&lhs == &rhs) || (lhs.seq == rhs.seq && lhs.stamp == rhs.stamp &&
                            lhs.frame_id == rhs.frame_id);
}

/*******************************************************************************
 * PCLPointField
 *******************************************************************************
 */
struct PCLPointField {
  std::string name;

  uindex_t offset = 0;
  std::uint8_t datatype = 0;
  uindex_t count = 0;

  // @brief modified by nick.liao
  enum PointFieldTypes {
    BOOL = 11,
    INT8 = 1,
    UINT8 = 2,
    INT16 = 3,
    UINT16 = 4,
    INT32 = 5,
    UINT32 = 6,
    INT64 = 9,
    UINT64 = 10,
    FLOAT32 = 7,
    FLOAT64 = 8
  };
  // enum PointFieldTypes {
  //   BOOL = traits::asEnum_v<bool>,
  //   INT8 = traits::asEnum_v<std::int8_t>,
  //   UINT8 = traits::asEnum_v<std::uint8_t>,
  //   INT16 = traits::asEnum_v<std::int16_t>,
  //   UINT16 = traits::asEnum_v<std::uint16_t>,
  //   INT32 = traits::asEnum_v<std::int32_t>,
  //   UINT32 = traits::asEnum_v<std::uint32_t>,
  //   INT64 = traits::asEnum_v<std::int64_t>,
  //   UINT64 = traits::asEnum_v<std::uint64_t>,
  //   FLOAT32 = traits::asEnum_v<float>,
  //   FLOAT64 = traits::asEnum_v<double>
  // };

 public:
  using Ptr = shared_ptr<::pcl::PCLPointField>;
  using ConstPtr = shared_ptr<const ::pcl::PCLPointField>;
};  // struct PCLPointField

using PCLPointFieldPtr = PCLPointField::Ptr;
using PCLPointFieldConstPtr = PCLPointField::ConstPtr;

inline std::ostream &operator<<(std::ostream &s,
                                const ::pcl::PCLPointField &v) {
  s << "name: ";
  s << "  " << v.name << std::endl;
  s << "offset: ";
  s << "  " << v.offset << std::endl;
  s << "datatype: ";
  switch (v.datatype) {
    case ::pcl::PCLPointField::PointFieldTypes::BOOL:
      s << "  BOOL" << std::endl;
      break;
    case ::pcl::PCLPointField::PointFieldTypes::INT8:
      s << "  INT8" << std::endl;
      break;
    case ::pcl::PCLPointField::PointFieldTypes::UINT8:
      s << "  UINT8" << std::endl;
      break;
    case ::pcl::PCLPointField::PointFieldTypes::INT16:
      s << "  INT16" << std::endl;
      break;
    case ::pcl::PCLPointField::PointFieldTypes::UINT16:
      s << "  UINT16" << std::endl;
      break;
    case ::pcl::PCLPointField::PointFieldTypes::INT32:
      s << "  INT32" << std::endl;
      break;
    case ::pcl::PCLPointField::PointFieldTypes::UINT32:
      s << "  UINT32" << std::endl;
      break;
    case ::pcl::PCLPointField::PointFieldTypes::INT64:
      s << "  INT64" << std::endl;
      break;
    case ::pcl::PCLPointField::PointFieldTypes::UINT64:
      s << "  UINT64" << std::endl;
      break;
    case ::pcl::PCLPointField::PointFieldTypes::FLOAT32:
      s << "  FLOAT32" << std::endl;
      break;
    case ::pcl::PCLPointField::PointFieldTypes::FLOAT64:
      s << "  FLOAT64" << std::endl;
      break;
    default:
      s << "  " << static_cast<int>(v.datatype) << std::endl;
  }
  s << "count: ";
  s << "  " << v.count << std::endl;
  return (s);
}

// Return true if the PCLPointField matches the expected name and data type.
// Written as a struct to allow partially specializing on Tag.
// template <typename PointT, typename Tag>
// struct FieldMatches {
//   bool operator()(const PCLPointField &field) {
//     return ((field.name == traits::name<PointT, Tag>::value) &&
//             (field.datatype == traits::datatype<PointT, Tag>::value) &&
//             ((field.count == traits::datatype<PointT, Tag>::size) ||
//              (field.count == 0 &&
//               traits::datatype<PointT, Tag>::size == 1 /* see bug #821 */)));
//   }
// };

/*******************************************************************************
 * getFileds
 *******************************************************************************
 */

/**
 * \brief Obtains the size of a specific field data type in bytes
 * \param[in] datatype the field data type (see PCLPointField.h)
 * \ingroup common
 */
inline uindex_t getFieldSize(const int datatype) {
  switch (datatype) {
    case PCLPointField::BOOL:
      return static_cast<uindex_t>(sizeof(bool));

    case PCLPointField::INT8:
    case PCLPointField::UINT8:
      return (1);

    case PCLPointField::INT16:
    case PCLPointField::UINT16:
      return (2);

    case PCLPointField::INT32:
    case PCLPointField::UINT32:
    case PCLPointField::FLOAT32:
      return (4);

    case PCLPointField::INT64:
    case PCLPointField::UINT64:
    case PCLPointField::FLOAT64:
      return (8);

    default:
      return (0);
  }
}

/**
 * \brief Obtains the type of the PCLPointField from a specific size and type
 * \param[in] size the size in bytes of the data field
 * \param[in] type a char describing the type of the field  ('B' = bool, 'F' =
 * float, 'I' = signed, 'U' = unsigned)
 * \ingroup common
 */
inline int getFieldType(unsigned int size, int type) {
  type = std::toupper(type); //, std::locale::classic());

  // extra logic for bool because its size is undefined
  if (type == 'B') {
    if (size == sizeof(bool)) {
      return PCLPointField::BOOL;
    } else {
      return -1;
    }
  }

  switch (size) {
    case 1:
      if (type == 'I') return (PCLPointField::INT8);
      if (type == 'U') return (PCLPointField::UINT8);
      break;

    case 2:
      if (type == 'I') return (PCLPointField::INT16);
      if (type == 'U') return (PCLPointField::UINT16);
      break;

    case 4:
      if (type == 'I') return (PCLPointField::INT32);
      if (type == 'U') return (PCLPointField::UINT32);
      if (type == 'F') return (PCLPointField::FLOAT32);
      break;

    case 8:
      if (type == 'I') return (PCLPointField::INT64);
      if (type == 'U') return (PCLPointField::UINT64);
      if (type == 'F') return (PCLPointField::FLOAT64);
      break;

    default:
      PCL_ERROR("[pcl::getFieldType] Unknown field type %d size %d.\n",
                type, size);
      return -1;
      break;
  }
  return (-1);
}

/** \brief Obtains the type of the PCLPointField from a specific PCLPointField
 * as a char \param[in] type the PCLPointField field type \ingroup common
 */
inline char getFieldType(const int type) {
  switch (type) {
    case pcl::PCLPointField::BOOL:
      return ('B');

    case pcl::PCLPointField::INT8:
    case pcl::PCLPointField::INT16:
    case pcl::PCLPointField::INT32:
    case pcl::PCLPointField::INT64:
      return ('I');

    case pcl::PCLPointField::UINT8:
    case pcl::PCLPointField::UINT16:
    case pcl::PCLPointField::UINT32:
    case pcl::PCLPointField::UINT64:
      return ('U');

    case pcl::PCLPointField::FLOAT32:
    case pcl::PCLPointField::FLOAT64:
      return ('F');

    default:
      return ('?');
  }
}

/*******************************************************************************
 * PCLPointCloud2
 *******************************************************************************
 */
struct PCLPointCloud2 {
  ::pcl::PCLHeader header;

  uindex_t height = 0;
  uindex_t width = 0;

  std::vector<::pcl::PCLPointField> fields;
  // @brief modified by nick.liao
  std::uint8_t is_bigendian = IsBigEndian();
  // static_assert(BOOST_ENDIAN_BIG_BYTE || BOOST_ENDIAN_LITTLE_BYTE,
  //               "unable to determine system endianness");
  // std::uint8_t is_bigendian = BOOST_ENDIAN_BIG_BYTE;
  uindex_t point_step = 0;
  uindex_t row_step = 0;

  std::vector<std::uint8_t> data;

  std::uint8_t is_dense = 0;

 public:
  using Ptr = shared_ptr<::pcl::PCLPointCloud2>;
  using ConstPtr = shared_ptr<const ::pcl::PCLPointCloud2>;

  //////////////////////////////////////////////////////////////////////////
  /** \brief Inplace concatenate two pcl::PCLPointCloud2
   *
   * IFF the layout of all the fields in both the clouds is the same, this
   * command doesn't remove any fields named "_" (aka marked as skip). For
   * comparison of field names, "rgb" and "rgba" are considered equivalent
   * However, if the order and/or number of non-skip fields is different, the
   * skip fields are dropped and non-skip fields copied selectively. This
   * function returns an error if
   *   * the total number of non-skip fields is different
   *   * the non-skip field names are named differently (excluding "rbg{a}") in
   * serial order
   *   * the endian-ness of both clouds is different
   * \param[in,out] cloud1 the first input and output point cloud dataset
   * \param[in] cloud2 the second input point cloud dataset
   * \return true if successful, false if failed (e.g., name/number of fields
   * differs)
   */
  static bool concatenate(pcl::PCLPointCloud2 &cloud1,
                          const pcl::PCLPointCloud2 &cloud2) {
    if (cloud1.is_bigendian != cloud2.is_bigendian) {
      // In future, it might be possible to convert based on
      // pcl::getFieldSize(fields.datatype)
      PCL_ERROR(
          "[pcl::PCLPointCloud2::concatenate] Endianness of clouds does not "
          "match\n");
      return (false);
    }

    const auto size1 = cloud1.width * cloud1.height;
    const auto size2 = cloud2.width * cloud2.height;
    // if one input cloud has no points, but the other input does, just select
    // the cloud with points
    if ((size1 == 0) && (size2 != 0)) {
      cloud1 = cloud2;
    }

    if ((size1 == 0) || (size2 == 0)) {
      cloud1.header.stamp = std::max(cloud1.header.stamp, cloud2.header.stamp);
      return true;
    }

    // Ideally this should be in PCLPointField class since this is global
    // behavior
    auto field_eq = [](const auto &field1, const auto &field2) {
      // We're fine with the special RGB vs RGBA use case
      return ((field1.name == field2.name) ||
              (field1.name == "rgb" && field2.name == "rgba") ||
              (field1.name == "rgba" && field2.name == "rgb"));
    };

    // A simple memcpy is possible if layout (name and order of fields) is same
    // for both clouds
    bool simple_layout =
        std::equal(cloud1.fields.begin(), cloud1.fields.end(),
                   cloud2.fields.begin(), cloud2.fields.end(), field_eq);

    struct FieldDetails {
      std::size_t idx1, idx2;
      pcl::uindex_t size;
      FieldDetails(std::size_t idx1_, std::size_t idx2_, pcl::uindex_t size_)
          : idx1(idx1_), idx2(idx2_), size(size_) {}
    };
    std::vector<FieldDetails> valid_fields;
    const auto max_field_size =
        std::max(cloud1.fields.size(), cloud2.fields.size());
    valid_fields.reserve(max_field_size);

    // @TODO: Refactor to return std::optional<std::vector<FieldDetails>>
    // Store the details of fields with common data in both cloud, exit early if
    // any errors are found
    if (!simple_layout) {
      std::size_t i = 0, j = 0;
      while (i < cloud1.fields.size() && j < cloud2.fields.size()) {
        if (cloud1.fields[i].name == "_") {
          ++i;
          continue;
        }
        if (cloud2.fields[j].name == "_") {
          ++j;
          continue;
        }

        if (field_eq(cloud1.fields[i], cloud2.fields[j])) {
          // Assumption: cloud1.fields[i].datatype == cloud2.fields[j].datatype
          valid_fields.emplace_back(
              i, j, pcl::getFieldSize(cloud2.fields[j].datatype));
          ++i;
          ++j;
          continue;
        }
        PCL_ERROR(
            "[pcl::PCLPointCloud2::concatenate] Name of field %zu in cloud1, "
            "%s, does not match name in cloud2, %s\n",
            i, cloud1.fields[i].name.c_str(), cloud2.fields[i].name.c_str());
        return (false);
      }
      // Both i and j should have exhausted their respective cloud.fields
      if (i != cloud1.fields.size() || j != cloud2.fields.size()) {
        PCL_ERROR(
            "[pcl::PCLPointCloud2::concatenate] Number of fields to copy in "
            "cloud1 (%zu) != Number of fields to copy in cloud2 (%zu)\n",
            i, j);
        return (false);
      }
    }

    // Save the latest timestamp in the destination cloud
    cloud1.header.stamp = std::max(cloud1.header.stamp, cloud2.header.stamp);

    cloud1.is_dense = cloud1.is_dense && cloud2.is_dense;
    cloud1.height = 1;
    cloud1.width = size1 + size2;
    cloud1.row_step = cloud1.width * cloud1.point_step;  // changed width

    if (simple_layout) {
      cloud1.data.insert(cloud1.data.end(), cloud2.data.begin(),
                         cloud2.data.end());
      return (true);
    }
    const auto data1_size = cloud1.data.size();
    cloud1.data.resize(data1_size + cloud2.data.size());
    for (uindex_t cp = 0; cp < size2; ++cp) {
      for (const auto &field_data : valid_fields) {
        const auto &i = field_data.idx1;
        const auto &j = field_data.idx2;
        const auto &size = field_data.size;
        // Leave the data for the skip fields untouched in cloud1
        // Copy only the required data from cloud2 to the correct location for
        // cloud1
        memcpy(
            reinterpret_cast<char *>(
                &cloud1.data[data1_size + cp * cloud1.point_step +
                             cloud1.fields[i].offset]),
            reinterpret_cast<const char *>(
                &cloud2.data[cp * cloud2.point_step + cloud2.fields[j].offset]),
            cloud2.fields[j].count * size);
      }
    }
    return (true);
  }

  /** \brief Concatenate two pcl::PCLPointCloud2
   * \param[in] cloud1 the first input point cloud dataset
   * \param[in] cloud2 the second input point cloud dataset
   * \param[out] cloud_out the resultant output point cloud dataset
   * \return true if successful, false if failed (e.g., name/number of fields
   * differs)
   */
  static bool concatenate(const PCLPointCloud2 &cloud1,
                          const PCLPointCloud2 &cloud2,
                          PCLPointCloud2 &cloud_out) {
    cloud_out = cloud1;
    return concatenate(cloud_out, cloud2);
  }

  /** \brief Add a point cloud to the current cloud.
   * \param[in] rhs the cloud to add to the current cloud
   * \return the new cloud as a concatenation of the current cloud and the new
   * given cloud
   */
  PCLPointCloud2 &operator+=(const PCLPointCloud2 &rhs) {
    if (concatenate((*this), rhs)) {
      return (*this);
    }
    throw std::logic_error(
        "Field or Endian mismatch. Please check log for more details");
    // PCL_THROW_EXCEPTION(
    //     ComputeFailedException,
    //     "Field or Endian mismatch. Please check log for more details");
  }

  /** \brief Add a point cloud to another cloud.
   * \param[in] rhs the cloud to add to the current cloud
   * \return the new cloud as a concatenation of the current cloud and the new
   * given cloud
   */
  inline PCLPointCloud2 operator+(const PCLPointCloud2 &rhs) {
    return (PCLPointCloud2(*this) += rhs);
  }

  /** \brief Get value at specified offset.
   * \param[in] point_index point index.
   * \param[in] field_offset offset.
   * \return value at the given offset.
   */
  template <typename T>
  inline const T &at(const pcl::uindex_t &point_index,
                     const pcl::uindex_t &field_offset) const {
    const auto position = point_index * point_step + field_offset;
    if (data.size() >= (position + sizeof(T)))
      return reinterpret_cast<const T &>(data[position]);
    else
      throw std::out_of_range("PCLPointCloud2::at");
  }

  /** \brief Get value at specified offset.
   * \param[in] point_index point index.
   * \param[in] field_offset offset.
   * \return value at the given offset.
   */
  template <typename T>
  inline T &at(const pcl::uindex_t &point_index,
               const pcl::uindex_t &field_offset) {
    const auto position = point_index * point_step + field_offset;
    if (data.size() >= (position + sizeof(T)))
      return reinterpret_cast<T &>(data[position]);
    else
      throw std::out_of_range("PCLPointCloud2::at");
  }
};  // struct PCLPointCloud2

using PCLPointCloud2Ptr = PCLPointCloud2::Ptr;
using PCLPointCloud2ConstPtr = PCLPointCloud2::ConstPtr;

inline std::ostream &operator<<(std::ostream &s,
                                const ::pcl::PCLPointCloud2 &v) {
  s << "header: " << std::endl;
  s << v.header;
  s << "height: ";
  s << "  " << v.height << std::endl;
  s << "width: ";
  s << "  " << v.width << std::endl;
  s << "fields[]" << std::endl;
  for (std::size_t i = 0; i < v.fields.size(); ++i) {
    s << "  fields[" << i << "]: ";
    s << std::endl;
    s << "    " << v.fields[i] << std::endl;
  }
  s << "is_bigendian: ";
  s << "  " << v.is_bigendian << std::endl;
  s << "point_step: ";
  s << "  " << v.point_step << std::endl;
  s << "row_step: ";
  s << "  " << v.row_step << std::endl;
  s << "data[]" << std::endl;
  for (std::size_t i = 0; i < v.data.size(); ++i) {
    s << "  data[" << i << "]: ";
    s << "  " << static_cast<int>(v.data[i]) << std::endl;
  }
  s << "is_dense: ";
  s << "  " << v.is_dense << std::endl;

  return (s);
}

/** \brief Get the available point cloud fields as a space separated string
 * \param[in] cloud a pointer to the PointCloud message
 * \ingroup common
 */
inline std::string getFieldsList(const PCLPointCloud2 &cloud) {
  if (cloud.fields.empty()) {
    return "";
  } else {
    return std::accumulate(std::next(cloud.fields.begin()), cloud.fields.end(),
                           cloud.fields[0].name,
                           [](const auto &acc, const auto &field) {
                             return acc + " " + field.name;
                           });
  }
}

/*******************************************************************************
 * Point
 *******************************************************************************
 */

#if defined(_MSC_VER)
    #define PCL_EIGEN_ALIGN16 __declspec(align(16))
#elif defined(__GNUC__)
    #define PCL_EIGEN_ALIGN16 __attribute__((aligned(16)))
#else
    #define PCL_EIGEN_ALIGN16
#endif

#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
#endif

struct PCL_EIGEN_ALIGN16 PointXYZI
{
    union PCL_EIGEN_ALIGN16
    {
        float data[4];
        struct
        {
          float x;
          float y;
          float z;
        };
    };
    union
    {
        struct
        {
          float intensity;
        };
        float data_c[4];
    };

    inline PointXYZI(const PointXYZI &p)
    {
      x = p.x; y = p.y; z = p.z; data[3] = 1.0f;
      intensity = p.intensity;
    }

    inline PointXYZI(float _intensity = 0.f)
      : PointXYZI(0.f, 0.f, 0.f, _intensity) {}

    inline PointXYZI(float _x, float _y, float _z, float _intensity = 0.f)
    {
      x = _x; y = _y; z = _z;
      data[3] = 1.0f;
      intensity = _intensity;
    }

    friend std::ostream& operator << (std::ostream& os, const PointXYZI& p);
};

#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif

inline std::ostream& operator << (std::ostream& os, const PointXYZI& p)
{
  os << "(" << p.x << "," << p.y << "," << p.z << " - " << p.intensity << ")";
  return (os);
}

/*******************************************************************************
 * PointCloud
 *******************************************************************************
 */
  template <typename PointT>
  class PointCloud
  {
    public:
      PCLHeader header;
      std::vector<PointT> points;
      std::uint32_t width = 0;
      std::uint32_t height = 0;
      bool is_dense = true;
      Vector4f    sensor_origin_ = Vector4f::Zero ();
      Quaternionf sensor_orientation_ = Quaternionf::Identity ();


      PointCloud () = default;

      //capacity
      inline std::size_t size () const { return points.size (); }
      inline index_t max_size() const noexcept { return static_cast<index_t>(points.max_size()); }
      inline void reserve (std::size_t n) { points.reserve (n); }
      inline bool empty () const { return points.empty (); }
      inline PointT* data() noexcept { return points.data(); }
      inline const PointT* data() const noexcept { return points.data(); }

      inline void
      resize(std::size_t count)
      {
        points.resize(count);
        if (width * height != count) {
          width = static_cast<std::uint32_t>(count);
          height = 1;
        }
      }

      //element access
      inline const PointT& operator[] (std::size_t n) const { return (points[n]); }
      inline PointT& operator[] (std::size_t n) { return (points[n]); }
      inline const PointT& at (std::size_t n) const { return (points.at (n)); }
      inline PointT& at (std::size_t n) { return (points.at (n)); }
      inline const PointT& front () const { return (points.front ()); }
      inline PointT& front () { return (points.front ()); }
      inline const PointT& back () const { return (points.back ()); }
      inline PointT& back () { return (points.back ()); }

      inline void
      clear ()
      {
        points.clear ();
        width = 0;
        height = 0;
      }
  };

  template <typename PointT> std::ostream&
  operator << (std::ostream& s, const PointCloud<PointT> &p)
  {
    s << "header: " << p.header << std::endl;
    s << "points[]: " << p.size () << std::endl;
    s << "width: " << p.width << std::endl;
    s << "height: " << p.height << std::endl;
    s << "is_dense: " << p.is_dense << std::endl;
    s << "sensor origin (xyz): [" <<
      p.sensor_origin_[0] << ", " <<
      p.sensor_origin_[1] << ", " <<
      p.sensor_origin_[2] << "] / orientation (xyzw): [" <<
      p.sensor_orientation_.x () << ", " <<
      p.sensor_orientation_.y () << ", " <<
      p.sensor_orientation_.z () << ", " <<
      p.sensor_orientation_.w () << "]" <<
      std::endl;
    return (s);
  }

inline bool fromPCLPointCloud2(const pcl::PCLPointCloud2& in,
                               pcl::PointCloud<pcl::PointXYZI>& out) {
  std::unordered_map<std::string, pcl::PCLPointField> fields;
  for (auto& f : in.fields) {
    fields[f.name] = f;
  }

  // check x,y,z
  std::string xyz_name[] = {"x", "y", "z"};
  for (int i = 0; i < 3; i++) {
    if (fields.end() == fields.find(xyz_name[i])) {
      PCL_ERROR("[pcl::fromPCLPointCloud2] not find field '%s'.",
                xyz_name[i].c_str());
      return false;
    }
  }

  auto ii = fields.find("intensity");
  bool has_intensity = fields.end() != ii;
  if (!has_intensity) {
    PCL_WARN("[pcl::fromPCLPointCloud2] not find field: intensity.");
  }

  bool intensity_uint8 = false;
  if (ii->second.datatype == pcl::PCLPointField::UINT8) {
    intensity_uint8 = true;
  } else if (ii->second.datatype == pcl::PCLPointField::FLOAT32) {
    intensity_uint8 = false;
  } else {
    PCL_ERROR("[pcl::fromPCLPointCloud2] unsupport inensity datatype %d.",
              static_cast<int>(ii->second.datatype));
    return false;
  }

  pcl::uindex_t N = in.width * in.height;
  out.clear();
  out.resize(N);

  // we assume [count = 1]
  pcl::uindex_t ox = fields["x"].offset;
  pcl::uindex_t oy = fields["y"].offset;
  pcl::uindex_t oz = fields["z"].offset;
  pcl::uindex_t oi = ii->second.offset;

  for (pcl::uindex_t i = 0; i < N; i++) {
    auto& p = out[i];
    p.x = in.at<float>(i, ox);
    p.y = in.at<float>(i, oy);
    p.z = in.at<float>(i, oz);
    if (intensity_uint8) {
      p.intensity = static_cast<float>(in.at<uint8_t>(i, oi)) / UINT8_MAX;
    } else {
      p.intensity = in.at<float>(i, oi);
    }
  }

  return true;
}

/*******************************************************************************
 * File IO
 *******************************************************************************
 */
namespace io {
#ifdef _WIN32
inline int raw_open(const char *pathname, int flags, int mode) {
  int fd = -1;
  errno_t err = ::_sopen_s(&fd, pathname, flags, _SH_DENYNO, mode);
  if (err != 0)
  {
    return -1;
  }
  return fd;
}

inline int raw_open(const char *pathname, int flags) {
  return raw_open(pathname, flags, _S_IREAD);
}

inline int raw_close(int fd) { return ::_close(fd); }

inline long raw_lseek(int fd, long offset, int whence) {
  return ::_lseek(fd, offset, whence);
}

inline int raw_read(int fd, void *buffer, std::size_t count) {
  return ::_read(fd, buffer, static_cast<unsigned int>(count));
}

inline int raw_write(int fd, const void *buffer, std::size_t count) {
  return ::_write(fd, buffer, static_cast<unsigned int>(count));
}

inline int raw_ftruncate(int fd, long length) { return ::_chsize(fd, length); }

inline int raw_fallocate(int fd, long length) {
  // Doesn't actually allocate, but best we can do?
  return raw_ftruncate(fd, length);
}
#else
inline int raw_open(const char *pathname, int flags, int mode) {
  return ::open(pathname, flags, mode);
}

inline int raw_open(const char *pathname, int flags) {
  return ::open(pathname, flags);
}

inline int raw_close(int fd) { return ::close(fd); }

inline off_t raw_lseek(int fd, off_t offset, int whence) {
  return ::lseek(fd, offset, whence);
}

inline ssize_t raw_read(int fd, void *buffer, std::size_t count) {
  return ::read(fd, buffer, count);
}

inline ssize_t raw_write(int fd, const void *buffer, std::size_t count) {
  return ::write(fd, buffer, count);
}

inline int raw_ftruncate(int fd, off_t length) {
  return ::ftruncate(fd, length);
}

#ifdef __APPLE__
inline int raw_fallocate(int fd, off_t length) {
  // OS X doesn't have posix_fallocate, but it has a fcntl that does the job.
  // It may make the file too big though, so we truncate before returning.

  // Try to allocate contiguous space first.
  ::fstore_t store = {F_ALLOCATEALL | F_ALLOCATECONTIG, F_PEOFPOSMODE, 0,
                      length, 0};
  if (::fcntl(fd, F_PREALLOCATE, &store) != -1)
    return raw_ftruncate(fd, length);

  // Try fragmented if it failed.
  store.fst_flags = F_ALLOCATEALL;
  if (::fcntl(fd, F_PREALLOCATE, &store) != -1)
    return raw_ftruncate(fd, length);

  // Fragmented also failed.
  return -1;
}

#else  // __APPLE__
inline int raw_fallocate(int fd, off_t length) {
#ifdef ANDROID
  // Android's libc doesn't have posix_fallocate.
  if (::fallocate(fd, 0, 0, length) == 0) return 0;

  // fallocate returns -1 on error and sets errno
  // EINVAL should indicate an unsupported filesystem.
  // All other errors are passed up.
  if (errno != EINVAL) return -1;
#elif defined(__OpenBSD__)
  // OpenBSD has neither posix_fallocate nor fallocate
  if (::ftruncate(fd, length) == 0) return 0;
  if (errno != EINVAL) return -1;
#else
  // Conforming POSIX systems have posix_fallocate.
  const int res = ::posix_fallocate(fd, 0, length);
  if (res == 0) return 0;

  // posix_fallocate does not set errno
  // EINVAL should indicate an unsupported filesystem.
  // All other errors are passed up.
  if (res != EINVAL) return res;
#endif

  // Try to deal with unsupported filesystems by simply seeking + writing.
  // This may not really allocate space, but the file size will be set.
  // Writes to the mmapped file may still trigger SIGBUS errors later.

  // Remember the old position and seek to the desired length.
  off_t old_pos = raw_lseek(fd, 0, SEEK_CUR);
  if (old_pos == -1) return -1;
  if (raw_lseek(fd, length - 1, SEEK_SET) == -1) return -1;

  // Write a single byte to resize the file.
  char buffer = 0;
  ssize_t written = raw_write(fd, &buffer, 1);

  // Seek back to the old position.
  if (raw_lseek(fd, old_pos, SEEK_SET) == -1) return -1;

  // Fail if we didn't write exactly one byte,
  if (written != 1) return -1;

  return 0;
}
#endif  // __APPLE
#endif  // _WIN32

}  // namespace io

/** \brief inserts a value of type Type (uchar, char, uint, int, float, double,
 * ...) into a stringstream.
 *
 * If the value is NaN, it inserts "nan".
 *
 * \param[in] cloud the cloud to copy from
 * \param[in] point_index the index of the point
 * \param[in] point_size the size of the point in the cloud
 * \param[in] field_idx the index of the dimension/field
 * \param[in] fields_count the current fields count
 * \param[out] stream the ostringstream to copy into
 */
template <typename Type>
inline std::enable_if_t<std::is_floating_point<Type>::value> copyValueString(
    const pcl::PCLPointCloud2 &cloud, const pcl::uindex_t point_index,
    const pcl::uindex_t point_size, const pcl::uindex_t field_idx,
    const pcl::uindex_t fields_count, std::ostream &stream) {
  Type value;
  memcpy(&value,
         &cloud.data[point_index * point_size + cloud.fields[field_idx].offset +
                     fields_count * sizeof(Type)],
         sizeof(Type));
  if (std::isnan(value))
    stream << "nan";
  else
    stream << value;
}

template <typename Type>
inline std::enable_if_t<std::is_integral<Type>::value> copyValueString(
    const pcl::PCLPointCloud2 &cloud, const pcl::uindex_t point_index,
    const pcl::uindex_t point_size, const pcl::uindex_t field_idx,
    const pcl::uindex_t fields_count, std::ostream &stream) {
  Type value;
  memcpy(&value,
         &cloud.data[point_index * point_size + cloud.fields[field_idx].offset +
                     fields_count * sizeof(Type)],
         sizeof(Type));
  stream << value;
}

template <>
inline void copyValueString<std::int8_t>(const pcl::PCLPointCloud2 &cloud,
                                         const pcl::uindex_t point_index,
                                         const pcl::uindex_t point_size,
                                         const pcl::uindex_t field_idx,
                                         const pcl::uindex_t fields_count,
                                         std::ostream &stream) {
  std::int8_t value;
  memcpy(&value,
         &cloud.data[point_index * point_size + cloud.fields[field_idx].offset +
                     fields_count * sizeof(std::int8_t)],
         sizeof(std::int8_t));
  // Cast to int to prevent value is handled as char
  // stream << boost::numeric_cast<int>(value);
  stream << static_cast<int>(value);
}

template <>
inline void copyValueString<std::uint8_t>(const pcl::PCLPointCloud2 &cloud,
                                          const pcl::uindex_t point_index,
                                          const pcl::uindex_t point_size,
                                          const pcl::uindex_t field_idx,
                                          const pcl::uindex_t fields_count,
                                          std::ostream &stream) {
  std::uint8_t value;
  memcpy(&value,
         &cloud.data[point_index * point_size + cloud.fields[field_idx].offset +
                     fields_count * sizeof(std::uint8_t)],
         sizeof(std::uint8_t));
  // Cast to unsigned int to prevent value is handled as char
  // stream << boost::numeric_cast<unsigned int>(value);
  stream << static_cast<unsigned int>(value);
}

/** \brief Check whether a given value of type T (uchar, char, uint, int,
 * float, double, ...) is finite or not
 *
 * \param[in] cloud the cloud that contains the data
 * \param[in] point_index the index of the point
 * \param[in] point_size the size of the point in the cloud
 * \param[in] field_idx the index of the dimension/field
 * \param[in] fields_count the current fields count
 *
 * \return true if the value is finite, false otherwise
 */
template <typename T,
          typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
inline bool isValueFinite(const pcl::PCLPointCloud2 &cloud,
                          pcl::uindex_t point_index,
                          pcl::uindex_t point_size,
                          pcl::uindex_t field_idx,
                          pcl::uindex_t fields_count)
{
  T value;
  memcpy(&value,
         &cloud.data[point_index * point_size + cloud.fields[field_idx].offset +
                     fields_count * sizeof(T)],
         sizeof(T));
  return std::isfinite(value);
}

template <typename T,
          typename std::enable_if_t<std::is_integral<T>::value, int> = 0>
inline bool isValueFinite(const pcl::PCLPointCloud2 & /* cloud */,
                          pcl::uindex_t /* point_index */,
                          pcl::uindex_t /* point_size */,
                          pcl::uindex_t /* field_idx */,
                          pcl::uindex_t /* fields_count */)
{
  return true;
}

template <typename Type>
inline void copyStringValue(const std::string &st, pcl::PCLPointCloud2 &cloud,
                            pcl::uindex_t point_index, unsigned int field_idx,
                            pcl::uindex_t fields_count, std::istringstream &is)
{
  Type value;
  if (iequals(st, "nan")) {
    value = std::numeric_limits<Type>::quiet_NaN();
    cloud.is_dense = false;
  } else {
    is.str(st);
    is.clear();  // clear error state flags
    if (!(is >> value)) value = static_cast<Type>(atof(st.c_str()));
  }

  memcpy(
      &cloud.data[point_index * cloud.point_step +
                  cloud.fields[field_idx].offset + fields_count * sizeof(Type)],
      reinterpret_cast<char *>(&value), sizeof(Type));
}

template <>
inline void copyStringValue<std::int8_t>(
    const std::string &st, pcl::PCLPointCloud2 &cloud,
    pcl::uindex_t point_index,
    unsigned int field_idx, pcl::uindex_t fields_count, std::istringstream &is
    )
{
  std::int8_t value;
  int val;
  is.str(st);
  is.clear();  // clear error state flags
  // is >> val;  -- unfortunately this fails on older GCC versions and CLANG on
  // MacOS
  if (!(is >> val)) {
    val = static_cast<int>(atof(st.c_str()));
  }
  value = static_cast<std::int8_t>(val);

  memcpy(&cloud.data[point_index * cloud.point_step +
                     cloud.fields[field_idx].offset +
                     fields_count * sizeof(std::int8_t)],
         reinterpret_cast<char *>(&value), sizeof(std::int8_t));
}

template <>
inline void copyStringValue<std::uint8_t>(
    const std::string &st, pcl::PCLPointCloud2 &cloud,
    pcl::uindex_t point_index,
    unsigned int field_idx, pcl::uindex_t fields_count, std::istringstream &is
    )
{
  std::uint8_t value;
  int val;
  is.str(st);
  is.clear();  // clear error state flags
  // is >> val;  -- unfortunately this fails on older GCC versions and CLANG on
  // MacOS
  if (!(is >> val)) {
    val = static_cast<int>(atof(st.c_str()));
  }
  value = static_cast<std::uint8_t>(val);

  memcpy(&cloud.data[point_index * cloud.point_step +
                     cloud.fields[field_idx].offset +
                     fields_count * sizeof(std::uint8_t)],
         reinterpret_cast<char *>(&value), sizeof(std::uint8_t));
}

namespace traits {
namespace detail {
/**
 * \brief Enumeration for different numerical types
 *
 * \details struct used to enable scope and implicit conversion to int
 */
struct PointFieldTypes {
  static const std::uint8_t INT8 = 1, UINT8 = 2, INT16 = 3, UINT16 = 4,
                            INT32 = 5, UINT32 = 6, FLOAT32 = 7, FLOAT64 = 8,
                            INT64 = 9, UINT64 = 10, BOOL = 11;
};
}  // namespace detail

// Metafunction to return enum value representing a type
template <typename T>
struct asEnum {};
template <>
struct asEnum<bool> {
  static const std::uint8_t value = detail::PointFieldTypes::BOOL;
};
template <>
struct asEnum<std::int8_t> {
  static const std::uint8_t value = detail::PointFieldTypes::INT8;
};
template <>
struct asEnum<std::uint8_t> {
  static const std::uint8_t value = detail::PointFieldTypes::UINT8;
};
template <>
struct asEnum<std::int16_t> {
  static const std::uint8_t value = detail::PointFieldTypes::INT16;
};
template <>
struct asEnum<std::uint16_t> {
  static const std::uint8_t value = detail::PointFieldTypes::UINT16;
};
template <>
struct asEnum<std::int32_t> {
  static const std::uint8_t value = detail::PointFieldTypes::INT32;
};
template <>
struct asEnum<std::uint32_t> {
  static const std::uint8_t value = detail::PointFieldTypes::UINT32;
};
template <>
struct asEnum<std::int64_t> {
  static const std::uint8_t value = detail::PointFieldTypes::INT64;
};
template <>
struct asEnum<std::uint64_t> {
  static const std::uint8_t value = detail::PointFieldTypes::UINT64;
};
template <>
struct asEnum<float> {
  static const std::uint8_t value = detail::PointFieldTypes::FLOAT32;
};
template <>
struct asEnum<double> {
  static const std::uint8_t value = detail::PointFieldTypes::FLOAT64;
};

template <typename T>
static constexpr std::uint8_t asEnum_v = asEnum<T>::value;

// Metafunction to return type of enum value
template <int>
struct asType {};
template <>
struct asType<detail::PointFieldTypes::BOOL> {
  using type = bool;
};
template <>
struct asType<detail::PointFieldTypes::INT8> {
  using type = std::int8_t;
};
template <>
struct asType<detail::PointFieldTypes::UINT8> {
  using type = std::uint8_t;
};
template <>
struct asType<detail::PointFieldTypes::INT16> {
  using type = std::int16_t;
};
template <>
struct asType<detail::PointFieldTypes::UINT16> {
  using type = std::uint16_t;
};
template <>
struct asType<detail::PointFieldTypes::INT32> {
  using type = std::int32_t;
};
template <>
struct asType<detail::PointFieldTypes::UINT32> {
  using type = std::uint32_t;
};
template <>
struct asType<detail::PointFieldTypes::INT64> {
  using type = std::int64_t;
};
template <>
struct asType<detail::PointFieldTypes::UINT64> {
  using type = std::uint64_t;
};
template <>
struct asType<detail::PointFieldTypes::FLOAT32> {
  using type = float;
};
template <>
struct asType<detail::PointFieldTypes::FLOAT64> {
  using type = double;
};

template <int index>
using asType_t = typename asType<index>::type;

}  // namespace traits

/*******************************************************************************
 * LZF
 *******************************************************************************
 */
/*
 * Size of hashtable is (1 << HLOG) * sizeof (char *)
 * decompression is independent of the hash table size
 * the difference between 15 and 14 is very small
 * for small blocks (and 14 is usually a bit faster).
 * For a low-memory/faster configuration, use HLOG == 13;
 * For best compression, use 15 or 16 (or more, up to 22).
 */
#define HLOG 13

using LZF_HSLOT = unsigned int;
using LZF_STATE = unsigned int[1 << (HLOG)];

#if !(defined(__i386) || defined(__amd64))
#define STRICT_ALIGN 1
#else
#define STRICT_ALIGN 0
#endif
#if !STRICT_ALIGN
/* for unaligned accesses we need a 16 bit datatype. */
#if USHRT_MAX == 65535
using u16 = unsigned short;
#elif UINT_MAX == 65535
using u16 = unsigned int;
#else
#undef STRICT_ALIGN
#define STRICT_ALIGN 1
#endif
#endif

// IDX works because it is very similar to a multiplicative hash, e.g.
// ((h * 57321 >> (3*8 - HLOG)) & ((1 << (HLOG)) - 1))
#define IDX(h) ((((h) >> (3 * 8 - HLOG)) - (h)) & ((1 << (HLOG)) - 1))

/** \brief Compress in_len bytes stored at the memory block starting at
 * \a in_data and write the result to \a out_data, up to a maximum length
 * of \a out_len bytes using Marc Lehmann's LZF algorithm.
 *
 * If the output buffer is not large enough or any error occurs return 0,
 * otherwise return the number of bytes used, which might be considerably
 * more than in_len (but less than 104% of the original size), so it
 * makes sense to always use out_len == in_len - 1), to ensure _some_
 * compression, and store the data uncompressed otherwise (with a flag, of
 * course.
 *
 * \note The buffers must not be overlapping.
 *
 * \param[in] in_data the input uncompressed buffer
 * \param[in] in_len the length of the input buffer
 * \param[out] out_data the output buffer where the compressed result will be
 * stored
 * \param[in] out_len the length of the output buffer
 *
 */
inline unsigned int lzfCompress(const void *const in_data, unsigned int in_len,
                                void *out_data, unsigned int out_len) {
  LZF_STATE htab{};
  const auto *ip = static_cast<const unsigned char *>(in_data);
  auto *op = static_cast<unsigned char *>(out_data);
  const unsigned char *in_end = ip + in_len;
  unsigned char *out_end = op + out_len;

  if (!in_len || !out_len) {
    PCL_WARN("[pcl::lzf_compress] Input or output has 0 size!\n");
    return (0);
  }

  // Start run
  int lit = 0;
  op++;

  unsigned int hval = (ip[0] << 8) | ip[1];
  while (ip < in_end - 2) {
    unsigned int *hslot;

    hval = (hval << 8) | ip[2];
    hslot = htab + IDX(hval);
    const unsigned char *ref =
        *hslot + (static_cast<const unsigned char *>(in_data));
    *hslot = static_cast<unsigned int>(
        ip - (static_cast<const unsigned char *>(in_data)));

    // off requires a type wide enough to hold a general pointer difference.
    // ISO C doesn't have that (std::size_t might not be enough and ptrdiff_t
    // only works for differences within a single object). We also assume that
    // no no bit pattern traps. Since the only platform that is both non-POSIX
    // and fails to support both assumptions is windows 64 bit, we make a
    // special workaround for it.
#if defined(_WIN32) && defined(_M_X64) && defined(_MSC_VER)
    // workaround for missing POSIX compliance
    long long off;
#else
    long long off;
#endif

    if (
        // The next test will actually take care of this, but this is faster if
        // htab is initialized
        ref < ip && (off = ip - ref - 1) < (1 << 13) &&
        ref > static_cast<const unsigned char *>(in_data) && ref[2] == ip[2]
#if STRICT_ALIGN
        && ((ref[1] << 8) | ref[0]) == ((ip[1] << 8) | ip[0])
#else
        && *reinterpret_cast<const u16 *>(ref) ==
               *reinterpret_cast<const u16 *>(ip)
#endif
    ) {
      // Match found at *ref++
      unsigned int len = 2;
      std::ptrdiff_t maxlen = in_end - ip - len;
      maxlen = maxlen > ((1 << 8) + (1 << 3)) ? ((1 << 8) + (1 << 3)) : maxlen;

      // First a faster conservative test
      if (op + 3 + 1 >= out_end) {
        // Second the exact but rare test
        if (op - !lit + 3 + 1 >= out_end) {
          PCL_WARN(
              "[pcl::lzf_compress] Attempting to write data outside the output "
              "buffer!\n");
          return (0);
        }
      }

      // Stop run
      op[-lit - 1] = static_cast<unsigned char>(lit - 1);
      // Undo run if length is zero
      op -= !lit;

      while (true) {
        if (maxlen > 16) {
          len++;
          if (ref[len] != ip[len]) break;
          len++;
          if (ref[len] != ip[len]) break;
          len++;
          if (ref[len] != ip[len]) break;
          len++;
          if (ref[len] != ip[len]) break;

          len++;
          if (ref[len] != ip[len]) break;
          len++;
          if (ref[len] != ip[len]) break;
          len++;
          if (ref[len] != ip[len]) break;
          len++;
          if (ref[len] != ip[len]) break;

          len++;
          if (ref[len] != ip[len]) break;
          len++;
          if (ref[len] != ip[len]) break;
          len++;
          if (ref[len] != ip[len]) break;
          len++;
          if (ref[len] != ip[len]) break;

          len++;
          if (ref[len] != ip[len]) break;
          len++;
          if (ref[len] != ip[len]) break;
          len++;
          if (ref[len] != ip[len]) break;
          len++;
          if (ref[len] != ip[len]) break;
        }

        do
          len++;
        while (len < static_cast<unsigned int>(maxlen) && ref[len] == ip[len]);

        break;
      }

      // Len is now #octets - 1
      len -= 2;
      ip++;

      if (len < 7) {
        *op++ = static_cast<unsigned char>((off >> 8) + (len << 5));
      } else {
        *op++ = static_cast<unsigned char>((off >> 8) + (7 << 5));
        *op++ = static_cast<unsigned char>(len - 7);
      }

      *op++ = static_cast<unsigned char>(off);

      // Start run
      lit = 0;
      op++;

      ip += len + 1;

      if (ip >= in_end - 2) break;

      --ip;
      hval = (ip[0] << 8) | ip[1];

      hval = (hval << 8) | ip[2];
      htab[IDX(hval)] = static_cast<unsigned int>(
          ip - (static_cast<const unsigned char *>(in_data)));
      ip++;
    } else {
      // One more literal byte we must copy
      if (op >= out_end) {
        PCL_WARN(
            "[pcl::lzf_compress] Attempting to copy data outside the output "
            "buffer!\n");
        return (0);
      }

      lit++;
      *op++ = *ip++;

      if (lit == (1 << 5)) {
        // Stop run
        op[-lit - 1] = static_cast<unsigned char>(lit - 1);
        // Start run
        lit = 0;
        op++;
      }
    }
  }

  // At most 3 bytes can be missing here
  if (op + 3 > out_end) return (0);

  while (ip < in_end) {
    lit++;
    *op++ = *ip++;

    if (lit == (1 << 5)) {
      // Stop run
      op[-lit - 1] = static_cast<unsigned char>(lit - 1);
      // Start run
      lit = 0;
      op++;
    }
  }

  // End run
  op[-lit - 1] = static_cast<unsigned char>(lit - 1);
  // Undo run if length is zero
  op -= !lit;

  return (
      static_cast<unsigned int>(op - static_cast<unsigned char *>(out_data)));
}

/** \brief Decompress data compressed with the \a lzfCompress function and
 * stored at location \a in_data and length \a in_len. The result will be
 * stored at \a out_data up to a maximum of \a out_len characters.
 *
 * If the output buffer is not large enough to hold the decompressed
 * data, a 0 is returned and errno is set to E2BIG. Otherwise the number
 * of decompressed bytes (i.e. the original length of the data) is
 * returned.
 *
 * If an error in the compressed data is detected, a zero is returned and
 * errno is set to EINVAL.
 *
 * This function is very fast, about as fast as a copying loop.
 * \param[in] in_data the input compressed buffer
 * \param[in] in_len the length of the input buffer
 * \param[out] out_data the output buffer (must be resized to \a out_len)
 * \param[in] out_len the length of the output buffer
 */
inline unsigned int lzfDecompress(const void *const in_data,
                                  unsigned int in_len,
                                  void *out_data, unsigned int out_len) {
  auto const *ip = static_cast<const unsigned char *>(in_data);
  auto *op = static_cast<unsigned char *>(out_data);
  unsigned char const *const in_end = ip + in_len;
  unsigned char *const out_end = op + out_len;

  do {
    unsigned int ctrl = *ip++;

    // Literal run
    if (ctrl < (1 << 5)) {
      ctrl++;

      if (op + ctrl > out_end) {
        errno = E2BIG;
        return (0);
      }

      // Check for overflow
      if (ip + ctrl > in_end) {
        errno = EINVAL;
        return (0);
      }
      for (unsigned ctrl_c = ctrl; ctrl_c; --ctrl_c) *op++ = *ip++;
    }
    // Back reference
    else {
      unsigned int len = ctrl >> 5;

      unsigned char *ref = op - ((ctrl & 0x1f) << 8) - 1;

      // Check for overflow
      if (ip >= in_end) {
        errno = EINVAL;
        return (0);
      }
      if (len == 7) {
        len += *ip++;
        // Check for overflow
        if (ip >= in_end) {
          errno = EINVAL;
          return (0);
        }
      }
      ref -= *ip++;

      if (op + len + 2 > out_end) {
        errno = E2BIG;
        return (0);
      }

      if (ref < static_cast<unsigned char *>(out_data)) {
        errno = EINVAL;
        return (0);
      }

      if (len > 9) {
        len += 2;

        if (op >= ref + len) {
          // Disjunct
          std::copy(ref, ref + len, op);
          op += len;
        } else {
          // Overlapping, use byte by byte copying
          do
            *op++ = *ref++;
          while (--len);
        }
      } else
        for (unsigned len_c = len + 2 /* case 0 iterates twice */; len_c;
             --len_c)
          *op++ = *ref++;
    }
  } while (ip < in_end);

  return (
      static_cast<unsigned int>(op - static_cast<unsigned char *>(out_data)));
}

/*******************************************************************************
 * PCDReader
 *******************************************************************************
 */
class PCDReader {
 public:
  PCDReader() = default;
  ~PCDReader() = default;

  /** \brief Various PCD file versions.
   *
   * PCD_V6 represents PCD files with version 0.6, which contain the following
   * fields:
   *   - lines beginning with # are treated as comments
   *   - FIELDS ...
   *   - SIZE ...
   *   - TYPE ...
   *   - COUNT ...
   *   - WIDTH ...
   *   - HEIGHT ...
   *   - POINTS ...
   *   - DATA ascii/binary
   *
   * Everything that follows \b DATA is interpreted as data points and
   * will be read accordingly.
   *
   * PCD_V7 represents PCD files with version 0.7 and has an important
   * addon: it adds sensor origin/orientation (aka viewpoint) information
   * to a dataset through the use of a new header field:
   *   - VIEWPOINT tx ty tz qw qx qy qz
   */
  enum { PCD_V6 = 0, PCD_V7 = 1 };

  /** \brief Read a point cloud data header from a PCD-formatted, binary
   * istream.
   *
   * Load only the meta information (number of points, their types, etc),
   * and not the points themselves, from a given PCD stream. Useful for fast
   * evaluation of the underlying data structure.
   *
   * \attention The PCD data is \b always stored in ROW major format! The
   * read/write PCD methods will detect column major input and automatically
   * convert it.
   *
   * \param[in] fs a std::istream with openmode set to std::ios::binary.
   * \param[out] cloud the resultant point cloud dataset (only
   * these members will be filled: width, height, point_step, row_step,
   * fields[]; data is resized but not written)
   * \param[out] origin the sensor
   * acquisition origin (only for > PCD_V7 - null if not present)
   * \param[out] orientation the sensor acquisition orientation (only for >
   * PCD_V7 - identity if not present) \param[out] pcd_version the PCD version
   * of the file (i.e., PCD_V6, PCD_V7) \param[out] data_type the type of data
   * (0 =ASCII, 1 = Binary, 2 = Binary compressed) \param[out] data_idx the
   * offset of cloud data within the file
   *
   * \return
   *  * < 0 (-1) on error
   *  * == 0 on success
   */
  int readHeader(std::istream &fs, PCLPointCloud2 &cloud, Vector4f &origin,
                 Quaternionf &orientation, int &pcd_version, int &data_type,
                 unsigned int &data_idx) {
    // Default values
    data_idx = 0;
    data_type = 0;
    pcd_version = PCD_V6;
    origin = Vector4f::Zero();
    orientation = Quaternionf::Identity();
    cloud.width = cloud.height = cloud.point_step = cloud.row_step = 0;
    cloud.data.clear();

    // By default, assume that there are _no_ invalid (e.g., NaN) points
    // cloud.is_dense = true;

    // Used to determine if a value has been read
    bool width_read = false;
    bool height_read = false;

    std::size_t nr_points = 0;
    std::string line;

    // field_sizes represents the size of one element in a field (e.g., float =
    // 4, char = 1) field_counts represents the number of elements in a field
    // (e.g., x = 1, normal_x = 1, fpfh = 33)
    std::vector<unsigned int> field_sizes, field_counts;
    // field_types represents the type of data in a field (e.g., F = float, U =
    // unsigned)
    std::vector<char> field_types;
    std::vector<std::string> st;

    // Read the header and fill it in with wonderful values
    try {
      while (!fs.eof()) {
        getline(fs, line);
        // Ignore empty lines
        if (line.empty()) continue;

        // Tokenize the line
        pcl::split(st, line, "\t\r ");

        std::stringstream sstream(line);
        sstream.imbue(std::locale::classic());

        std::string line_type;
        sstream >> line_type;

        // Ignore comments
        if (line_type.substr(0, 1) == "#") continue;

        // Version numbers are not needed for now, but we are checking to see if
        // they're there
        if (line_type.substr(0, 7) == "VERSION") continue;

        // Get the field indices (check for COLUMNS too for backwards
        // compatibility)
        if ((line_type.substr(0, 6) == "FIELDS") ||
            (line_type.substr(0, 7) == "COLUMNS")) {
          size_t specified_channel_count = st.size() - 1;

          // Allocate enough memory to accommodate all fields
          cloud.fields.resize(specified_channel_count);
          for (size_t i = 0; i < specified_channel_count; ++i) {
            std::string col_type = st.at(i + 1);
            cloud.fields[i].name = col_type;
          }

          // Default the sizes and the types of each field to float32 to avoid
          // crashes while using older PCD files
          unsigned int offset = 0;
          for (size_t i = 0; i < specified_channel_count; ++i, offset += 4) {
            cloud.fields[i].offset = offset;
            cloud.fields[i].datatype = pcl::PCLPointField::FLOAT32;
            cloud.fields[i].count = 1;
          }
          cloud.point_step = offset;
          continue;
        }

        // Get the field sizes
        if (line_type.substr(0, 4) == "SIZE") {
          size_t specified_channel_count = st.size() - 1;

          // Allocate enough memory to accommodate all fields
          if (specified_channel_count != cloud.fields.size())
            throw "The number of elements in <SIZE> differs than the number of elements in <FIELDS>!";

          // Resize to accommodate the number of values
          field_sizes.resize(specified_channel_count);

          unsigned int offset = 0;
          for (size_t i = 0; i < specified_channel_count; ++i) {
            unsigned int col_type;
            sstream >> col_type;
            cloud.fields[i].offset =
                offset;  // estimate and save the data offsets
            offset += col_type;
            field_sizes[i] = col_type;  // save a temporary copy
          }
          cloud.point_step = offset;
          // if (cloud.width != 0)
          // cloud.row_step   = cloud.point_step * cloud.width;
          continue;
        }

        // Get the field types
        if (line_type.substr(0, 4) == "TYPE") {
          if (field_sizes.empty())
            throw "TYPE of FIELDS specified before SIZE in header!";

          size_t specified_channel_count = st.size() - 1;

          // Allocate enough memory to accommodate all fields
          if (specified_channel_count != cloud.fields.size())
            throw "The number of elements in <TYPE> differs than the number of elements in <FIELDS>!";

          // Resize to accommodate the number of values
          field_types.resize(specified_channel_count);

          for (size_t i = 0; i < specified_channel_count; ++i) {
            field_types[i] = st.at(i + 1).c_str()[0];
            cloud.fields[i].datatype = static_cast<std::uint8_t>(
                getFieldType(field_sizes[i], field_types[i]));
          }
          continue;
        }

        // Get the field counts
        if (line_type.substr(0, 5) == "COUNT") {
          if (field_sizes.empty() || field_types.empty())
            throw "COUNT of FIELDS specified before SIZE or TYPE in header!";

          size_t specified_channel_count = st.size() - 1;

          // Allocate enough memory to accommodate all fields
          if (specified_channel_count != cloud.fields.size())
            throw "The number of elements in <COUNT> differs than the number of elements in <FIELDS>!";

          field_counts.resize(specified_channel_count);

          unsigned int offset = 0;
          for (size_t i = 0; i < specified_channel_count; ++i) {
            cloud.fields[i].offset = offset;
            unsigned int col_count;
            sstream >> col_count;
            if (col_count < 1)
              PCL_WARN(
                  "[pcl::PCDReader::readHeader] Invalid COUNT value specified "
                  "(%i, should be larger than 0). This field will be "
                  "removed.\n",
                  col_count);
            cloud.fields[i].count = col_count;
            offset += col_count * field_sizes[i];
          }
          // Adjust the offset for count (number of elements)
          cloud.point_step = offset;
          continue;
        }

        // Get the width of the data (organized point cloud dataset)
        if (line_type.substr(0, 5) == "WIDTH") {
          sstream >> cloud.width;
          if (sstream.fail()) throw "Invalid WIDTH value specified.";
          width_read = true;
          if (cloud.point_step != 0)
            cloud.row_step =
                cloud.point_step *
                cloud
                    .width;  // row_step only makes sense for organized datasets
          continue;
        }

        // Get the height of the data (organized point cloud dataset)
        if (line_type.substr(0, 6) == "HEIGHT") {
          sstream >> cloud.height;
          if (sstream.fail()) throw "Invalid HEIGHT value specified.";
          height_read = true;
          continue;
        }

        // Get the acquisition viewpoint
        if (line_type.substr(0, 9) == "VIEWPOINT") {
          pcd_version = PCD_V7;
          if (st.size() < 8)
            throw "Not enough number of elements in <VIEWPOINT>! Need 7 values (tx ty tz qw qx qy qz).";

          float x, y, z, w;
          sstream >> x >> y >> z;
          origin = Vector4f(x, y, z, 0.0f);
          sstream >> w >> x >> y >> z;
          orientation = Quaternionf(w, x, y, z);
          continue;
        }

        // Get the number of points
        if (line_type.substr(0, 6) == "POINTS") {
          if (!cloud.point_step)
            throw "Number of POINTS specified before COUNT in header!";
          sstream >> nr_points;
          // Need to allocate: N * point_step
          cloud.data.resize(nr_points * cloud.point_step);
          continue;
        }

        // Read the header + comments line by line until we get to <DATA>
        if (line_type.substr(0, 4) == "DATA") {
          data_idx = static_cast<unsigned int>(fs.tellg());
          if (st.at(1).substr(0, 17) == "binary_compressed")
            data_type = 2;
          else if (st.at(1).substr(0, 6) == "binary")
            data_type = 1;
          continue;
        }
        break;
      }
    } catch (const char *exception) {
      PCL_ERROR("[pcl::PCDReader::readHeader] %s\n", exception);
      return (-1);
    }
    cloud.fields.erase(
        std::remove_if(cloud.fields.begin(), cloud.fields.end(),
                       [](const pcl::PCLPointField &field) -> bool {
                         return field.count < 1;
                       }),
        cloud.fields.end());

    if (nr_points == 0) {
      PCL_WARN("[pcl::PCDReader::readHeader] number of points is zero.\n");
    }

    // Compatibility with older PCD file versions
    if (!width_read && !height_read) {
      cloud.width = nr_points;
      cloud.height = 1;
      cloud.row_step =
          cloud.point_step *
          cloud.width;  // row_step only makes sense for organized datasets
    }
    // assert (cloud.row_step != 0);       // If row_step = 0, either point_step
    // was not set or width is 0

    // if both height/width are not given, assume an unorganized dataset
    if (!height_read) {
      cloud.height = 1;
      PCL_WARN(
          "[pcl::PCDReader::readHeader] no HEIGHT given, setting to 1 "
          "(unorganized).\n");
      if (cloud.width == 0) cloud.width = nr_points;
    } else {
      if (cloud.width == 0 && nr_points != 0) {
        PCL_ERROR(
            "[pcl::PCDReader::readHeader] HEIGHT given (%zu) but no WIDTH!\n",
            static_cast<size_t>(cloud.height));
        return (-1);
      }
    }

    if (cloud.width * cloud.height != nr_points) {
      PCL_ERROR(
          "[pcl::PCDReader::readHeader] HEIGHT (%zu) x WIDTH (%zu) "
          "!= number of points (%zu)\n",
          static_cast<size_t>(cloud.height),
          static_cast<size_t>(cloud.width),
          static_cast<size_t>(nr_points));
      return (-1);
    }

    return (0);
  }

  /** \brief Read a point cloud data header from a PCD file.
   *
   * Load only the meta information (number of points, their types, etc),
   * and not the points themselves, from a given PCD file. Useful for fast
   * evaluation of the underlying data structure.
   *
   * \attention The PCD data is \b always stored in ROW major format! The
   * read/write PCD methods will detect column major input and automatically
   * convert it.
   *
   * \param[in] file_name the name of the file to load
   * \param[out] cloud the resultant point cloud dataset (only these
   *             members will be filled: width, height, point_step,
   *             row_step, fields[]; data is resized but not written)
   * \param[out] origin the sensor acquisition origin (only for > PCD_V7 - null
   * if not present) \param[out] orientation the sensor acquisition orientation
   * (only for > PCD_V7 - identity if not present) \param[out] pcd_version the
   * PCD version of the file (i.e., PCD_V6, PCD_V7) \param[out] data_type the
   * type of data (0 = ASCII, 1 = Binary, 2 = Binary compressed) \param[out]
   * data_idx the offset of cloud data within the file \param[in] offset the
   * offset of where to expect the PCD Header in the file (optional parameter).
   * One usage example for setting the offset parameter is for reading data from
   * a TAR "archive containing multiple PCD files: TAR files always add a 512
   * byte header in front of the actual file, so set the offset to the next byte
   * after the header (e.g., 513).
   *
   * \return
   *  * < 0 (-1) on error
   *  * == 0 on success
   */
  int readHeader(const std::string &file_name, PCLPointCloud2 &cloud,
                 Vector4f &origin, Quaternionf &orientation, int &pcd_version,
                 int &data_type, unsigned int &data_idx,
                 unsigned int offset = 0) {
    if (file_name.empty()) {
      PCL_ERROR("[pcl::PCDReader::readHeader] No file name given!\n");
      return (-1);
    }

    // Open file in binary mode to avoid problem of
    // std::getline() corrupting the result of ifstream::tellg()
    std::ifstream fs;
    fs.open(file_name.c_str(), std::ios::binary);

    if (!fs.good()) {
      PCL_ERROR("[pcl::PCDReader::readHeader] Could not find file '%s'.\n",
                file_name.c_str());
      return (-1);
    }

    if (!fs.is_open() || fs.fail()) {
      PCL_ERROR(
          "[pcl::PCDReader::readHeader] Could not open file '%s'!\n",
          file_name.c_str());
      fs.close();
      return (-1);
    }

    if (fs.peek() == std::ifstream::traits_type::eof()) {
      PCL_ERROR("[pcl::PCDReader::readHeader] File '%s' is empty.\n",
                file_name.c_str());
      fs.close();
      return (-1);
    }

    // Seek at the given offset
    fs.seekg(offset, std::ios::beg);

    // Delegate parsing to the istream overload.
    int result = readHeader(fs, cloud, origin, orientation, pcd_version,
                            data_type, data_idx);

    // Close file
    fs.close();

    return result;
  }

  /** \brief Read a point cloud data header from a PCD file.
   *
   * Load only the meta information (number of points, their types, etc),
   * and not the points themselves, from a given PCD file. Useful for fast
   * evaluation of the underlying data structure.
   *
   * \attention The PCD data is \b always stored in ROW major format! The
   * read/write PCD methods will detect column major input and automatically
   * convert it.
   *
   * \param[in] file_name the name of the file to load
   * \param[out] cloud the resultant point cloud dataset (only these
   *             members will be filled: width, height, point_step,
   *             row_step, fields[]; data is resized but not written)
   * \param[in] offset the offset of where to expect the PCD Header in the
   * file (optional parameter). One usage example for setting the offset
   * parameter is for reading data from a TAR "archive containing multiple
   * PCD files: TAR files always add a 512 byte header in front of the
   * actual file, so set the offset to the next byte after the header
   * (e.g., 513).
   *
   * \return
   *  * < 0 (-1) on error
   *  * == 0 on success
   */
  int readHeader(const std::string &file_name, PCLPointCloud2 &cloud,
                 const unsigned int offset = 0) {
    Vector4f origin = Vector4f::Zero();
    Quaternionf orientation = Quaternionf::Identity();
    int pcd_version = 0;
    int data_type = 0;
    unsigned int data_idx = 0;

    return readHeader(file_name, cloud, origin, orientation, pcd_version,
                      data_type, data_idx, offset);
  }

  /** \brief Read the point cloud data (body) from a PCD stream.
   *
   * Reads the cloud points from a text-formatted stream.  For use after
   * readHeader(), when the resulting data_type == 0.
   *
   * \attention This assumes the stream has been seeked to the position
   * indicated by the data_idx result of readHeader().
   *
   * \param[in] fs the stream from which to read the body.
   * \param[out] cloud the resultant point cloud dataset to be filled.
   * \param[in] pcd_version the PCD version of the stream (from readHeader()).
   *
   * \return
   *  * < 0 (-1) on error
   *  * == 0 on success
   */
  int readBodyASCII(std::istream &fs, PCLPointCloud2 &cloud, int pcd_version) {
    (void)pcd_version;
    // Get the number of points the cloud should have
    uindex_t nr_points = cloud.width * cloud.height;
    // The number of elements each line/point should have
    const unsigned int elems_per_line = std::accumulate(
        cloud.fields.cbegin(), cloud.fields.cend(), 0u,
        [](const auto &i, const auto &field) { return (i + field.count); });
    PCL_DEBUG(
        "[pcl::PCDReader::readBodyASCII] Will check that each line in the PCD "
        "file has %u elements.\n",
        elems_per_line);

    // Setting the is_dense property to true by default
    cloud.is_dense = true;

    uindex_t idx = 0;
    std::string line;
    std::vector<std::string> st;
    std::istringstream is;
    is.imbue(std::locale::classic());

    st.reserve(elems_per_line);

    try {
      while (idx < nr_points && !fs.eof()) {
        getline(fs, line);
        // Ignore empty lines
        if (line.empty()) continue;

        // Tokenize the line
        pcl::split(st, line, "\r\t ");

        if (st.size() !=
            elems_per_line)  // If this is not checked, an exception might occur
                             // while accessing st
        {
          PCL_WARN(
              "[pcl::PCDReader::readBodyASCII] Possibly malformed PCD file: "
              "point number %zu has %zu elements, but should have %u\n",
              static_cast<size_t>(idx + 1), st.size(), elems_per_line);
          ++idx;  // Skip this line/point, but read all others
          continue;
        }

        if (idx >= nr_points) {
          PCL_WARN(
              "[pcl::PCDReader::read] input has more points (%zu) than "
              "advertised (%zu)!\n",
              static_cast<size_t>(idx), static_cast<size_t>(nr_points));
          break;
        }

        std::size_t total = 0;
        // Copy data
        for (unsigned int d = 0;
             d < static_cast<unsigned int>(cloud.fields.size()); ++d) {
          // Ignore invalid padded dimensions that are inherited from binary
          // data
          if (cloud.fields[d].name == "_") {
            total +=
                cloud.fields[d]
                    .count;  // jump over this many elements in the string token
            continue;
          }
          for (uindex_t c = 0; c < cloud.fields[d].count; ++c) {
#define COPY_STRING(CASE_LABEL)                           \
  case CASE_LABEL: {                                      \
    copyStringValue<pcl::traits::asType_t<(CASE_LABEL)>>( \
        st.at(total + c), cloud, idx, d, c, is);          \
    break;                                                \
  }
            switch (cloud.fields[d].datatype) {
              //COPY_STRING(pcl::PCLPointField::BOOL)
              COPY_STRING(pcl::PCLPointField::INT8)
              COPY_STRING(pcl::PCLPointField::UINT8)
              COPY_STRING(pcl::PCLPointField::INT16)
              COPY_STRING(pcl::PCLPointField::UINT16)
              COPY_STRING(pcl::PCLPointField::INT32)
              COPY_STRING(pcl::PCLPointField::UINT32)
              COPY_STRING(pcl::PCLPointField::INT64)
              COPY_STRING(pcl::PCLPointField::UINT64)
              COPY_STRING(pcl::PCLPointField::FLOAT32)
              COPY_STRING(pcl::PCLPointField::FLOAT64)
              default:
                PCL_WARN(
                    "[pcl::PCDReader::read] Incorrect field data type "
                    "specified (%d)!\n",
                    static_cast<int>(cloud.fields[d].datatype));
                break;
            }
#undef COPY_STRING
          }
          total +=
              cloud.fields[d]
                  .count;  // jump over this many elements in the string token
        }
        idx++;
      }
    } catch (const char *exception) {
      PCL_ERROR("[pcl::PCDReader::read] %s\n", exception);
      return (-1);
    }

    if (idx != nr_points) {
      PCL_ERROR(
          "[pcl::PCDReader::read] Number of points read (%zu) is different than "
          "expected (%zu)\n",
          static_cast<size_t>(idx), static_cast<size_t>(nr_points));
      return (-1);
    }

    return (0);
  }

  /** \brief Read the point cloud data (body) from a block of memory.
   *
   * Reads the cloud points from a binary-formatted memory block.  For use
   * after readHeader(), when the resulting data_type is nonzero.
   *
   * \param[in] map the memory location from which to read the body.
   * \param[out] cloud the resultant point cloud dataset to be filled.
   * \param[in] pcd_version the PCD version of the stream (from readHeader()).
   * \param[in] compressed indicates whether the PCD block contains compressed
   * data.  This should be true if the data_type returned by readHeader() == 2.
   * \param[in] data_idx the offset of the body, as reported by readHeader().
   *
   * \return
   *  * < 0 (-1) on error
   *  * == 0 on success
   */
  int readBodyBinary(const unsigned char *map, PCLPointCloud2 &cloud,
                     int pcd_version, bool compressed, unsigned int data_idx) {
    (void)pcd_version;
    // Setting the is_dense property to true by default
    cloud.is_dense = true;

    /// ---[ Binary compressed mode only
    if (compressed) {
      // Uncompress the data first
      unsigned int compressed_size = 0, uncompressed_size = 0;
      memcpy(&compressed_size, &map[data_idx + 0], 4);
      memcpy(&uncompressed_size, &map[data_idx + 4], 4);
      PCL_DEBUG(
          "[pcl::PCDReader::read] Read a binary compressed file with %u bytes "
          "compressed and %u original.\n",
          compressed_size, uncompressed_size);

      if (uncompressed_size != cloud.data.size()) {
        PCL_WARN(
            "[pcl::PCDReader::read] The estimated cloud.data size (%zu) is "
            "different than the saved uncompressed value (%u)! Data "
            "corruption?\n",
            cloud.data.size(), uncompressed_size);
        cloud.data.resize(uncompressed_size);
      }

      auto data_size = static_cast<unsigned int>(cloud.data.size());
      if (data_size == 0) {
        PCL_WARN(
            "[pcl::PCDReader::read] Binary compressed file has data size of "
            "zero.\n");
        return 0;
      }
      std::vector<char> buf(data_size);
      // The size of the uncompressed data better be the same as what we stored
      // in the header
      unsigned int tmp_size = pcl::lzfDecompress(
          &map[data_idx + 8], compressed_size, buf.data(), data_size);
      if (tmp_size != uncompressed_size) {
        PCL_ERROR(
            "[pcl::PCDReader::read] Size of decompressed lzf data (%u) does "
            "not match value stored in PCD header (%u). Errno: %d\n",
            tmp_size, uncompressed_size, errno);
        return (-1);
      }

      // Get the fields sizes
      std::vector<pcl::PCLPointField> fields(cloud.fields.size());
      std::vector<pcl::uindex_t> fields_sizes(cloud.fields.size());
      std::size_t nri = 0;
      pcl::uindex_t fsize = 0;
      for (const auto &field : cloud.fields) {
        if (field.name == "_") continue;
        fields_sizes[nri] = field.count * pcl::getFieldSize(field.datatype);
        fsize += fields_sizes[nri];
        fields[nri] = field;
        ++nri;
      }
      fields.resize(nri);
      fields_sizes.resize(nri);

      // Unpack the xxyyzz to xyz
      std::vector<char *> pters(fields.size());
      std::size_t toff = 0;
      for (std::size_t i = 0; i < pters.size(); ++i) {
        pters[i] = &buf[toff];
        toff += fields_sizes[i] * cloud.width * cloud.height;
      }
      // Copy it to the cloud
      for (uindex_t i = 0; i < cloud.width * cloud.height; ++i) {
        for (std::size_t j = 0; j < pters.size(); ++j) {
          memcpy(&cloud.data[i * fsize + fields[j].offset], pters[j],
                 fields_sizes[j]);
          // Increment the pointer
          pters[j] += fields_sizes[j];
        }
      }
      // memcpy (&cloud.data[0], &buf[0], uncompressed_size);
    } else
      // Copy the data
      memcpy((cloud.data).data(), &map[0] + data_idx, cloud.data.size());

    // Extra checks (not needed for ASCII)
    uindex_t point_size = (cloud.width * cloud.height == 0)
                          ? 0
                          : (static_cast<uindex_t>(cloud.data.size()) /
                             (cloud.height * cloud.width));
    // Once copied, we need to go over each field and check if it has NaN/Inf
    // values and assign cloud.is_dense to true or false
    for (uindex_t i = 0; i < cloud.width * cloud.height; ++i) {
      for (uindex_t d = 0;
           d < static_cast<uindex_t>(cloud.fields.size()); ++d) {
        for (uindex_t c = 0; c < cloud.fields[d].count; ++c) {
#define SET_CLOUD_DENSE(CASE_LABEL)                                            \
  case CASE_LABEL: {                                                           \
    if (!isValueFinite<pcl::traits::asType_t<(CASE_LABEL)>>(cloud, i,          \
                                                            point_size, d, c)) \
      cloud.is_dense = false;                                                  \
    break;                                                                     \
  }
          switch (cloud.fields[d].datatype) {
            SET_CLOUD_DENSE(pcl::PCLPointField::BOOL)
            SET_CLOUD_DENSE(pcl::PCLPointField::INT8)
            SET_CLOUD_DENSE(pcl::PCLPointField::UINT8)
            SET_CLOUD_DENSE(pcl::PCLPointField::INT16)
            SET_CLOUD_DENSE(pcl::PCLPointField::UINT16)
            SET_CLOUD_DENSE(pcl::PCLPointField::INT32)
            SET_CLOUD_DENSE(pcl::PCLPointField::UINT32)
            SET_CLOUD_DENSE(pcl::PCLPointField::INT64)
            SET_CLOUD_DENSE(pcl::PCLPointField::UINT64)
            SET_CLOUD_DENSE(pcl::PCLPointField::FLOAT32)
            SET_CLOUD_DENSE(pcl::PCLPointField::FLOAT64)
            default:
              PCL_ERROR("[pcl::PCDReader::read] Unknown data type %d.\n",
                        static_cast<int>(cloud.fields[d].datatype));
              return -1;
            break;
          }
#undef SET_CLOUD_DENSE
        }
      }
    }

    return (0);
  }

  /** \brief Read a point cloud data from a PCD file and store it into a
   * pcl/PCLPointCloud2.
   *
   * \param[in] file_name the name of the file containing
   * the actual PointCloud data
   * \param[out] cloud the resultant PointCloud
   * message read from disk
   * \param[out] origin the sensor acquisition origin
   * (only for > PCD_V7 - null if not present)
   * \param[out] orientation the
   * sensor acquisition orientation (only for > PCD_V7 - identity if not
   * present)
   * \param[out] pcd_version the PCD version of the file (either PCD_V6
   * or PCD_V7)
   * \param[in] offset the offset of where to expect the PCD Header
   * in the file (optional parameter). One usage example for setting the offset
   * parameter is for reading data from a TAR "archive containing multiple
   * PCD files: TAR files always add a 512 byte header in front of the
   * actual file, so set the offset to the next byte after the header
   * (e.g., 513).
   *
   * \return
   *  * < 0 (-1) on error
   *  * == 0 on success
   */
  int read(const std::string &file_name, PCLPointCloud2 &cloud,
           Vector4f &origin, Quaternionf &orientation, int &pcd_version,
           const unsigned int offset = 0) {
    pcl::console::TicToc tt;
    tt.tic();

    if (file_name.empty()) {
      PCL_ERROR("[pcl::PCDReader::read] No file name given!\n");
      return (-1);
    }

    // @brief modified by nick.liao
    // if (!pcl_fs::exists(file_name)) {
    //   PCL_ERROR("[pcl::PCDReader::read] Could not find file '%s'.\n",
    //             file_name.c_str());
    //   return (-1);
    // }

    int data_type;
    unsigned int data_idx;

    int res = readHeader(file_name, cloud, origin, orientation, pcd_version,
                         data_type, data_idx, offset);

    if (res < 0) return (res);

    // if ascii
    if (data_type == 0) {
      // Re-open the file (readHeader closes it)
      std::ifstream fs;
      fs.open(file_name.c_str());
      if (!fs.is_open() || fs.fail()) {
        PCL_ERROR("[pcl::PCDReader::read] Could not open file %s.\n",
                  file_name.c_str());
        return (-1);
      }

      fs.seekg(data_idx + offset);

      // Read the rest of the file
      res = readBodyASCII(fs, cloud, pcd_version);

      // Close file
      fs.close();
    } else
    /// ---[ Binary mode only
    /// We must re-open the file and read with mmap () for binary
    {
      // Open for reading
      int fd = io::raw_open(file_name.c_str(), O_RDONLY);
      if (fd == -1) {
        PCL_ERROR("[pcl::PCDReader::read] Failure to open file %s\n",
                  file_name.c_str());
        return (-1);
      }

      // Infer file size
      long end_position = io::raw_lseek(fd, 0, SEEK_END);
      if (end_position < 0)
      {
        PCL_ERROR("[pcl::PCDReader::read] lseek errno: %d\n",
                  errno);
        PCL_ERROR("[pcl::PCDReader::read] Error during lseek ()!\n");
        io::raw_close(fd);
        return -1;
      }
      std::size_t file_size = static_cast<size_t>(end_position);
      io::raw_lseek(fd, 0, SEEK_SET);

      std::size_t mmap_size =
          offset + data_idx;  // ...because we mmap from the start of the file.
      if (data_type == 2) {
        // Seek to real start of data.
        if (offset + data_idx > static_cast<unsigned int>(LONG_MAX))
        {
          PCL_ERROR("[pcl::PCDReader::read] File is too big.\n");
          io::raw_close(fd);
          return -1;
        }
        long data_offset = static_cast<long>(offset + data_idx);
        long result = io::raw_lseek(fd, data_offset, SEEK_SET);
        if (result < 0) {
          io::raw_close(fd);
          PCL_ERROR("[pcl::PCDReader::read] lseek errno: %d\n",
                    errno);
          PCL_ERROR("[pcl::PCDReader::read] Error during lseek ()!\n");
          return (-1);
        }

        // Read compressed size to compute how much must be mapped
        unsigned int compressed_size = 0;
        ssize_t num_read = io::raw_read(fd, &compressed_size, 4);
        if (num_read < 0) {
          io::raw_close(fd);
          PCL_ERROR("[pcl::PCDReader::read] read errno: %d\n",
                    errno);
          PCL_ERROR("[pcl::PCDReader::read] Error during read()!\n");
          return (-1);
        }
        mmap_size += compressed_size;
        // Add the 8 bytes used to store the compressed and uncompressed size
        mmap_size += 8;

        // Reset position
        io::raw_lseek(fd, 0, SEEK_SET);
      } else {
        mmap_size += cloud.data.size();
      }

      if (mmap_size > file_size) {
        io::raw_close(fd);
        PCL_ERROR(
            "[pcl::PCDReader::read] Corrupted PCD file. The file is smaller "
            "than expected!\n");
        return (-1);
      }

      // Prepare the map
#ifdef _WIN32
      // As we don't know the real size of data (compressed or not),
      // we set dwMaximumSizeHigh = dwMaximumSizeLow = 0 so as to map the whole
      // file
      HANDLE fm = CreateFileMapping(
          reinterpret_cast<HANDLE>(_get_osfhandle(fd)), NULL,
          PAGE_READONLY, 0, 0, NULL);
      // As we don't know the real size of data (compressed or not),
      // we set dwNumberOfBytesToMap = 0 so as to map the whole file
      unsigned char *map = static_cast<unsigned char *>(
          MapViewOfFile(fm, FILE_MAP_READ, 0, 0, 0));
      if (map == NULL) {
        CloseHandle(fm);
        io::raw_close(fd);
        PCL_ERROR("[pcl::PCDReader::read] Error mapping view of file, %s\n",
                  file_name.c_str());
        return (-1);
      }
#else
      auto *map = static_cast<unsigned char *>(
          ::mmap(nullptr, mmap_size, PROT_READ, MAP_SHARED, fd, 0));
      if (map == reinterpret_cast<unsigned char *>(-1))  // MAP_FAILED
      {
        io::raw_close(fd);
        PCL_ERROR(
            "[pcl::PCDReader::read] Error preparing mmap for binary PCD "
            "file.\n");
        return (-1);
      }
#endif

      res = readBodyBinary(map, cloud, pcd_version, data_type == 2,
                           offset + data_idx);

      // Unmap the pages of memory
#ifdef _WIN32
      UnmapViewOfFile(map);
      CloseHandle(fm);
#else
      if (::munmap(map, mmap_size) == -1) {
        io::raw_close(fd);
        PCL_ERROR("[pcl::PCDReader::read] Munmap failure\n");
        return (-1);
      }
#endif
      io::raw_close(fd);
    }
    double total_time = tt.toc();
    PCL_DEBUG(
        "[pcl::PCDReader::read] Loaded %s as a %s cloud in %g ms with %zu "
        "points. Available dimensions: %s.\n",
        file_name.c_str(), (cloud.is_dense ? "dense" : "non-dense"), total_time,
        static_cast<size_t>(cloud.width * cloud.height),
        pcl::getFieldsList(cloud).c_str());
    return res;
  }

  /** \brief Read a point cloud data from a PCD (PCD_V6) and store it into a
   * pcl/PCLPointCloud2.
   *
   * \note This function is provided for backwards compatibility only and
   * it can only read PCD_V6 files correctly, as pcl::PCLPointCloud2
   * does not contain a sensor origin/orientation. Reading any file
   * > PCD_V6 will generate a warning.
   *
   * \param[in] file_name the name of the file containing the actual PointCloud
   * data
   * \param[out] cloud the resultant PointCloud message read from disk
   * \param[in] offset the offset of where to expect the PCD Header in the
   * file (optional parameter). One usage example for setting the offset
   * parameter is for reading data from a TAR "archive containing multiple
   * PCD files: TAR files always add a 512 byte header in front of the
   * actual file, so set the offset to the next byte after the header
   * (e.g., 513).
   *
   * \return
   *  * < 0 (-1) on error
   *  * == 0 on success
   */
  int read(const std::string &file_name, PCLPointCloud2 &cloud,
           const unsigned int offset = 0) {
    int pcd_version;
    Vector4f origin;
    Quaternionf orientation;
    // Load the data
    int res = read(file_name, cloud, origin, orientation, pcd_version, offset);

    if (res < 0) return (res);

    return (0);
  }
};

/*******************************************************************************
 * PCDWriter
 *******************************************************************************
 */
class PCDWriter {
 public:
  PCDWriter() = default;
  ~PCDWriter() = default;

  /** \brief Set whether mmap() synchornization via msync() is desired before
   * munmap() calls. Setting this to true could prevent NFS data loss (see
   * http://www.pcl-developers.org/PCD-IO-consistency-on-NFS-msync-needed-td4885942.html).
   * Default: false
   * \note This option should be used by advanced users only!
   * \note Please note that using msync() on certain systems can reduce the I/O
   * performance by up to 80%! \param[in] sync set to true if msync() should be
   * called before munmap()
   */
  void setMapSynchronization(bool sync) { map_synchronization_ = sync; }

  /** \brief Generate the header of a PCD file format
   * \param[in] cloud the point cloud data message
   * \param[in] origin the sensor acquisition origin
   * \param[in] orientation the sensor acquisition orientation
   */
  std::string generateHeaderBinary(const pcl::PCLPointCloud2 &cloud,
                                   const Vector4f &origin,
                                   const Quaternionf &orientation) {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());

    oss << "# .PCD v0.7 - Point Cloud Data file format"
           "\nVERSION 0.7"
           "\nFIELDS";

    auto fields = cloud.fields;
    std::sort(fields.begin(), fields.end(),
              [](const auto &field_a, const auto &field_b) {
                return field_a.offset < field_b.offset;
              });

    // Compute the total size of the fields
    pcl::uindex_t fsize = 0;
    for (const auto &field : fields)
      fsize += field.count * getFieldSize(field.datatype);

    // The size of the fields cannot be larger than point_step
    if (fsize > cloud.point_step) {
      PCL_ERROR(
          "[pcl::PCDWriter::generateHeaderBinary] The size of the fields (%zu) "
          "is larger than point_step (%zu)! Something is wrong here...\n",
          static_cast<size_t>(fsize), static_cast<size_t>(cloud.point_step));
      return ("");
    }

    std::stringstream field_names, field_types, field_sizes, field_counts;
    // Check if the size of the fields is smaller than the size of the point
    // step
    pcl::uindex_t toffset = 0;
    for (std::size_t i = 0; i < fields.size(); ++i) {
      // If field offsets do not match, then we need to create fake fields
      if (toffset != fields[i].offset) {
        // If we're at the last "valid" field
        uindex_t fake_offset =
            (i == 0) ?
                     // Use the current_field offset
                (fields[i].offset)
                     :
                     // Else, do cur_field.offset - prev_field.offset + sizeof
                     // (prev_field)
                (fields[i].offset -
                 (fields[i - 1].offset +
                  fields[i - 1].count * getFieldSize(fields[i - 1].datatype)));

        toffset += fake_offset;

        field_names << " _";  // By convention, _ is an invalid field name
        field_sizes << " 1";  // Make size = 1
        field_types << " U";  // Field type = unsigned byte (uint8)
        field_counts << " " << fake_offset;
      }

      // Add the regular dimension
      toffset += fields[i].count * getFieldSize(fields[i].datatype);
      field_names << " " << fields[i].name;
      field_sizes << " " << pcl::getFieldSize(fields[i].datatype);
      field_types << " " << pcl::getFieldType(fields[i].datatype);
      int count = std::abs(static_cast<int>(fields[i].count));
      if (count == 0)
        count = 1;  // check for 0 counts (coming from older converter code)
      field_counts << " " << count;
    }
    // Check extra padding
    if (toffset < cloud.point_step) {
      field_names << " _";  // By convention, _ is an invalid field name
      field_sizes << " 1";  // Make size = 1
      field_types << " U";  // Field type = unsigned byte (uint8)
      field_counts << " " << (cloud.point_step - toffset);
    }
    oss << field_names.str();
    oss << "\nSIZE" << field_sizes.str() << "\nTYPE" << field_types.str()
        << "\nCOUNT" << field_counts.str();
    oss << "\nWIDTH " << cloud.width << "\nHEIGHT " << cloud.height << "\n";

    oss << "VIEWPOINT " << origin[0] << " " << origin[1] << " " << origin[2]
        << " " << orientation.w() << " " << orientation.x() << " "
        << orientation.y() << " " << orientation.z() << "\n";

    oss << "POINTS " << cloud.width * cloud.height << "\n";

    return (oss.str());
  }

  /** \brief Generate the header of a BINARY_COMPRESSED PCD file format
   * \param[out] os the stream into which to write the header
   * \param[in] cloud the point cloud data message
   * \param[in] origin the sensor acquisition origin
   * \param[in] orientation the sensor acquisition orientation
   *
   * \return
   *  * < 0 (-1) on error
   *  * == 0 on success
   */
  int generateHeaderBinaryCompressed(std::ostream &os,
                                     const pcl::PCLPointCloud2 &cloud,
                                     const Vector4f &origin,
                                     const Quaternionf &orientation) {
    os.imbue(std::locale::classic());

    os << "# .PCD v0.7 - Point Cloud Data file format"
          "\nVERSION 0.7"
          "\nFIELDS";

    // Compute the total size of the fields
    pcl::uindex_t fsize = 0;
    for (const auto &field : cloud.fields)
      fsize += field.count * getFieldSize(field.datatype);

    // The size of the fields cannot be larger than point_step
    if (fsize > cloud.point_step) {
      PCL_ERROR(
          "[pcl::PCDWriter::generateHeaderBinaryCompressed] The size of the "
          "fields (%zu) is larger than point_step (%zu)! Something is wrong "
          "here...\n",
          static_cast<size_t>(fsize), static_cast<size_t>(cloud.point_step));
      return (-1);
    }

    std::stringstream field_names, field_types, field_sizes, field_counts;
    // Check if the size of the fields is smaller than the size of the point
    // step
    for (const auto &field : cloud.fields) {
      if (field.name == "_") continue;
      // Add the regular dimension
      field_names << " " << field.name;
      field_sizes << " " << pcl::getFieldSize(field.datatype);
      field_types << " " << pcl::getFieldType(field.datatype);
      int count = std::abs(static_cast<int>(field.count));
      if (count == 0)
        count = 1;  // check for 0 counts (coming from older converter code)
      field_counts << " " << count;
    }
    os << field_names.str();
    os << "\nSIZE" << field_sizes.str() << "\nTYPE" << field_types.str()
       << "\nCOUNT" << field_counts.str();
    os << "\nWIDTH " << cloud.width << "\nHEIGHT " << cloud.height << "\n";

    os << "VIEWPOINT " << origin[0] << " " << origin[1] << " " << origin[2]
       << " " << orientation.w() << " " << orientation.x() << " "
       << orientation.y() << " " << orientation.z() << "\n";

    os << "POINTS " << cloud.width * cloud.height << "\n";

    return (os ? 0 : -1);
  }

  /** \brief Generate the header of a BINARY_COMPRESSED PCD file format
   * \param[in] cloud the point cloud data message
   * \param[in] origin the sensor acquisition origin
   * \param[in] orientation the sensor acquisition orientation
   */
  std::string generateHeaderBinaryCompressed(const pcl::PCLPointCloud2 &cloud,
                                             const Vector4f &origin,
                                             const Quaternionf &orientation) {
    std::ostringstream oss;
    generateHeaderBinaryCompressed(oss, cloud, origin, orientation);
    return oss.str();
  }

  /** \brief Generate the header of a PCD file format
   * \param[in] cloud the point cloud data message
   * \param[in] origin the sensor acquisition origin
   * \param[in] orientation the sensor acquisition orientation
   */
  std::string generateHeaderASCII(const pcl::PCLPointCloud2 &cloud,
                                  const Vector4f &origin,
                                  const Quaternionf &orientation) {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());

    oss << "# .PCD v0.7 - Point Cloud Data file format"
           "\nVERSION 0.7"
           "\nFIELDS ";

    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    std::string result;

    for (std::size_t d = 0; d < cloud.fields.size() - 1; ++d) {
      // Ignore invalid padded dimensions that are inherited from binary data
      if (cloud.fields[d].name != "_") result += cloud.fields[d].name + " ";
    }
    // Ignore invalid padded dimensions that are inherited from binary data
    if (cloud.fields[cloud.fields.size() - 1].name != "_")
      result += cloud.fields[cloud.fields.size() - 1].name;

    // Remove trailing spaces
    trim(result);
    oss << result << "\nSIZE ";

    stream.str("");
    // Write the SIZE of each field
    for (std::size_t d = 0; d < cloud.fields.size() - 1; ++d) {
      // Ignore invalid padded dimensions that are inherited from binary data
      if (cloud.fields[d].name != "_")
        stream << pcl::getFieldSize(cloud.fields[d].datatype) << " ";
    }
    // Ignore invalid padded dimensions that are inherited from binary data
    if (cloud.fields[cloud.fields.size() - 1].name != "_")
      stream << pcl::getFieldSize(
          cloud.fields[cloud.fields.size() - 1].datatype);

    // Remove trailing spaces
    result = stream.str();
    trim(result);
    oss << result << "\nTYPE ";

    stream.str("");
    // Write the TYPE of each field
    for (std::size_t d = 0; d < cloud.fields.size() - 1; ++d) {
      // Ignore invalid padded dimensions that are inherited from binary data
      if (cloud.fields[d].name != "_") {
        if (cloud.fields[d].name == "rgb")
          stream << "U ";
        else
          stream << pcl::getFieldType(cloud.fields[d].datatype) << " ";
      }
    }
    // Ignore invalid padded dimensions that are inherited from binary data
    if (cloud.fields[cloud.fields.size() - 1].name != "_") {
      if (cloud.fields[cloud.fields.size() - 1].name == "rgb")
        stream << "U";
      else
        stream << pcl::getFieldType(
            cloud.fields[cloud.fields.size() - 1].datatype);
    }

    // Remove trailing spaces
    result = stream.str();
    trim(result);
    oss << result << "\nCOUNT ";

    stream.str("");
    // Write the TYPE of each field
    for (std::size_t d = 0; d < cloud.fields.size() - 1; ++d) {
      // Ignore invalid padded dimensions that are inherited from binary data
      if (cloud.fields[d].name == "_") continue;
      int count = std::abs(static_cast<int>(cloud.fields[d].count));
      if (count == 0)
        count = 1;  // we simply cannot tolerate 0 counts (coming from older
                    // converter code)

      stream << count << " ";
    }
    // Ignore invalid padded dimensions that are inherited from binary data
    if (cloud.fields[cloud.fields.size() - 1].name != "_") {
      int count = std::abs(
          static_cast<int>(cloud.fields[cloud.fields.size() - 1].count));
      if (count == 0) count = 1;

      stream << count;
    }

    // Remove trailing spaces
    result = stream.str();
    trim(result);
    oss << result << "\nWIDTH " << cloud.width << "\nHEIGHT " << cloud.height
        << "\n";

    oss << "VIEWPOINT " << origin[0] << " " << origin[1] << " " << origin[2]
        << " " << orientation.w() << " " << orientation.x() << " "
        << orientation.y() << " " << orientation.z() << "\n";

    oss << "POINTS " << cloud.width * cloud.height << "\n";

    return (oss.str());
  }

  /** \brief Save point cloud data to a PCD file containing n-D points, in ASCII
   * format
   * \param[in] file_name the output file name
   * \param[in] cloud the point cloud data message
   * \param[in] origin the sensor acquisition origin
   * \param[in] orientation the sensor acquisition orientation
   * \param[in] precision the specified output numeric stream precision
   * (default: 8)
   *
   * Caution: PointCloud structures containing an RGB field have
   * traditionally used packed float values to store RGB data. Storing a
   * float as ASCII can introduce variations to the smallest bits, and
   * thus significantly alter the data. This is a known issue, and the fix
   * involves switching RGB data to be stored as a packed integer in
   * future versions of PCL.
   *
   * As an intermediary solution, precision 8 is used, which guarantees lossless
   * storage for RGB.
   */
  int writeASCII(const std::string &file_name, const pcl::PCLPointCloud2 &cloud,
                 const Vector4f &origin = Vector4f::Zero(),
                 const Quaternionf &orientation = Quaternionf::Identity(),
                 const int precision = 8) {
    if (cloud.data.empty()) {
      PCL_WARN("[pcl::PCDWriter::writeASCII] Input point cloud has no data!\n");
    }
    if (cloud.fields.empty()) {
      PCL_ERROR(
          "[pcl::PCDWriter::writeASCII] Input point cloud has no field "
          "data!\n");
      return (-1);
    }

    std::ofstream fs;
    fs.precision(precision);
    fs.imbue(std::locale::classic());
    fs.open(file_name.c_str(), std::ios::binary);  // Open file
    if (!fs.is_open() || fs.fail()) {
      PCL_ERROR(
          "[pcl::PCDWriter::writeASCII] Could not open file '%s' for writing!"
          "\n", file_name.c_str());
      return (-1);
    }
    // Mandatory lock file
    // boost::interprocess::file_lock file_lock;
    // setLockingPermissions(file_name, file_lock);

    pcl::uindex_t nr_points = cloud.width * cloud.height;
    pcl::uindex_t point_size = (nr_points == 0) ? 0 :
                      static_cast<pcl::uindex_t>(cloud.data.size()) / nr_points;

    // Write the header information
    fs << generateHeaderASCII(cloud, origin, orientation) << "DATA ascii\n";

    std::ostringstream stream;
    stream.precision(precision);
    stream.imbue(std::locale::classic());

    // Iterate through the points
    for (pcl::uindex_t i = 0; i < nr_points; ++i) {
      for (pcl::uindex_t d = 0;
           d < static_cast<pcl::uindex_t>(cloud.fields.size()); ++d) {
        // Ignore invalid padded dimensions that are inherited from binary data
        if (cloud.fields[d].name == "_") continue;

        pcl::uindex_t count = cloud.fields[d].count;
        if (count == 0)
          count = 1;  // we simply cannot tolerate 0 counts (coming from older
                      // converter code)

        for (pcl::uindex_t c = 0; c < count; ++c) {
#define COPY_VALUE(CASE_LABEL)                                                 \
  case CASE_LABEL: {                                                           \
    copyValueString<pcl::traits::asType_t<(CASE_LABEL)>>(cloud, i, point_size, \
                                                         d, c, stream);        \
    break;                                                                     \
  }
          switch (cloud.fields[d].datatype) {
            COPY_VALUE(pcl::PCLPointField::BOOL)
            COPY_VALUE(pcl::PCLPointField::INT8)
            COPY_VALUE(pcl::PCLPointField::UINT8)
            COPY_VALUE(pcl::PCLPointField::INT16)
            COPY_VALUE(pcl::PCLPointField::UINT16)
            COPY_VALUE(pcl::PCLPointField::INT32)
            COPY_VALUE(pcl::PCLPointField::UINT32)
            COPY_VALUE(pcl::PCLPointField::INT64)
            COPY_VALUE(pcl::PCLPointField::UINT64)
            COPY_VALUE(pcl::PCLPointField::FLOAT64)

            case pcl::PCLPointField::FLOAT32: {
              /*
               * Despite the float type, store the rgb field as uint32
               * because several fully opaque color values are mapped to
               * nan.
               */
              if ("rgb" == cloud.fields[d].name)
                copyValueString<
                    pcl::traits::asType_t<pcl::PCLPointField::UINT32>>(
                    cloud, i, point_size, d, c, stream);
              else
                copyValueString<
                    pcl::traits::asType_t<pcl::PCLPointField::FLOAT32>>(
                    cloud, i, point_size, d, c, stream);
              break;
            }
            default:
              PCL_WARN(
                  "[pcl::PCDWriter::writeASCII] Incorrect field data type "
                  "specified "
                  "(%d)!\n",
                  cloud.fields[d].datatype);
              break;
          }
#undef COPY_VALUE

          if ((d < cloud.fields.size() - 1) ||
              (c < cloud.fields[d].count - 1))
            stream << " ";
        }
      }
      // Copy the stream, trim it, and write it to disk
      std::string result = stream.str();
      trim(result);
      stream.str("");
      fs << result << "\n";
    }
    fs.close();  // Close file
    // resetLockingPermissions(file_name, file_lock);
    return (0);
  }

  /** \brief Save point cloud data to a PCD file containing n-D points, in
   * BINARY format \param[in] file_name the output file name \param[in] cloud
   * the point cloud data message \param[in] origin the sensor acquisition
   * origin \param[in] orientation the sensor acquisition orientation
   */
  int writeBinary(const std::string &file_name,
                  const pcl::PCLPointCloud2 &cloud,
                  const Vector4f &origin = Vector4f::Zero(),
                  const Quaternionf &orientation = Quaternionf::Identity()) {
    if (cloud.data.empty()) {
      PCL_WARN(
          "[pcl::PCDWriter::writeBinary] Input point cloud has no data!\n");
    }
    if (cloud.fields.empty()) {
      PCL_ERROR(
          "[pcl::PCDWriter::writeBinary] Input point cloud has no field "
          "data!\n");
      return (-1);
    }

    std::ostringstream oss;
    oss.imbue(std::locale::classic());

    oss << generateHeaderBinary(cloud, origin, orientation) << "DATA binary\n";
    oss.flush();
    const auto data_idx = static_cast<unsigned int>(oss.tellp());

#ifdef _WIN32
#ifdef UNICODE
    std::wstring w_file_name(file_name.begin(), file_name.end());
    HANDLE h_native_file =
        CreateFile(w_file_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
    HANDLE h_native_file =
        CreateFile(file_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
    if (h_native_file == INVALID_HANDLE_VALUE) {
      PCL_ERROR("[pcl::PCDWriter::writeBinary] Error during CreateFile (%s)!\n",
                file_name.c_str());
      return (-1);
    }
    // Mandatory lock file
    // boost::interprocess::file_lock file_lock;
    // setLockingPermissions(file_name, file_lock);

#else
    int fd = io::raw_open(file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC,
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
      PCL_ERROR("[pcl::PCDWriter::writeBinary] Error during open (%s)!\n",
                file_name.c_str());
      return (-1);
    }
    // Mandatory lock file
    // boost::interprocess::file_lock file_lock;
    // setLockingPermissions(file_name, file_lock);

    // Stretch the file size to the size of the data
    long result =
        io::raw_lseek(fd, getpagesize() + cloud.data.size() - 1, SEEK_SET);
    if (result < 0) {
      io::raw_close(fd);
      // resetLockingPermissions(file_name, file_lock);
      PCL_ERROR("[pcl::PCDWriter::writeBinary] lseek errno: %d\n",
                errno);
      PCL_ERROR("[pcl::PCDWriter::writeBinary] Error during lseek ()!\n");
      return (-1);
    }
    // Write a bogus entry so that the new file size comes in effect
    result = static_cast<int>(::write(fd, "", 1));
    if (result != 1) {
      io::raw_close(fd);
      // resetLockingPermissions(file_name, file_lock);
      PCL_ERROR("[pcl::PCDWriter::writeBinary] Error during write ()!\n");
      return (-1);
    }
#endif
    // Prepare the map
#ifdef _WIN32
    HANDLE fm = CreateFileMapping(h_native_file, NULL, PAGE_READWRITE,
                                  static_cast<DWORD>((data_idx +
                                                      cloud.data.size()) >> 32),
                                  static_cast<DWORD>(data_idx +
                                                     cloud.data.size()), NULL);
    char *map =
        static_cast<char *>(MapViewOfFile(fm, FILE_MAP_READ | FILE_MAP_WRITE, 0,
                                          0, data_idx + cloud.data.size()));
    CloseHandle(fm);

#else
    char *map = static_cast<char *>(
        mmap(nullptr, static_cast<std::size_t>(data_idx + cloud.data.size()),
             PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (map == reinterpret_cast<char *>(-1))  // MAP_FAILED
    {
      io::raw_close(fd);
      // resetLockingPermissions(file_name, file_lock);
      PCL_ERROR("[pcl::PCDWriter::writeBinary] Error during mmap ()!\n");
      return (-1);
    }
#endif

    // copy header
    memcpy(&map[0], oss.str().c_str(), static_cast<std::size_t>(data_idx));

    // Copy the data
    memcpy(&map[0] + data_idx, cloud.data.data(), cloud.data.size());

#ifndef _WIN32
    // If the user set the synchronization flag on, call msync
    if (map_synchronization_)
      msync(map, static_cast<std::size_t>(data_idx + cloud.data.size()),
            MS_SYNC);
#endif

      // Unmap the pages of memory
#ifdef _WIN32
    UnmapViewOfFile(map);
#else
    if (::munmap(map, static_cast<std::size_t>(data_idx + cloud.data.size())) ==
        -1) {
      io::raw_close(fd);
      // resetLockingPermissions(file_name, file_lock);
      PCL_ERROR("[pcl::PCDWriter::writeBinary] Error during munmap ()!\n");
      return (-1);
    }
#endif
    // Close file
#ifdef _WIN32
    CloseHandle(h_native_file);
#else
    io::raw_close(fd);
#endif
    // resetLockingPermissions(file_name, file_lock);
    return (0);
  }

  /** \brief Save point cloud data to a std::ostream containing n-D points, in
   * BINARY format \param[out] os the stream into which to write the data
   * \param[in] cloud the point cloud data message
   * \param[in] origin the sensor acquisition origin
   * \param[in] orientation the sensor acquisition orientation
   */
  int writeBinary(std::ostream &os, const pcl::PCLPointCloud2 &cloud,
                  const Vector4f &origin = Vector4f::Zero(),
                  const Quaternionf &orientation = Quaternionf::Identity()) {
    if (cloud.data.empty()) {
      PCL_WARN(
          "[pcl::PCDWriter::writeBinary] Input point cloud has no data!\n");
    }
    if (cloud.fields.empty()) {
      PCL_ERROR(
          "[pcl::PCDWriter::writeBinary] Input point cloud has no field "
          "data!\n");
      return (-1);
    }

    os.imbue(std::locale::classic());
    os << generateHeaderBinary(cloud, origin, orientation) << "DATA binary\n";
    std::copy(cloud.data.cbegin(), cloud.data.cend(),
              std::ostream_iterator<char>(os));
    os.flush();

    return (os ? 0 : -1);
  }

  /** \brief Save point cloud data to a PCD file containing n-D points, in
   * BINARY_COMPRESSED format \param[in] file_name the output file name
   * \param[in] cloud the point cloud data message
   * \param[in] origin the sensor acquisition origin
   * \param[in] orientation the sensor acquisition orientation
   * \return
   * (-1) for a general error
   * (-2) if the input cloud is too large for the file format
   * 0 on success
   */
  int writeBinaryCompressed(
      const std::string &file_name, const pcl::PCLPointCloud2 &cloud,
      const Vector4f &origin = Vector4f::Zero(),
      const Quaternionf &orientation = Quaternionf::Identity()) {
    // Format output
    std::ostringstream oss;
    int status = writeBinaryCompressed(oss, cloud, origin, orientation);
    if (status) {
      throw std::ios_base::failure(
          "[pcl::PCDWriter::writeBinaryCompressed] Error during compression!");
      return status;
    }
    std::string ostr = oss.str();

#ifdef _WIN32
#ifdef UNICODE
    std::wstring w_file_name(file_name.begin(), file_name.end());
    HANDLE h_native_file =
        CreateFile(w_file_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
    HANDLE h_native_file =
        CreateFile(file_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
    if (h_native_file == INVALID_HANDLE_VALUE) {
      PCL_ERROR(
          "[pcl::PCDWriter::writeBinaryCompressed] Error during CreateFile "
          "(%s)!\n",
          file_name.c_str());
      return (-1);
    }
#else
    int fd = io::raw_open(file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC,
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
      PCL_ERROR(
          "[pcl::PCDWriter::writeBinaryCompressed] Error during open (%s)!\n",
          file_name.c_str());
      return (-1);
    }
#endif
    // Mandatory lock file
    // boost::interprocess::file_lock file_lock;
    // setLockingPermissions(file_name, file_lock);

#ifndef _WIN32
    // Stretch the file size to the size of the data
    std::size_t page_size = getpagesize();
    std::size_t size_pages = ostr.size() / page_size;
    std::size_t partial_pages = (size_pages * page_size < ostr.size()) ? 1 : 0;
    long result = io::raw_lseek(
        fd, (size_pages + partial_pages) * page_size - 1, SEEK_SET);
    if (result < 0) {
      io::raw_close(fd);
      // resetLockingPermissions(file_name, file_lock);
      PCL_ERROR(
          "[pcl::PCDWriter::writeBinaryCompressed] lseek errno: %d\n",
          errno);
      PCL_ERROR(
          "[pcl::PCDWriter::writeBinaryCompressed] Error during lseek ()!\n");
      return (-1);
    }
    // Write a bogus entry so that the new file size comes in effect
    result = static_cast<int>(::write(fd, "", 1));
    if (result != 1) {
      io::raw_close(fd);
      // resetLockingPermissions(file_name, file_lock);
      PCL_ERROR(
          "[pcl::PCDWriter::writeBinaryCompressed] Error during write ()!\n");
      return (-1);
    }
#endif

    // Prepare the map
#ifdef _WIN32
    HANDLE fm = CreateFileMapping(h_native_file, NULL, PAGE_READWRITE,
                                  static_cast<DWORD>((ostr.size()) >> 32),
                                  static_cast<DWORD>(ostr.size()), NULL);
    char *map = static_cast<char *>(
        MapViewOfFile(fm, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, ostr.size()));
    CloseHandle(fm);

#else
    char *map = static_cast<char *>(
        ::mmap(nullptr, ostr.size(), PROT_WRITE, MAP_SHARED, fd, 0));
    if (map == reinterpret_cast<char *>(-1))  // MAP_FAILED
    {
      io::raw_close(fd);
      // resetLockingPermissions(file_name, file_lock);
      PCL_ERROR(
          "[pcl::PCDWriter::writeBinaryCompressed] Error during mmap ()!\n");
      return (-1);
    }
#endif

    // copy header + compressed data
    memcpy(map, ostr.data(), ostr.size());

#ifndef _WIN32
    // If the user set the synchronization flag on, call msync
    if (map_synchronization_) msync(map, ostr.size(), MS_SYNC);
#endif

      // Unmap the pages of memory
#ifdef _WIN32
    UnmapViewOfFile(map);
#else
    if (::munmap(map, ostr.size()) == -1) {
      io::raw_close(fd);
      // resetLockingPermissions(file_name, file_lock);
      PCL_ERROR(
          "[pcl::PCDWriter::writeBinaryCompressed] Error during munmap ()!\n");
      return (-1);
    }
#endif
    // Close file
#ifdef _WIN32
    CloseHandle(h_native_file);
#else
    io::raw_close(fd);
#endif
    // resetLockingPermissions(file_name, file_lock);

    return (0);
  }

  /** \brief Save point cloud data to a std::ostream containing n-D points, in
   * BINARY_COMPRESSED format \param[out] os the stream into which to write the
   * data \param[in] cloud the point cloud data message \param[in] origin the
   * sensor acquisition origin \param[in] orientation the sensor acquisition
   * orientation \return
   * (-1) for a general error
   * (-2) if the input cloud is too large for the file format
   * 0 on success
   */
  int writeBinaryCompressed(
      std::ostream &os, const pcl::PCLPointCloud2 &cloud,
      const Vector4f &origin = Vector4f::Zero(),
      const Quaternionf &orientation = Quaternionf::Identity()) {
    if (cloud.data.empty()) {
      PCL_WARN(
          "[pcl::PCDWriter::writeBinaryCompressed] Input point cloud has no "
          "data!\n");
    }
    if (cloud.fields.empty()) {
      PCL_ERROR(
          "[pcl::PCDWriter::writeBinaryCompressed] Input point cloud has no "
          "field data!\n");
      return (-1);
    }

    if (generateHeaderBinaryCompressed(os, cloud, origin, orientation)) {
      return (-1);
    }

    pcl::uindex_t fsize = 0;
    pcl::uindex_t data_size = 0;
    std::size_t nri = 0;
    std::vector<pcl::PCLPointField> fields(cloud.fields.size());
    std::vector<pcl::uindex_t> fields_sizes(cloud.fields.size());
    // Compute the total size of the fields
    for (const auto &field : cloud.fields) {
      if (field.name == "_") continue;

      fields_sizes[nri] = field.count * pcl::getFieldSize(field.datatype);
      fsize += fields_sizes[nri];
      fields[nri] = field;
      ++nri;
    }
    fields_sizes.resize(nri);
    fields.resize(nri);

    // Compute the size of data
    data_size = cloud.width * cloud.height * fsize;

    // If the data is too large the two 32 bit integers used to store the
    // compressed and uncompressed size will overflow.
    if (data_size * 3 / 2 > std::numeric_limits<std::uint32_t>::max()) {
      PCL_ERROR(
          "[pcl::PCDWriter::writeBinaryCompressed] The input data exceeds the "
          "maximum size for compressed version 0.7 pcds of %l bytes.\n",
          static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()) *
              2 / 3);
      return (-2);
    }

    std::vector<char> temp_buf(data_size * 3 / 2 + 8);
    if (data_size != 0) {
      //////////////////////////////////////////////////////////////////////
      // Empty array holding only the valid data
      // data_size = nr_points * point_size
      //           = nr_points * (sizeof_field_1 + sizeof_field_2 + ...
      //           sizeof_field_n) = sizeof_field_1 * nr_points + sizeof_field_2
      //           * nr_points + ... sizeof_field_n * nr_points
      std::vector<char> only_valid_data(data_size);

      // Convert the XYZRGBXYZRGB structure to XXYYZZRGBRGB to aid compression.
      // For this, we need a vector of fields.size () (4 in this case), which
      // points to each individual plane:
      //   pters[0] = &only_valid_data[offset_of_plane_x];
      //   pters[1] = &only_valid_data[offset_of_plane_y];
      //   pters[2] = &only_valid_data[offset_of_plane_z];
      //   pters[3] = &only_valid_data[offset_of_plane_RGB];
      //
      std::vector<char *> pters(fields.size());
      std::size_t toff = 0;
      for (std::size_t i = 0; i < pters.size(); ++i) {
        pters[i] = &only_valid_data[toff];
        toff += fields_sizes[i] * cloud.width * cloud.height;
      }

      // Go over all the points, and copy the data in the appropriate places
      for (uindex_t i = 0; i < cloud.width * cloud.height; ++i) {
        for (std::size_t j = 0; j < pters.size(); ++j) {
          memcpy(pters[j], &cloud.data[i * cloud.point_step + fields[j].offset],
                 fields_sizes[j]);
          // Increment the pointer
          pters[j] += fields_sizes[j];
        }
      }

      // Compress the valid data
      unsigned int compressed_size = pcl::lzfCompress(
          &only_valid_data.front(), static_cast<unsigned int>(data_size),
          &temp_buf[8], static_cast<unsigned int>(data_size) * 3 / 2);
      // Was the compression successful?
      if (compressed_size == 0) {
        return (-1);
      }
      memcpy(temp_buf.data(), &compressed_size, 4);
      memcpy(&temp_buf[4], &data_size, 4);
      temp_buf.resize(compressed_size + 8);
    } else {
      auto *header = reinterpret_cast<std::uint32_t *>(temp_buf.data());
      header[0] = 0;  // compressed_size is 0
      header[1] = 0;  // data_size is 0
    }

    os.imbue(std::locale::classic());
    os << "DATA binary_compressed\n";
    std::copy(temp_buf.cbegin(), temp_buf.cend(),
              std::ostream_iterator<char>(os));
    os.flush();

    return (os ? 0 : -1);
  }

  /** \brief Save point cloud data to a PCD file containing n-D points
   * \param[in] file_name the output file name
   * \param[in] cloud the point cloud data message
   * \param[in] origin the sensor acquisition origin
   * \param[in] orientation the sensor acquisition orientation
   * \param[in] binary set to true if the file is to be written in a binary
   * PCD format, false (default) for ASCII
   *
   * Caution: PointCloud structures containing an RGB field have
   * traditionally used packed float values to store RGB data. Storing a
   * float as ASCII can introduce variations to the smallest bits, and
   * thus significantly alter the data. This is a known issue, and the fix
   * involves switching RGB data to be stored as a packed integer in
   * future versions of PCL.
   *
   * As an intermediary solution, precision 8 is used, which guarantees lossless
   * storage for RGB.
   */
  inline int write(const std::string &file_name,
                   const pcl::PCLPointCloud2 &cloud,
                   const Vector4f &origin = Vector4f::Zero(),
                   const Quaternionf &orientation = Quaternionf::Identity(),
                   const bool binary = false) {
    if (binary) return (writeBinary(file_name, cloud, origin, orientation));
    return (writeASCII(file_name, cloud, origin, orientation, 8));
  }

  /** \brief Save point cloud data to a PCD file containing n-D points
   * \param[in] file_name the output file name
   * \param[in] cloud the point cloud data message (boost shared pointer)
   * \param[in] binary set to true if the file is to be written in a binary PCD
   * format, false (default) for ASCII
   * \param[in] origin the sensor acquisition origin
   * \param[in] orientation the sensor acquisition orientation
   *
   * Caution: PointCloud structures containing an RGB field have
   * traditionally used packed float values to store RGB data. Storing a
   * float as ASCII can introduce variations to the smallest bits, and
   * thus significantly alter the data. This is a known issue, and the fix
   * involves switching RGB data to be stored as a packed integer in
   * future versions of PCL.
   */
  inline int write(const std::string &file_name,
                   const pcl::PCLPointCloud2::ConstPtr &cloud,
                   const Vector4f &origin = Vector4f::Zero(),
                   const Quaternionf &orientation = Quaternionf::Identity(),
                   const bool binary = false) {
    return (write(file_name, *cloud, origin, orientation, binary));
  }

 private:
  /** \brief Set to true if msync() should be called before munmap(). Prevents
   * data loss on NFS systems. */
  bool map_synchronization_{false};
};

/*******************************************************************************
 * IO
 *******************************************************************************
 */
namespace io {
/** \brief Load a PCD v.6 file into a templated PointCloud type.
 *
 * Any PCD files > v.6 will generate a warning as a
 * pcl/PCLPointCloud2 message cannot hold the sensor origin.
 *
 * \param[in] file_name the name of the file to load
 * \param[out] cloud the resultant templated point cloud
 * \ingroup io
 */
inline int loadPCDFile(const std::string &file_name,
                       pcl::PCLPointCloud2 &cloud) {
  pcl::PCDReader p;
  return (p.read(file_name, cloud));
}

/** \brief Load any PCD file into a templated PointCloud type.
 * \param[in] file_name the name of the file to load
 * \param[out] cloud the resultant templated point cloud
 * \param[out] origin the sensor acquisition origin (only for > PCD_V7 - null if
 * not present)
 * \param[out] orientation the sensor acquisition orientation (only
 * for > PCD_V7 - identity if not present) \ingroup io
 */
inline int loadPCDFile(const std::string &file_name,
                       pcl::PCLPointCloud2 &cloud,
                       Vector4f &origin,
                       Quaternionf &orientation) {
  pcl::PCDReader p;
  int pcd_version;
  return (p.read(file_name, cloud, origin, orientation, pcd_version));
}

/** \brief Save point cloud data to a PCD file containing n-D points
 * \param[in] file_name the output file name
 * \param[in] cloud the point cloud data message
 * \param[in] origin the sensor acquisition origin
 * \param[in] orientation the sensor acquisition orientation
 * \param[in] binary_mode true for binary mode, false (default) for ASCII
 *
 * Caution: PointCloud structures containing an RGB field have
 * traditionally used packed float values to store RGB data. Storing a
 * float as ASCII can introduce variations to the smallest bits, and
 * thus significantly alter the data. This is a known issue, and the fix
 * involves switching RGB data to be stored as a packed integer in
 * future versions of PCL.
 * \ingroup io
 */
inline int savePCDFile(const std::string &file_name,
                       const pcl::PCLPointCloud2 &cloud,
                       const Vector4f &origin = Vector4f::Zero(),
                       const Quaternionf &orientation = Quaternionf::Identity(),
                       const bool binary_mode = false) {
  PCDWriter w;
  return (w.write(file_name, cloud, origin, orientation, binary_mode));
}

/**
 * @brief Save point cloud dato to a binary compressed PCD file
 */
inline int savePCDFileBinaryCompressed(
    const std::string &file_name,
    const pcl::PCLPointCloud2 &cloud,
    const Vector4f &origin = Vector4f::Zero(),
    const Quaternionf &orientation = Quaternionf::Identity()) {
  PCDWriter w;
  return w.writeBinaryCompressed(file_name, cloud, origin, orientation);
}

}  // namespace io

}  // namespace pcl

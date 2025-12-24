/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
using namespace std::filesystem;
using namespace boost::iostreams;

namespace {
  std::string MakeFilePath(const std::string_view& filename) {
    try {
      path temp_path = temp_directory_path();
      temp_path.append("test");
      temp_path.append("iostreams");
      create_directories(temp_path);

      temp_path.append(filename);
      remove(temp_path);
      return temp_path.string();
    } catch( const std::exception& err) {
      std::cout << "Error: " << err.what() << std::endl;
    }
    return {};
  }
}

namespace mdf::test {
TEST(BoostIostreams, SimpleFile) {

  const std::string test_file = MakeFilePath("simple_file.txt");

  stream_buffer<file_sink> out_buffer(test_file);

  std::ostream  out_file(&out_buffer);
  constexpr std::string_view test_text = "Hello Boost IO-streams.";
  out_file << test_text << std::endl;

  stream_buffer<file_source> input_buffer(test_file);
  std::istream input_file(&input_buffer);
  while (!input_file.eof()) {
    std::string input_text;
    input_file >> input_text;
    std::cout << input_text << std::endl;
  }
}

TEST(BoostIostreams, SimpleByteArray) {
  std::vector<uint8_t> data_list(256,0);

  stream_buffer< array_sink > out_buffer(reinterpret_cast<char*>(data_list.data()),
                                       data_list.size());
  std::ostream  out_array(&out_buffer);

  for (int index = 0; index < data_list.size(); ++index ) {
    out_array.put(static_cast<char>(index));
  }
  out_buffer.close();

  stream_buffer<array> input_buffer(reinterpret_cast<char*>(data_list.data()),
                                           data_list.size());
  std::istream input_array(&input_buffer);
  int byte_index = 0;
  while (!input_array.eof()) {
    int input = input_array.get();
    if (input < 0) {
      break;
    }
    EXPECT_EQ(input, byte_index) << byte_index << "/" << input;
    ++byte_index;
  }
  EXPECT_EQ(byte_index, 256);
  EXPECT_TRUE(input_array.eof());
  input_array.seekg(0,std::ios_base::beg);
  EXPECT_FALSE(input_array.eof());

}

TEST(BoostIostreams, SimpleWrite) {
  std::vector<char> data_list;
  back_insert_device<std::vector<char>> insert_device(data_list);

  stream_buffer< back_insert_device<std::vector<char> >> out_buffer(insert_device);

  std::basic_streambuf<char>& temp = out_buffer;

  std::ostream out_array(&temp);

  for (int index = 0; index < 256; ++index ) {
    out_array.put(static_cast<char>(index));

  }

  out_buffer.close();

  std::cout << "Size: " << data_list.size() << std::endl;

  stream_buffer<array_source> input_buffer(data_list.data(), data_list.size());
  std::istream input_array(&input_buffer);
  int byte_index = 0;
  while (!input_array.eof()) {
    int input = input_array.get();
    if (input < 0) {
      break;
    }
    EXPECT_EQ(input, byte_index) << byte_index << "/" << input;
    ++byte_index;
  }
  EXPECT_EQ(byte_index, 256);
  EXPECT_TRUE(input_array.eof());
  input_array.seekg(0,std::ios_base::beg);
  EXPECT_FALSE(input_array.eof());
}

} // end namespace
/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/samplerecord.h"
#include <vector>
#include <pybind11/stl.h>

using namespace pybind11;
using namespace mdf;

namespace pymdf {

void InitSampleRecord(pybind11::module& m) {
  constexpr std::string_view record_doc = R"(
Structure that holds one sample (CG record).

Simple support structure that holds a CG records sample values. The structure
is used with the internal sample cache. The timestamp property
shall absolute time (ns since 1970). Note that the structure can hold one
variable length data item (VLSD).
)";

  auto sample = class_<SampleRecord>(m, "SampleRecord",
                                     record_doc.data());
  sample.def(init<>());

  constexpr std::string_view msg_init_doc = R"(
Constructor that wraps a CG message.

Constructor that wraps a CG message.

Attributes:
  time: Nano-seconds since 1970.
  record_id: CG record ID.
  data: Data bytes.
  vlsd: True if the sample has some variable length data
  vlsd_data: Variable length data.
)";

  sample.def_property("timestamp",
                   [] (SampleRecord& self) {
                        return self.timestamp;},
                   [] (SampleRecord& self, uint64_t ns1970) {
                     self.timestamp = ns1970;});

  sample.def_property("record_id",
                      [] (SampleRecord& self) {
                        return self.record_id;},
                      [] (SampleRecord& self, uint64_t id) {
                        self.record_id = id;});

  sample.def_property("data",
                      [] (SampleRecord& self) {
                        return pybind11::cast(self.record_buffer); },
                      [] (SampleRecord& self,
                         const pybind11::list& data_list) {
              self.record_buffer = data_list.cast<std::vector<uint8_t>>(); });

  sample.def_property("vlsd",
                      [] (SampleRecord& self) {
                        return self.vlsd_data;},
                      [] (SampleRecord& self, bool vlsd) {
                        self.vlsd_data = vlsd; });

  sample.def_property("vlsd_data",
                      [] (SampleRecord& self) {
                        return pybind11::cast(self.vlsd_buffer); },
                      [] (SampleRecord& self,
                         const pybind11::list& data_list) {
               self.vlsd_buffer = data_list.cast<std::vector<uint8_t>>(); });
}



} // pydbc


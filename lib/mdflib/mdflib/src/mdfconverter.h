/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf4writer.h"
#include <fstream>
namespace mdf::detail {

class MdfConverter : public Mdf4Writer {
 public:
  MdfConverter();
  ~MdfConverter() override;

  bool InitMeasurement() override;

  bool FinalizeMeasurement() override;
 protected:
 private:
  // void ConverterThread();
};

}  // namespace mdf::detail


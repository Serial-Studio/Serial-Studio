/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/ichannelconversion.h"

#include <cmath>

namespace mdf {
IChannelConversion *IChannelConversion::CreateInverse() { return nullptr; }

IChannelConversion *IChannelConversion::Inverse() const {
  return nullptr;
}

void IChannelConversion::Flags(uint16_t flags) {}

uint16_t IChannelConversion::Flags() const { return 0; }

void IChannelConversion::Range(double min, double max) {}

std::optional<std::pair<double, double>> IChannelConversion::Range() const {
  return {};
}

void IChannelConversion::ChannelDataType(uint8_t channel_data_type) {
  channel_data_type_ = channel_data_type;
}

bool IChannelConversion::IsChannelInteger() const {
  return channel_data_type_ <= 3;
}

bool IChannelConversion::IsChannelFloat() const {
  return channel_data_type_ > 3 && channel_data_type_ <= 5;
}

bool IChannelConversion::ConvertLinear(double channel_value,
                                       double &eng_value) const {
  if (value_list_.empty()) {
    return false;
  }
  if (value_list_.size() == 1) {
    eng_value = Parameter(0);  // Constant value
  } else {
    eng_value = Parameter(0) + (Parameter(1) * channel_value);
  }
  return true;
}

bool IChannelConversion::ConvertRational(double channel_value,
                                         double &eng_value) const {
  if (value_list_.size() < 6) {
    return false;
  }

  eng_value = (Parameter(0) * std::pow(channel_value, 2)) +
              (Parameter(1) * channel_value) + Parameter(2);
  const double div = (Parameter(3) * std::pow(channel_value, 2)) +
                     (Parameter(4) * channel_value) + Parameter(5);
  if (div == 0.0) {
    return false;
  }
  eng_value /= div;
  return true;
}

bool IChannelConversion::ConvertPolynomial(double channel_value,
                                           double &eng_value) const {
  if (value_list_.size() < 6) {
    return false;
  }
  const double temp = channel_value - Parameter(4) - Parameter(5);
  eng_value = Parameter(1) - (Parameter(3) * temp);
  const double div = Parameter(2) * temp - Parameter(0);
  if (div == 0.0) {
    return false;
  }
  eng_value /= div;
  return true;
}

bool IChannelConversion::ConvertLogarithmic(double channel_value,
                                            double &eng_value) const {
  if (value_list_.size() < 7) {
    return false;
  }
  if (Parameter(3) == 0.0) {
    eng_value =
        (channel_value - Parameter(6)) * Parameter(5) - Parameter(2);
    if (Parameter(0) == 0) {
      return false;
    }

    eng_value /= Parameter(0);
    eng_value = std::log(eng_value);
    if (Parameter(1) == 0.0) {
      return false;
    }
    eng_value /= Parameter(1);
  } else if (Parameter(0) == 0.0) {
    eng_value = Parameter(2);
    const double temp2 = channel_value - Parameter(6);
    if (temp2 == 0) {
      return false;
    }
    eng_value /= temp2;
    eng_value -= Parameter(5);
    if (Parameter(3) == 0) {
      return false;
    }
    eng_value /= Parameter(3);
    eng_value = std::log(eng_value);
    if (Parameter(4) == 0.0) {
      return false;
    }
    eng_value /= Parameter(4);
  } else {
    return false;
  }
  return true;
}

bool IChannelConversion::ConvertExponential(double channel_value,
                                            double &eng_value) const {
  if (value_list_.size() < 7) {
    return false;
  }
  if (Parameter(3) == 0.0) {
    eng_value =
        (channel_value - Parameter(6)) * Parameter(5) - Parameter(2);
    if (Parameter(0) == 0) {
      return false;
    }

    eng_value /= Parameter(0);
    eng_value = std::exp(eng_value);
    if (Parameter(1) == 0.0) {
      return false;
    }
    eng_value /= Parameter(1);
  } else if (Parameter(0) == 0.0) {
    eng_value = Parameter(2);
    const double temp2 = channel_value - Parameter(6);
    if (temp2 == 0) {
      return false;
    }
    eng_value /= temp2;
    eng_value -= Parameter(5);
    if (Parameter(3) == 0) {
      return false;
    }
    eng_value /= Parameter(3);
    eng_value = std::exp(eng_value);
    if (Parameter(4) == 0.0) {
      return false;
    }
    eng_value /= Parameter(4);
  } else {
    return false;
  }
  return true;
}
bool IChannelConversion::ConvertAlgebraic(double channel_value,
                                          double &eng_value) const {
  // Todo (ihedvall): This requires a flex and bison formula calculator.
  // Currently not supported.
  return false;
}

bool IChannelConversion::ConvertValueToValueInterpolate(
    double channel_value, double &eng_value) const {
  if (value_list_.size() < 2) {
    return false;
  }

  for (uint16_t n = 0; n < nof_values_; ++n) {
    const uint32_t key_index = n * 2;
    const uint32_t value_index = key_index + 1;
    if (value_index >= value_list_.size()) {
      break;
    }
    const auto key = Parameter(key_index);
    const auto value = Parameter(value_index);
    if (channel_value == key) {
      eng_value = value;
      return true;
    }
    if (channel_value < key) {
      if (n == 0) {
        eng_value = value;
        return true;
      }
      const auto prev_key = Parameter(key_index - 2);
      const auto prev_value = Parameter(value_index - 2);
      const double key_range = key - prev_key;
      const double value_range = value - prev_value;

      if (key_range == 0.0) {
        return false;
      }
      const double x = (channel_value - prev_key) / key_range;
      eng_value = prev_value + (x * value_range);
      return true;
    }
  }
  eng_value = std::get<double>(value_list_.back());
  return true;
}

bool IChannelConversion::ConvertValueToValue(double channel_value,
                                             double &eng_value) const {
  if (value_list_.size() < 2) {
    return false;
  }

  for (uint16_t n = 0; n < nof_values_; ++n) {
    const uint32_t key_index = n * 2;
    const uint32_t value_index = key_index + 1;
    if (value_index >= value_list_.size()) {
      break;
    }
    const double key = Parameter(key_index);
    const double value = Parameter(value_index);
    if (channel_value == key) {
      eng_value = value;
      return true;
    }

    if (channel_value < key) {
      if (n == 0) {
        eng_value = value;
        return true;
      }

      const double prev_key = Parameter(key_index - 2);
      const double prev_value = Parameter(value_index - 2);
      const double key_range = key - prev_key;
      if (key_range == 0.0) {
        return false;
      }
      const double x = (channel_value - prev_key) / key_range;
      eng_value = x <= 0.5 ? prev_value : value;
      return true;
    }
  }
  eng_value = std::get<double>(value_list_.back());
  return true;
}

bool IChannelConversion::ConvertValueRangeToValue(double channel_value,
                                                  double &eng_value) const {
  if (value_list_.empty()) {
    return false;
  }

  for (uint16_t n = 0; n < nof_values_; ++n) {
    const uint32_t key_min_index = n * 3;
    const uint32_t key_max_index = key_min_index + 1;
    const uint32_t value_index = key_min_index + 2;
    if (value_index >= value_list_.size()) {
      break;
    }
    const double key_min = Parameter(key_min_index);
    const double key_max = Parameter(key_max_index);
    const double value = Parameter(value_index);
    if (IsChannelInteger() && channel_value >= key_min &&
        channel_value <= key_max) {
      eng_value = value;
      return true;
    }

    if (IsChannelFloat() && channel_value >= key_min &&
        channel_value < key_max) {
      eng_value = value;
      return true;
    }
  }
  eng_value = std::get<double>(value_list_.back());
  return true;
}

bool IChannelConversion::ConvertTextToValue(const std::string &channel_value,
                                            double &eng_value) const {
  return false;
}

bool IChannelConversion::ConvertValueToText(double channel_value,
                                            std::string &eng_value) const {
  return false;
}

bool IChannelConversion::ConvertValueRangeToText(double channel_value,
                                                 std::string &eng_value) const {
  return false;
}

bool IChannelConversion::ConvertTextToTranslation(
    const std::string &channel_value, std::string &eng_value) const {
  return false;
}

uint16_t IChannelConversion::NofParameters() const {
  return nof_values_;
} 

void IChannelConversion::Parameter(uint16_t index, double parameter) {
  while (index >= value_list_.size()) {
    value_list_.emplace_back(0.0);
  }
  value_list_[index] = parameter;
}

double IChannelConversion::Parameter(uint16_t index) const {
  double value = 0.0;
  if (index < value_list_.size() ) {
    const auto& val = value_list_[index];
    if (std::holds_alternative<double>(val)) {
      value = std::get<double>(val);
    } else if (std::holds_alternative<uint64_t>(val)) {
      value = static_cast<double>(std::get<uint64_t>(val));
    }
  }
  return value;
}

uint64_t IChannelConversion::ParameterUint(uint16_t index) const {
  uint64_t value = 0;
  if (index < value_list_.size() ) {
    const auto& val = value_list_[index];
    if (std::holds_alternative<double>(val)) {
      value = static_cast<uint64_t>(std::get<double>(val));
    } else if (std::holds_alternative<uint64_t>(val)) {
      value = std::get<uint64_t>(val);
    }
  }
  return value;
}

void IChannelConversion::ParameterUint(uint16_t index, uint64_t parameter) {
  switch (Type()) {
    case ConversionType::BitfieldToText:
      break;

    default:
      Parameter(index, static_cast<double>(parameter));
      return;
  }
  while (index >= value_list_.size()) {
    value_list_.emplace_back(static_cast<uint64_t>(0));
  }
  value_list_[index] = parameter;
}

void IChannelConversion::Name(const std::string &name) {}

std::string IChannelConversion::Name() const { return {}; }

void IChannelConversion::Description(const std::string &desc) {}

std::string IChannelConversion::Description() const { return {}; }

void IChannelConversion::Decimals(uint8_t decimals) {}
IMetaData *IChannelConversion::CreateMetaData() { return nullptr; }
IMetaData *IChannelConversion::MetaData() const { return nullptr; }

void IChannelConversion::Formula(const std::string &formula) {
  formula_ = formula;
}

const std::string &IChannelConversion::Formula() const {
  return formula_;
}

uint16_t IChannelConversion::NofReferences() const {
  // Not supported function for MDF3
  return 0;
}

void IChannelConversion::Reference(uint16_t index, const std::string &text) {
   // Not supported function for MDF3 
}

std::string IChannelConversion::Reference(uint16_t index) const {
  // Not supported function for MDF3 
  return {};
}

void IChannelConversion::SetCcComment(const CcComment &cc_comment) {
  if (IMetaData* meta_data = CreateMetaData();
      meta_data != nullptr ) {
    meta_data->XmlSnippet(cc_comment.ToXml());
  }
}

void IChannelConversion::GetCcComment(CcComment &cc_comment) const {
  if (const IMetaData* meta_data = MetaData();
      meta_data != nullptr) {
    cc_comment.FromXml(meta_data->XmlSnippet());
  }
}

void IChannelConversion::SetCcUnit(const CcUnit &unit) {
  Unit(unit.Comment().Text());
}

void IChannelConversion::GetCcUnit(CcUnit &unit) const {
  unit.Comment(MdString(Unit()));
}

}  // end namespace mdf

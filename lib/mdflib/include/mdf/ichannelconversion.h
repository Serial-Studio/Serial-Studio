/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file ichannelconversion.h
 * \brief Defines a channel conversion (CC) block.
 *
 */
#pragma once
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include <variant>

#include "mdf/iblock.h"
#include "mdf/imetadata.h"
#include "mdf/mdfhelper.h"
#include "mdf/cccomment.h"
#include "mdf/ccunit.h"

namespace mdf {

/** \brief Type of conversion formula
 *
 * The type together with the Parameter() function defines
 * the conversion between channel and engineering value.
 *
 */
enum class ConversionType : uint8_t {
  /** \brief 1:1 conversion. No parameters needed. */
  NoConversion = 0,

  /** \brief Linear conversion. 2 parameters.
   * Eng = Ch * Par(1) + Par(0).
   */
  Linear = 1,

  /** \brief Rational function conversion. 6 parameters.
   *  Eng = (Par(0)*Ch^2 + Par(1)*Ch + Par(2)) /
   *  (Par(3)*Ch^2 + Par(4)*Ch + Par(5))
   */
  Rational = 2,
  Algebraic = 3, ///< Text formula.

  /** \brief Value to value conversion with interpolation.
   * Defined by a list of Key value pairs.
   * Par(n) = key and Par(n+1) value.
   */
  ValueToValueInterpolation = 4,

  /** \brief Value to value conversion without interpolation.
   * Defined by a list of Key value pairs.
   * Par(n) = key and Par(n+1) value.
   */
  ValueToValue = 5,

  /** \brief Value range to value conversion without interpolation.
   * Defined by a list of Key min/max value triplets.
   * Par(3*n) = key min, Par(3*(n+1)) = key max and Par(3*(n+2)) value. Add a default
   * value last, after all the triplets.
   */
  ValueRangeToValue = 6,

  /** \brief Value to text conversion.
   * Defined by a list of key values to text string conversions. This is
   * normally used for enumerated channels.
   * Par(n) value to Ref(n) text. Add a default
   * referenced text last.
   */
  ValueToText = 7,

  /** \brief Value range to text conversion.
   * Defined by a list of key min/max values to text string conversions. This is
   * normally used for enumerated channels.
   * Par(2*n) min key, Par(2(n+1)) max key to Ref(n) value. Add a default
   * referenced text  last.
   */
  ValueRangeToText = 8,

  /** \brief Text to value conversion.
   * Defines a list of text string to value conversion.
   * Ref(n) key to Par(n) value. Add a default value last to the parameter list.
   */
  TextToValue = 9,

  /** \brief Text to text conversion.
    * Defines a list of text string to text conversion.
    * Ref(2*n) key to Ref(2*(n+1)) value.
    * Add a text value last to the parameter list.
    */
  TextToTranslation = 10,

  /** \brief Bitfield to text conversion
   * Currently unsupported conversion.
   */
  BitfieldToText = 11,
  // MDF 3 types
  Polynomial = 30, ///< MDF 3 polynomial conversion.
  Exponential = 31, ///< MDF 3 exponential conversion.
  Logarithmic = 32, ///< MDF 3 logarithmic conversion.
  DateConversion = 33, ///< MDF 3 Date conversion
  TimeConversion = 34 ///< MDF 3 Time conversion
};

/** \brief MDF 3 text conversion structure. Not used
 * in MDF 4. Key to text conversion.
 */
struct TextConversion {
  double value = 0; ///< Key
  std::string text; ///< Text string
};

/** \brief MDF 3 range conversion structure. Not used in MDF 4.
 * Key min/max to text conversion
 */
struct TextRangeConversion {
  double lower = 0; ///< Key min value.
  double upper = 0; ///< Key max value.
  uint32_t link_text = 0; ///< File position of the text value
  std::string text; ///< The text value.
};

/** \brief Channel conversion flags.
 *
 */
namespace CcFlag {
constexpr uint16_t PrecisionValid = 0x0001; ///< Precision is used.
constexpr uint16_t RangeValid = 0x0002; ///< Range is used.
constexpr uint16_t StatusString = 0x0004; ///< Status string flag.
}  // namespace CcFlag

/** \brief Defines a channel conversion (CC) block.
 *
 * The channel conversion (CC) block shall convert the channel value to
 * an engineering value. If the CN block doesn't reference any CC block, the
 * engineer and channel values are identical.
 *
 * Depending on the type of conversion (ConversionType), none or more parameters
 * (Parameter()) and none or more text string (Reference()) are used to define
 * the conversion. Check description in ConversionType.
 */
class IChannelConversion : public IBlock {

 public:
  ~IChannelConversion() override = default;

  virtual void Name(const std::string& name); ///< Sets the CC name.
  [[nodiscard]] virtual std::string Name() const; ///< Name.

  virtual void Description(const std::string& desc); ///< Sets the description.
  [[nodiscard]] virtual std::string Description() const; ///< Description.

  virtual void Unit(const std::string& unit) = 0; ///< Sets the unit.
  [[nodiscard]] virtual std::string Unit() const = 0; ///< Unit of measure.
  [[nodiscard]] virtual bool IsUnitValid() const = 0; ///< True if unit exist.

  virtual void SetCcUnit(const CcUnit& unit);
  virtual void GetCcUnit(CcUnit& unit) const;

  virtual void Type(ConversionType type) = 0; ///< Sets the conversion type.
  [[nodiscard]] virtual ConversionType Type() const = 0; ///< Conversion type.

  [[nodiscard]] virtual bool IsDecimalUsed()
      const = 0; ///< True if decimal is used.
  virtual void Decimals(uint8_t decimals); ///< Sets number of decimals.
  [[nodiscard]] virtual uint8_t Decimals() const = 0; ///< Number of decimals.

  /** \brief Creates an inverse conversion block. */
  [[nodiscard]] virtual IChannelConversion* CreateInverse();

  /** \brief Returns the inverse conversion block. Seldom in use. */
  [[nodiscard]] virtual IChannelConversion* Inverse() const;

  virtual void Range(double min, double max); ///< Sets the range.
  [[nodiscard]] virtual std::optional<std::pair<double, double>> Range()
      const; ///< Returns the range if exist.

  virtual void Flags(uint16_t flags); ///< Sets the CcFlag
  [[nodiscard]] virtual uint16_t Flags() const; ///< Returns CcFlag

  /** \brief Creates a meta-data (MD) block. */
  [[nodiscard]] virtual IMetaData* CreateMetaData();

  /** \brief Returns the meta-data block. */
  [[nodiscard]] virtual IMetaData* MetaData() const;

  void SetCcComment(const CcComment& cc_comment);
  void GetCcComment(CcComment& cc_comment) const;

  /** \brief Sets the formula string. */
  virtual void Formula( const std::string& formula);
  /** \brief Returns formula string. */
  [[nodiscard]] virtual const std::string& Formula() const;

  /** \brief Returns number of parameters in the block. */
  [[nodiscard]] uint16_t NofParameters() const;

  /** \brief Sets a floating point parameter value.
   *
   * @param index Parameter index to set.
   * @param parameter Value to set.
   */
  void Parameter(uint16_t index, double parameter);

  /** \brief Returns the parameter (double)
   *
   * @param index
   * @return Parameter floating point value
   */
  [[nodiscard]] double Parameter(uint16_t index) const;

  /** \brief Returns the parameter as a bit field (uint64_t)
   *
   * @param index
   * @return Parameter floating point value
   */
  [[nodiscard]] uint64_t ParameterUint(uint16_t index) const;
  /** \brief Sets an unsigned integer parameter value.
   *
   * @param index Parameter index to set.
   * @param parameter Value to set.
   */
  void ParameterUint(uint16_t index, uint64_t parameter);

  /** \brief Number of references in the block. */
  [[nodiscard]] virtual uint16_t NofReferences() const;

  /** \brief Sets text reference (TX) block
   *
   * @param index Index of the text block (TX).
   * @param text Text content of the TX block.
   */
  virtual void Reference(uint16_t index, const std::string& text);

  /** \brief Returns the reference string by its index */ 
  [[nodiscard]] virtual std::string Reference(uint16_t index) const;

  /** \brief Sets the CN block data type.
   *
   * Some of the conversion methods need to know the CN channels data type.
   * @param channel_data_type Channel data type.
   */
  void ChannelDataType(uint8_t channel_data_type);

  /** \brief Converts a channel value to an engineering (scaled) value.
   *
   * Converts the channel value to a scaled value.
   * @tparam T Channel data type.
   * @tparam V Engineering value data type.
   * @param channel_value The channel value.
   * @param eng_value The scaled engineering value.
   * @return True if the conversion is valid.
   */
  template <typename T, typename V>
  bool Convert(const T& channel_value, V& eng_value) const;

  /** \brief Converts a channel value to an engineering string value.
   *
   * Converts a channel value to an scaled value converted to a string. This
   * function is typical used when presenting data in GUI.
   * @tparam T Channel data type
   * @tparam V String type.
   * @param channel_value The channel value.
   * @param eng_value The output string.
   * @return True if the conversion is valid.
   */
  template <typename T, typename V = std::string>
  bool Convert(const T& channel_value, std::string& eng_value) const;

  /** \brief Convert from string to double with full precision */
  template <typename T = std::string, typename V = double>
  bool Convert(const std::string& channel_value, double& eng_value) const;

  /** \brief Converts from string to string. */
  template <typename T = std::string, typename V = std::string>
  bool Convert(const std::string& channel_value, std::string& eng_value) const {
    if (Type() == ConversionType::TextToTranslation) {
      ConvertTextToTranslation(channel_value, eng_value);
    } else if (Type() == ConversionType::NoConversion) {
      eng_value = channel_value;
    } else {
      return false;
    }
    return true;
  }

 protected:
  uint16_t nof_values_ = 0; ///< Number of parameter values (MDF3).

  /** \brief List of floating point constants. */
  using ParameterList = std::vector<std::variant<uint64_t, double>>;

  ParameterList value_list_; ///< List of parameters.
  uint8_t channel_data_type_ =
      0;  ///< The channels data type. Needed by some conversions

  // The formula and conversion list is used in MDF 3 while MDF4
  // uses the reference list.
  mutable std::string formula_; ///< Formula string (MDF3).
  std::vector<TextConversion> text_conversion_list_; ///< MDF3
  std::vector<TextRangeConversion> text_range_conversion_list_; ///< MDF3

  [[nodiscard]] bool IsChannelInteger()
      const; ///< Returns true if channel is an integer.
  [[nodiscard]] bool IsChannelFloat()
      const; ///< Returns true if the channel is a float.

  virtual bool ConvertLinear(double channel_value, double& eng_value)
      const; ///< Linear conversion.
  virtual bool ConvertRational(double channel_value, double& eng_value)
      const; ///< Rational conversion.
  virtual bool ConvertAlgebraic(double channel_value, double& eng_value)
      const; ///< Algebraic conversion.
  virtual bool ConvertValueToValueInterpolate(double channel_value,
      double& eng_value) const; ///< Value to value interpolation conversion.
  virtual bool ConvertValueToValue(double channel_value,
      double& eng_value) const; ///< Value to value conversion.
  virtual bool ConvertValueRangeToValue(double channel_value,
      double& eng_value) const; ///< Value range to value conversion.

  virtual bool ConvertValueToText(double channel_value,
      std::string& eng_value) const; ///< Value to text conversion.
  virtual bool ConvertValueRangeToText(double channel_value,
      std::string& eng_value) const; ///< Value range to text value.
  virtual bool ConvertTextToValue(const std::string& channel_value,
      double& eng_value) const; ///< Text to value conversion.
  virtual bool ConvertTextToTranslation(const std::string& channel_value,
      std::string& eng_value) const; ///< Text to text conversion.
  virtual bool ConvertPolynomial(double channel_value, double& eng_value)
      const; ///< Polynomial conversion (MDF3).
  virtual bool ConvertLogarithmic(double channel_value,
      double& eng_value) const; ///< Logarithmic conversion (MDF3).
  virtual bool ConvertExponential(double channel_value,
      double& eng_value) const; ///< Exponential conversion (MDF3).
};

template <typename T, typename V>
inline bool IChannelConversion::Convert(const T& channel_value,
  V& eng_value) const {
  bool valid = false;
  double value = 0.0;
  switch (Type()) {
    case ConversionType::Linear: {
      valid = ConvertLinear(static_cast<double>(channel_value), value);
      eng_value = static_cast<V>(value);
      break;
    }

    case ConversionType::Rational: {
      valid = ConvertRational(static_cast<double>(channel_value), value);
      eng_value = static_cast<V>(value);
      break;
    }

    case ConversionType::Algebraic: {
      valid = ConvertAlgebraic(static_cast<double>(channel_value), value);
      eng_value = static_cast<V>(value);
      break;
    }

    case ConversionType::ValueToValueInterpolation: {
      valid = ConvertValueToValueInterpolate(
          static_cast<double>(channel_value), value);
      eng_value = static_cast<V>(value);
      break;
    }

    case ConversionType::ValueToValue: {
      valid = ConvertValueToValue(static_cast<double>(channel_value), value);
      eng_value = static_cast<V>(value);
      break;
    }

    case ConversionType::ValueRangeToValue: {
      valid =
          ConvertValueRangeToValue(static_cast<double>(channel_value), value);
      eng_value = static_cast<V>(value);
      break;
    }

    case ConversionType::ValueToText: {
      std::string text;
      valid = ConvertValueToText(static_cast<double>(channel_value), text);
      std::istringstream s(text);
      s >> eng_value;
      break;
    }

    case ConversionType::ValueRangeToText: {
      std::string text;
      valid =
          ConvertValueRangeToText(static_cast<double>(channel_value), text);
      std::istringstream s(text);
      s >> eng_value;
      break;
    }

    case ConversionType::Polynomial: {
      valid = ConvertPolynomial(static_cast<double>(channel_value), value);
      eng_value = static_cast<V>(value);
      break;
    }

    case ConversionType::Exponential: {
      valid = ConvertExponential(static_cast<double>(channel_value), value);
      eng_value = static_cast<V>(value);
      break;
    }

    case ConversionType::Logarithmic: {
      valid = ConvertLogarithmic(static_cast<double>(channel_value), value);
      eng_value = static_cast<V>(value);
      break;
    }
    case ConversionType::NoConversion:
    default: {
      eng_value = static_cast<V>(channel_value);
      valid = true;
      break;
    }
  }
  return valid;
}

template <typename T, typename V>
inline bool IChannelConversion::Convert(const T& channel_value,
    std::string& eng_value) const {
  bool valid = false;
  double value = 0.0;
  switch (Type()) {
    case ConversionType::Linear: {
      valid = ConvertLinear(static_cast<double>(channel_value), value);
      eng_value =
          MdfHelper::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
      break;
    }

    case ConversionType::Rational: {
      valid = ConvertRational(static_cast<double>(channel_value), value);
      eng_value =
          MdfHelper::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
      break;
    }

    case ConversionType::Algebraic: {
      valid = ConvertAlgebraic(static_cast<double>(channel_value), value);
      eng_value =
          MdfHelper::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
      break;
    }

    case ConversionType::ValueToValueInterpolation: {
      valid = ConvertValueToValueInterpolate(
          static_cast<double>(channel_value), value);
      eng_value =
          MdfHelper::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
      break;
    }

    case ConversionType::ValueToValue: {
      valid = ConvertValueToValue(static_cast<double>(channel_value), value);
      eng_value =
          MdfHelper::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
      break;
    }

    case ConversionType::ValueRangeToValue: {
      valid =
          ConvertValueRangeToValue(static_cast<double>(channel_value), value);
      eng_value =
          MdfHelper::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
      break;
    }

    case ConversionType::ValueToText: {
      valid =
          ConvertValueToText(static_cast<double>(channel_value), eng_value);
      break;
    }

    case ConversionType::ValueRangeToText: {
      valid = ConvertValueRangeToText(static_cast<double>(channel_value),
                                      eng_value);
      break;
    }

    case ConversionType::Polynomial: {
      valid = ConvertPolynomial(static_cast<double>(channel_value), value);
      eng_value =
          MdfHelper::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
      break;
    }

    case ConversionType::Exponential: {
      valid = ConvertExponential(static_cast<double>(channel_value), value);
      eng_value =
          MdfHelper::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
      break;
    }

    case ConversionType::Logarithmic: {
      valid = ConvertLogarithmic(static_cast<double>(channel_value), value);
      eng_value =
          MdfHelper::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
      break;
    }

    case ConversionType::NoConversion:
    default: {
      eng_value = MdfHelper::FormatDouble(static_cast<double>(channel_value),
                                          IsDecimalUsed() ? Decimals() : 6);
      valid = true;
      break;
    }
  }
  return valid;
}

template <typename T, typename V>
bool IChannelConversion::Convert(const std::string& channel_value,
    double& eng_value) const {
  if (Type() == ConversionType::TextToValue) {
    ConvertTextToValue(channel_value, eng_value);
  } else if (Type() == ConversionType::NoConversion) {
    eng_value = std::stod(channel_value);
  } else {
    return false;
  }
  return true;
}

}  // namespace mdf

-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Steinhart-Hart Thermistor
-- Converts NTC thermistor resistance to temperature using the
-- Steinhart-Hart equation. Input 'value' should be the measured
-- resistance in ohms. Coefficients A, B, C are specific to your
-- thermistor — check its datasheet.
-- Adjust R_series, R_nominal, A, B, C to match your sensor.
--

function transform(value)
  local R_series = 10000
  local R_nominal = 10000
  local adc_max = 1023

  local R = R_series * value / (adc_max - value)
  local ln_R = math.log(R / R_nominal)

  local A = 1.009249e-3
  local B = 2.378405e-4
  local C = 2.019202e-7

  local inv_T = A + B * ln_R + C * ln_R * ln_R * ln_R
  return 1.0 / inv_T - 273.15
end

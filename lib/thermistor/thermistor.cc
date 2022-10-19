#include "thermistor.h"

const float Thermistor::resistance_50c_ =
    ResistanceAtTemp(50, b25_50_);
const float Thermistor::resistance_85c_ =
    ResistanceAtTemp(85, b25_85_);
const float Thermistor::resistance_100c_ =
    ResistanceAtTemp(100, b25_100_);

const float Thermistor::T1_ = kKelvin + 25.0;
const float Thermistor::T2_ = kKelvin + 50.0;
const float Thermistor::T3_ = kKelvin + 85.0;

const float Thermistor::L1_ = logf(resistance_);
const float Thermistor::L2_ = logf(resistance_50c_);
const float Thermistor::L3_ = logf(resistance_85c_);

//const float Thermistor::L1_ = logf(resistance_50c_);
//const float Thermistor::L2_ = logf(resistance_85c_);
//const float Thermistor::L3_ = logf(resistance_100c_);

const float Thermistor::Y1_ = 1 / T1_;
const float Thermistor::Y2_ = 1 / T2_;
const float Thermistor::Y3_ = 1 / T3_;

const float Thermistor::G2_ = (Y2_ - Y1_) / (L2_ - L1_);
const float Thermistor::G3_ = (Y3_ - Y1_) / (L3_ - L1_);

const float Thermistor::C_ = (G3_ - G2_) / (L3_ - L2_) / (L1_ + L2_ + L3_);
const float Thermistor::B_ = G2_ - C_ * (powf(L1_, 2) + L1_ * L2_ + powf(L2_, 2));
const float Thermistor::A_ = Y1_ - (B_ + powf(L1_, 2) * C_) * L1_;

float Thermistor::ConvertResistanceToKelvin(float resistance) {
  return 1.0 / (A_ + B_ * logf(resistance) + C_ * powf(logf(resistance), 3));
}

float Thermistor::ConvertResistanceToCelsius(float resistance) {
    return ConvertResistanceToKelvin(resistance) - kKelvin;
}

float Thermistor::ConvertResistanceToFahrenheit(float resistance) {
    return ConvertResistanceToCelsius(resistance) * 1.8 + 32;
}

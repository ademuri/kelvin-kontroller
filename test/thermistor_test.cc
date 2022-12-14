#include "thermistor.h"

#include <gtest/gtest.h>

#include "types.h"

namespace {

TEST(Thermistor, ConvertsDatasheetValues) {
  EXPECT_FLOAT_EQ(
      Thermistor::kKelvin + 25.0,
      Thermistor::ConvertResistanceToKelvin(Thermistor::resistance_));

  EXPECT_FLOAT_EQ(Thermistor::kKelvin + 50.0,
                  Thermistor::ConvertResistanceToKelvin(
                      Thermistor::ResistanceAtTemp(50.0, Thermistor::b25_50_)));
  EXPECT_FLOAT_EQ(Thermistor::kKelvin + 85.0,
                  Thermistor::ConvertResistanceToKelvin(
                      Thermistor::ResistanceAtTemp(85.0, Thermistor::b25_85_)));
}

TEST(Thermistor, PredictsNonDatasheetValues) {
  EXPECT_NEAR(Thermistor::kKelvin + 100.0,
              Thermistor::ConvertResistanceToKelvin(
                  Thermistor::ResistanceAtTemp(100.0, Thermistor::b25_100_)),
              0.1);
  // Note: I found these magic numbers through trial-and-error
  EXPECT_NEAR(Thermistor::kKelvin + 31,
              Thermistor::ConvertResistanceToKelvin(8000), 0.1);
}

TEST(Thermistor, ConvertsToCelsius) {
  EXPECT_FLOAT_EQ(
      25.0, Thermistor::ConvertResistanceToCelsius(Thermistor::resistance_));
  EXPECT_FLOAT_EQ(85.0,
                  Thermistor::ConvertResistanceToCelsius(
                      Thermistor::ResistanceAtTemp(85.0, Thermistor::b25_85_)));
}

TEST(Thermistor, ConvertsToFahrenheit) {
  EXPECT_FLOAT_EQ(
      77.0, Thermistor::ConvertResistanceToFahrenheit(Thermistor::resistance_));
  EXPECT_FLOAT_EQ(185.0,
                  Thermistor::ConvertResistanceToFahrenheit(
                      Thermistor::ResistanceAtTemp(85.0, Thermistor::b25_85_)));
}

TEST(Thermistor, ReadsResistance) {
  static constexpr float kReferenceVoltage = 3.3;
  static constexpr int kPin = 1;
  Thermistor thermistor{kPin, /*analog_reference_volts=*/kReferenceVoltage};

  SetAnalogRead(kPin, Thermistor::kAnalogReadBase / 2);
  EXPECT_FLOAT_EQ(thermistor.ReadResistance(), Thermistor::kDividerOhms);

  SetAnalogRead(kPin, Thermistor::kAnalogReadBase / 4);
  EXPECT_NEAR(thermistor.ReadResistance(), Thermistor::kDividerOhms / 3, 10);

  SetAnalogRead(kPin, 0);
  EXPECT_FLOAT_EQ(thermistor.ReadResistance(), 0);

  SetAnalogRead(kPin, (Thermistor::kAnalogReadBase * 3) / 4);
  EXPECT_NEAR(thermistor.ReadResistance(), Thermistor::kDividerOhms * 3, 10);
}

TEST(Thermistor, ReadsFahrenheit) {
  static constexpr float kReferenceVoltage = 3.3;
  static constexpr int kPin = 1;
  Thermistor thermistor{kPin, /*analog_reference_volts=*/kReferenceVoltage};

  SetAnalogRead(kPin, Thermistor::kAnalogReadBase / 2);
  float resistance = thermistor.ReadResistance();
  EXPECT_FLOAT_EQ(thermistor.ReadFahrenheit(),
                  Thermistor::ConvertResistanceToFahrenheit(resistance));
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  // if you plan to use GMock, replace the line above with
  // ::testing::InitGoogleMock(&argc, argv);

  if (RUN_ALL_TESTS())
    ;

  // Always return zero-code and allow PlatformIO to parse results
  return 0;
}

#include "thermistor.h"

#include <gtest/gtest.h>

TEST(Thermistor, ConvertsDatasheetValues) {
  EXPECT_FLOAT_EQ(
      Thermistor::kKelvin + 25.0, Thermistor::ConvertResistanceToKelvin(Thermistor::resistance_));

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
                      Thermistor::ResistanceAtTemp(100.0, Thermistor::b25_100_)), 0.1);
  // Note: I found these magic numbers through trial-and-error
  EXPECT_NEAR(Thermistor::kKelvin + 31,
                  Thermistor::ConvertResistanceToKelvin(8000), 0.1);
}

TEST(Thermistor, ConvertsToCelsius) {
  EXPECT_FLOAT_EQ(
      25.0, Thermistor::ConvertResistanceToCelsius(Thermistor::resistance_));
  EXPECT_FLOAT_EQ(
      85.0, Thermistor::ConvertResistanceToCelsius(Thermistor::ResistanceAtTemp(85.0, Thermistor::b25_85_)));
}

TEST(Thermistor, ConvertsToFahrenheit) {
  EXPECT_FLOAT_EQ(
      77.0, Thermistor::ConvertResistanceToFahrenheit(Thermistor::resistance_));
  EXPECT_FLOAT_EQ(
      185.0, Thermistor::ConvertResistanceToFahrenheit(Thermistor::ResistanceAtTemp(85.0, Thermistor::b25_85_)));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  // if you plan to use GMock, replace the line above with
  // ::testing::InitGoogleMock(&argc, argv);

  if (RUN_ALL_TESTS())
    ;

  // Always return zero-code and allow PlatformIO to parse results
  return 0;
}

#include "thermistor.h"

#include <gtest/gtest.h>

TEST(Thermistor, ConvertsDatasheetValues) {
  EXPECT_FLOAT_EQ(
      25.0, Thermistor::ConvertResistanceToKelvin(Thermistor::resistance_));

  EXPECT_FLOAT_EQ(
      50, Thermistor::ConvertResistanceToKelvin(4100));

  EXPECT_FLOAT_EQ(50.0,
                  Thermistor::ConvertResistanceToKelvin(
                      Thermistor::ResistanceAtTemp(50.0, Thermistor::b25_50_)));
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

#include <gtest/gtest.h>

#include <ostream>
#include <sstream>

#include "fake-controller.h"
#include "thermistor.h"
#include "types.h"

namespace {

std::ostream& operator<<(std::ostream& os, const RunnerStatus& status) {
  return os << "RunnerStatus: {fault: " << status.fault.bean_temp_low
            << status.fault.bean_temp_high << status.fault.env_temp_low
            << status.fault.env_temp_high << status.fault.ambient_temp_low
            << status.fault.ambient_temp_high << ", bean_temp_read_error: " << std::to_string(status.fault.bean_temp_read_error)
            << ", env_temp_read_error: " << std::to_string(status.fault.env_temp_read_error) << "}";
}

std::string to_string(const RunnerStatus& status) {
  std::ostringstream ss;
  ss << status;
  return std::move(ss).str();
}

TEST(Controller, SetsFaultIfTempRangesExceeded) {
  FakeController controller;
  controller.Init();

  EXPECT_EQ(controller.GetStatus().fault.Faulty(), false)
      << to_string(controller.GetStatus());

  controller.SetBeanTempF(70);
  controller.SetEnvTempF(70);
  controller.SetAmbientTempF(70);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), false)
      << to_string(controller.GetStatus());

  // Bean temp
  controller.SetBeanTempF(0);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), true)
      << to_string(controller.GetStatus());

  controller.ResetStatus();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), false)
      << to_string(controller.GetStatus());

  controller.SetBeanTempF(600);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), true)
      << to_string(controller.GetStatus());
  controller.SetBeanTempF(70);

  // Env temp
  controller.SetEnvTempF(0);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), true)
      << to_string(controller.GetStatus());

  controller.ResetStatus();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), false)
      << to_string(controller.GetStatus());

  controller.SetEnvTempF(900);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), true)
      << to_string(controller.GetStatus());
  controller.SetEnvTempF(70);

  // Ambient temp
  controller.SetAmbientTempF(0);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), true)
      << to_string(controller.GetStatus());

  controller.ResetStatus();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), false)
      << to_string(controller.GetStatus());

  controller.SetAmbientTempF(140);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), true)
      << to_string(controller.GetStatus());
}

TEST(Controller, SetsFaultOnReadError) {
  FakeController controller;
  controller.Init();

  EXPECT_EQ(controller.GetStatus().fault.Faulty(), false)
      << to_string(controller.GetStatus());

  controller.SetBeanTempF(70);
  controller.SetEnvTempF(70);
  controller.SetAmbientTempF(70);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), false)
      << to_string(controller.GetStatus());

  controller.SetBeanTempReadError(0b100);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), true)
      << to_string(controller.GetStatus());

  controller.SetBeanTempReadError(0b000);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), true)
      << to_string(controller.GetStatus());

  controller.ResetStatus();
  EXPECT_EQ(controller.GetStatus().fault.Faulty(), false)
      << to_string(controller.GetStatus());
}

TEST(Controller, SafeDefaultsOnFault) {
  FakeController controller;
  controller.Init();
  EXPECT_EQ(controller.GetRelayValue(), true);

  controller.SetAmbientTempF(0);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 255);
  EXPECT_EQ(controller.GetHeaterValue(), 0);
  EXPECT_EQ(controller.GetStirValue(), true);
  EXPECT_EQ(controller.GetRelayValue(), false);
}

}  // namespace

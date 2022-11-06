#include <gtest/gtest.h>

#include <ostream>
#include <sstream>

#include "fake-controller.h"
#include "thermistor.h"
#include "types.h"

namespace {

std::ostream& operator<<(std::ostream& os, const RunnerStatus& status) {
  return os << "RunnerStatus: {fault: "
            << status.fault_since_reset.bean_temp_low
            << status.fault_since_reset.bean_temp_high
            << status.fault_since_reset.env_temp_low
            << status.fault_since_reset.env_temp_high
            << status.fault_since_reset.ambient_temp_low
            << status.fault_since_reset.ambient_temp_high
            << ", bean_temp_read_error: "
            << std::to_string(status.fault_since_reset.bean_temp_read_error)
            << ", env_temp_read_error: "
            << std::to_string(status.fault_since_reset.env_temp_read_error)
            << "}";
}

std::string to_string(const RunnerStatus& status) {
  std::ostringstream ss;
  ss << status;
  return std::move(ss).str();
}

TEST(Controller, SetsFaultIfTempRangesExceeded) {
  FakeController controller;
  controller.Init();
  // TODO: this skips the turn-on delay, and affects all tests. Extract this
  // into a test fixture.
  AdvanceMillis(2000);

  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), false)
      << to_string(controller.GetStatus());

  controller.SetBeanTempF(70);
  controller.SetEnvTempF(70);
  controller.SetAmbientTempF(70);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), false)
      << to_string(controller.GetStatus());

  // Bean temp
  controller.SetBeanTempF(0);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), true)
      << to_string(controller.GetStatus());

  controller.ResetStatus();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), false)
      << to_string(controller.GetStatus());

  controller.SetBeanTempF(600);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), true)
      << to_string(controller.GetStatus());
  controller.SetBeanTempF(70);

  // Env temp
  controller.SetEnvTempF(0);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), true)
      << to_string(controller.GetStatus());

  controller.ResetStatus();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), false)
      << to_string(controller.GetStatus());

  controller.SetEnvTempF(900);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), true)
      << to_string(controller.GetStatus());
  controller.SetEnvTempF(70);

  // Ambient temp
  controller.SetAmbientTempF(0);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), true)
      << to_string(controller.GetStatus());

  controller.ResetStatus();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), false)
      << to_string(controller.GetStatus());

  controller.SetAmbientTempF(140);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), true)
      << to_string(controller.GetStatus());
}

TEST(Controller, SetsFaultOnReadError) {
  FakeController controller;
  controller.Init();

  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), false)
      << to_string(controller.GetStatus());

  controller.SetBeanTempF(70);
  controller.SetEnvTempF(70);
  controller.SetAmbientTempF(70);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), false)
      << to_string(controller.GetStatus());

  controller.SetBeanTempReadError(0b100);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), true)
      << to_string(controller.GetStatus());

  controller.SetBeanTempReadError(0b000);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), true)
      << to_string(controller.GetStatus());

  controller.ResetStatus();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), false)
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

TEST(Controller, SetsStatusTemps) {
  FakeController controller;
  controller.Init();
  controller.SetBeanTempF(60);
  controller.SetEnvTempF(70);
  controller.SetAmbientTempF(80);
  controller.Step();

  EXPECT_EQ(controller.GetStatus().bean_temp, 60);
  EXPECT_EQ(controller.GetStatus().env_temp, 70);
  EXPECT_EQ(controller.GetStatus().ambient_temp, 80);

  controller.SetBeanTempF(10);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), true)
      << to_string(controller.GetStatus());
  EXPECT_EQ(controller.GetStatus().bean_temp, 10);

  controller.SetEnvTempF(100);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), true)
      << to_string(controller.GetStatus());
  EXPECT_EQ(controller.GetStatus().env_temp, 100);
}

TEST(Controller, CommandSetsFan) {
  FakeController controller;
  controller.Init();
  controller.SetBeanTempF(70);
  controller.SetEnvTempF(70);
  controller.SetAmbientTempF(70);
  controller.Step();

  EXPECT_EQ(controller.GetFanValue(), 0);

  RunnerCommand command{};
  command.fan_speed = 200;
  controller.ReceiveCommand(command);
  EXPECT_EQ(controller.GetFanValue(), 200);

  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 200);
}

TEST(Controller, CommandResetsStatus) {
  FakeController controller;
  controller.Init();
  controller.SetBeanTempF(10);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), true)
      << to_string(controller.GetStatus());

  RunnerCommand command{};
  command.reset = true;
  controller.ReceiveCommand(command);
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), false)
      << to_string(controller.GetStatus());
}

TEST(Controller, CommandSetsPidTemp) {
  FakeController controller;
  controller.Init();
  controller.SetBeanTempF(70);
  controller.SetEnvTempF(70);
  controller.SetAmbientTempF(70);
  controller.Step();
  EXPECT_EQ(controller.GetHeaterValue(), false);

  AdvanceMillis(100);
  RunnerCommand command{};
  command.p = 1;
  command.target_temp = 400;
  command.fan_speed = 255;
  controller.ReceiveCommand(command);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), false)
      << to_string(controller.GetStatus());
  EXPECT_EQ(controller.GetHeaterValue(), true);

  AdvanceMillis(100);
  controller.SetBeanTempF(500);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), false)
      << to_string(controller.GetStatus());
  EXPECT_EQ(controller.GetHeaterValue(), false);
}

TEST(Controller, OnlySetsHeaterIfFanIsOn) {
  FakeController controller;
  controller.Init();
  controller.SetBeanTempF(70);
  controller.SetEnvTempF(70);
  controller.SetAmbientTempF(70);
  controller.SetFan(0);
  controller.Step();
  EXPECT_EQ(controller.GetHeaterValue(), false);

  AdvanceMillis(100);
  RunnerCommand command{};
  command.target_temp = 400;
  command.fan_speed = 0;
  controller.ReceiveCommand(command);
  controller.Step();
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), false)
      << to_string(controller.GetStatus());
  EXPECT_EQ(controller.GetHeaterValue(), false);

  AdvanceMillis(100);
  controller.SetFanTarget(255);
  controller.Step();
  EXPECT_EQ(controller.GetHeaterValue(), true);
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), false)
      << to_string(controller.GetStatus());
}

TEST(Controller, RunsFanWhileBeansStillHot) {
  FakeController controller;
  controller.Init();
  controller.SetBeanTempF(70);
  controller.SetEnvTempF(70);
  controller.SetAmbientTempF(70);
  controller.SetFanTarget(0);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 0);

  controller.SetBeanTempF(200);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 255);

  controller.SetBeanTempF(70);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 0);

  controller.SetBeanTempF(200);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 255);
  controller.SetFanTarget(200);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 200);
  controller.SetBeanTempF(70);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 200);
}

TEST(Controller, RunsFanWhileEnvStillHot) {
  FakeController controller;
  controller.Init();
  controller.SetBeanTempF(70);
  controller.SetEnvTempF(70);
  controller.SetAmbientTempF(70);
  controller.SetFanTarget(0);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 0);

  controller.SetEnvTempF(200);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 255);

  controller.SetEnvTempF(70);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 0);

  controller.SetEnvTempF(200);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 255);
  controller.SetFanTarget(200);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 200);
  controller.SetEnvTempF(70);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), 200);
}

TEST(Controller, StirsWhileHot) {
  FakeController controller;
  controller.Init();
  controller.SetBeanTempF(70);
  controller.SetEnvTempF(70);
  controller.SetAmbientTempF(70);
  controller.SetFanTarget(0);
  controller.Step();
  EXPECT_EQ(controller.GetStirValue(), false);

  controller.SetBeanTempF(200);
  controller.Step();
  EXPECT_EQ(controller.GetStirValue(), true);

  controller.SetBeanTempF(70);
  controller.Step();
  EXPECT_EQ(controller.GetStirValue(), false);
}

TEST(RunnerStatus, FaultyOnNoComms) {
  RunnerStatus status;
  EXPECT_EQ(status.fault_since_reset.Faulty(), false);

  status.fault_since_reset.no_comms = true;
  EXPECT_EQ(status.fault_since_reset.Faulty(), true);
}

TEST(Controller, SafeModeOnFault) {
  FakeController controller;
  controller.Init();
  controller.SetBeanTempF(70);
  controller.SetEnvTempF(70);
  controller.SetAmbientTempF(70);
  controller.SetFanTarget(0);
  controller.Step();
  EXPECT_EQ(controller.GetFanValue(), false);
  EXPECT_EQ(controller.GetStirValue(), false);

  controller.SetBeanTempReadError(1);
  for (uint32_t i = 0; i < 10; i++) {
    AdvanceMillis(200);
    controller.fault_filter_.SetMillis(millis());
    controller.Step();
  }
  EXPECT_EQ(controller.GetStatus().fault_since_reset.Faulty(), true);
  EXPECT_EQ(controller.GetStatus().fatal_fault, true);
  EXPECT_EQ(controller.GetFanValue(), 255);
  EXPECT_EQ(controller.GetStirValue(), true);

  controller.SetBeanTempReadError(0);
  for (uint32_t i = 0; i < 100; i++) {
    AdvanceMillis(200);
    controller.fault_filter_.SetMillis(millis());
    controller.Step();
    EXPECT_EQ(controller.GetStatus().fatal_fault, true);
    EXPECT_EQ(controller.GetFanValue(), 255);
    EXPECT_EQ(controller.GetStirValue(), true);
  }
}

}  // namespace

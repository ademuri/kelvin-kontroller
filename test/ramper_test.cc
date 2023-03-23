#include <gtest/gtest.h>

#include "types.h"

#include "ramper.h"

namespace {

TEST(Ramper, WorksNormallyBelowRamp) {
  Ramper ramper{/*period=*/10, /*max_change=*/ 10};

  EXPECT_EQ(ramper.Get(), 0);

  ramper.SetTarget(0);
  EXPECT_EQ(ramper.Get(), 0);
  EXPECT_EQ(ramper.Step(), 0);
  EXPECT_EQ(ramper.Get(), 0);

  ramper.SetTarget(9);
  EXPECT_EQ(ramper.Get(), 0);
  EXPECT_EQ(ramper.Step(), 9);
  EXPECT_EQ(ramper.Get(), 9);

  ramper.SetTarget(0);
  EXPECT_EQ(ramper.Get(), 0);
  EXPECT_EQ(ramper.Step(), 0);
  EXPECT_EQ(ramper.Get(), 0);
}

TEST(Ramper, Ramps) {
  Ramper ramper{/*period=*/10, /*max_change=*/ 10};

  ramper.SetTarget(21);
  EXPECT_EQ(ramper.Get(), 0);
  EXPECT_EQ(ramper.Step(), 10);
  AdvanceMillis(9);
  EXPECT_EQ(ramper.Step(), 10);
  AdvanceMillis(1);
  EXPECT_EQ(ramper.Step(), 20);
  AdvanceMillis(10);
  EXPECT_EQ(ramper.Step(), 21);

  ramper.SetTarget(1);
  EXPECT_EQ(ramper.Get(), 1);
  EXPECT_EQ(ramper.Step(), 1);
}

};

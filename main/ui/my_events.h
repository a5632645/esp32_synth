#pragma once

#include <cstdint>

namespace events {
static constexpr uint16_t kButtonUp = 0;
static constexpr uint16_t kButtonDown = 1;

static constexpr uint16_t kMidiOn = 2;
static constexpr uint16_t kMidiOff = 3;

static constexpr uint16_t kButton13 = 12;
static constexpr uint16_t kButton14 = 13;
static constexpr uint16_t kButton15 = 14;
static constexpr uint16_t kButton16 = 15;

static constexpr uint16_t kEnterComp = kButton13;
static constexpr uint16_t kExitComp  = kButton15;
};
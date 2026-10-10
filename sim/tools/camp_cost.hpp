#pragma once
#include <span>
#include <string_view>
namespace kd::tool {
int camp_cost(std::span<const std::string_view> args);
}

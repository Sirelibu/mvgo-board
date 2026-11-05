#pragma once

#include <vector>
#include <string_view>

#include "departure/Departure.hpp"

namespace parser {

std::vector<Departure> parseHttpResp(std::string_view json);

} // namespace parser

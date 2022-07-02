#ifndef THERMOMETER_HPP
#define THERMOMETER_HPP

#include <optional>

#include "Device.hpp"

class Thermometer : public Device
{
public:
   [[nodiscard]] virtual std::optional<float> temperature() = 0;
};

#endif // THERMOMETER_HPP

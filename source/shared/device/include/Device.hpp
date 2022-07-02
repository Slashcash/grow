#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <string>

class Device
{
public:
   Device() = default;
   Device(const Device& device) = delete;
   Device(const Device&& device) = delete;
   auto operator=(const Device& device) = delete;
   auto operator=(const Device&& device) = delete;

   [[nodiscard]] virtual std::string name() const noexcept = 0;

   virtual ~Device() = default;
};

#endif // DEVICE_HPP

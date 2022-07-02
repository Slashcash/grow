#include "Temperature.hpp"

#include <nlohmann/json.hpp>

#include "SimulatedThermometer.hpp"

Temperature::Temperature()
{
#ifndef THERMOMETER_DEVICE
   static_assert(false, "The thermometer device was not set for this build");
#else
   if (std::string(THERMOMETER_DEVICE) == "SimulatedThermometer")
   {
      mThermometer = std::make_unique<SimulatedThermometer>();
   }
   else
   {
      logger().warn(
          "No valid thermometer device specified at build time, going with simulated hardware");
      mThermometer = std::make_unique<SimulatedThermometer>();
   }
#endif
}

void Temperature::mainLoop()
{
   // Registering and publishing temperature
   auto measured = mThermometer->temperature();

   if (measured.has_value())
   {
      logger().debug("Registered temperature {}C", measured.value());
      auto publishError = publishTemperature(measured.value());
      if (publishError.has_value())
      {
         logger().err("Unable to publish temperature value: {}", publishError.value().asString());
      }
      else
      {
         logger().trace("Temperature value published ({}C)", measured.value());
      }
   }
   else
   {
      logger().err("Unable to get temperature from {}", mThermometer->name());
   }

   std::this_thread::sleep_for(std::chrono::milliseconds(pollTime()));
}

bool Temperature::onStarted()
{
   logger().info("Using {} as measuring hardware", mThermometer->name());

   // Checking if the polling period has a valid configuration value, emit a warning otherwise
   if (!settingValue<unsigned int>(POLL_TIME_CFG).has_value())
   {
      logger().warn("Polling time has no valid configuration value, using default instead ({}msec)",
                    POLL_TIME_DEFAULT);
   }

   return true;
}

unsigned int Temperature::pollTime()
{
   auto result = settingValue<unsigned int>(POLL_TIME_CFG);
   if (!result.has_value())
   {
      logger().trace(
          "Polling time has no valid configuration value, using default instead ({}msec)",
          POLL_TIME_DEFAULT);

      return POLL_TIME_DEFAULT;
   }

   return result.value();
}

void Temperature::onStopped()
{
}

std::optional<Error<Temperature::PublishError>>
Temperature::publishTemperature(const float temperature)
{
   nlohmann::json message;
   message["temperature"] = temperature;
   return publish(TEMPERATURE_TOPIC, message);
}

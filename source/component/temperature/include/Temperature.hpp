#ifndef TEMPERATURE_HPP
#define TEMPERATURE_HPP

#include <memory>

#include "TemperatureBase.hpp"
#include "Thermometer.hpp"

class Temperature : public TemperatureBase
{
public:
   Temperature();

private:
   static constexpr unsigned int POLL_TIME_DEFAULT = 1000;
   static constexpr const char* POLL_TIME_CFG = "poll_time";
   static constexpr const char* TEMPERATURE_TOPIC = "TEMPERATURE";

   std::unique_ptr<Thermometer> mThermometer;

   [[nodiscard]] unsigned int pollTime();
   [[nodiscard]] bool onStarted() override;
   void onStopped() override;
   void mainLoop() override;

   [[nodiscard]] std::optional<Error<PublishError>> publishTemperature(float temperature);
};

#endif // TEMPERATURE_HPP

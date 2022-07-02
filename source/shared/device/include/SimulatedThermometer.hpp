#ifndef SIMULATEDTHERMOMETER_HPP
#define SIMULATEDTHERMOMETER_HPP

#include <random>

#include "Thermometer.hpp"

class SimulatedThermometer : public Thermometer
{
private:
   unsigned int mCalled = 0;
   float mStartingPoint;
   std::mt19937 rng;

public:
   SimulatedThermometer();

   [[nodiscard]] inline std::string name() const noexcept override
   {
      return "SimulatedThermometer";
   }

   [[nodiscard]] std::optional<float> temperature() override;
};

#endif // SIMULATEDTHERMOMETER_HPP

#include "SimulatedThermometer.hpp"

SimulatedThermometer::SimulatedThermometer() : rng(std::random_device{}())
{
   constexpr float RANDOM_CENTER = 15;
   constexpr float RADIUS = 5;
   std::uniform_real_distribution<float> dist(RANDOM_CENTER - RADIUS, RANDOM_CENTER + RADIUS);

   mStartingPoint = dist(rng);
}

std::optional<float> SimulatedThermometer::temperature()
{
   constexpr unsigned int APPLY_RANDOM = 5;
   mCalled++;

   if ((mCalled % APPLY_RANDOM) == 0)
   {
      constexpr float LO = -1.0;
      constexpr float HI = 1.0;

      std::uniform_real_distribution<float> dist(LO, HI);
      auto r = dist(rng);
      mStartingPoint += r;

      return mStartingPoint;
   }

   return mStartingPoint;
}

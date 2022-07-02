#ifndef TEST_HPP
#define TEST_HPP

#include "Component.hpp"

class TestComponent : public Component
{
public:
   static constexpr unsigned int PORT = 7000;
   static constexpr const char* TOPIC = "topic";
   static constexpr const char* PAYLOAD = "payload";

   bool mTestVariableStarting = false;
   bool mTestVariableStopping = false;
   bool mTestVariableLooping = false;
   bool infinite = false;

private:
   inline bool onStarted() override
   {
      mTestVariableStarting = true;
      return true;
   }

   inline void onStopped() override
   {
      mTestVariableStopping = true;
   }

   inline void mainLoop() override
   {
      mTestVariableLooping = true;
      if (infinite)
      {
         std::this_thread::sleep_for(std::chrono::milliseconds(250));
      }
      else
      {
         stop();
      }
   }
};

#endif

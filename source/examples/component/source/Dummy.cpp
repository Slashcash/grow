#include "Dummy.hpp"

void Dummy::mainLoop()
{
   logger().info("I'm a dummy example, I'm just waiting and printing as my main activity. Sorry.");
   std::this_thread::sleep_for(std::chrono::seconds(1));
}

bool Dummy::onStarted()
{
   logger().info("I'm a dummy example, I'm not doing anything at startup. Sorry.");

   return true;
}

void Dummy::onStopped()
{
   logger().info("I'm a dummy example, I'm not doing anything when stopping. Sorry.");
}

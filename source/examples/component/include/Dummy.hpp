#ifndef DUMMY_HPP
#define DUMMY_HPP

#include "DummyBase.hpp"

class Dummy : public DummyBase
{
private:
   [[nodiscard]] bool onStarted() override;
   void onStopped() override;
   void mainLoop() override;
};

#endif // DUMMY_HPP

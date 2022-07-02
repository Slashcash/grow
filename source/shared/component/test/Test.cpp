#include <fstream>

#include "gtest/gtest.h"

#include "Test.hpp"

TEST(Component, Lifecycle)
{
   TestComponent c;
   c.setLogLevel(Logger::Level::off);
   auto result = c.startBlocking();
   ASSERT_TRUE(result) << "Unable to start component";
   ASSERT_TRUE(c.mTestVariableStarting) << "Starting function did not execute correctly";
   ASSERT_TRUE(c.mTestVariableLooping) << "Looping function did not execute correctly";
   ASSERT_TRUE(c.mTestVariableStopping) << "Stopping function did not execute correctly";

   result = c.stop();
   ASSERT_FALSE(result) << "Component was stopped twice";
}

TEST(Component, AlreadyRunning)
{
   TestComponent c;
   c.infinite = true;
   c.setLogLevel(Logger::Level::off);
   auto result = c.start();
   ASSERT_TRUE(result) << "Unable to start component";
   result = c.start();
   ASSERT_FALSE(result) << "Component was run twice";
   result = c.stop();
   ASSERT_TRUE(result) << "Unable to stop component";
}

TEST(Component, NoConfiguration)
{
   TestComponent c;
   c.infinite = true;
   c.setLogLevel(Logger::Level::off);
   auto result = c.start();
   ASSERT_TRUE(result) << "Unable to start component";
   auto settingResult = c.settingValue<unsigned int>("temp");
   EXPECT_FALSE(settingResult.has_value());
   result = c.stop();
   ASSERT_TRUE(result) << "Unable to stop component";
}

TEST(Component, InvalidConfiguration)
{
   TestComponent c;
   c.infinite = true;
   c.setLogLevel(Logger::Level::off);
   c.setConfigurationPath("tmp/nonexistant");
   auto result = c.start();
   ASSERT_FALSE(result) << "Component started with unexisting configuration file";
   result = c.stop();
   ASSERT_FALSE(result) << "Component started with unexisting configuration file";
}

TEST(Component, ReadConfiguration)
{
   const std::string filePath = "/tmp/grow_component_test.cfg";
   const std::string field = "Game";
   const std::string value = "Metal Gear Solid";

   TestComponent c;

   std::ofstream stream(filePath);
   stream << "project_version = \"" << c.version() << "\";\n"
          << c.name() << " = { " << field << " = \"" << value << "\" };";
   stream.close();
   ASSERT_TRUE(stream) << "Unable to write test file to " << filePath;

   c.infinite = false;
   c.setLogLevel(Logger::Level::off);
   c.setConfigurationPath(filePath);
   auto result = c.startBlocking();
   ASSERT_TRUE(result) << "Unable to start component";
   auto configValue = c.settingValue<std::string>(field);
   ASSERT_TRUE(configValue.has_value());
   ASSERT_EQ(configValue.value(), value);
}

TEST(Component, UnexistingValue)
{
   const std::string filePath = "/tmp/grow_component_test.cfg";
   const std::string field = "Game";
   const std::string value = "Metal Gear Solid";

   TestComponent c;

   std::ofstream stream(filePath);
   stream << "project_version = \"" << c.version() << "\";\n"
          << c.name() << " = { " << field << " = \"" << value << "\" };";
   stream.close();
   ASSERT_TRUE(stream) << "Unable to write test file to " << filePath;

   c.infinite = false;
   c.setLogLevel(Logger::Level::off);
   c.setConfigurationPath(filePath);
   auto result = c.startBlocking();
   ASSERT_TRUE(result) << "Unable to start component";
   auto configValue = c.settingValue<std::string>("nonexistant");
   ASSERT_FALSE(configValue.has_value());
}

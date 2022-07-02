#include <fstream>

#include "gtest/gtest.h"

#include "Configuration.hpp"

TEST(Configuration, MissingFile)
{
   Configuration cfg;
   auto loadResult = cfg.loadFromFile("/tmp/missing_conf.cfg");
   ASSERT_TRUE(loadResult.has_value()) << "No error thrown when the configuration file is missing";
   ASSERT_EQ(loadResult.value(), Configuration::ConfigurationError::UNABLE_TO_LOAD_FILE)
       << "Erroneous error reported for a missing configuration file";
}

TEST(Configuration, UnparsableFile)
{
   const std::string filePath = "/tmp/grow_config_test.cfg";

   std::ofstream stream(filePath);
   stream << "Hey this is a wrong configuration file";
   stream.close();
   ASSERT_TRUE(stream) << "Unable to write test file to " << filePath;

   Configuration cfg;
   auto loadResult = cfg.loadFromFile(filePath);
   ASSERT_TRUE(loadResult.has_value())
       << "No error thrown when the configuration file is unparsable";
   ASSERT_EQ(loadResult.value(), Configuration::ConfigurationError::UNABLE_TO_PARSE_FILE)
       << "Erroneous error reported for an unparsable file";
}

TEST(Configuration, ReadFromFile)
{
   const std::string filePath = "/tmp/grow_config_test.cfg";
   const std::string field = "Game";
   const std::string value = "Metal Gear Solid";

   std::ofstream stream(filePath);
   stream << field << " = \"" << value << "\"";
   stream.close();
   ASSERT_TRUE(stream) << "Unable to write test file to " << filePath;

   Configuration cfg;
   auto loadResult = cfg.loadFromFile(filePath);
   ASSERT_FALSE(loadResult.has_value()) << "Unable to read test file from " << filePath;

   auto confValue = cfg.settingValue<std::string>(field);

   ASSERT_TRUE(confValue.has_value()) << "Unable to retrieve configuration value";
   ASSERT_EQ(confValue.value(), value);
}

TEST(Configuration, SetValue)
{
   const std::string filePath = "/tmp/grow_config_test.cfg";
   const std::string field = "Game";
   const std::string value = "Metal Gear Solid";
   const std::string newValue = "Bioshock";

   std::ofstream stream(filePath);
   stream << field << " = \"" << value << "\"";
   stream.close();
   ASSERT_TRUE(stream) << "Unable to write test file to " << filePath;

   Configuration cfg;
   auto loadResult = cfg.loadFromFile(filePath);
   ASSERT_FALSE(loadResult.has_value()) << "Unable to read test file from " << filePath;

   ASSERT_FALSE(cfg.setValue(field, newValue).has_value())
       << "Setting value in configuration failed";
   ASSERT_EQ(cfg.settingValue<std::string>(field), newValue)
       << "New value has not been set correctly";
}

TEST(Configuration, WriteToFile)
{
   const std::string filePath = "/tmp/grow_config_test.cfg";
   const std::string field = "Game";
   const std::string value = "Metal Gear Solid";
   const std::string newValue = "Bioshock";

   std::ofstream stream(filePath);
   stream << field << " = \"" << value << "\"";
   stream.close();
   ASSERT_TRUE(stream) << "Unable to write test file to " << filePath;

   Configuration cfg;
   auto loadResult = cfg.loadFromFile(filePath);
   ASSERT_FALSE(loadResult.has_value()) << "Unable to read test file from " << filePath;

   ASSERT_FALSE(cfg.setValue(field, newValue).has_value())
       << "Setting value in configuration failed";

   ASSERT_FALSE(cfg.writeToFile(filePath).has_value())
       << "Unable to write new configuration to file " << filePath;
   ASSERT_FALSE(cfg.loadFromFile(filePath).has_value()) << "Unable to reload from " << filePath;
   ASSERT_EQ(cfg.settingValue<std::string>(field), newValue)
       << "New configuration has not been wrote correctly";
}

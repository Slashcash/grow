#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <filesystem>
#include <sstream>

#include "libconfig.h++"

#include "Error.hpp"

class Configuration
{
public:
   using Setting = libconfig::Setting;
   enum class ConfigurationError
   {
      UNABLE_TO_LOAD_FILE,
      UNABLE_TO_PARSE_FILE,
      UNABLE_TO_WRITE_FILE,
      SETTING_NOT_FOUND,
      SETTING_TYPE_MISMATCH
   };

   Configuration() = default;
   Configuration(const Configuration& config) = delete;
   Configuration(const Configuration&& config) = delete;
   auto operator=(const Configuration& config) = delete;
   auto operator=(const Configuration&& config) = delete;

   [[nodiscard]] inline bool contains(const std::string& path) const
   {
      return mConfiguration.exists(path);
   }

   [[nodiscard]] std::optional<Error<ConfigurationError>>
   loadFromFile(const std::filesystem::path& filePath) noexcept;

   [[nodiscard]] std::optional<Error<ConfigurationError>>
   writeToFile(const std::filesystem::path& filePath) noexcept;

   template <class T>[[nodiscard]] inline auto settingValue(const std::string& path) const noexcept
   {
      static_assert(std::is_integral_v<T> || std::is_same_v<T, std::string>,
                    "Invalid template type for retrieving a setting value");

      T result;
      if (!mConfiguration.lookupValue(path, result))
      {
         return std::optional<T>{};
      }
      return std::optional<T>{result};
   }

   template <class T>
   [[nodiscard]] inline auto setValue(const std::string& path, const T& value) noexcept
   {
      static_assert(std::is_integral_v<T> || std::is_same_v<T, std::string>,
                    "Invalid template type for retrieving a setting value");
      try
      {
         auto& setting = mConfiguration.getRoot().lookup(path);
         setting = value;
      }
      catch (const libconfig::SettingNotFoundException& sex)
      {
         return make_optional_error<ConfigurationError>(ConfigurationError::SETTING_NOT_FOUND,
                                                        "Setting not found at " + path);
      }
      catch (const libconfig::SettingTypeException& sex)
      {
         return make_optional_error<ConfigurationError>(ConfigurationError::SETTING_TYPE_MISMATCH,
                                                        "Setting type mismatch at " + path);
      }

      return make_optional_error<ConfigurationError>();
   }

   ~Configuration() = default;

private:
   libconfig::Config mConfiguration;
};

#endif // CONFIGURATION_HPP

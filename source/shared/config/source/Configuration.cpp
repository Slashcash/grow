#include "Configuration.hpp"

std::optional<Error<Configuration::ConfigurationError>>
Configuration::loadFromFile(const std::filesystem::path& filePath) noexcept
{
   try
   {
      mConfiguration.clear();
      mConfiguration.readFile(filePath);
   }

   catch (const libconfig::FileIOException& fioex)
   {
      std::ostringstream errorText;
      errorText << "Unable to load file at " << filePath << " (" << fioex.what() << ")";

      return make_optional_error<ConfigurationError>(ConfigurationError::UNABLE_TO_LOAD_FILE,
                                                     errorText.str());
   }
   catch (const libconfig::ParseException& pex)
   {
      std::ostringstream errorText;
      errorText << "Unable to parse file at " << filePath << "(Line: " << pex.getLine()
                << " Error: " << pex.getError() << ")";

      return make_optional_error<ConfigurationError>(ConfigurationError::UNABLE_TO_PARSE_FILE,
                                                     errorText.str());
   }

   return make_optional_error<ConfigurationError>();
}

std::optional<Error<Configuration::ConfigurationError>>
Configuration::writeToFile(const std::filesystem::path& filePath) noexcept
{
   try
   {
      mConfiguration.writeFile(filePath);
   }

   catch (const libconfig::FileIOException& fioex)
   {
      std::ostringstream errorText;
      errorText << "Unable to write file at " << filePath << " (" << fioex.what() << ")";

      return make_optional_error<ConfigurationError>(ConfigurationError::UNABLE_TO_WRITE_FILE,
                                                     errorText.str());
   }

   return make_optional_error<ConfigurationError>();
}

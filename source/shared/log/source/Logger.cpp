#include <iostream>

#include "Logger.hpp"
#include "spdlog/spdlog.h"

Logger::Logger(const std::string& name) : mFormatter(name)
{
   try
   {
      if (!(mLogger = spdlog::get(name)))
      {

         mLogger = spdlog::stdout_color_mt(name);
         mLogger->set_error_handler([this](const std::string& msg) {
            err("Trying to log an invalid message: ({})", msg);
         });

         auto formatter = std::make_unique<spdlog::pattern_formatter>();
         formatter->add_flag<NameFormatter>('*', mFormatter)
             .set_pattern("[%H:%M:%S %z] [%*] [%^%L%$] %v");
         mLogger->set_formatter(std::move(formatter));

         mInitialized = true;
      }
   }
   catch (const spdlog::spdlog_ex& ex)
   {
      std::cout << "Logger initialization failed: " << ex.what() << std::endl;
      mInitialized = false;
   }
}

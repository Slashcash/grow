#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class Logger final
{
private:
   class NameFormatter : public spdlog::custom_flag_formatter
   {
   private:
      std::string mName;

   public:
      NameFormatter() = delete;
      explicit inline NameFormatter(const std::string& name)
      {
         mName = name;
      }

      void inline format([[maybe_unused]] const spdlog::details::log_msg& logMsg,
                         [[maybe_unused]] const std::tm& tm, spdlog::memory_buf_t& dest) override
      {
         dest.append(mName.data(), mName.data() + mName.size());
      }

      [[nodiscard]] inline std::unique_ptr<custom_flag_formatter> clone() const override
      {
         return spdlog::details::make_unique<NameFormatter>(mName);
      }

      [[nodiscard]] inline std::string name() const
      {
         return mName;
      }

      void inline setName(const std::string& name)
      {
         mName = name;
      }
   };

   std::shared_ptr<spdlog::logger> mLogger;
   bool mInitialized = false;
   std::string mName;
   NameFormatter mFormatter;

public:
   using Level = spdlog::level::level_enum;

   inline Logger() : Logger("general"){};
   explicit Logger(const std::string& name);
   Logger(const Logger& config) = delete;
   Logger(const Logger&& config) = delete;
   auto operator=(const Logger& config) = delete;
   auto operator=(const Logger&& config) = delete;

   template <typename... Args>
   inline void log(const Level& level, const std::string& msg, Args... args)
   {
      if (mInitialized)
      {
         mLogger->log(level, msg, args...);
      }
   }

   template <typename... Args> inline void trace(const std::string& msg, Args... args)
   {
      log(Level::trace, msg, args...);
   }

   template <typename... Args> inline void debug(const std::string& msg, Args... args)
   {
      log(Level::debug, msg, args...);
   }

   template <typename... Args> inline void info(const std::string& msg, Args... args)
   {
      log(Level::info, msg, args...);
   }

   template <typename... Args> inline void warn(const std::string& msg, Args... args)
   {
      log(Level::warn, msg, args...);
   }

   template <typename... Args> inline void err(const std::string& msg, Args... args)
   {
      log(Level::err, msg, args...);
   }

   template <typename... Args> inline void critical(const std::string& msg, Args... args)
   {
      log(Level::critical, msg, args...);
   }

   void inline setLevel(const Level& level)
   {
      if (mInitialized)
      {
         mLogger->set_level(level);
      }
   };

   void inline setLevel(const std::string& level)
   {
      setLevel(spdlog::level::from_str(level));
   }

   [[nodiscard]] inline Level level() const
   {
      if (!mInitialized)
      {
         return Level::off;
      }

      return mLogger->level();
   }

   [[nodiscard]] inline std::string name() const
   {
      return mFormatter.name();
   }

   ~Logger() = default;
};

#endif // LOGGER_HPP

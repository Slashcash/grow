#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <tcp_pubsub/executor.h>
#include <tcp_pubsub/publisher.h>
#include <tcp_pubsub/subscriber.h>
#include <thread>

#include "Configuration.hpp"
#include "Error.hpp"
#include "Logger.hpp"

class Component
{
public:
   enum class PublishError
   {
      UNABLE_TO_SEND,
      NETWORK_CONFIGURATION_MISSING,
      INVALID_TOPIC,
      INVALID_PAYLOAD
   };

   enum class SubscriberError
   {
      NETWORK_CONFIGURATION_MISSING,
      INVALID_TOPIC,
      INVALID_PAYLOAD,
      ALREADY_SUBSCRIBED
   };

   using SubscriberCallback = std::function<void(const std::optional<Error<SubscriberError>>&,
                                                 const std::string&, const nlohmann::json&)>;

   Component();
   Component(const Component& component) = delete;
   Component(const Component&& component) = delete;
   auto operator=(const Component& component) = delete;
   auto operator=(const Component&& component) = delete;

   [[nodiscard]] virtual inline std::string name() const noexcept
   {
      return DEFAULT_NAME;
   }
   [[nodiscard]] virtual inline std::string description() const noexcept
   {
      return DEFAULT_DESCRIPTION;
   }
   [[nodiscard]] virtual inline std::string version() const noexcept
   {
      return DEFAULT_VERSION;
   }

   [[nodiscard]] virtual inline std::string signature() const noexcept
   {
      return name() + " v" + version();
   }

   void inline setLogLevel(const Logger::Level& level)
   {
      logger().setLevel(level);
   };

   void inline setLogLevel(const std::string& level)
   {
      logger().setLevel(level);
   };

   [[nodiscard]] virtual bool inline running() const noexcept final
   {
      return mShouldRun;
   }

   virtual inline void setConfigurationPath(const std::filesystem::path& filePath) noexcept final
   {
      mConfigFilePath = filePath;
   }

   template <class T>[[nodiscard]] inline auto settingValue(const std::string& path) const noexcept
   {
      return mConfiguration.settingValue<T>(name() + "." + path);
   }

   [[nodiscard]] virtual bool start() final;
   virtual bool stop() final;
   [[nodiscard]] virtual bool startBlocking() final;
   virtual bool stopBlocking() final;

   [[nodiscard]] virtual bool parseCmdArguments(int argc, char** argv) final;

   [[nodiscard]] std::optional<Error<PublishError>> publish(const std::string& topic,
                                                            const nlohmann::json& payload);

   [[nodiscard]] std::optional<Error<SubscriberError>>
   subscribe(const std::string& componentName, const SubscriberCallback& callback);

   ~Component();

protected:
   [[nodiscard]] virtual inline Logger& logger() noexcept final
   {
      if (!mLogger)
      {
         mLogger = std::make_unique<Logger>(name());
      }

      return *mLogger;
   }

private:
   const std::string DEFAULT_NAME = "Generic";
   const std::string DEFAULT_DESCRIPTION = "This is a generic component";
   const std::string DEFAULT_VERSION = "UNKNOWN_VERSION";
   static constexpr unsigned int MAXIMUM_PUBSUB_THREADS = 6;
   static constexpr char PUBSUB_TOPIC_DELIMITER = '#';
   static constexpr const char* JSON_COMPONENT_VERSION_FIELD = "version";

   std::unique_ptr<Logger> mLogger;
   static Component* mInstance;
   std::mutex mMutex;
   std::condition_variable mConditionVariable;

   std::unique_ptr<std::thread> mThread;
   std::atomic_bool mShouldRun = false;
   std::atomic_bool mGracefulStop = true;
   bool mFinished = false;
   std::optional<std::filesystem::path> mConfigFilePath;
   Configuration mConfiguration;
   std::shared_ptr<tcp_pubsub::Executor> mPubSubExecutor;
   std::map<const unsigned int, tcp_pubsub::Publisher> mPublisherMap;
   std::map<const unsigned int, tcp_pubsub::Subscriber> mSubscriberMap;

   static void stopSignalHandler(int signal);
   [[nodiscard]] virtual bool onStarted() = 0;
   virtual void onStopped() = 0;
   virtual void mainLoop() = 0;

   [[nodiscard]] std::optional<Error<PublishError>> publish(unsigned int port,
                                                            const std::string& payload);

   [[nodiscard]] std::optional<Error<PublishError>>
   publish(unsigned int port, const std::string& topic, const std::string& payload);

   [[nodiscard]] std::optional<Error<PublishError>> publish(const std::string& topic,
                                                            const std::string& payload);

   [[nodiscard]] std::optional<Error<SubscriberError>>
   subscribe(unsigned int port, const SubscriberCallback& callback);

   [[nodiscard]] inline auto publishPort(const std::string& componentName) const
   {
      return mConfiguration.settingValue<unsigned int>(std::string("Publisher") + "." +
                                                       componentName);
   }
};

#endif // COMPONENT_HPP

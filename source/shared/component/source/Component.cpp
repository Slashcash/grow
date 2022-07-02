#include "Component.hpp"
#include "cxxopts.hpp"

#include <csignal>

Component* Component::mInstance = nullptr;

Component::Component()
{
   if (mInstance != nullptr)
   {
      throw std::runtime_error("Created two components within the same context");
   }
   mInstance = this;
   std::signal(SIGINT, &Component::stopSignalHandler);
}

bool Component::start()
{
   if (!running())
   {
      logger().info("Starting component");

      if (mConfigFilePath.has_value())
      {
         logger().info("Loading configuration file from {}", mConfigFilePath.value().string());
         auto loadError = mConfiguration.loadFromFile(mConfigFilePath.value());
         if (loadError.has_value())
         {
            logger().err("Error loading configuration file: {}", loadError.value().asString());
            return false;
         }

         auto configVersion = mConfiguration.settingValue<std::string>("project_version");
         if (!configVersion.has_value())
         {
            logger().err("Configuration file does not contain project version information");
            return false;
         }

         if (configVersion.value() != version())
         {
            logger().err("Configuration file version does not match project version {} vs {}",
                         configVersion.value(), version());
            return false;
         }
      }

      if (!onStarted())
      {
         return false;
      }

      mFinished = false;

      mShouldRun = true;
      mThread = std::make_unique<std::thread>([this]() {
         while (mShouldRun)
         {
            mainLoop();
         }

         if (mGracefulStop)
         {
            onStopped();
         }

         {
            std::lock_guard lk(mMutex);
            mFinished = true;
         }
         mConditionVariable.notify_one();
      });
   }
   else
   {
      logger().warn("Trying to start an already running component. No effect");
      return false;
   }

   return true;
}

bool Component::stop()
{
   if (running())
   {
      logger().info("Stopping component");
      mShouldRun = false;
   }
   else
   {
      logger().warn("Trying to stop an already stopped component. No effect");
      return false;
   }

   return true;
}

bool Component::startBlocking()
{
   if (!start())
   {
      return false;
   }

   {
      std::unique_lock lk(mMutex);
      mConditionVariable.wait(lk, [this] { return mFinished; });
   }

   return true;
}

bool Component::stopBlocking()
{
   auto result = stop();

   {
      std::unique_lock lk(mMutex);
      mConditionVariable.wait(lk, [this] { return mFinished; });
   }

   return result;
}

void Component::stopSignalHandler([[maybe_unused]] int signal)
{
   mInstance->stop();
}

Component::~Component()
{
   mShouldRun = false;
   mGracefulStop = false;
   if (mThread)
   {
      mThread->join();
   }
   mInstance = nullptr;
   std::signal(SIGINT, SIG_DFL);
}

bool Component::parseCmdArguments(int argc, char** argv)
{
   cxxopts::Options options{name(), description()};

   try
   {
      const std::string LOGLEVEL_STR_L = "loglevel";
      const std::string HELP_STR_L = "help";
      const std::string CONFIGPATH_STR_L = "config";

      const std::string LOGLEVEL_STR_S = "l";
      const std::string HELP_STR_S = "h";
      const std::string CONFIGPATH_STR_S = "c";

      options.add_options()(CONFIGPATH_STR_S + "," + CONFIGPATH_STR_L,
                            "Path of the configuration file", cxxopts::value<std::string>())(
          LOGLEVEL_STR_S + "," + LOGLEVEL_STR_L, "Logging level",
          cxxopts::value<std::string>()->default_value("info"))(HELP_STR_S + "," + HELP_STR_L,
                                                                "Print usage");

      auto result = options.parse(argc, argv);

      if (static_cast<bool>(result.count(HELP_STR_L)))
      {
         std::cout << options.help() << std::endl;
         return false;
      }

      // Setting the parsed log level
      setLogLevel(result[LOGLEVEL_STR_L].as<std::string>());

      // Setting the parsed configuration file
      if (static_cast<bool>(result.count(CONFIGPATH_STR_L)))
      {
         mConfigFilePath = result["config"].as<std::string>();
      }
   }
   catch (const cxxopts::OptionException& exp)
   {
      std::cout << exp.what() << std::endl;
      std::cout << options.help() << std::endl;
      return false;
   }

   return true;
}

std::optional<Error<Component::PublishError>> Component::publish(const unsigned int port,
                                                                 const std::string& payload)
{
   if (!mPubSubExecutor)
   {
      mPubSubExecutor = std::make_shared<tcp_pubsub::Executor>(
          MAXIMUM_PUBSUB_THREADS,
          [](const tcp_pubsub::logger::LogLevel& logLevel, const std::string& msg) {});
   }

   auto publisher = mPublisherMap.try_emplace(port, mPubSubExecutor, port).first;
   if (!publisher->second.send(&payload[0], payload.size()))
   {
      return make_optional_error<PublishError>(PublishError::UNABLE_TO_SEND,
                                               "Error in sending payload");
   }

   return make_optional_error<PublishError>();
}

std::optional<Error<Component::PublishError>>
Component::publish(const unsigned int port, const std::string& topic, const std::string& payload)
{
   if (topic.empty())
   {
      return make_optional_error<PublishError>(PublishError::INVALID_TOPIC,
                                               "Trying to publish with empty topic");
   }

   if (topic.find(PUBSUB_TOPIC_DELIMITER) != std::string::npos)
   {
      return make_optional_error<PublishError>(PublishError::INVALID_TOPIC,
                                               "Topic contains invalid character");
   }

   if (payload.empty())
   {
      return make_optional_error<PublishError>(PublishError::INVALID_PAYLOAD,
                                               "Trying to publish with empty payload");
   }

   return publish(port, topic + PUBSUB_TOPIC_DELIMITER + payload);
}

std::optional<Error<Component::PublishError>> Component::publish(const std::string& topic,
                                                                 const std::string& payload)
{
   auto port = publishPort(name());
   if (!port.has_value())
   {
      return make_optional_error<PublishError>(
          PublishError::NETWORK_CONFIGURATION_MISSING,
          "Network configuration is missing for this component");
   }

   return publish(port.value(), topic, payload);
}

std::optional<Error<Component::SubscriberError>>
Component::subscribe(unsigned int port, const SubscriberCallback& callback)
{
   if (!mPubSubExecutor)
   {
      mPubSubExecutor = std::make_shared<tcp_pubsub::Executor>(
          MAXIMUM_PUBSUB_THREADS,
          [](const tcp_pubsub::logger::LogLevel& logLevel, const std::string& msg) {});
   }

   auto subscriberPair = mSubscriberMap.try_emplace(port, mPubSubExecutor);
   if (!subscriberPair.second)
   {
      return make_optional_error<SubscriberError>(
          SubscriberError::ALREADY_SUBSCRIBED, "Subscribe requested on an already subscribed port");
   }

   auto& subscriber = subscriberPair.first->second;

   subscriber.setCallback([this, callback](const tcp_pubsub::CallbackData& callback_data) {
      const auto receivedMessage =
          std::string(callback_data.buffer_->data(), callback_data.buffer_->size());

      const auto delimiterPos = receivedMessage.find(PUBSUB_TOPIC_DELIMITER);
      if (delimiterPos == std::string::npos || delimiterPos == 0)
      {
         callback(make_optional_error<SubscriberError>(SubscriberError::INVALID_TOPIC,
                                                       "Message received with empty topic"),
                  "", "");
         return;
      }
      if (delimiterPos + 1 == std::string::npos)
      {
         callback(make_optional_error<SubscriberError>(SubscriberError::INVALID_PAYLOAD,
                                                       "Message received with empty payload"),
                  "", "");
         return;
      }

      const auto topic = receivedMessage.substr(0, delimiterPos);
      const auto message = receivedMessage.substr(delimiterPos + 1, std::string::npos);

      nlohmann::json parsedJson;
      try
      {
         parsedJson = nlohmann::json::parse(message);
      }
      catch (const nlohmann::json::parse_error& ex)
      {
         callback(make_optional_error<SubscriberError>(SubscriberError::INVALID_PAYLOAD,
                                                       "Failed to parse json"),
                  "", "");

         return;
      }

      if (parsedJson[JSON_COMPONENT_VERSION_FIELD].get<std::string>() != version())
      {
         callback(make_optional_error<SubscriberError>(
                      SubscriberError::INVALID_PAYLOAD,
                      "Sender and receiver were on different software version"),
                  "", "");

         return;
      }

      callback(make_optional_error<SubscriberError>(), topic, parsedJson);
   });

   subscriber.addSession("127.0.0.1", port);

   return make_optional_error<SubscriberError>();
}

std::optional<Error<Component::SubscriberError>>
Component::subscribe(const std::string& componentName, const SubscriberCallback& callback)
{
   auto port = publishPort(componentName);
   if (!port.has_value())
   {
      return make_optional_error<SubscriberError>(SubscriberError::NETWORK_CONFIGURATION_MISSING,
                                                  "Network configuration is missing for the " +
                                                      componentName + " component");
   }

   return subscribe(port.value(), callback);
}

std::optional<Error<Component::PublishError>> Component::publish(const std::string& topic,
                                                                 const nlohmann::json& payload)
{
   if (payload.contains(JSON_COMPONENT_VERSION_FIELD))
   {
      return make_optional_error<PublishError>(PublishError::INVALID_PAYLOAD,
                                               "Json payload contains invalid field");
   }

   nlohmann::json modifiedJson = payload;
   modifiedJson[JSON_COMPONENT_VERSION_FIELD] = version();

   std::string serializedJson;
   try
   {
      serializedJson = modifiedJson.dump();
   }
   catch (const nlohmann::json::type_error& ex)
   {
      return make_optional_error<PublishError>(PublishError::INVALID_PAYLOAD,
                                               "Failed to serialize json");
   }

   return publish(topic, serializedJson);
}
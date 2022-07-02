#ifndef ERROR_HPP
#define ERROR_HPP

#include <optional>
#include <string>

#include "magic_enum.hpp"

template <class T> class Error
{
public:
   Error() = delete;

   inline explicit Error(const T& error) : mError{error}
   {
      static_assert(std::is_enum<T>::value,
                    "Error instance should be constructed from an enum type");
   }

   inline Error(const T& error, const std::string& furtherInfo)
       : mError{error}, mFurtherInfo{furtherInfo}
   {
      static_assert(std::is_enum<T>::value,
                    "Error instance should be constructed from an enum type");
   }

   [[nodiscard]] inline auto error() const noexcept
   {
      return mError;
   }

   [[nodiscard]] inline auto furtherInfo() const noexcept
   {
      return mFurtherInfo;
   }

   [[nodiscard]] inline auto asString() const
   {

      return magic_enum::enum_name(mError);
   }

   [[nodiscard]] inline bool operator==(const T& value) const
   {
      return mError == value;
   }

private:
   T mError;
   std::optional<std::string> mFurtherInfo;
};

template <class T, class... Types>
[[nodiscard]] static inline auto make_optional_error(Types... args)
{
   return std::make_optional<Error<T>>(args...);
}

template <class T>[[nodiscard]] static inline auto make_optional_error()
{
   return std::optional<Error<T>>();
}

#endif // ERROR_HPP

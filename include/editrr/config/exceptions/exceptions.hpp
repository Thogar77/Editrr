//
// Created by jakub on 6/13/26.
//

#ifndef EDITRR_EXCEPTIONS_HPP
#define EDITRR_EXCEPTIONS_HPP
#include <exception>
#include <string>
namespace exceptions {
class Exception : public std::exception {
  protected:
  std::string msg_;
public:
  explicit Exception(std::string msg) : msg_{std::move(msg)} {};
  const char* what() const noexcept override {
    return msg_.c_str();
  }
};

// Config Loading Exceptions
class MissingSection : public Exception {public: using Exception::Exception; };
class UnknownCommand : public Exception {public: using Exception::Exception; };
class DuplicateKey : public Exception {public: using Exception::Exception; };
class InvalidKey : public Exception {public: using Exception::Exception; };

};
#endif  // EDITRR_EXCEPTIONS_HPP

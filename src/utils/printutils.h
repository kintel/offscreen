#pragma once

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

enum class message_group {
  NONE,
  Error,
  Warning,
  Echo,
  UI_Error,
  UI_Warning,
};

namespace offscreen_log {

class Formatter {
  std::string fmt_;
  size_t pos_ = 0;
  std::ostringstream out_;

public:
  Formatter(const std::string& fmt) : fmt_(fmt) {}

  template <typename T>
  Formatter& operator%(const T& val) {
    size_t pct = fmt_.find('%', pos_);
    if (pct != std::string::npos) {
      out_ << fmt_.substr(pos_, pct - pos_);
      out_ << val;
      pos_ = pct + 1;
      while (pos_ < fmt_.size() &&
             (isdigit(static_cast<unsigned char>(fmt_[pos_])) || fmt_[pos_] == '$' || fmt_[pos_] == '.')) {
        pos_++;
      }
      if (pos_ < fmt_.size()) {
        pos_++;  // Skip format type letter (s, d, x, etc.)
      }
    } else {
      out_ << val;
    }
    return *this;
  }

  std::string str() const {
    std::ostringstream ss;
    ss << out_.str();
    if (pos_ < fmt_.size()) {
      ss << fmt_.substr(pos_);
    }
    return ss.str();
  }
};

inline std::string formatPositional(const std::string& fmt, const std::vector<std::string>& args) {
  std::ostringstream out;
  size_t i = 0;
  size_t arg_idx = 0;
  while (i < fmt.size()) {
    if (fmt[i] == '%' && i + 1 < fmt.size()) {
      if (fmt[i + 1] == '%') {
        out << '%';
        i += 2;
        continue;
      }
      size_t j = i + 1;
      int explicit_idx = -1;
      if (isdigit(static_cast<unsigned char>(fmt[j]))) {
        size_t k = j;
        while (k < fmt.size() && isdigit(static_cast<unsigned char>(fmt[k]))) k++;
        if (k < fmt.size() && fmt[k] == '$') {
          explicit_idx = std::stoi(fmt.substr(j, k - j)) - 1;
          j = k + 1;
        }
      }
      while (j < fmt.size() &&
             (isdigit(static_cast<unsigned char>(fmt[j])) || fmt[j] == '.' || fmt[j] == '-')) {
        j++;
      }
      if (j < fmt.size()) j++;  // skip type specifier
      size_t target = (explicit_idx >= 0) ? static_cast<size_t>(explicit_idx) : arg_idx++;
      if (target < args.size()) {
        out << args[target];
      }
      i = j;
    } else {
      out << fmt[i++];
    }
  }
  return out.str();
}

template <typename T>
std::string toString(const T& val) {
  std::ostringstream ss;
  ss << val;
  return ss.str();
}

inline std::string toString(const std::string& val) {
  return val;
}

inline std::string toString(const char* val) {
  return val ? std::string(val) : std::string("(null)");
}

template <typename... Args>
void logImpl(message_group group, const std::string& fmt, Args&&... args) {
  std::vector<std::string> str_args = { toString(args)... };
  std::string msg = formatPositional(fmt, str_args);
  if (group == message_group::Error) {
    std::cerr << msg << std::endl;
  } else {
    std::cout << msg << std::endl;
  }
}

inline void logImpl(message_group group, const std::string& msg) {
  if (group == message_group::Error) {
    std::cerr << msg << std::endl;
  } else {
    std::cout << msg << std::endl;
  }
}

}  // namespace offscreen_log

#define PRINTD(_arg)                                                           \
  do {                                                                         \
    std::cerr << (_arg) << std::endl;                                          \
  } while (0)

#define PRINTDB(_fmt, _arg)                                                    \
  do {                                                                         \
    std::cerr << (offscreen_log::Formatter(_fmt) % _arg).str() << std::endl;   \
  } while (0)


inline void LOG(message_group group, const std::string& msg) {
  offscreen_log::logImpl(group, msg);
}

template <typename... Args>
void LOG(message_group group, const std::string& fmt, Args&&... args) {
  offscreen_log::logImpl(group, fmt, std::forward<Args>(args)...);
}

inline void LOG(const std::string& msg) {
  offscreen_log::logImpl(message_group::NONE, msg);
}

template <typename... Args>
void LOG(const std::string& fmt, Args&&... args) {
  offscreen_log::logImpl(message_group::NONE, fmt, std::forward<Args>(args)...);
}

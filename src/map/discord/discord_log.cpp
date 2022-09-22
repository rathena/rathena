#include "discord_log.hpp"
#ifdef TESTING
#include "showmsg_testing.hpp"
#else
#include "../../common/showmsg.hpp"
#endif

#include <iostream>

namespace rocord {
log::log()
{
}

log::~log()
{
}

void
log::set_level(int level)
{
  this->level = level;
}

void
log::print(std::string message, log_type ltype, bool need_sync)
{
  // HARDCODED logging level
  // int level = 0xFF;

  if (this->level & ltype) {
    std::shared_ptr<log_entry> entry(new log_entry(message, ltype));
    if (!need_sync) {
      do_print(*entry);
    } else {
      std::lock_guard<std::mutex> lock(m);
      print_queue.push(entry);
    }
  }
}

void
log::handle_print()
{
  if (!m.try_lock())
    return;

  if (print_queue.empty()) {
    m.unlock();
    return;
  }
  while (!print_queue.empty()) {
    auto entry = print_queue.front();
    print_queue.pop();
    do_print(*entry);
  }
  m.unlock();
}

void
log::welcome()
{
  ShowStatus("%s", "Loading roCORD by norm\n");
}

void
log::do_print(log_entry& entry)
{
  const char* format = "%s\n";
  switch (entry.get_type()) {
    case log_type::warning:
      ShowWarning(format, entry.get_message().c_str());
      break;
    case log_type::info:
      ShowInfo(format, entry.get_message().c_str());
      break;
    case log_type::status:
      ShowStatus(format, entry.get_message().c_str());
      break;
    case log_type::error:
      ShowError(format, entry.get_message().c_str());
      break;
    case log_type::debug:
      ShowDebug(format, entry.get_message().c_str());
      break;
    default:
      ShowError(format, "[roCORD]: error in handle print");
  }
}
} // namespace rocord

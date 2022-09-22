//
//  discord_log_entry.hpp
//  roCORD
//
//  Created by Norman Ziebal on 21.08.18.
//  Copyright © 2018 Norman Ziebal. All rights reserved.
//

#ifndef discord_log_entry_hpp
#define discord_log_entry_hpp

#include <string>

namespace rocord {

enum log_type
{
  debug = 0x01,
  info = 0x02,
  status = 0x04,
  warning = 0x08,
  error = 0x10
};

class log_entry
{
public:
  log_entry(std::string message, log_type ltype);
  virtual ~log_entry();

  std::string get_message();
  log_type get_type();

private:
  // time
  std::string message;
  log_type ltype;
};
}
#endif /* discord_log_entry_hpp*/

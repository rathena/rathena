//
//  discord_error.hpp
//  roCORD
//
//  Created by Norman Ziebal on 28.08.18.
//  Copyright © 2018 Norman Ziebal. All rights reserved.
//

#ifndef discord_error_hpp
#define discord_error_hpp

#include <stdio.h>
#include <string>

struct Error
{
  int id;
  std::string message;
};

#endif /* discord_error_hpp */

//
//  showmsg_testing.hpp
//  roCORD
//
//  Created by Norman Ziebal on 22.08.18.
//  Copyright © 2018 Norman Ziebal. All rights reserved.
//

#ifndef showmsg_testing_hpp
#define showmsg_testing_hpp

#include <stdio.h>

void ShowMessage(const char* format, ...);
void ShowStatus(const char* format, ...);
void ShowInfo(const char* format, ...);
void ShowNotice(const char* format, ...);
void ShowWarning(const char* format, ...);
void ShowDebug(const char* format, ...);
void ShowError(const char* format, ...);

#endif /* showmsg_testing_hpp */

//
//  main.cpp
//  roCORD
//
//  Created by Norman Ziebal on 15.08.18.
//  Copyright © 2018 Norman Ziebal. All rights reserved.
//

#include "discord_bot.hpp"
#include "gtest/gtest.h"
#include <future>
#include <iostream>
#include <stdio.h>
#include <string>

int
main(int argc, char** argv)
{

  if (argc > 1) {
    if (strcmp(argv[1], "-test") == 0) {
      ::testing::InitGoogleTest(&argc, argv);
      return RUN_ALL_TESTS();
    } else if (strcmp(argv[1], "-stress") == 0) {
      int err = discord_init();
      if (err == -1)
        return 0;
      int i = 0;
      std::string name = "test_user";
      std::string channel = "general";
      std::string msg = "Test message!";

      while (true) {
        ++i;
        discord_handle();
        if (argc < 3) {
          std::cout << "This simulates rAthena SRC! Round: " << i << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (i == 3 || i % 1200 == 0) {
          discord_send(msg.c_str(), channel.c_str(), name.c_str());
        }
      }
      return 0;
    } else {
      std::cout << "No argument runs the bot in test mode for 10 iterations.\n "
                   "With -test all unit test will be executed!"
                << std::endl;
      return -1;
    }
  } else {

    // Temporary test
    discord_send(nullptr, nullptr, nullptr);

    int err = discord_init();
    if (err == -1)
      return 0;

    int i = 0;
    std::string name = "test_user";
    std::string channel = "general";
    std::string msg = "Test message!";
    // Temporary nullptr test until the actual test is coded!
    discord_send(NULL, NULL, NULL);
    discord_send(nullptr, nullptr, nullptr);

    while (i < 10) {
      discord_handle();
      std::cout << "This simulates rAthena SRC! Round: " << ++i << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      if (true)
        discord_send(msg.c_str(), channel.c_str(), name.c_str());
    }
    return 0;
  }
}

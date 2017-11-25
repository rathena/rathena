#include "test_thread.hpp"
#include <chrono>
#include <thread>

#include "../common/thread.h"
#include "../common/showmsg.h"


namespace ra
{
  namespace unit_tests
  {
    void* thread_test(void *x)
    {
       ShowStatus( "Entering thread_test\n" );
       int lTHID = rathread_get_tid();
       ShowStatus( "test th id=%d\n",lTHID); 
       std::this_thread::sleep_for(std::chrono::milliseconds(500));
       ShowStatus( "Leaving thread_test\n" );
       return nullptr;
    }

    void test_thread_creation_and_wait()
    {
      ShowStatus( "Testing thread_creation_and_wait\n" );
      prAthread lnd2th = rathread_create(&thread_test,nullptr); 
      //rathread_yield();
      std::this_thread::sleep_for(std::chrono::milliseconds(100)); //fake doing stuff
      int lMainID = rathread_get_tid();
      ShowStatus( "Continuing main th id=%d\n",lMainID); 
      while ( !rathread_wait( lnd2th, nullptr ) )
      {
          ShowStatus( "Waiting 2nd thread\n" );
          std::this_thread::sleep_for(std::chrono::seconds(1));
      }
      rathread_destroy(lnd2th); //will be destroy anyway by manager if not
    }

  }
}

/* Expected result
[Status]: Testing thread_creation_and_wait
[Status]: Entering thread_test
[Status]: test th id=1
[Status]: Continuing main th id=0
[Status]: Leaving thread_test
*/
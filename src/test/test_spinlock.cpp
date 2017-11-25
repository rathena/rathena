#include "test_spinlock.hpp"
#include <chrono>
#include <thread>

#include "../common/thread.h"
#include "../common/spinlock.h"
#include "../common/showmsg.h"
#include "test_thread.hpp"

namespace ra 
{
  namespace unit_tests
  {
    static SPIN_LOCK lSpinLock;

    void* thread_test_critical( void *x )
    {
      size_t count = 0;
      while ( count++ < 10 )
      { 
        EnterSpinLock( &lSpinLock );
        ShowStatus( "TH Entering critical_section recursion_count=%d iteration_count=%d\n", lSpinLock.nest,count );
        std::this_thread::sleep_for(std::chrono::milliseconds( 100 )); //a simple test with critical section
        ShowStatus( "TH Leaving critical_section recursion_count=%d\n iteration_count=%d", lSpinLock.nest,count );
        LeaveSpinLock( &lSpinLock );
        rathread_yield();
        std::this_thread::sleep_for(std::chrono::milliseconds( 200 )); //let try to lose the lock here
      }
      return nullptr;
    }

    void test_thread_spinlock()
    {
      ShowStatus( "Testing test_thread_spinlock\n" );
      InitializeSpinLock( &lSpinLock );

      EnterSpinLock( &lSpinLock ); //fake critical section (we take it before creation to ensure we have the lock)
      ShowStatus( "Main Entering critical_section recursion_count=%d\n", lSpinLock.nest );

      prAthread lnd2th = rathread_create( &thread_test_critical, nullptr );
      int lMainID = rathread_get_tid();
      ShowStatus( "Continuing main th id=%d\n", lMainID );
      rathread_yield();
      std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) ); //fake doing stuff
      ShowStatus( "Main Leaving critical_section recursion_count=%d\n", lSpinLock.nest );
      LeaveSpinLock( &lSpinLock );

      std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) ); //fake doing stuff
      EnterSpinLock( &lSpinLock ); //now we should be waiting
      ShowStatus( "Main Entering critical_section recursion_count=%d\n", lSpinLock.nest );
      
      EnterSpinLock( &lSpinLock ); //testing with a recursion
      ShowStatus( "Main Entering critical_section recursion_count=%d\n", lSpinLock.nest );
      std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) ); //fake doing stuff
      ShowStatus( "Main Leaving critical_section recursion_count=%d\n", lSpinLock.nest );
      LeaveSpinLock( &lSpinLock );

      ShowStatus( "Main Leaving critical_section recursion_count=%d\n", lSpinLock.nest );
      LeaveSpinLock( &lSpinLock );

      while ( !rathread_wait( lnd2th, nullptr ) )
      {
        ShowStatus( "Waiting 2nd thread\n" );
        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
      }
      rathread_destroy( lnd2th );
      FinalizeSpinLock( &lSpinLock );
    }

  }
}

/* Expected result
[Status]: Testing test_thread_spinlock
[Status]: Main Entering critical_section recursion_count=1
[Status]: Continuing main th id=0
[Status]: Main Leaving critical_section recursion_count=1
[Status]: TH Entering critical_section recursion_count=1 iteration_count=1
[Status]: TH Leaving critical_section recursion_count=1 iteration_count=1
[Status]: Main Entering critical_section recursion_count=1
[Status]: Main Entering critical_section recursion_count=2
[Status]: Main Leaving critical_section recursion_count=2
[Status]: Main Leaving critical_section recursion_count=1
[Status]: TH Entering critical_section recursion_count=1 iteration_count=2
[Status]: TH Leaving critical_section recursion_count=1 iteration_count=2
[Status]: TH Entering critical_section recursion_count=1 iteration_count=3
[Status]: TH Leaving critical_section recursion_count=1 iteration_count=3
[Status]: TH Entering critical_section recursion_count=1 iteration_count=4
[Status]: TH Leaving critical_section recursion_count=1 iteration_count=4
[Status]: TH Entering critical_section recursion_count=1 iteration_count=5
[Status]: TH Leaving critical_section recursion_count=1 iteration_count=5
[Status]: TH Entering critical_section recursion_count=1 iteration_count=6
[Status]: TH Leaving critical_section recursion_count=1 iteration_count=6
[Status]: TH Entering critical_section recursion_count=1 iteration_count=7
[Status]: TH Leaving critical_section recursion_count=1 iteration_count=7
[Status]: TH Entering critical_section recursion_count=1 iteration_count=8
[Status]: TH Leaving critical_section recursion_count=1 iteration_count=8
[Status]: TH Entering critical_section recursion_count=1 iteration_count=9
[Status]: TH Leaving critical_section recursion_count=1 iteration_count=9
[Status]: TH Entering critical_section recursion_count=1 iteration_count=10
[Status]: TH Leaving critical_section recursion_count=1 iteration_count=10
*/
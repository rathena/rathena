#pragma once

namespace ra 
{
  namespace unit_tests
  {
    //main
    void test_thread_spinlock();
    //subs
    void* thread_test_critical( void *x ); //must match rAthreadProc signature
  }
}

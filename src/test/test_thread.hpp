#pragma once

namespace ra 
{
  namespace unit_tests
  {
    //main
    void test_thread_creation_and_wait();
    //sub
    void* thread_test( void *x ); //must match rAthreadProc signature
  }
}

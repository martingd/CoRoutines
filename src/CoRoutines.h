/*
  Simple co-routine library for Arduino.
  
  Version 1.0 - 2011-01-01

  Copyright 2011 Martin Gamwell Dawids.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

  This library provides a simple way to schedule individual tasks.

  CoRoutine
  ---------
  Each task is implemented as a subclass of class CoRoutine.

  Override method CoRoutine::worker() to define what the task should do.
  The return value from CoRoutine::worker() tells the CoRoutine class
  in how many milli seconds the worker would like to be invoked again.
  Return 0 to be invoked again as soon as possible.
  Returning -1 means that routine should be considered suspended and the
  worker will not be invoked again until awakened.
  
  To invoke a task call the CoRoutine::resume(). This method will determine
  whether it is time to invoke the worker. If it is CoRoutine::worker()
  is called. Otherwise, resume() returns immediately which is also the case
  if the task is suspended.
  
  A suspended task can be awakened by calling CoRoutine::awake().
  Subsequent calls to CoRoutine::resume() will invoke the worker again
  (until it returns -1 to signal it is suspended again.)

  If you need state in your co-routine tasks, place that in your subclass.

  This co-routine implementation does not make use of ugly tricks
  (like macros with unmatched braces, switch statements and case labels inside
  other control structures) like you will see in other implementation as
  this often leads to very strange error messages if used incorrectly.
  
  Scheduler
  ---------
  If you do not want to handle scheduling of multiple co-routines yourself,
  you can use class Scheduler.
  
  Instantiate this class and call Scheduler::addCoRoutine() to add your
  co-routines one by one. Now, call Scheduler::runOnce() to resume each of
  the added co-routines one by one. This could be placed in the loop() of
  your Arduino sketch like this:
  
    Scheduler scheduler;
    scheduler.addCoRoutine(coRoutine1);
    scheduler.addCoRoutine(coRoutine2);
    ...
    scheduler.addCoRoutine(coRoutineN);
    
    void loop()
    {
      scheduler.runOnce();
    }
  
  Co-routines can be removed from the scheduler either manually by calling
  Scheduler::removeCoRoutine() or automatically by calling passing 'true'
  when calling Scheduler::runOnce(). The latter imposes a small overhead in the
  scheduler even if no tasks are suspended, as the scheduler needs to keep track
  of which tasks are suspended.
 */

#ifndef __coroutines_h__
#define __coroutines_h__

#include <stdlib.h>

namespace coroutines {

  // A simple co-routine.
  class CoRoutine
  {
  private:
    bool suspended;
    const bool waitRelativeToWorkerExit;
    unsigned long nextRun;
    
  protected:
    // Override to implement what the co-routine should do.
    // Should return he number of milliseconds before next invocation.
    // 0 means as soon as possible.
    // -1 indicates that the co-routine should be suspended and no longer run.
    virtual int worker() = 0;
    
  public:
    // Create a co-routine.
    //
    // Parameters:
    //   'waitRelativeToWorkerExit'
    //       If 'false' (default), next run time is calculated relative to
    //     the time when the worker expected to be invoked (which might
    //     be earlier than the time it was actually invoked.) This means that
    //     if the invocation of the worker was delayed by some other task, this
    //     delay will not be added to the expected run time of the next
    //     invocation of the worker.
    //       If 'true' the next run time is calculated relative to the time when
    //     when the worker exited.
    CoRoutine(bool waitRelativeToWorkerExit = false);
    
    // Call this whenever the routine can have a time slot.
    // If it is time for the co-routine to run, 'worker()' will be called.
    // Otherwise, nothing happens.
    void resume();
    
    // Returns 'true' iff the co-routine is suspended.
    bool isSuspended();
    
    // Call this to awake the suspended co-routine.
    void awake();

    // Call this to suspend the co-routine.
    void suspend();
  };


  // A scheduler for co-routines.
  class Scheduler
  {
  private:
    CoRoutine** coRoutines; // An array of pointers to co-routines.
    size_t arraySize;
    size_t noEntries;
    
    void resize(size_t newSize);
    
  public:
    Scheduler();
    virtual ~Scheduler();
    
    // Add a co-routine to this scheduler.
    // Note: Do not add tha same co-routine twice or it will be invoked two
    //       times per run.
    void addCoRoutine(CoRoutine& coRoutine);

    // If the co-routine is not member of this scheduler, nothing happens.
    // If a co-routine was added multiple times, it will only be removed once.
    void removeCoRoutine(CoRoutine& coRoutine);

    // Call 'resume' on all co-routines of this scheduler once.
    // Set 'removeSuspendedCoRoutines' to 'true' to automatically remove
    // suspended co-routines from this scheduler.
    // Using this feature imposes a slight overhead in the scheduler.
    void runOnce(bool removeSuspendedCoRoutines = false);
  };

} // end of namespace coroutines

#endif // __coroutines_hpp__

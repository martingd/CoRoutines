/*
  Simple co-routine library for Arduino.
  
  Version 1.0.1 - 2017-04-10

  Copyright 2011-2017 Martin Gamwell Dawids.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

  For an overview of the functionality in this library,
  please see the file "CoRoutines.h".
*/

/*
 Uncomment this define to include debug messages to the serial port.
 You must remember to call
   Serial.begin(9600);
 in your 'void setup()' function.
 */
// #define CoRoutinesDebug


#include <CoRoutines.h>

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

namespace coroutines {

  CoRoutine::CoRoutine(bool waitRelativeToWorkerExit)
    : suspended(false),
      waitRelativeToWorkerExit(waitRelativeToWorkerExit),
      nextRun(0) // This is the first run.
  { }
  
  void CoRoutine::resume()
  {
    // Is it time to run?
    const unsigned long startOfRun = millis();
    if (!suspended && startOfRun >= nextRun)
    {
      // Run now.
      const int waitTime = worker();
      
      if (waitTime == -1)
      {
        // Worker signalled we are suspended.
        suspended = true;
      }
      else
      {
        // Schedule next run.
        if (waitRelativeToWorkerExit)
        {
          // Set next run relative to now (when worker is completed).
          nextRun = millis() + waitTime;
        }
        else
        {
          if (nextRun != 0)
          {
            // Set next run relative to this run.
            nextRun += waitTime;
          }
          else
          {
            // This is the first run.
            nextRun = startOfRun + waitTime;
          }
        }
      }
    }
  }
  
  bool CoRoutine::isSuspended()
  {
    return suspended;
  }
  
  void CoRoutine::awake()
  {
    if (suspended)
    {
      nextRun = 0;
      suspended = false;
    }
  }

  void CoRoutine::suspend()
  {
    suspended = true;
  }

  Scheduler::Scheduler()
    : coRoutines(0),
      arraySize(0),
      noEntries(0)
  { }

  Scheduler::~Scheduler()
  {
    // De-allocate the array of co-routines.
    // (The pointers in the array are not owned by this class.)
    free(coRoutines);
  }

  void Scheduler::resize(size_t newSize)
  {
    // Is array large enough?
    // If not enlarge it. (We never shrink it.)
    if (newSize > arraySize)
    {
      // Double the array (starting out with one place.)
      const size_t newSize = (arraySize == 0 ? 1 : 2 * arraySize);
      
      // Allocate new array of pointers.
      CoRoutine** const newArray = (CoRoutine**) malloc(newSize * sizeof(CoRoutine*));
      
      // Copy over contents.
      memcpy(newArray, coRoutines, arraySize * sizeof(CoRoutine*));

      // De-allocate the old array.
      free(coRoutines);
      
      // Use new array from now.
      coRoutines = newArray;
      arraySize = newSize;
    }
  }

  void Scheduler::addCoRoutine(CoRoutine& coRoutine)
  {
    #ifdef CoRoutinesDebug
      Serial.print("Adding co-routine to scheduler: ");
      Serial.println((unsigned long) &coRoutine);
    #endif

    // Increment entry counter and make sure there is room in the array.
    ++noEntries;
    resize(noEntries);
    
    // Insert the new coRoutine in the back.
    coRoutines[noEntries-1] = &coRoutine;
  }
  
  void Scheduler::removeCoRoutine(CoRoutine& coRoutine)
  {
    if (noEntries == 0)
    {
      // Nothing to remove.
      return;
    }
    
    // Find the index of the co-routine we would like to remove.
    size_t foundIndex = noEntries;
    for (size_t i = noEntries-1; i != 0; --i)
    {
      if (coRoutines[i] == &coRoutine)
      {
        foundIndex = i;
        break;
      }
    }
    
    // If found remove it, otherwise do nothing.
    if (foundIndex != noEntries)
    {
      // Number of entries after this one.
      const size_t entriesToMove = noEntries - (foundIndex + 1);
      
      // Remove this entry by moving the entries after one to the left.
      memcpy(&coRoutines[foundIndex], &coRoutines[foundIndex+1], entriesToMove);
      --noEntries;
    }
  }
   
  void Scheduler::runOnce(bool removeCompletedCoRoutines)
  {
    // Run each co-routine.
    for (size_t i = 0; i != noEntries; ++i)
    {
      CoRoutine* const coRoutine = coRoutines[i];
      coRoutine->resume();
    }
    
    if (noEntries != 0 && removeCompletedCoRoutines)
    {
      // Remove suspended co-routines.
      // This could be implemented more effeciently, but I guess it is not
      // really a problem unless you have many co-routines.
      // We iterate from the back of the array to make sure deletions
      // during the iteration do not make us skip an entry.
      for (size_t i = noEntries-1; i != 0; --i)
      {
        if (coRoutines[i]->isSuspended())
        {
          removeCoRoutine(*coRoutines[i]);
        }
      }
    }
  }

} // end of namespace coroutines
  

#if !defined(ARDUINO) || ARDUINO < 100
// We need this code to allow pure virtual functions
// as the Arduino C++ compiler does not include it.
// It should *never* be called.
extern "C" void __cxa_pure_virtual(void); 
void __cxa_pure_virtual(void) 
{
  abort();
}

// Also 'delete' is referenced from virtual destructors.
void operator delete(void * ptr) 
{ 
  free(ptr); 
} 
#endif

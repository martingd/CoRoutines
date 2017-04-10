# CoRoutines
A simple co-routine library for Arduino.

When you are going to work on an Arduino project where you are communicating with a device that support some sort of asynchronous operations. This small library can come in handy.

It could be you are working with a device that supports scheduling some sort of measurement which will be available after some time and you will have to poll the device to determine when it is ready.

Or it could be you are going to send a command, wait for a while and then send another command.

In both cases it is a bad idea to busy wait because it prevents your Arduino from doing other work while waiting. This library can help you utilising the waiting time for something else.

# API
The API consists of two classes contained in a namespace names `coroutines`.

## `class CoRoutine`
The `CoRoutine` class is used for defining tasks.
Each task is implemented as a subclass of class CoRoutine.

Override method `CoRoutine::worker()` to define what the task should do.
The return value from `CoRoutine::worker()` tells the `CoRoutine` class
in how many milli seconds the worker would like to be invoked again.
Return 0 to be invoked again as soon as possible.
Returning -1 means that routine should be considered suspended and the
worker will not be invoked again until awakened.

To invoke a task call `CoRoutine::resume()`. This method will determine
whether it is time to invoke the worker. If it is `CoRoutine::worker()`
is called. Otherwise, `CoRoutine::resume()` returns immediately which is also the case
if the task is suspended.

A suspended task can be awakened by calling `CoRoutine::awake()`.
Subsequent calls to `CoRoutine::resume()` will invoke the worker again
(until it returns -1 to signal it is suspended again.)

If you need state in your co-routine tasks, place that in your subclass.

This co-routine implementation does not make use of ugly tricks
(like macros with unmatched braces, switch statements and case labels inside
other control structures) like you will see in other implementation as
this often leads to very strange error messages if used incorrectly.

## `class Scheduler`
If you do not want to handle scheduling of multiple co-routines yourself,
you can use class `Scheduler`.

Instantiate this class and call `Scheduler::addCoRoutine()` to add your
co-routines one by one. Now, call `Scheduler::runOnce()` to resume each of
the added co-routines one by one. This could be placed in the `loop()` of
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
`Scheduler::removeCoRoutine()` or automatically by calling passing `true`
when calling `Scheduler::runOnce()`. The latter imposes a small overhead in the
scheduler even if no tasks are suspended, as the scheduler needs to keep track
of which tasks are suspended.

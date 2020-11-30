#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    // to wait for and receive new messages and pull them from the queue using move semantics.
    // The received object should then be returned by the receive function.

    // perform vector modification under the lock
    std::unique_lock<std::mutex> uniqueLock(_mutex);
    _cond.wait(uniqueLock, [this] { return !_queue.empty(); });

    // remove last vector element from queue
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::lock_guard<std::mutex> lockGuard(_mutex);
    _queue.emplace_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.

    while(true)
    {
      // wait 1ms to save CPU processing power
      std::this_thread::sleep_for(std::chrono::milliseconds(1));

      if (_trafficLights.receive() == TrafficLightPhase::green)
      {
        return;
      }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.

    // add thread to thread _queue
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds.
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    // create random cycle duration between 4000 and 6000 ms
    const int cycleDuration = rand() % 2000 + 4000;

    // set time of last update
    auto timeLastUpdate = std::chrono::system_clock::now();

    while(true)
    {
      // wait 1ms between two cycles
      std::this_thread::sleep_for(std::chrono::milliseconds(1));

      // measure passed time between cycleThroughPhases
      long timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timeLastUpdate).count();

      // if time of random cycle duration passed -> switch traffic lights
      if (timePassed >= cycleDuration)
      {
        switch (_currentPhase)
        {
          case green  : _currentPhase = red; break;
          case red    : _currentPhase = green; break;
        }

        // send current thaffic light state
        _trafficLights.send(std::move(_currentPhase));

        // set time of last update
        timeLastUpdate = std::chrono::system_clock::now();
      }
    }

}

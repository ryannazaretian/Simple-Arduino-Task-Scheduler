/* 
 *  Simple Task Scheduler - Create repetitive tasks to call in your Arduino project.
 *  Created by: Ryan Nazaretian - ryannazaretian@gmail.com
 *  Copyright (C) 2018 - Ryan Nazaretian
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  
 */

#include <stdint.h>
#include <Arduino.h>


#ifndef SIMPLE_TASK_SCHEDULER
#define SIMPLE_TASK_SCHEDULER

#define TS_MILLIS 0
#define TS_MICROS 1

//provide a typedef for a void function pointer
typedef void (*FuncPtr)(void);

// This is a task object, which includes everythin the SimpleTaskScheduler needs to know about a specific task.
typedef struct {
  FuncPtr fptr_callback; // Function pointer to the callback
  bool b_enabled; // Whether or not the task is enabled
  unsigned long ul_period; // The period, in milliseconds 
  unsigned long ul_timer_start;
} TaskObject;

class SimpleTaskScheduler {
  public:
    SimpleTaskScheduler(uint8_t, uint8_t);
    uint8_t addTask(FuncPtr, uint16_t, bool);
    void loop(void);
    void disableTask(uint8_t);
    void enableTask(uint8_t, bool);
    void callTask(uint8_t);
    void changeTaskPeriod(uint8_t, unsigned long);

  private:
      TaskObject *st_tasks;
      uint8_t u8_numOfTasks;
      uint8_t u8_timerType;
      
      void resetTimer(uint8_t);
      void checkTask(uint8_t);
      unsigned long getCurrentTime(void);
};

SimpleTaskScheduler::SimpleTaskScheduler(uint8_t u8_maxNumOfTasks, uint8_t u8_timerType) {
  /* Class: SimpleTaskScheduler
   *  Description: The SimpleTaskScheduler class is designed to allow the user to setup multiple
   *  tasks on a time schedule based on millis() or micros().
   * 
   * This is a cooperative task scheduler. Each callback is blocking, so keep your tasks
   * as short as possible.
   * 
   *  This is also a pretty poorly written task scheduler. This could be made better using
   *  timers. Don't use this for any real-time tasks. This should be used as a loosly-timed
   *  task scheduler, i.e. tasks that don't need strict time constraints. 
   * 
   *  If you have tasks that will likely block, add them last. 
   * 
   *  Place the SimpleTaskScheduler::loop() inside your Arduino void loop(void) {} function. 
   *  Add tasks using SimpleTaskScheduler::addTask(your callback function, period, enabled)
   * 
   *  Arguments:
   *    uint8_t u8_maxNumOfTasks - The maximum number of tasks to allocate at program start.
   *      The maximum maximum number of tasks supported is 256. (Seriously though, this is
   *      an Arduino, don't ask too much of it!)
   *    uint8_t u8_timerType - The type of timer to use for the schedule. Available options:
   *       - TS_MILLIS - Uses the millis() function, so you have millisecond resolution
   *       - TS_MICROS - Uses the micros() function, so you have microsecond resolution
   */
  this->u8_numOfTasks = 0;
  this->st_tasks = new TaskObject[u8_maxNumOfTasks];
  this->u8_timerType = u8_timerType;
}

uint8_t SimpleTaskScheduler::addTask(FuncPtr fptr_callback, uint16_t ul_period, bool b_enable) {
  /* Function: addTasks
   *  Description: Adds a task to the task schedler. 
   * 
   *  Arguments: 
   *    fptr_callback - Function pointer callback, your void<name>(void) function callback. 
   *      Remember this is a cooperative scheduler which requires your functions to finish
   *      as quickly as possible. Don't use sleeps or while-loops that wait on a condition. 
   *    ul_period - The period at which to call your callback function. This could be 
   *      milliseconds or microseconds depending on what you setup.
   *    b_enable - Whether or not this task is enabled at startup
   *   
   *  Returns:
   *    taskId - This is the tasks unique ID. If you want to go back and change any
   *      parameters of your task after adding it, save this ID. 
   *  
   */
  uint8_t u8_taskId = this->u8_numOfTasks++;
  this->st_tasks[u8_taskId].fptr_callback = fptr_callback;
  this->st_tasks[u8_taskId].ul_period = ul_period;
  this->st_tasks[u8_taskId].b_enabled = b_enable;
  this->st_tasks[u8_taskId].ul_timer_start = 0;
  return u8_taskId;
}

void SimpleTaskScheduler::loop(void) {
  /* Function: loop
   *  Description: Loop should be called as often as possible, preferrably in your Arduino loop function. 
   *  This is what checks what tasks are scheduled to run and runs them. 
   */
  uint8_t u8_taskId;
  for(u8_taskId = 0; u8_taskId < this->u8_numOfTasks; u8_taskId++) 
      this->checkTask(u8_taskId);
}

void SimpleTaskScheduler::disableTask(uint8_t u8_taskId) {
  /* Function disableTask
   *  Descriptions: Disables the given task from auto-running on a schedule. 
   *  
   *  Arguments:
   *    u8_taskId - The unique task ID of the task you want to disable.
   */
  this->st_tasks[u8_taskId].b_enabled = false;
}

void SimpleTaskScheduler::enableTask(uint8_t u8_taskId, bool b_trigger_now) {
   /* Function enableTask
   *  Descriptions: Enables the given task from auto-running on a schedule. 
   *  
   *  Arguments:
   *    u8_taskId - The unique task ID of the task you want to enable.
   *    b_trigger_now - Call the task right now and reset its timer.
   *      Otherwise, the task timer will reset to the current time + the period.
   */
  if (!this->st_tasks[u8_taskId].b_enabled) {
    this->st_tasks[u8_taskId].b_enabled = true;
    if (b_trigger_now) 
      this->callTask(u8_taskId);
    else
      this->resetTimer(u8_taskId);
  }
}

void SimpleTaskScheduler::callTask(uint8_t u8_taskId) {
  /* Function: callTask
   *  Description: Calls the task by the given unique task ID and resets its timer
   *  
   *  Arguments:
   *    u8_taskId - The unique task ID of the task you want to call.
   */
  this->resetTimer(u8_taskId);
  this->st_tasks[u8_taskId].fptr_callback();
}

void SimpleTaskScheduler::resetTimer(uint8_t u8_taskId) {
  /* Private Function: resetTimer
   *  Description: Resets the timer to begin again at its next period.
   *  
   *  Arguments:
   *    u8_taskId - The unique task ID for the task to reset the tiemr
   */
   if (this->st_tasks[u8_taskId].ul_period)
     this->st_tasks[u8_taskId].ul_timer_start = this->getCurrentTime();
}

void SimpleTaskScheduler::checkTask(uint8_t u8_taskId) {
  /* Private Function: checkTask 
   *  Description: Checks the given unique task ID to see if it's time to call it.
   *  
   *  Arguments:
   *    u8_taskId - The unique task ID of the task you want to check.
   */
  // If enabled and (Timer Disabled or Time Now - Time Start > Period
	if ( (this->st_tasks[u8_taskId].b_enabled) && 
	        ( (this->st_tasks[u8_taskId].ul_period == 0) || 
	            (this->getCurrentTime() - this->st_tasks[u8_taskId].ul_timer_start > this->st_tasks[u8_taskId].ul_period)
	         )
	    )
		this->callTask(u8_taskId);
}

unsigned long SimpleTaskScheduler::getCurrentTime(void) {
  /* Private Function: getCurrentTime
   *  Description: Returns the current time based on which time basis
   *  the task scheduler is using. 
   *  
   *  Returns:
   *    The current time used by the task scheduler. 
   */
  switch(this->u8_timerType) {
    case TS_MICROS:
      return micros();
      break;
    case TS_MILLIS:
      return millis();
      break;
    default:
      return millis();
  }
}

#endif

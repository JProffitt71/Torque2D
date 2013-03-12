//-----------------------------------------------------------------------------
// Unofficial Scene Event Manager
// Justin Proffitt
//-----------------------------------------------------------------------------

#ifndef _SCENEEVENTMANAGER_
#define _SCENEEVENTMANAGER_

#include "sim/simBase.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "2d/sceneobject/SceneObject.h"
#include "2d/scene/SceneEventManager.h"

///-----------------------------------------------------------------------------
/// Externs.
///-----------------------------------------------------------------------------

extern ExprEvalState gEvalState;

namespace SceneEventManager {

SceneGraphEventQueue *gGraphQueueRoot = new SceneGraphEventQueue(NULL);
bool gAbortEvents = false;

}

///-----------------------------------------------------------------------------
/// Console Methods
///-----------------------------------------------------------------------------

ConsoleMethod(Scene, cancel, void, 3, 3,"( eventID ) Use the cancel function to cancel a previously scheduled event as specified by eventID.\n"
              "@param eventID The numeric ID of a previously scheduled event.\n"
              "@return No return value.\n"
              "@sa getEventTimeLeft, getScheduleDuration, getTimeSinceStart, isEventPending, sceneSchedule.")
{
	return SceneEventManager::cancelEvent(object, dAtoi(argv[2]));
}

ConsoleMethod(Scene, isEventPending, bool, 3, 3, "( eventID ) Use the isEventPending function to see if the event associated with eventID is still pending.\n"
              "When an event passes, the eventID is removed from the event queue, becoming invalid, so there is no discnerable difference between a completed event and a bad event ID.\n"
              "@param eventID The numeric ID of a previously scheduled event.\n"
              "@return Returns true if this event is still outstanding and false if it has passed or eventID is invalid.\n"
              "@sa cancel, getEventTimeLeft, getScheduleDuration, getTimeSinceStart, sceneSchedule.")
{
	return SceneEventManager::isEventPending(object, dAtoi(argv[2]));
}

ConsoleMethod(Scene, getEventTimeLeft, F32, 3, 3, "( eventID ) Use the getEventTimeLeft function to determine how much time remains until the event specified by eventID occurs.\n"
              "@param eventID The numeric ID of a previously scheduled event.\n"
              "@return Returns a float value equal to the number of seconds until the event specified by eventID will occur. However, if eventID is invalid, or the event has passed, this function will return zero.\n"
              "@sa cancel, getScheduleDuration, getTimeSinceStart, isEventPending, sceneSchedule.")
{
	return SceneEventManager::getEventTimeLeft(object, dAtoi(argv[2]));
}

ConsoleMethod(Scene, getScheduleDuration, F32, 3, 3, "( eventID ) Use the getScheduleDuration function to determine how long the event associated with eventID was scheduled for.\n"
              "@param eventID The numeric ID of a previously scheduled event.\n"
              "@return Returns a float value equal to the seconds used in the schedule call that created this event. However, if eventID is invalid, this function will return zero.\n"
              "@sa cancel, getEventTimeLeft, getTimeSinceStart, isEventPending, sceneSchedule.")
{
	return SceneEventManager::getScheduleDuration(object, dAtoi(argv[2]));
}

ConsoleMethod(Scene, getTimeSinceStart, F32, 3, 3, "( eventID ) Use the getTimeSinceStart function to determine how much time has passed since the event specified by eventID was scheduled.\n"
              "@param eventID The numeric ID of a previously scheduled event.\n"
              "@return Returns a float value equal to the seconds that have passed since this event was scheduled. However, if eventID is invalid, or the event has passed, this function will return zero.\n"
              "@sa cancel, getEventTimeLeft, getScheduleDuration, isEventPending, sceneSchedule.")
{
	return SceneEventManager::getTimeSinceStart(object, dAtoi(argv[2]));
}

ConsoleMethod(Scene, sceneSchedule, S32, 4, 0, "(time , objID || 0, functionName, arg0, ... , argN ) Use the sceneSchedule method to schedule functionName to be executed time seconds in the future (relative to scene). SceneSchedule uses scene time instead of sim time.\n"
              "@param time Time in seconds till action is scheduled to occur.\n"
              "@param objID An optional ID to call this function as a method on.\n"
              "@param functionName Name of the function to execute.\n"
              "@param arg0, .. , argN Any number of optional arguments to be passed to functionName.\n"
              "@return Returns an integer schedule ID.\n"
              "@sa cancel, getEventTimeLeft, getScheduleDuration, isEventPending.")
{
   F32 timeDelta = F32(((timeDelta = dAtof(argv[2])) > 0)?timeDelta:0);
   SimObject *refObject = Sim::findObject(argv[3]);
   bool onObject = true;
   if(!refObject)
   {
      if(argv[3][0] != '0')
         return 0;

      refObject = Sim::getRootGroup();
	  onObject = false;
   }
   
   int offset = 4;

   // Swap it so that the object is the first parameter
   // And shift the resulting parameter start point left 1
   if(onObject){
	   argv[2] = argv[3];
	   argv[3] = argv[4];
	   argv[4] = argv[2];
	   offset--;
   }

   SceneEventManager::SceneEvent *evt = new SceneEventManager::SceneEvent(argc - offset, argv + offset, onObject);
   
   S32 ret = SceneEventManager::postEvent(object, refObject, evt, object->getSceneTime() + timeDelta);
// #ifdef DEBUG
//    Con::printf("obj %s schedule(%s) = %d", argv[3], argv[2], ret);
//    Con::executef(1, "backtrace");
// #endif
   return ret;
}

ConsoleMethod(SceneObject,sceneSchedule, S32, 4, 0, "(time , command , <arg1 ... argN> ) Use the sceneSchedule method to schedule an action to be executed upon this object time seconds in the future. SceneSchedule uses scene time instead of sim time.\n"
              "@param time Time in seconds till action is scheduled to occur.\n"
              "@param command Name of the command to execute. This command must be scoped to this object (i.e. It must exist in the namespace of the object), otherwise the schedule call will fail.\n"
              "@param arg1...argN These are optional arguments which will be passed to command. This version of schedule automatically passes the ID of %obj as arg0 to command.\n"
              "@return Returns an integer schedule ID.\n"
              "@sa cancel, getEventTimeLeft, getScheduleDuration, isEventPending.")
{
	if(!object->getScene())
		Con::warnf("SceneObject::sceneSchedule(...) - Object is not in a scene graph!");

	F32 timeDelta = F32(((timeDelta = dAtof(argv[2])) > 0)?timeDelta:0);
   argv[2] = argv[3];
   argv[3] = argv[1];

   SceneEventManager::SceneEvent *evt = new SceneEventManager::SceneEvent(argc - 2, argv + 2, true);
   S32 ret = SceneEventManager::postEvent(object->getScene(), object, evt, object->getScene()->getSceneTime() + timeDelta);
// #ifdef DEBUG
//    Con::printf("obj %s schedule(%s) = %d", argv[3], argv[2], ret);
//    Con::executef(1, "backtrace");
// #endif
   return ret;
}

///-----------------------------------------------------------------------------
/// C++ Side
///-----------------------------------------------------------------------------


U32 SceneEventManager::postEvent(SimObjectId sID, SimObjectId oID, SceneEvent *evt, F32 targetTime){
	return SceneEventManager::postEvent(Sim::findObject(sID), Sim::findObject(oID), evt, targetTime);
}

U32 SceneEventManager::postEvent(SimObjectId sID, const char *objectName, SceneEvent *evt, F32 targetTime){
	return SceneEventManager::postEvent(Sim::findObject(sID), Sim::findObject(objectName), evt, targetTime);
}
U32 SceneEventManager::postEvent(const char *graphName, SimObjectId oID, SceneEvent *evt, F32 targetTime){
	return SceneEventManager::postEvent(Sim::findObject(graphName), Sim::findObject(oID), evt, targetTime);
}
U32 SceneEventManager::postEvent(const char *graphName, const char *objectName, SceneEvent *evt, F32 targetTime){
	return SceneEventManager::postEvent(Sim::findObject(graphName), Sim::findObject(objectName), evt, targetTime);
}

U32 SceneEventManager::postEvent(SimObject *destScene, SimObject *destObject, SceneEvent *event, F32 time) {
	AssertFatal(destObject, "Destination object for event doesn't exist.");
	
	// Find the graph queue to update
	SceneGraphEventQueue *queueWalk = SceneEventManager::gGraphQueueRoot;
	SceneGraphEventQueue *queueCurrent = SceneEventManager::gGraphQueueRoot;
	
	while(queueWalk != NULL && (SimObject*)(queueCurrent->pScene) != destScene){
		queueCurrent = queueWalk;
		queueWalk = queueCurrent->nextQueue;
	}
	// Didn't find the graph queue
	if((SimObject*)(queueCurrent->pScene) != destScene){
		Con::errorf("Could not find queue for scenegraph.");		
		return InvalidEventId;
	}


	// Post the event to the queue
	AssertFatal(time == -1 || time >= queueCurrent->currentSceneTime,
		"Scene::postEvent: Cannot go back in time. (flux capacitor unavailable -- BJG)");

	if( time == -1 )
	  time = queueCurrent->currentSceneTime;

	event->time = time;
	event->startTime = queueCurrent->currentSceneTime;
	event->destObject = destObject;


	if(!destObject)
	{
	  delete event;
	  return InvalidEventId;
	}
	event->sequenceCount = queueCurrent->eventSequence++;
	SceneEvent **walk = &queueCurrent->eventQueue;
	SceneEvent *current;

	while((current = *walk) != NULL && (current->time < event->time))
	  walk = &(current->nextEvent);

	// [tom, 6/24/2005] This ensures that SimEvents are dispatched in the same order that they are posted.
	// This is needed to ensure Con::threadSafeExecute() executes script code in the correct order.
	while((current = *walk) != NULL && (current->time == event->time))
	  walk = &(current->nextEvent);

	event->nextEvent = current;
	*walk = event;

	U32 seqCount = event->sequenceCount;

	return seqCount;
}

void SceneEventManager::cancelEvent(SimObject *destScene, U32 eventSequence) {	
	// Find the graph queue to update
	SceneGraphEventQueue *queueWalk = SceneEventManager::gGraphQueueRoot->nextQueue;
	SceneGraphEventQueue *queueCurrent = SceneEventManager::gGraphQueueRoot;

	while(queueWalk != NULL && (SimObject *)(queueCurrent->pScene) != destScene){
		queueCurrent = queueWalk;
		queueWalk = queueCurrent->nextQueue;
	}
	// Didn't find the graph queue
	if((SimObject *)(queueCurrent->pScene) != destScene){
		Con::errorf("Could not find queue for scenegraph.");
		
		return;
	}


   SceneEvent **walk = &queueCurrent->eventQueue;
   SceneEvent *current;
   
   while((current = *walk) != NULL)
   {
      if(current->sequenceCount == eventSequence)
      {
         *walk = current->nextEvent;
         delete current;
         return;
      }
      else
         walk = &(current->nextEvent);
   }

}

void SceneEventManager::cancelPendingEvents(SimObject *obj) {

	// Cycle through the graphs
	SceneGraphEventQueue *queueWalk = SceneEventManager::gGraphQueueRoot->nextQueue;
	SceneGraphEventQueue *queueCurrent = SceneEventManager::gGraphQueueRoot;
	SceneGraphEventQueue *queuePrev = NULL;
	SceneEvent **walk = NULL;
	SceneEvent *current = NULL;
	
	while(queueWalk != NULL && (SimObject *)(queueCurrent->pScene) != obj) {
		queuePrev = queueCurrent;
		queueCurrent = queueWalk;
		queueWalk = queueCurrent->nextQueue;
		walk = &(queueCurrent->eventQueue);
		current = NULL;

		// If the scenegraph is to be deleted, clear it
		if((SimObject *)(queueCurrent->pScene) == obj) {

			while((current = *walk) != NULL) {
				*walk = current->nextEvent;
				delete current;
			}
			
			delete queueCurrent;
			queuePrev->nextQueue = queueWalk;
		}
		// Else cycle through this graph searching for events on this object
		else {
			while((current = *walk) != NULL) {

				if(current->destObject == obj){
					*walk = current->nextEvent;
					delete current;
				} 
				else
					walk = &(current->nextEvent);
			}
		}
	}
	
}

bool SceneEventManager::isEventPending(SimObject *destScene, U32 eventSequence) {
	
	// Find the graph queue to update
	SceneGraphEventQueue *queueWalk = SceneEventManager::gGraphQueueRoot;
	SceneGraphEventQueue *queueCurrent = SceneEventManager::gGraphQueueRoot;

	while(queueWalk != NULL && (SimObject *)(queueCurrent->pScene) != destScene){
		queueCurrent = queueWalk;
		queueWalk = queueCurrent->nextQueue;
	}
	// Didn't find the graph queue
	if((SimObject *)(queueCurrent->pScene) != destScene){
		Con::errorf("Could not find queue for scenegraph.");
		
		return false;
	}

	for(SceneEvent *walk = queueCurrent->eventQueue; walk; walk = walk->nextEvent)
	  if(walk->sequenceCount == eventSequence)
	  {
		 return true;
	  }

	return false;
}

F32 SceneEventManager::getEventTimeLeft(SimObject *destScene, U32 eventSequence) {
	
	// Find the graph queue to update
	SceneGraphEventQueue *queueWalk = SceneEventManager::gGraphQueueRoot;
	SceneGraphEventQueue *queueCurrent = SceneEventManager::gGraphQueueRoot;

	while(queueWalk != NULL && (SimObject *)(queueCurrent->pScene) != destScene){
		queueCurrent = queueWalk;
		queueWalk = queueCurrent->nextQueue;
	}
	// Didn't find the graph queue
	if((SimObject *)(queueCurrent->pScene) != destScene){
		Con::errorf("Could not find queue for scenegraph.");
		
		return 0;
	}



   for(SceneEvent *walk = queueCurrent->eventQueue; walk; walk = walk->nextEvent)
      if(walk->sequenceCount == eventSequence)
      {
         F32 t = walk->time - queueCurrent->currentSceneTime;
         return t;
      }

   return 0;   
}

F32 SceneEventManager::getScheduleDuration(SimObject *destScene, U32 eventSequence){ 
	
	// Find the graph queue to update
	SceneGraphEventQueue *queueWalk = SceneEventManager::gGraphQueueRoot;
	SceneGraphEventQueue *queueCurrent = SceneEventManager::gGraphQueueRoot;

	while(queueWalk != NULL && (SimObject *)(queueCurrent->pScene) != destScene){
		queueCurrent = queueWalk;
		queueWalk = queueCurrent->nextQueue;
	}
	// Didn't find the graph queue
	if((SimObject *)(queueCurrent->pScene) != destScene){
		Con::errorf("Could not find queue for scenegraph.");
		
		return 0;
	}


   for(SceneEvent *walk = queueCurrent->eventQueue; walk; walk = walk->nextEvent)
      if(walk->sequenceCount == eventSequence)
      {
         F32 t = walk->time - walk->startTime;
         return t;
      }

   return 0;   
}

F32 SceneEventManager::getTimeSinceStart(SimObject *destScene, U32 eventSequence) {
	
	// Find the graph queue to update
	SceneGraphEventQueue *queueWalk = SceneEventManager::gGraphQueueRoot;
	SceneGraphEventQueue *queueCurrent = SceneEventManager::gGraphQueueRoot;

	while(queueWalk != NULL && (SimObject *)(queueCurrent->pScene) != destScene){
		queueCurrent = queueWalk;
		queueWalk = queueCurrent->nextQueue;
	}
	// Didn't find the graph queue
	if((SimObject *)(queueCurrent->pScene) != destScene){
		Con::errorf("Could not find queue for scenegraph.");
		
		return 0;
	}


   for(SceneEvent *walk = queueCurrent->eventQueue; walk; walk = walk->nextEvent)
      if(walk->sequenceCount == eventSequence)
      {
         F32 t = queueCurrent->currentSceneTime - walk->startTime;
         return t;
      }

   return 0;   
}

void SceneEventManager::advanceToTime(SimObject *destScene, F32 targetTime) {
	    
	// Find the queue to update
	SceneGraphEventQueue *queueWalk = SceneEventManager::gGraphQueueRoot->nextQueue;
	SceneGraphEventQueue *queueCurrent = SceneEventManager::gGraphQueueRoot;

	while(queueWalk != NULL && (SimObject *)(queueCurrent->pScene) != destScene){
		queueCurrent = queueWalk;
		queueWalk = queueCurrent->nextQueue;
	}
	// Didn't find the scene graph 
	if((SimObject *)(queueCurrent->pScene) != destScene){
		Con::errorf("Could not find queue for scenegraph.");
		return;
	}

	// Advance the scenes queued events
	AssertFatal(targetTime >= queueCurrent->currentSceneTime, "SceneEventQueue::process: cannot advance to time in the past.");
	for(SceneEvent *event = queueCurrent->eventQueue; event && event->time <= targetTime; event = event->nextEvent) {
		if(event->execute)
			continue;
		AssertFatal(event->time >= queueCurrent->currentSceneTime,
			"SimEventQueue::pop: Cannot go back in time (flux capacitor not installed - BJG).");
		queueCurrent->currentSceneTime = event->time;
		event->execute = true;

	}
	queueCurrent->currentSceneTime = targetTime;
}

void SceneEventManager::executeEvents() {
	// Walk through the queues executing events
	SceneGraphEventQueue *queueWalk = SceneEventManager::gGraphQueueRoot->nextQueue;
	SceneGraphEventQueue *queueCurrent = SceneEventManager::gGraphQueueRoot;

	while(queueWalk != NULL){
		queueCurrent = queueWalk;
		queueWalk = queueCurrent->nextQueue;

		queueCurrent->executingEvents = true;

		while(!SceneEventManager::gAbortEvents && queueCurrent->eventQueue && queueCurrent->eventQueue->execute) {
			SceneEvent *event = queueCurrent->eventQueue;
			queueCurrent->eventQueue = queueCurrent->eventQueue->nextEvent;
			SimObject *obj = event->destObject;

			if(!obj->isDeleted())
				event->process(obj);
			delete event;
			// Skip rest of scenegraphs events
			// if scenegraph has been deleted
			if(SceneEventManager::gAbortEvents)
				break;
		}
		
		queueCurrent->executingEvents = false;
	}
}

SceneEventManager::SceneGraphEventQueue::SceneGraphEventQueue(Scene *destScene){
    
	this->pScene = destScene;
	
	SceneGraphEventQueue *walk = SceneEventManager::gGraphQueueRoot;
	SceneGraphEventQueue *current = SceneEventManager::gGraphQueueRoot;
	
	// Add this to the master linked list
	if(walk != NULL) {
		while(walk != NULL){
			current = walk;
			walk = current->nextQueue;
		}
		current->nextQueue = this;
	}
	this->nextQueue = NULL;
	this->eventQueue = NULL;

	this->currentSceneTime = 0;
	this->eventSequence = 1;
	this->executingEvents = false;


}

SceneEventManager::SceneGraphEventQueue::~SceneGraphEventQueue(){
	if(this->executingEvents)
		SceneEventManager::gAbortEvents = true;
}

SceneEventManager::SceneEvent::SceneEvent(S32 argc, const char **argv, bool onObject)
{
   destObject = NULL;

   mOnObject = onObject;
   mArgc = argc;
   U32 totalSize = 0;
   S32 i;
   for(i = 0; i < argc; i++)
      totalSize += dStrlen(argv[i]) + 1;
   totalSize += sizeof(char *) * argc;

   mArgv = (char **) dMalloc(totalSize);
   char *argBase = (char *) &mArgv[argc];

   for(i = 0; i < argc; i++)
   {
      mArgv[i] = argBase;
      dStrcpy(mArgv[i], argv[i]);
      argBase += dStrlen(argv[i]) + 1;
   }

   this->execute = false;
}

SceneEventManager::SceneEvent::~SceneEvent()
{
   dFree(mArgv);
}

void SceneEventManager::SceneEvent::process(SimObject *object) {
// #ifdef DEBUG
//    Con::printf("Executing schedule: %d", sequenceCount);
// #endif
	if(mOnObject){
      Con::execute(object, mArgc, const_cast<const char**>( mArgv ));
	}
   else
   {

      // Grab the function name. If '::' doesn't exist, then the schedule is
      // on a global function.
      char* func = dStrstr( mArgv[0], (char*)"::" );
      if( func )
      {
         // Set the first colon to NULL, so we can reference the namespace.
         // This is okay because events are deleted immediately after
         // processing. Maybe a bad idea anyway?
         func[0] = '\0';

         // Move the pointer forward to the function name.
         func += 2;

         // Lookup the namespace and function entry.
         Namespace* ns = Namespace::find( StringTable->insert( mArgv[0] ) );
         if( ns )
         {
            Namespace::Entry* nse = ns->lookup( StringTable->insert( func ) );
            if( nse )
               // Execute.
               nse->execute( mArgc, (const char**)mArgv, &gEvalState );
         }
      }

      else
         Con::execute(mArgc, const_cast<const char**>( mArgv ));
   }
}

#endif

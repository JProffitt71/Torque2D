//-----------------------------------------------------------------------------
// Unofficial Scene Event Manager
// Justin Proffitt
//-----------------------------------------------------------------------------

#ifndef _SCENEEVENTMANAGER_H_
#define _SCENEEVENTMANAGER_H_

#ifndef _SCENE_H_
#include "2d/scene/Scene.h"
#endif

namespace SceneEventManager {
	
	class SceneGraphEventQueue;
	class SceneEvent;

	void advanceToTime(SimObject *, F32);
	void executeEvents();
	
	U32 postEvent(SimObject *, SimObject *, SceneEvent *, F32);
	U32 postEvent(SimObjectId, SimObjectId, SceneEvent *, F32);
	U32 postEvent(SimObjectId, const char *, SceneEvent *, F32);
	U32 postEvent(const char *, SimObjectId, SceneEvent *, F32);
	U32 postEvent(const char *, const char *, SceneEvent *, F32);
	
	void cancelEvent(SimObject *, U32);
	void cancelPendingEvents(SimObject *);
	bool isEventPending(SimObject *, U32);
	F32 getEventTimeLeft(SimObject *, U32);
	F32 getScheduleDuration(SimObject *, U32);
	F32 getTimeSinceStart(SimObject *, U32);
	
	class SceneGraphEventQueue : public SimObject {
		public:
			Scene *pScene;
			SceneGraphEventQueue *nextQueue;
			SceneEvent *eventQueue;

			F32 currentSceneTime;
			U32 eventSequence;
			bool executingEvents;

			SceneGraphEventQueue(Scene *destGraph);

			~SceneGraphEventQueue();
	};

	class SceneEvent : public SimObject {
		protected:
		   S32 mArgc;
		   char **mArgv;
		   bool mOnObject;
		public:
			SceneEvent *nextEvent;
			F32 startTime;
			F32 time;
			U32 sequenceCount;

			SimObject *destObject;

			bool execute;
			
			SceneEvent(S32 argc, const char **argv, bool onObject);
			
			~SceneEvent();
			virtual void process(SimObject *object);
	};
}
#endif
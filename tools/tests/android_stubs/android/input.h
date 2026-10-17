#pragma once
#include <cstddef>
#include <cstdint>
struct AInputEvent;
int32_t AInputEvent_getType(const AInputEvent*);
int32_t AMotionEvent_getAction(const AInputEvent*);
size_t AMotionEvent_getPointerCount(const AInputEvent*);
int32_t AMotionEvent_getPointerId(const AInputEvent*, size_t);
float AMotionEvent_getX(const AInputEvent*, size_t);
float AMotionEvent_getY(const AInputEvent*, size_t);
#define AINPUT_EVENT_TYPE_MOTION 2
#define AMOTION_EVENT_ACTION_DOWN 0
#define AMOTION_EVENT_ACTION_UP 1
#define AMOTION_EVENT_ACTION_MOVE 2
#define AMOTION_EVENT_ACTION_CANCEL 3
#define AMOTION_EVENT_ACTION_POINTER_DOWN 5
#define AMOTION_EVENT_ACTION_POINTER_UP 6
#define AMOTION_EVENT_ACTION_MASK 255
#define AMOTION_EVENT_ACTION_POINTER_INDEX_MASK 65280
#define AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT 8

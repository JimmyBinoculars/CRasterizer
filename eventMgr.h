#include "calcs.h"

#ifndef EVENTMGR_H
#define EVENTMGR_H

void HandleEvents(bool *running, Camera *cam, float rotSpeed, float moveSpeed, float PITCH_LIMIT, 
        float deltaTime, float MOUSE_SENSITIVITY);

#endif

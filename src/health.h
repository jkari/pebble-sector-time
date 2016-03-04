#pragma once

#define BLOCK_MINUTES 12
#define ACTIVITY_SCALE 0.1f
#define ACTIVITY_LEVELS 3

void health_update_activity();
void health_get_activity(uint8_t *data);
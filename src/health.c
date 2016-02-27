#include <pebble.h>
#include "health.h"

void health_get_activity(float *list) {
  for (int i = 0; i < 60; i++) {
    list[i] = 0;
  }
  
  const uint32_t max_records = BLOCK_MINUTES * 60;
  HealthMinuteData *minute_data = (HealthMinuteData*)malloc(max_records * sizeof(HealthMinuteData));

  time_t end = time(NULL);
  int round = 720 - (end % 720);
  time_t start = end - 12 * SECONDS_PER_HOUR + round;
  
  uint32_t num_records = health_service_get_minute_history(
    minute_data, 
    max_records,
    &start,
    &end
  );
  
  APP_LOG(APP_LOG_LEVEL_INFO, "num_records: %d", (int)num_records);
  
  for(uint32_t i = 0; i < num_records; i += BLOCK_MINUTES) {
    float block_steps = 0;
    
    for (uint32_t k = 0; k < BLOCK_MINUTES; k++) {
      if (i + k < num_records) {
        block_steps += (int)minute_data[i + k].steps;
      }
    }

    list[i / BLOCK_MINUTES] = block_steps / ACTIVITY_MAX <= 1.0f ? block_steps / ACTIVITY_MAX : 1.0f;    
  }
  
  free(minute_data);
}
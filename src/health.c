#include <pebble.h>
#include "health.h"
#include "config.h"

void _save_activity_update(time_t now, int total) {
  persist_write_int(PERSIST_KEY_LAST_ACTIVITY_UPDATE, now);
  persist_write_int(PERSIST_KEY_LAST_ACTIVITY_VALUE, total);
}
                    
int _get_today_total() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  tick_time->tm_sec = 0;
  tick_time->tm_min = 0;
  tick_time->tm_hour = 0;
  time_t start = mktime(tick_time);
  
  HealthServiceAccessibilityMask result = 
      health_service_metric_accessible(HealthMetricStepCount, start, time(NULL));
  
  if (result & HealthServiceAccessibilityMaskAvailable) {
      HealthValue steps = health_service_sum(HealthMetricStepCount, start, time(NULL));
    
      if (steps < PERSIST_KEY_LAST_ACTIVITY_VALUE) {
        _save_activity_update(time(NULL), 0);
      }
    
      return (int)steps;
  }
  
  return 0;
}

time_t _get_last_update() {
  return persist_exists(PERSIST_KEY_LAST_ACTIVITY_UPDATE) ? persist_read_int(PERSIST_KEY_LAST_ACTIVITY_UPDATE) : 0;
}

void _save_activity(int index, uint8_t value) {
  uint8_t data[60];
  health_get_activity(data);
  data[index] = value;
  persist_write_data(PERSIST_KEY_ACTIVITY, data, 60 * sizeof(uint8_t));
}

void health_get_activity(uint8_t *data) {
  if (persist_exists(PERSIST_KEY_ACTIVITY)) {
    persist_read_data(PERSIST_KEY_ACTIVITY, data, 60 * sizeof(uint8_t));
  } else {
    for (int i = 0; i < 60; i++) {
      data[i] = 0;
    }
  }
  
  /*
  data[16] = 10;
  data[17] = 20;
  data[18] = 50;
  data[19] = 100;
  data[20] = 70;
  data[21] = 30;
  data[40] = 10;
  data[41] = 70;
  data[42] = 150;
  data[43] = 170;
  data[44] = 100;
  data[45] = 30;
  */
}

void health_update_activity() {  
  time_t last_time = _get_last_update();
  struct tm *tick_last = localtime(&last_time);
  int last_index = (tick_last->tm_hour % 12) * 5 + tick_last->tm_min / 12;
  
  time_t current_time = time(NULL);
  struct tm *tick_current = localtime(&current_time);
  int current_index = (tick_current->tm_hour % 12) * 5 + tick_current->tm_min / 12;
  
  int total = _get_today_total();
  int new_value = (0.0f + total - persist_read_int(PERSIST_KEY_LAST_ACTIVITY_VALUE)) * ACTIVITY_SCALE;
  new_value = new_value > 255 ? 255 : new_value;
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Set %d: %d (%d)", last_index, new_value, total);
  
  _save_activity(last_index, new_value);
  
  if (last_index != current_index) {
    _save_activity_update(current_time, total);
  }
}
 /*
  * @file reading_queue.c
  * @brief Implementation of the ring buffer queue using a circular buffer.
  *
  * Author: Gioele Giunta (refactored)
  * Date: 2025-05-28
  */
 
 #include "reading_queue.h"
 #include <string.h> // for memcpy
 #include "freertos/semphr.h"
 
 reading_queue_t queue;
 
 void reading_queue_init(void) {
     queue.head = 0;
     queue.tail = 0;
     queue.mutex = xSemaphoreCreateMutex();
 }
 
 void reading_queue_enqueue(const complex_g3_t *value) {
     if (queue.mutex == NULL) return; // not initialized
     xSemaphoreTake(queue.mutex, portMAX_DELAY);
     uint16_t next_head = (queue.head + 1) % READING_QUEUE_SIZE;
     // If full, advance tail to overwrite oldest
     if (next_head == queue.tail) {
         queue.tail = (queue.tail + 1) % READING_QUEUE_SIZE;
     }
     queue.data[queue.head] = *value;
     queue.head = next_head;
     xSemaphoreGive(queue.mutex);
 }
 
 bool reading_queue_range(uint16_t from, uint16_t range, complex_g3_t *out_array) {
     if (queue.mutex == NULL) return false;
     xSemaphoreTake(queue.mutex, portMAX_DELAY);
     // Compute available samples
     uint16_t available = (queue.head >= queue.tail)
                          ? (queue.head - queue.tail)
                          : (READING_QUEUE_SIZE - queue.tail + queue.head);
     if (from + range > available) {
         xSemaphoreGive(queue.mutex);
         return false;
     }
     // Copy elements
     for (uint16_t i = 0; i < range; i++) {
         uint16_t idx = (queue.tail + from + i) % READING_QUEUE_SIZE;
         out_array[i] = queue.data[idx];
     }
     xSemaphoreGive(queue.mutex);
     return true;
 }
 
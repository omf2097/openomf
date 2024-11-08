#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

void fps_counter_init(void);
void fps_counter_render(void);
void fps_counter_add_measurement(double measurement);

#endif // FPS_COUNTER_H

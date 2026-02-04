/*
 * gui_task.c
 *
 *  Created on: 28/01/2026
 *      Author:
 */
#include "gui_task.h"

static void GuiTask(void *pvParameters) {
	(void) pvParameters;
	const TickType_t xDelay = pdMS_TO_TICKS(10);

	for (;;) {
		lv_timer_handler();   // or lv_task_handler();

		if (time_count < LOGO_TIME) {
			commanded_display_state = LOGO;
		} else if (vcu.fault != 0 || !vcu.active) {
			commanded_display_state = DIAGNOSTIC;
		} else {
			commanded_display_state = vcu.rtd ? DRIVE : PRE_DRIVE;
		}
		static bool init_screen = false;
		if (init_screen) {
			init_screen = false;
			initialize_display_colors();
			initialize_display_state(commanded_display_state);
		} else if (commanded_display_state != current_display_state) {
			clear_display_state(current_display_state);
			init_screen = true;
			current_display_state = commanded_display_state;
		}

		if ((time_count % 20) == 0 && !init_screen) { // since loop ticks every 10 ms
			update_display_state(current_display_state);
		}

		time_count++;

		vcu.active = (time_count - vcu.last_comm_time) <= 50;

		osDelay(xDelay);
	}
}

void CreateGuiTask(void) {

	osThreadNew(GuiTask, NULL, &(osThreadAttr_t ) { .name = "gui", .priority =
					osPriorityNormal, .stack_size = 1024 * 4 });

}


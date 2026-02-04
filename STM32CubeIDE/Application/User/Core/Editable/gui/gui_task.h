/*
 * gui_task.h
 *
 *  Created on: 28/01/2026
 *      Author:
 */

#ifndef APPLICATION_USER_CORE_EDITABLE_GUI_GUI_TASK_H_
#define APPLICATION_USER_CORE_EDITABLE_GUI_GUI_TASK_H_

#ifndef GUI_TASK_H_
#define GUI_TASK_H_

#include "lvgl/lvgl.h"
#include "cmsis_os2.h"
#include "../dashboard.h"
#include <stdbool.h>

/* Create the GUI task (call once during system init) */
void CreateGuiTask(void);

/* Optional: allow starting/stopping the GUI task from other code (not required) */
void GuiTask_Stop(void);
void GuiTask_Start(void);

#endif /* GUI_TASK_H_ */

#endif /* APPLICATION_USER_CORE_EDITABLE_GUI_GUI_TASK_H_ */

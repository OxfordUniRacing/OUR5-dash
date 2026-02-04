/*
 * dashboard.h
 *
 *  Created on: 28/01/2026
 *      Author:
 */

#ifndef APPLICATION_USER_CORE_EDITABLE_DASHBOARD_H_
#define APPLICATION_USER_CORE_EDITABLE_DASHBOARD_H_

#include "lvgl/lvgl.h"
#include "fdcan.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "lv_conf.h"
#include <stdbool.h>

#define LOGO_TIME 350 //time to show logo before switching to pre-drive state

#define LVGL_GREEN "009632"
#define LVGL_YELLOW "c8c800"
#define LVGL_RED "ff0000"
#define LVGL_BLACK "000000"
#define LVGL_WHITE "ffffff"

#define GREEN_HEX 0x009632
#define YELLOW_HEX 0xc8c800
#define RED_HEX 0xff0000
#define LIGHT_GRAY_HEX 0x646464

#define INVERTER_CUTOFF_TEMP 86

//persistent lv_objs for logo state
extern lv_obj_t *our_logo;

//persistent lv_objs for pre_drive state
extern lv_obj_t *pre_drive_grid;
extern lv_obj_t *pre_drive_labels[4];

//persistent lv_objs for drive state
extern lv_obj_t *drive_grid;
extern lv_obj_t *rpm_arc;
extern lv_obj_t *rpm_arc_label;
extern lv_obj_t *speed_arc;
extern lv_obj_t *acceleration_arc;
extern lv_obj_t *battery_temp_label;
extern lv_obj_t *current_limiting_factor_label;
extern lv_obj_t *battery_soc_bar;
extern lv_obj_t *battery_soc_label;
extern lv_obj_t *inverter_temp_label;
extern lv_obj_t *motor_temp_label;

//persistent lv_objs for diagnostic state
extern lv_obj_t *diagnostic_label;

extern const lv_img_dsc_t our_logo_screenshot;

typedef enum {
	UNINITIALIZED = 0, LOGO = 1, PRE_DRIVE = 2, DRIVE = 3, DIAGNOSTIC = 4
} display_state_t;

typedef enum {
	STATUSWORD_NOTREADY = 0x01,
	STATUSWORD_SHUTDOWN = 0x02,
	STATUSWORD_PRECHARGE = 0x04,
	STATUSWORD_ENERGISED = 0x07,
	STATUSWORD_ENABLED = 0x08,
	STATUSWORD_FAULTREACTION = 0x0B,
	STATUSWORD_FAULTOFF = 0x0D
} statusword_t;

const char* inverter_statusword(statusword_t word);

typedef struct {
	float output_torque;
	int16_t motor_speed;
	int16_t battery_current;
	float available_forward_torque;
	float available_reverse_torque;
	statusword_t statusword;
	float capacitor_voltage;
	bool active;
	int16_t temperature;
	int16_t motor_temp;
} inv_t;

typedef struct {
	uint16_t pack_dcl;
	uint8_t temperature;
	float pack_voltage;
	uint8_t pack_soc;
	float pack_current;
	bool active;
} battery_t;

typedef struct {
	float lv_voltage;
	uint16_t max_rpm;
	uint8_t current_limit;
	uint16_t max_torque;
	bool rtd;
	bool rtd_switch_state;
	uint8_t fault;
	bool active;
	uint32_t last_comm_time;
} vcu_t;

typedef struct {
	lv_obj_t *arc;
	lv_obj_t *value_label;
	lv_obj_t *units_label;
} arc_with_label_t;

extern uint32_t time_count;

extern inv_t inv1;
extern inv_t inv2;

extern battery_t battery;

extern vcu_t vcu;

extern display_state_t current_display_state;
extern display_state_t commanded_display_state;

void update_inverter(lv_obj_t *label, inv_t *inv);
void update_battery(lv_obj_t *label, battery_t *battery);
void update_vcu(lv_obj_t *label, vcu_t *vcu);

void initialize_display_colors(void);
void initialize_display_state(display_state_t display_state);
void initialize_display_state_logo(void);
void initialize_display_state_pre_drive(void);
void initialize_display_state_drive(void);
void initialize_display_state_diagnostic(void);

void clear_display_state(display_state_t display_state);
void clear_display_state_logo(void);
void clear_display_state_pre_drive(void);
void clear_display_state_drive(void);
void clear_display_state_diagnostic(void);

void update_display_state(display_state_t display_state);
void update_display_state_logo(void);
void update_display_state_pre_drive(void);
void update_display_state_drive(void);
void update_display_state_diagnostic(void);

void set_display_background(void);

#endif /* APPLICATION_USER_CORE_EDITABLE_DASHBOARD_H_ */

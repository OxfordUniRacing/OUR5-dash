#include "dashboard.h"
#include "fdcan/fdcan_handlers.h"
#include "gui/gui_task.h"

uint32_t time_count = 0;

inv_t inv1 = { 0 };
inv_t inv2 = { 0 };
battery_t battery = { 0 };
vcu_t vcu = { 0 };

display_state_t current_display_state = UNINITIALIZED;
display_state_t commanded_display_state = LOGO;

//persistent lv_objs for logo state
lv_obj_t *our_logo;

//persistent lv_objs for pre_drive state
lv_obj_t *pre_drive_grid;
lv_obj_t *pre_drive_labels[4];

//persistent lv_objs for drive state
lv_obj_t *drive_grid;
lv_obj_t *rpm_arc;
lv_obj_t *rpm_arc_label;
lv_obj_t *speed_arc;
lv_obj_t *acceleration_arc;
lv_obj_t *battery_temp_label;
lv_obj_t *current_limiting_factor_label;
lv_obj_t *battery_soc_bar;
lv_obj_t *battery_soc_label;
lv_obj_t *inverter_temp_label;
lv_obj_t *motor_temp_label;

//persistent lv_objs for diagnostic state
lv_obj_t *diagnostic_label;

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

const char* inverter_statusword(statusword_t word) {
	switch (word) {
	case STATUSWORD_NOTREADY:
		return "NOT READY";
	case STATUSWORD_SHUTDOWN:
		return "SHUTDOWN";
	case STATUSWORD_PRECHARGE:
		return "PRECHARGE";
	case STATUSWORD_ENERGISED:
		return "ENERGISED";
	case STATUSWORD_ENABLED:
		return "ENABLED";
	case STATUSWORD_FAULTREACTION:
		return "FAULT REACTION";
	case STATUSWORD_FAULTOFF:
		return "FAULT OFF";
	default:
		return "";
	}
}

void initialize_display_state(display_state_t display_state) {
	switch (display_state) {
	case LOGO:
		initialize_display_state_logo();
		break;
	case PRE_DRIVE:
		initialize_display_state_pre_drive();
		break;
	case DRIVE:
		initialize_display_state_drive();
		break;
	case DIAGNOSTIC:
		initialize_display_state_diagnostic();
		break;
	default:
		break;
	}

}

void clear_display_state(display_state_t display_state) {
	switch (display_state) {
	case LOGO:
		clear_display_state_logo();
		break;
	case PRE_DRIVE:
		clear_display_state_pre_drive();
		break;
	case DRIVE:
		clear_display_state_drive();
		break;
	case DIAGNOSTIC:
		clear_display_state_diagnostic();
		break;
	default:
		break;
	}
}

void update_display_state(display_state_t display_state) {
	switch (display_state) {
	case LOGO:
		update_display_state_logo();
		break;
	case PRE_DRIVE:
		update_display_state_pre_drive();
		break;
	case DRIVE:
		update_display_state_drive();
		break;
	case DIAGNOSTIC:
		update_display_state_diagnostic();
		break;
	default:
		break;
	}
}

lv_obj_t* generate_grid(bool border) {
	lv_obj_t *grid = lv_obj_create(lv_scr_act());
	; // make use of the additional variable so I don't have to change chatgpt's code
	lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_size(grid, 800, 480);
	lv_obj_center(grid); // Optional: center the grid on the screen

	// Enable grid layout
	static lv_coord_t col_dsc[] = { 400, 400, LV_GRID_TEMPLATE_LAST };
	static lv_coord_t row_dsc[] = { 240, 240, LV_GRID_TEMPLATE_LAST };

	lv_obj_set_layout(grid, LV_LAYOUT_GRID);
	lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
	lv_obj_set_style_pad_all(grid, 0, 0);
	lv_obj_set_style_pad_row(grid, 0, 0);
	lv_obj_set_style_pad_column(grid, 0, 0);
	lv_obj_set_style_bg_color(grid, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(grid, LV_OPA_COVER, 0);
	if (border) {
		lv_obj_set_style_border_color(grid, lv_color_make(0x64, 0x64, 0x64), 0);
		lv_obj_set_style_border_width(grid, 2, 0);
	} else {
		lv_obj_set_style_border_color(grid, lv_color_black(), 0);
		lv_obj_set_style_border_width(grid, 2, 0);
	}

	return grid;
}

void generate_style(lv_style_t *style_label, const lv_font_t *font, bool border,
bool center_align) {
	lv_style_init(style_label);
	lv_style_set_text_color(style_label, lv_color_white());
	lv_style_set_bg_color(style_label, lv_color_black());
	lv_style_set_bg_opa(style_label, LV_OPA_COVER);
	if (border) {
		lv_style_set_border_color(style_label, lv_color_make(0x64, 0x64, 0x64));
		lv_style_set_border_width(style_label, 2);
	} else {
		lv_style_set_border_color(style_label, lv_color_black());
		lv_style_set_border_width(style_label, 2);
	}
	if (center_align) {
		lv_style_set_text_align(style_label, LV_TEXT_ALIGN_CENTER);
		lv_style_set_align(style_label, LV_ALIGN_CENTER);
	} else {
		lv_style_set_text_align(style_label, LV_TEXT_ALIGN_LEFT);
		lv_style_set_align(style_label, LV_ALIGN_TOP_LEFT);
	}
	lv_style_set_pad_left(style_label, 5); // padding for left alignment
	lv_style_set_text_font(style_label, font);
}
lv_color_t LV_COLOR_LIGHT_GRAY;

static arc_with_label_t create_arc_with_label(lv_obj_t *parent, int init_value,
		int range, const char *units) {
	arc_with_label_t a = { 0 };
	a.arc = lv_arc_create(parent);
	lv_obj_set_size(a.arc, 200, 200);
	lv_arc_set_range(a.arc, 0, range);
	lv_arc_set_value(a.arc, init_value);
	lv_arc_set_bg_angles(a.arc, 0, 270);
	lv_obj_clear_flag(a.arc, LV_OBJ_FLAG_CLICKABLE);
	lv_arc_set_mode(a.arc, LV_ARC_MODE_NORMAL);
	lv_arc_set_rotation(a.arc, 135);
	lv_obj_set_style_arc_color(a.arc, LV_COLOR_LIGHT_GRAY, LV_PART_MAIN);
	lv_obj_set_style_arc_width(a.arc, 6, LV_PART_INDICATOR);
	lv_obj_set_style_bg_opa(a.arc, LV_OPA_TRANSP, LV_PART_KNOB);

	// container for labels
	lv_obj_t *container = lv_obj_create(a.arc);
	lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_center(container);
	lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
	lv_obj_set_style_pad_all(container, 0, 0);
	lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

	a.value_label = lv_label_create(container);
	lv_label_set_text_fmt(a.value_label, "%d", init_value);
	lv_obj_set_style_text_font(a.value_label, &lv_font_montserrat_48, 0);
	a.units_label = lv_label_create(container);
	lv_label_set_text(a.units_label, units);
	lv_obj_set_style_text_font(a.units_label, &lv_font_montserrat_24, 0);

	return a;
}

void set_display_background(void) {
	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
	lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0); // Ensure it's not transparent
	lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
}

void initialize_display_state_logo(void) {
	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_white(), 0);
	lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0); // Ensure it's not transparent
	lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

	our_logo = lv_img_create(lv_scr_act());   // Create image object
	lv_img_set_src(our_logo, &our_logo_screenshot);          // Set image source
	lv_img_set_zoom(our_logo, 450);
	lv_obj_align(our_logo, LV_ALIGN_CENTER, 0, 0); // Align to center (optional)
}

void clear_display_state_logo(void) {
	lv_obj_del(our_logo);
}

void update_display_state_logo(void) {/*do nothing*/
}

void initialize_display_state_pre_drive(void) {
	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
	lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0); // Ensure it's not transparent
	lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

	// Create a container for the grid
	pre_drive_grid = generate_grid(true);

	static lv_style_t style_label;
	generate_style(&style_label, &lv_font_montserrat_30, true, false);

	static lv_style_t style_label_small;
	generate_style(&style_label_small, &lv_font_montserrat_24, true, false);

	// Helper function to create a label in a grid cell
	for (int row = 0; row < 2; row++) {
		for (int col = 0; col < 2; col++) {
			lv_obj_t *label = lv_label_create(pre_drive_grid);
			lv_label_set_recolor(label, true);
			if (row == 0 && col == 0)
				lv_obj_add_style(label, &style_label, 0);
			else
				lv_obj_add_style(label, &style_label_small, 0);
			pre_drive_labels[2 * row + col] = label;

			// Set label to occupy one cell
			lv_obj_set_grid_cell(label, LV_GRID_ALIGN_STRETCH, col, 1,
					LV_GRID_ALIGN_STRETCH, row, 1);
		}
	}

}

void clear_display_state_pre_drive(void) {
	lv_obj_del(pre_drive_grid);
}

void update_display_state_pre_drive() {
	const char *lv_battery_color_state =
			vcu.lv_voltage >= 12.7 ? LVGL_GREEN : LVGL_YELLOW;
	const char *rtd_color_state =
			vcu.rtd_switch_state && time_count % 80 >= 40 ?
			LVGL_RED :
															LVGL_WHITE;
	const char *rtd_state_string = vcu.rtd_switch_state ? "ON" : "OFF";

	lv_label_set_text_fmt(pre_drive_labels[0], "LV Batt: #%s %.1f V#\n"
			"RTD Switch: #%s %s#", lv_battery_color_state, vcu.lv_voltage,
			rtd_color_state, rtd_state_string);

	lv_label_set_text_fmt(pre_drive_labels[2], "TS Battery Pack\n"
			"Temperature: %d °C\n"
			"SOC: %d%%\n"
			"Pack Voltage: %.1f\n"
			"Pack DCL: %d A", battery.temperature, battery.pack_soc,
			battery.pack_voltage, battery.pack_dcl);

	lv_label_set_text_fmt(pre_drive_labels[1], "Inverters\n"
			"Inv1 Status: %s\n"
			"Inv1 Cap Voltage: %.2f\n\n"
			"Inv2 Status: %s\n"
			"Inv2 Cap Voltage: %.2f", inverter_statusword(inv1.statusword),
			inv1.capacitor_voltage, inverter_statusword(inv2.statusword),
			inv2.capacitor_voltage);

	lv_label_set_text_fmt(pre_drive_labels[3], "VCU Config\n"
			"Max Torque: %d N*m\n"
			"Max Inverter Current: %d A\n"
			"Max RPM: %d", vcu.max_torque, vcu.current_limit, vcu.max_rpm);

}

void initialize_display_state_drive(void) {
	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
	lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0); // Ensure it's not transparent
	lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

	drive_grid = generate_grid(false);

	// RPM
	arc_with_label_t rpm = create_arc_with_label(drive_grid, 0, 60, "rpm");
	rpm_arc = rpm.arc;
	rpm_arc_label = rpm.value_label;
	// Align it to the center of the cell
	int row = 1;
	int col = 0;
	lv_obj_set_grid_cell(rpm_arc, LV_GRID_ALIGN_CENTER, col, 1,
			LV_GRID_ALIGN_CENTER, row, 1);

	// VEHICLE SPEED
	arc_with_label_t speed = create_arc_with_label(drive_grid, 0, 150, "mph");
	speed_arc = speed.arc;
	// Align it to the center of the cell
	row = 1;
	col = 1;
	lv_obj_set_grid_cell(speed_arc, LV_GRID_ALIGN_CENTER, col, 1,
			LV_GRID_ALIGN_CENTER, row, 1);

	// ACCELERATION
	arc_with_label_t acceleration = create_arc_with_label(drive_grid, 0, 20,
			"G");
	acceleration_arc = acceleration.arc;
	row = 1;
	col = 2;
	lv_obj_set_grid_cell(acceleration_arc, LV_GRID_ALIGN_CENTER, col, 1,
			LV_GRID_ALIGN_CENTER, row, 1);

	// DIAGNOSTICS
	lv_obj_t *diagnostics_container = lv_obj_create(drive_grid);
	lv_obj_set_size(diagnostics_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_center(diagnostics_container);

	lv_obj_set_flex_flow(diagnostics_container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_bg_opa(diagnostics_container, LV_OPA_TRANSP, 0); // Transparent
	lv_obj_set_style_pad_all(diagnostics_container, 0, 0);
	lv_obj_clear_flag(diagnostics_container, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_border_width(diagnostics_container, 0, 0);
	lv_obj_set_style_border_opa(diagnostics_container, LV_OPA_TRANSP, 0);

	// LIMITING FACTOR LABEL
	static lv_style_t limiting_factor_text_style;
	generate_style(&limiting_factor_text_style, &lv_font_montserrat_24, false,
	false);
	lv_obj_t *limiting_factor_label = lv_label_create(diagnostics_container);
	lv_obj_add_style(limiting_factor_label, &limiting_factor_text_style, 0);
	lv_label_set_text(limiting_factor_label, "Limiting factor");

	// FACTOR
	static lv_style_t limiting_factor_style;
	generate_style(&limiting_factor_style, &lv_font_montserrat_48, false, true);
	current_limiting_factor_label = lv_label_create(diagnostics_container);
	lv_obj_add_style(current_limiting_factor_label, &limiting_factor_style, 0);
	lv_label_set_recolor(current_limiting_factor_label, true);
	lv_label_set_text(current_limiting_factor_label, "Banana");

	row = 0;
	col = 2;
	lv_obj_set_grid_cell(diagnostics_container, LV_GRID_ALIGN_CENTER, col, 1,
			LV_GRID_ALIGN_CENTER, row, 1);

	// BATTERY

	// Create a container to hold battery info
	lv_obj_t *battery_container = lv_obj_create(drive_grid);
	lv_obj_set_size(battery_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_center(battery_container); // Center the whole label group in the arc

	// Use vertical flex layout to stack labels
	lv_obj_set_flex_flow(battery_container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_bg_opa(battery_container, LV_OPA_TRANSP, 0); // Transparent
	lv_obj_set_style_pad_all(battery_container, 0, 0);
	lv_obj_clear_flag(battery_container, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_border_width(battery_container, 0, 0);
	lv_obj_set_style_border_opa(battery_container, LV_OPA_TRANSP, 0);

	//add battery text label
	static lv_style_t text_style;
	generate_style(&text_style, &lv_font_montserrat_24, false, false);
	lv_obj_t *battery_text_label = lv_label_create(battery_container);
	lv_obj_add_style(battery_text_label, &text_style, 0);
	lv_label_set_text(battery_text_label, "TS Battery");

	//add battery temperature label
	static lv_style_t battery_temp_style;
	generate_style(&battery_temp_style, &lv_font_montserrat_48, false, true);
	battery_temp_label = lv_label_create(battery_container);
	lv_obj_add_style(battery_temp_label, &battery_temp_style, 0);
	lv_label_set_recolor(battery_temp_label, true);

	//add soc text label
	battery_soc_label = lv_label_create(battery_container);
	lv_obj_add_style(battery_soc_label, &text_style, 0);
	lv_label_set_text(battery_soc_label, "SOC: 0\%");

	//add battery SOC bar
	battery_soc_bar = lv_bar_create(battery_container);
	lv_obj_set_size(battery_soc_bar, 250, 15);
	lv_obj_center(battery_soc_bar);
	lv_bar_set_value(battery_soc_bar, 0, LV_ANIM_OFF);

	lv_obj_set_style_bg_color(battery_soc_bar, lv_color_hex(0x646464),
			LV_PART_MAIN);     // background
	lv_obj_set_style_bg_color(battery_soc_bar, lv_color_hex(0x009632),
			LV_PART_INDICATOR);  // fill
	lv_obj_set_style_radius(battery_soc_bar, 5, 0);

	lv_obj_clear_flag(battery_soc_bar, LV_OBJ_FLAG_CLICKABLE);

	// Align it to the center of the cell
	row = 0;
	col = 0;
	lv_obj_set_grid_cell(battery_container, LV_GRID_ALIGN_CENTER, col, 1,
			LV_GRID_ALIGN_CENTER, row, 1);

	// INVERTER

	// Create a container to hold inverter info
	lv_obj_t *inverter_container = lv_obj_create(drive_grid);
	lv_obj_set_size(inverter_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_center(inverter_container); // Center the whole label group in the arc

	// Use vertical flex layout to stack labels
	lv_obj_set_flex_flow(inverter_container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_bg_opa(inverter_container, LV_OPA_TRANSP, 0); // Transparent
	lv_obj_set_style_pad_all(inverter_container, 0, 0);
	lv_obj_clear_flag(inverter_container, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_border_width(inverter_container, 0, 0);
	lv_obj_set_style_border_opa(inverter_container, LV_OPA_TRANSP, 0);

	//add inverter text label
	lv_obj_t *inverter_text_label = lv_label_create(inverter_container);
	lv_obj_add_style(inverter_text_label, &text_style, 0);
	lv_label_set_text(inverter_text_label, "Inverters");

	//add inverter temperature label
	static lv_style_t inverter_temp_style;
	generate_style(&inverter_temp_style, &lv_font_montserrat_36, false, true);
	inverter_temp_label = lv_label_create(inverter_container);
	lv_obj_add_style(inverter_temp_label, &inverter_temp_style, 0);
	lv_label_set_recolor(inverter_temp_label, true);

	//add motor text label
	lv_obj_t *motor_text_label = lv_label_create(inverter_container);
	lv_obj_add_style(motor_text_label, &text_style, 0);
	lv_label_set_text(motor_text_label, "Motors");

	//add motor temperature label
	motor_temp_label = lv_label_create(inverter_container);
	lv_obj_add_style(motor_temp_label, &inverter_temp_style, 0);
	lv_label_set_recolor(motor_temp_label, true);

	// Align it to the center of the cell
	row = 0;
	col = 1;
	lv_obj_set_grid_cell(inverter_container, LV_GRID_ALIGN_CENTER, col, 1,
			LV_GRID_ALIGN_CENTER, row, 1);
}

void clear_display_state_drive(void) {
	lv_obj_del(drive_grid);
}

int green_yellow_red_range(int value, int greenThreshold, int yellowThreshold,
		int redThreshold) {
	if (value >= greenThreshold)
		return 3;
	else if (value >= yellowThreshold)
		return 2;
	else if (value >= redThreshold)
		return 1;

	return 0;
}

int red_yellow_green_range(int value, int greenThreshold, int yellowThreshold,
		int redThreshold) {
	if (value <= greenThreshold)
		return 3;
	else if (value <= yellowThreshold)
		return 2;
	else if (value <= redThreshold)
		return 1;

	return 0;
}

void update_display_state_drive() {
	//battery temperature
	const char *battery_temp_color;
	switch (red_yellow_green_range(battery.temperature, 35, 45, 70)) {
	case 3:
		battery_temp_color = LVGL_GREEN;
		break;
	case 2:
		battery_temp_color = LVGL_YELLOW;
		break;
	case 1:
		battery_temp_color = LVGL_RED;
		break;
	default:
		battery_temp_color = LVGL_WHITE;
	}
	lv_label_set_text_fmt(battery_temp_label, "#%s %d °C#", battery_temp_color,
			battery.temperature);

	//battery SOC
	uint32_t battery_soc_color;
	switch (green_yellow_red_range(battery.pack_soc, 50, 20, 0)) {
	case 3:
		battery_soc_color = GREEN_HEX;
		break;
	case 2:
		battery_soc_color = YELLOW_HEX;
		break;
	case 1:
		battery_soc_color = RED_HEX;
		break;
	default:
		battery_soc_color = 0xffffff;
		break;
	}
	lv_label_set_text_fmt(battery_soc_label, "SOC: %d%%", battery.pack_soc);
	lv_bar_set_value(battery_soc_bar, battery.pack_soc, LV_ANIM_OFF);
	lv_obj_set_style_bg_color(battery_soc_bar, lv_color_hex(battery_soc_color),
			LV_PART_INDICATOR);

	//inverter temperatures
	lv_label_set_text_fmt(inverter_temp_label,
			"#%s %d °C#\t#646464 |#\t#%s %d °C#",
			LVGL_GREEN, inv1.temperature, LVGL_GREEN, inv2.temperature);

	//motor temperatures
	lv_label_set_text_fmt(motor_temp_label,
			"#%s %d °C#\t#646464 |#\t#%s %d °C#",
			LVGL_GREEN, inv1.motor_temp, LVGL_GREEN, inv2.motor_temp);

	//rpm
	int mph = (inv1.motor_speed + inv2.motor_speed) * 0.02975f / 5.0f;
	lv_arc_set_value(rpm_arc, mph);
	lv_label_set_text_fmt(rpm_arc_label, "%d", mph);

	// acceleration

	// traction control status

	//vehicle speed

}

void initialize_display_state_diagnostic(void) {
	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
	lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0); // Ensure it's not transparent
	lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

	diagnostic_label = lv_label_create(lv_scr_act());
	lv_obj_set_size(diagnostic_label, 800, 480);
	lv_obj_center(diagnostic_label);

	static lv_style_t style_label;
	generate_style(&style_label, &lv_font_montserrat_30, false, false);

	lv_obj_add_style(diagnostic_label, &style_label, 0);
}

void clear_display_state_diagnostic(void) {
	lv_obj_del(diagnostic_label);
}

void update_display_state_diagnostic() {
	lv_label_set_text_fmt(diagnostic_label, "VCU Fault: %d", vcu.fault);
}

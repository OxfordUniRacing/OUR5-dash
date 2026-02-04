/*
 * fdcan_handlers.c
 *
 *  Created on: 28/01/2026
 *      Author:
 */
#include "fdcan_handlers.h"
#include <stddef.h>

// If hfdcan1 is declared elsewhere (e.g. in main.c), include its extern or header
extern FDCAN_HandleTypeDef hfdcan1;

void FDCAN1_IT0_IRQHandler(void) {
	HAL_FDCAN_IRQHandler(&hfdcan1);
}

/* Receive new CAN messages */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
	FDCAN_RxHeaderTypeDef rxHeader;
	uint8_t rxData[8];

	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == 0) {
		return;
	}

	HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxHeader, rxData);

	if (rxHeader.IdType == FDCAN_EXTENDED_ID) {
		switch (rxHeader.Identifier & 0x01FFFFFF) {
		case 0x118ff71:
			inv1.output_torque = ((rxData[0]) | (rxData[1] << 8)) / 160.0f;
			inv1.motor_speed = (int16_t) ((rxData[2]) | (rxData[3] << 8));
			inv1.battery_current = (int16_t) ((rxData[4]) | (rxData[5] << 8));
			break;
		case 0x118ff72:
			inv2.output_torque = ((rxData[0]) | (rxData[1] << 8)) / 160.0f;
			inv2.motor_speed = (int16_t) ((rxData[2]) | (rxData[3] << 8));
			inv2.battery_current = (int16_t) ((rxData[4]) | (rxData[5] << 8));
			break;
		case 0x119ff71:
			inv1.available_forward_torque = ((rxData[0]) | (rxData[1] << 8))
					/ 160.0f;
			inv1.available_reverse_torque = ((rxData[2]) | (rxData[3] << 8))
					/ 160.0f;
			inv1.statusword = (statusword_t) rxData[4];
			break;
		case 0x119ff72:
			inv2.available_forward_torque = ((rxData[0]) | (rxData[1] << 8))
					/ 160.0f;
			inv2.available_reverse_torque = ((rxData[2]) | (rxData[3] << 8))
					/ 160.0f;
			inv2.statusword = (statusword_t) rxData[4];
			break;
		case 0x11aff71:
			inv1.capacitor_voltage = ((rxData[4]) | (rxData[5] << 8)) / 16.0f;
			inv1.temperature = INVERTER_CUTOFF_TEMP
					- (int16_t) ((rxData[0]) | (rxData[1] << 8));
			inv1.motor_temp = (int16_t) ((rxData[2]) | (rxData[3] << 8));
			break;
		case 0x11aff72:
			inv2.capacitor_voltage = ((rxData[4]) | (rxData[5] << 8)) / 16.0f;
			inv2.temperature = INVERTER_CUTOFF_TEMP
					- (int16_t) ((rxData[0]) | (rxData[1] << 8));
			inv2.motor_temp = (int16_t) ((rxData[2]) | (rxData[3] << 8));
			break;
		case 0x19107101:
			// placeholder
			break;
		case 0x19117101:
			break;
		default:
			break;
		}
	} else {
		switch (rxHeader.Identifier) {
		case 0x6B1:
			battery.pack_dcl = (uint16_t) (rxData[0] << 8) | rxData[1];
			battery.temperature = rxData[4];
			break;
		case 0x6B0:
			battery.pack_soc = rxData[4] / 2;
			battery.pack_voltage = (((uint16_t) rxData[2] << 8) | rxData[3])
					* 0.1f;
			battery.pack_current = (((uint16_t) rxData[0] << 8) | rxData[1])
					* 0.1f;
			break;
		case 0x7A4:
			vcu.lv_voltage = (rxData[0] + ((uint16_t) rxData[1] << 8)) * 12.58f
					/ 2561.0f;
			vcu.current_limit = rxData[4];
			vcu.rtd_switch_state = (rxData[6] >> 4) & 1;
			vcu.rtd = (rxData[6] >> 3) & 1;
			battery.active = (rxData[6] >> 2) & 1;
			inv1.active = (rxData[6] >> 1) & 1;
			inv2.active = (rxData[6] >> 0) & 1;
			vcu.last_comm_time = time_count;
			vcu.fault = rxData[7];
			break;
		default:
			break;
		}
	}
}


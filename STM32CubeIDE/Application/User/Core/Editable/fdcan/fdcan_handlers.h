/*
 * fdcan_handlers.h
 *
 *  Created on: 28/01/2026
 *      Author:
 */

#ifndef APPLICATION_USER_CORE_EDITABLE_FDCAN_FDCAN_HANDLERS_H_
#define APPLICATION_USER_CORE_EDITABLE_FDCAN_FDCAN_HANDLERS_H_

#ifndef FDCAN_HANDLERS_H_
#define FDCAN_HANDLERS_H_

#include "../dashboard.h"
#include <stdint.h>// for data structures / externs

// IRQ handler (must match startup vector name)
void FDCAN1_IT0_IRQHandler(void);

// HAL callback called by HAL_FDCAN on Rx FIFO0 new message
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan,
		uint32_t RxFifo0ITs);

#endif // FDCAN_HANDLERS_H_

#endif /* APPLICATION_USER_CORE_EDITABLE_FDCAN_FDCAN_HANDLERS_H_ */

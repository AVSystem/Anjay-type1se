/*
 * Copyright ##year## AVSystem <avsystem@avsystem.com>
 * AVSystem Commons library
 * All rights reserved.
 */

#include <avs_commons_init.h>

#include <avsystem/commons/avs_defs.h>
#include <avsystem/commons/avs_init_once.h>

#include <cmsis_compiler.h>
#include <stdbool.h>


enum init_state { INIT_NOT_STARTED, INIT_IN_PROGRESS, INIT_DONE };

int avs_init_once(volatile avs_init_once_handle_t *handle,
                  avs_init_once_func_t *func,
                  void *func_arg) {
    volatile int *state = (volatile int *) handle;

	int expected;

	bool is_expected = false;
	do {
		expected = INIT_NOT_STARTED;

		__disable_irq();
		if (*state == expected) {
			*state = INIT_IN_PROGRESS;
			is_expected = true;
		} else {
			expected = *state;
		}
		__enable_irq();
	} while (!is_expected && expected != INIT_DONE);

	int result = 0;
    if (*state != INIT_DONE) {
        result = func(func_arg);
        if (result) {
            *state = INIT_NOT_STARTED;
        } else {
            *state = INIT_DONE;
        }
    }
    *state = INIT_DONE;
    return result;
}

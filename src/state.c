#include <string.h>

#include "state.h"


void kee_state_zero(kee_state_t *state) {
	memset(state, 0, sizeof(kee_state_t));
}

char kee_state_add(kee_state_t *target, kee_state_t *delta) {
	char hint;

	hint = 0;
	if (delta->ui_menu) {
		target->ui_menu |= delta->ui_menu;
		hint |= KEE_ST_HINT_UI_MENU;
	}
	if (delta->key) {
		target->key |= delta->key;
		hint |= KEE_ST_HINT_KEY;
	}
	return hint;
}

char kee_state_sub(kee_state_t *target, kee_state_t *delta) {
	char hint;

	hint = 0;
	return hint;
}

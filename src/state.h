#ifndef _KEE_STATE_H
#define _KEE_STATE_H

#define KEE_ST_CTRL 0x0100
#define KEE_ST_CTRL_MAN 0x0101
#define KEE_ST_CTRL_MACHINE 0x0102
#define KEE_ST_CTRL_TRANSPORT 0x0104

#define KEE_ST_SCAN 0x0200
#define KEE_ST_SCAN_INIT 0x0201
#define KEE_ST_SCAN_SEARCH 0x0202

#define KEE_ST_HINT_KEY 0x01
#define KEE_ST_HINT_UI_MENU 0x10

#define KEE_IS_SCANNING(c) c->state & KEE_ST_SCAN_SEARCH

typedef struct {
	long ui_menu;
	long key;
} kee_state_t;

void kee_state_zero(kee_state_t *state);
char kee_state_add(kee_state_t *target, kee_state_t *delta);
char kee_state_sub(kee_state_t *target, kee_state_t *delta);

#endif // _KEE_STATE_H

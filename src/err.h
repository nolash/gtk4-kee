#ifndef KEE_ERR_H_
#define KEE_ERR_H_
//
///**
// * 
// * Error codes within context of the kee application and backend.
// *
// */
//
//#define ERR_OK 0x0000
//#define ERR_FAIL 0x0001
//#define ERR_UNSUPPORTED 0x0002
//
//#ifndef RERR_N_PFX 
//#define RERR_N_PFX 0
//#endif
//
//void rerr_register(int pfx, char *label, void *start);
//char* rerrstr(int code, char *buf);
//

void err_init();

#endif // _KEE_ERR_H

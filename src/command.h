#ifndef _KEE_CMD_H
#define _KEE_CMD_H

int process_rpc_command(void *backend, char *buf, size_t buf_len, bool preview);
int preview_command(char *in, size_t in_len, char *out, size_t *out_len);

#endif // _KEE_CMD_H

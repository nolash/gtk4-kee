#ifndef BEAMENU_H_
#define BEAMENU_H_

#define BEAMENU_INACTIVE 0x0
#define BEAMENU_ROOT 0xffffffff
#define BEAMENU_CN_MAXLEN 32

#ifndef BEAMENU_N_DST
#define BEAMENU_N_DST 1
#endif

#ifndef BEAMENU_N_EXITS
#define BEAMENU_N_EXITS 0
#endif

#define BEAMENU_EXIT_SIZE 1


struct beamenu_node {
	int i;
	char *cn;
	int dst[BEAMENU_N_EXITS]; // all are 0
};

int beamenu_register(int idx, char *cn);
void beamenu_free();
void beamenu_set(int idx_node, int idx_exit, int idx_dst);
struct beamenu_node *beamenu_get(int idx_node);
int beamenu_load_file(const char *path, int msize);
int beamenu_export(char *out, int width);
int beamenu_use_exit(int idx_exit);
int beamenu_get_exit(int idx_exit);
int beamenu_jump(int idx);

#endif

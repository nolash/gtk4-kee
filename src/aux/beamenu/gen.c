#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <search.h>
#include <stdlib.h>
#include <stdio.h>

#include "beamenu.h"

#define BUFSIZE 4096
#define KEYMAXLEN BEAMENU_CN_MAXLEN
#define MAXENTRIES 256

#define MODE_READ 0
#define MODE_SEEK 1

/// \todo forces use of char in output, should allow 2 and 4 width too

char buf[BUFSIZE];
char tmpk[KEYMAXLEN];
char *keys[MAXENTRIES];
int vals[MAXENTRIES];
int crsr;
int tmpbi;
int tmpki;
int tmpc = 0;
int tmpl = 0;
int tmpm = 0;
int debug = 0;

int uc(char *b) {
	int i;
	char v;

	i = 0;
	v = 1;
	while(v > 0) {
		v = *(b+i);
		if (v > 0x60 && v < 0x7b) {
			*(b+i) -= 0x20;
		}
		i++;
	}
	return 0;
}

int fill(int f) {
	int c;

	c = read(f, buf, BUFSIZE);
	tmpbi = 0;

	return c;
}

int addkey(char *k, int v) {
	ENTRY o;
	int l;

	l = strlen(k);
	keys[crsr] = malloc(l);
	strcpy(keys[crsr], k);
	o.key = keys[crsr];

	vals[crsr] = v;
	o.data = &vals[crsr];

	if (beamenu_register(tmpc, keys[crsr])) {
		return 1;
	}
	if (!hsearch(o, ENTER)) {
		return 1;
	}

	crsr++;
	return 0;
}

int scan(int f, int l) {
	int r;

	if (tmpm == MODE_READ) {
		while(tmpbi < l) {
			tmpk[tmpki] = buf[tmpbi];
			if (tmpk[tmpki] == ',') {
				tmpk[tmpki] = 0;
				r = addkey((char*)tmpk, tmpc);
				if (r) {
					return -1;
				}
				if (debug) {
					fprintf(stderr, "found key %d: %s\n", tmpc, tmpk);
				}
				tmpm = MODE_SEEK;
				tmpki = 0;
				tmpbi++;
				tmpc++;
				break;
			}
			tmpki++;
			tmpbi++;
		}
	}

	if (tmpm == MODE_SEEK) {
		while(tmpbi < l) {
			if (buf[tmpbi] == 0x0a) {
				tmpm = MODE_READ;
				tmpbi++;
				break;
			}
			tmpbi++;
		}
	}

	if (tmpbi < l) {
		return 0;
	}

	return 1;
}

int set(int c) {
	ENTRY o;
	ENTRY *p;
	int v;
	int *entry_index;

	o.key = (char*)tmpk;
	if (*o.key == 0) {
		beamenu_set(c, tmpc, 0);
		if (debug) {
			fprintf(stderr, "set zero %d %d\n", c, tmpc);
		}
		return 0;
	}
	p = hsearch(o, FIND);
	if (!p) {
		return 1;
	}
	entry_index = (int*)(p->data);
	entry_index += c;
	v = *entry_index;
	beamenu_set(c, tmpc, v);
	if (debug) {
		fprintf(stderr, "set %d %d %d\n", c, tmpc, v);
	}
	return 0;
}

int linkscan(int f, int l, int *c) {
	int r;
	char v;

	if (tmpm == MODE_SEEK) {
		while(tmpbi < l) {
			if (buf[tmpbi] == ',') {
				tmpm = MODE_READ;
				tmpbi++;
				break;
			}
			tmpbi++;
		}
	}

	if (tmpm == MODE_READ) {
		while(tmpbi < l) {
			v = buf[tmpbi];
			if (v == 0x0a) {
				tmpk[tmpki] = 0;
				r = set(*c);
				if (r) {
					return -1;
				}
				tmpk[0] = 0;
				tmpm = MODE_SEEK;
				tmpbi++;
				*c += 1;
				tmpc = 0;
				break;
			} else if (v == ',') {
				tmpk[tmpki] = 0;
				r = set(*c);
				if (r) {
					return -1;
				}
				tmpc++;
				tmpki = 0;
			} else {
				tmpk[tmpki] = v;
				tmpki++;
			}
			tmpbi++;
		}
	}

	if (tmpbi < l) {
		return 0;
	}

	return 1;
}

int write_data() {
	char *buf;
	int c;
	int f;
	int l;
	int r;

	buf = malloc(BEAMENU_N_DST * (BEAMENU_CN_MAXLEN + BEAMENU_N_EXITS + 1));
	if (!buf) {
		return 0;
	}
	
	l = beamenu_export(buf, BEAMENU_EXIT_SIZE);
	f = open("beamenu.dat", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	while (l > 0) {
		c = write(f, buf, l);
		if (!c) {
			close(f);
			free(buf);
			return 0;
		}
		l -= c;
		r += c;
	}

	close(f);
	free(buf);
	return r;
}

int write_defs() {
	char *p;
	struct beamenu_node *o;
	char buf[1024];
	char k[KEYMAXLEN + 1];
	int c;
	int f;
	int l;
	int r;
	int i;

	f = open("beamenu_defs.h", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

	r = 0;
	p = "#ifndef BEAMENU_DEFS_H_\n#define BEAMENU_DEFS_H_\n\n";
	strcpy(buf, p);
	l = strlen(p);
	c = write(f, buf, l);
	if (c != l) {
		close(f);
		return 0;
	}
	r += c;

	for (i = 0; i < BEAMENU_N_DST; i++) {
		o = beamenu_get(i);
		strcpy(k, o->cn);
		r = uc(k);
		sprintf(buf, "#define BEAMENU_DST_%s %d\n", k, i);
		l = strlen(buf);
		c = write(f, buf, l);
		if (c != l) {
			close(f);
			return 0;
		}
		r += c;
	}
	sprintf(buf, "\nextern char *beamenu_dst_r[];\n");
	l = strlen(buf);
	c = write(f, buf, l);
	if (c != l) {
		close(f);
		return 0;
	}
	r += c;

	p = "\n#endif\n";
	strcpy(buf, p);
	l = strlen(p);
	c = write(f, buf, l);
	if (c != l) {
		close(f);
		return 0;
	}
	r += c;

	close(f);
	return r;
}


int write_impl() {
	char *p;
	struct beamenu_node *o;
	char buf[1024];
	char k[KEYMAXLEN + 1];
	int r;
	int c;
	int i;
	int l;
	int f;

	f = open("beamenu_defs.c", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

	r = 0;
	p = "char *beamenu_dst_r[] = {\n";
	strcpy(buf, p);
	l = strlen(p);
	c = write(f, buf, l);
	if (c != l) {
		close(f);
		return 0;
	}
	r += c;
	p += c;

	for (i = 0; i < BEAMENU_N_DST; i++) {
		o = beamenu_get(i);
		strcpy(k, o->cn);
		sprintf(buf, "\t\"%s\",\n", k);
		l = strlen(buf);
		c = write(f, buf, l);
		if (c != l) {
			close(f);
			return 0;
		}
		r += c;
	}

	p = "};\n";
	strcpy(buf, p);
	l = strlen(p);
	c = write(f, buf, l);
	if (c != l) {
		close(f);
		return 0;
	}
	r += c;

	close(f);
	return r;
}

int main(int argc, char **argv) {
	int i;
	int r;
	int f;
	int l;
	int c;

	crsr = 0;
	tmpbi = 0;
	tmpki = 0;
	tmpc = 0;
	tmpm = MODE_READ;

	if (getenv("BEAMENU_DEBUG")) {
		debug = 1;
	}

	r = hcreate(BEAMENU_N_DST);
	if (!r) {
		return 1;
	}

	f = open(*(argv+1), O_RDONLY);
	l = fill(f);
	if (l == 0) {
		return 1;
	}
	if (debug) {
		fprintf(stderr, "initial read is %d\n", l);
	}

	while (1) {
		r = scan(f, l);
		if (r < 0) {
			return 1;
		} else if (r) {
			l = fill(f);
			if (l == 0) {
				break;
			}
		}
	}

	tmpbi = 0;
	tmpki = 0;
	tmpm = MODE_SEEK;
	r = lseek(f, 0, SEEK_SET);
	if (r < 0) {
		return 1;
	}
	l = fill(f);
	if (l == 0) {
		return 1;
	}
	i = 0;
	tmpki = 0;
	c = tmpc;
	tmpc = 0;
	while (i < c) {
		r = linkscan(f, l, &i);
		if (r < 0) {
			return 1;
		} else if (r) {
			l = fill(f);
			if (l == 0) {
				break;
			}
		}
	}

	close(f);
	hdestroy();

	r = write_data();
	if (!r) {
		return 1;
	}

	r = write_defs();
	if (!r) {
		return 1;
	}

	r = write_impl();
	if (!r) {
		return 1;
	}

	return 0;
}

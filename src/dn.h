#ifndef KEE_DN_H_
#define KEE_DN_H_

#ifndef KEE_DN_DEFAULT_CAP
#define KEE_DN_DEFAULT_CAP 1024
#endif

struct kee_dn_t {
	char *mem;
	char *p;
	char *cn;
	char *c;
	char *o;
	char *uid;
	char *dc;
};

struct kee_dn_t* kee_dn_init(struct kee_dn_t *dn, size_t cap);
int kee_dn_from_str(struct kee_dn_t *dn, const char *s, size_t l);
void kee_dn_free(struct kee_dn_t *dn);

#endif


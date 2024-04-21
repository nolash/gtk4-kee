#include <stdlib.h>
#include <string.h>

#include <ldap.h>

#include "err.h"
#include "dn.h"

struct kee_dn_t* kee_dn_init(struct kee_dn_t *dn, size_t cap) {
	if (cap == 0) {
		cap = KEE_DN_DEFAULT_CAP;
	}
	dn->mem = malloc(cap);
	dn->p = (char*)dn->mem;
	dn->cn = NULL;
	return dn;
}

int kee_dn_from_str(struct kee_dn_t *dn, const char *s, size_t l) {
	int r;
	int i;
	LDAPDN ldn;
	LDAPRDN lrdn;
	LDAPAVA *ldnav;
	char tmp[1024];
	char *dst;

	memcpy(tmp, s, l);
	*(tmp+l) = 0;
	r = ldap_str2dn(tmp, &ldn, LDAP_DN_FORMAT_LDAPV3);
	if (r) {
		return ERR_FAIL;
	}

	i = 0;
	while(1) {
		lrdn = *(ldn+i);
		if (lrdn == NULL) {
			break;	
		}
		ldnav = *lrdn;
	
		memcpy(tmp, ldnav->la_attr.bv_val, ldnav->la_attr.bv_len);
		tmp[ldnav->la_attr.bv_len] = 0;
		if (!strcmp(tmp, "CN")) {
			dn->cn = dn->p;
			dst = dn->cn;
		} else if (!strcmp(tmp, "O")) {
			dn->o = dn->p;
			dst = dn->o;
		} else {
			return 1;
		}
		memcpy(dst, ldnav->la_value.bv_val, ldnav->la_value.bv_len);
		*(dst+ldnav->la_value.bv_len) = 0;
		dn->p += ldnav->la_value.bv_len + 1;
		i++;
	}
	if (dn->cn == NULL) {
		return 1;
	}

	ldap_dnfree(ldn);

	return 0;
}

void kee_dn_free(struct kee_dn_t *dn) {
	free(dn->mem);
}

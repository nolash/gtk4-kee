#include <stddef.h>
#include <libtasn1.h>
#include <gcrypt.h>

//#include "cmime.h"

#include "ledger.h"
#include "cadiz.h"
#include "err.h"
#include "debug.h"
#include "digest.h"
#include "strip.h"


extern const asn1_static_node schema_entry_asn1_tab[];

int kee_ledger_resolve(struct kee_ledger_t *ledger, struct Cadiz *resolver) {
		return ERR_OK;
}	
//
//	if (o->resolver) {
//		r = cadiz_resolve(o->resolver, o->body, o->body, &out_len);
//		if (!r) {
//			msg = cmime_message_new();
//			o->subject = o->body + out_len;
//			r = cmime_message_from_string(&msg, o->body, 0);
//			if (!r) {
//				o->subject = cmime_message_get_subject(msg);
//				o->subject = cmime_string_strip(o->subject);
//				out_len += strlen(o->subject) + 1;
//			}
//			remaining -= out_len;
//		} else {
//			remaining -= c;
//		}
//	}


static char *get_message(asn1_node item, char *out_digest, char *out_data, size_t *out_len) {
	int r;
	size_t c;
	asn1_node root;
	char err[1024];
	char buf[64];
	char sig[64];

	memset(&root, 0, sizeof(root));
	r = asn1_array2tree(schema_entry_asn1_tab, &root, err);
	if (r != ASN1_SUCCESS) {
		debug_log(DEBUG_ERROR, err);
		return NULL;
	}

	/// \todo there must be a better way to do this
	r = asn1_copy_node(root, "Kee.KeeEntry.parent", item, "parent");
	if (r != ASN1_SUCCESS) {
		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
		return NULL;
	}

	r = asn1_copy_node(root, "Kee.KeeEntry.signatureRequest", item, "signatureRequest");
	if (r != ASN1_SUCCESS) {
		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
		return NULL;
	}
	r = asn1_copy_node(root, "Kee.KeeEntry.response", item, "response");
	if (r != ASN1_SUCCESS) {
		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
		return NULL;
	}
//
//	r = asn1_copy_node(root, "Kee.KeeEntry.signatureResponse", item, "signatureResponse");
//	if (r != ASN1_SUCCESS) {
//		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
//		return NULL;
//	}

	r = asn1_copy_node(root, "Kee.KeeEntry.timestamp", item, "timestamp");
	if (r != ASN1_SUCCESS) {
		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
		return NULL;
	}

	r = asn1_copy_node(root, "Kee.KeeEntry.creditDelta", item, "creditDelta");
	if (r != ASN1_SUCCESS) {
		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
		return NULL;
	}

	r = asn1_copy_node(root, "Kee.KeeEntry.collateralDelta", item, "collateralDelta");
	if (r != ASN1_SUCCESS) {
		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
		return NULL;
	}

	r = asn1_copy_node(root, "Kee.KeeEntry.body", item, "body");
	if (r != ASN1_SUCCESS) {
		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
		return NULL;
	}

	c = 64;
	r = asn1_read_value(item, "signatureResponse", sig, (int*)&c);
	if (r != ASN1_SUCCESS) {
		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
		return NULL;
	}

	buf[0] = 0;
	c = 0;
	r = asn1_write_value(root, "Kee.KeeEntry.signatureResponse", buf, c);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}

	r = asn1_der_coding(root, "Kee.KeeEntry", out_data, (int*)out_len, err);
	if (r != ASN1_SUCCESS) {
		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
		return NULL;
	}

	r = calculate_digest_algo(out_data, *out_len, out_digest, GCRY_MD_SHA512);
	if (r) {
		return NULL;
	}

	return out_digest;
}


static int verify_item(asn1_node item, const char *pubkey_first_data, const char *pubkey_last_data) {
	int r;
	gcry_sexp_t sig;
	gcry_sexp_t msg;
	gcry_sexp_t pubkey;
	gcry_error_t err;
	size_t c;
	gcry_mpi_t sr;
	gcry_mpi_t ss;
	char sig_data[64];
	char msg_data[1024 + 64];
	char *p = (char*)msg_data;
	char pubkey_sexp_data[1024];
	size_t pubkey_sexp_len;

	c = 1024;
	p = get_message(item, p, p+64, &c);
	if (p == NULL) {
		return 1;
	}

	c = 64;
	r = asn1_read_value(item, "signatureResponse", sig_data, (int*)&c);
	if (r != ASN1_SUCCESS) {
		return 1;
	}

	c = 0;
	err = gcry_mpi_scan(&sr, GCRYMPI_FMT_STD, sig_data, 32, &c);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}
	if (c != 32) {
		return 1;
	}

	c = 0;
	err = gcry_mpi_scan(&ss, GCRYMPI_FMT_STD, sig_data+32, 32, &c);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}
	if (c != 32) {
		return 1;
	}

	c = 0;
	err = gcry_sexp_build(&sig, &c, "(sig-val(eddsa(r %m)(s %m)))", sr, ss);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}

	c = 0;
	err = gcry_sexp_build(&msg, &c, "(data(flags eddsa)(hash-algo sha512)(value %b))", 64, p);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}

/// \todo "string too long" error when build same string as can "new" - bug in gcrypt?
//	c = 0;
//	err = gcry_sexp_build(&pubkey, &c, "(8:key-data(10:public-key(3:ecc(5:curve7:Ed25519)(1:q32:%b))))", 32, pubkey_last_data);
//	if (err != GPG_ERR_NO_ERROR) {
//		return 1;
//	}

	strcpy(pubkey_sexp_data, "(8:key-data(10:public-key(3:ecc(5:curve7:Ed25519)(1:q32:");
	c = strlen(pubkey_sexp_data);
	memcpy(pubkey_sexp_data + c, pubkey_last_data, 32);
	strcpy(pubkey_sexp_data + c + 32, "))))");

	pubkey_sexp_len = c + 32 + 4;
	c = 0;
	err = gcry_sexp_new(&pubkey, pubkey_sexp_data, pubkey_sexp_len, 1);
	if (err != GPG_ERR_NO_ERROR) {
		return 1;
	}

	err = gcry_pk_verify(sig, msg, pubkey);
	if (err != GPG_ERR_NO_ERROR) {
		fprintf(stderr, "verify fail: %s\n", gcry_strerror(err));
		return 1;
	}

	return 0;
}

struct kee_ledger_item_t *kee_ledger_parse_item(struct kee_ledger_t *ledger, const char *data, size_t data_len) {
	int r;
	int c;
	char err[1024];
	asn1_node root;
	asn1_node item;
	struct kee_ledger_item_t *prev;
	struct kee_ledger_item_t *cur;
	int *credit_delta;
	int *collateral_delta;
	const char *pubkey_first;
	const char *pubkey_last;
	int v;


	prev = ledger->last_item;
	ledger->last_item = calloc(sizeof(struct kee_ledger_item_t), 1);
	cur = ledger->last_item;
	cur->prev_item = prev;

	memset(&root, 0, sizeof(root));
	memset(&item, 0, sizeof(item));
	r = asn1_array2tree(schema_entry_asn1_tab, &root, err);
	if (r != ASN1_SUCCESS) {
		debug_log(DEBUG_ERROR, err);
		return NULL;
	}

	r = asn1_create_element(root, "Kee.KeeEntry", &item);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return NULL;
	}

	c = (int)data_len - 1;
	if (*(data+c)) {
		cur->initiator = BOB;
		credit_delta = &cur->bob_credit_delta;
		collateral_delta = &cur->bob_collateral_delta;
		pubkey_first = (const char*)ledger->pubkey_bob; // alice countersigns bobs
		pubkey_last = (const char*)ledger->pubkey_alice; // alice countersigns bobs
	} else {
		credit_delta = &cur->alice_credit_delta;
		collateral_delta = &cur->alice_collateral_delta;
		pubkey_first = (const char*)ledger->pubkey_alice;
		pubkey_last = (const char*)ledger->pubkey_bob;
	}

	r = asn1_der_decoding(&item, data, c, err);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return NULL;
	}

	r = verify_item(item, pubkey_first, pubkey_last);
	if (r) {
		return NULL;
	}

	c = sizeof(v);
	r = asn1_read_value(item, "creditDelta", &v, &c);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return NULL;
	}

	strap_be((char*)&v, c, (char*)credit_delta, sizeof(v));
	if (is_le()) {
		flip_endian(sizeof(v), (void*)credit_delta);
	}

	c = sizeof(v);
	r = asn1_read_value(item, "collateralDelta", &v, &c);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return NULL;
	}

	strap_be((char*)&v, c, (char*)collateral_delta, sizeof(v));
	if (is_le()) {
		flip_endian(sizeof(v), (void*)collateral_delta);
	}

	return cur;
}

void kee_ledger_item_free(struct kee_ledger_item_t *item) {
	if (item->prev_item != NULL) {
		kee_ledger_item_free(item->prev_item);
	}
	free(item);
}

void kee_ledger_free(struct kee_ledger_t *ledger) {
	kee_ledger_item_free(ledger->last_item);
}

int kee_ledger_parse(struct kee_ledger_t *ledger, const char *data, size_t data_len) {
	int r;
	char err[1024];
	asn1_node root;
	asn1_node item;
	int c;
	//CMimeMessage_T *msg;

	memset(ledger, 0, sizeof(struct kee_ledger_t));
	memset(&root, 0, sizeof(root));
	memset(&item, 0, sizeof(item));
	r = asn1_array2tree(schema_entry_asn1_tab, &root, err);
	if (r != ASN1_SUCCESS) {
		debug_log(DEBUG_ERROR, err);
		return ERR_FAIL;
	}

	r = asn1_create_element(root, "Kee.KeeEntryHead", &item);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}
	
	c = (int)data_len;
	r = asn1_der_decoding(&item, data, c, err);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}

//	r = calculate_digest_algo(data, data_len, o->current_id, GCRY_MD_SHA512);	
//	if (r) {
//		return ERR_DIGESTFAIL;
//	}

	c = 64;
	r = asn1_read_value(item, "uoa", ledger->uoa, &c);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}
	
	c = 1;
	r = asn1_read_value(item, "uoaDecimals", &ledger->uoa_decimals, &c);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}

	c = 32;
	r = asn1_read_value(item, "alicePubKey", ledger->pubkey_alice, &c);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}

	c = 32;
	r = asn1_read_value(item, "bobPubKey", ledger->pubkey_bob, &c);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}
	
	c = 4096 - 64;
	r = asn1_read_value(item, "body", ledger->body, &c);
	if (r != ASN1_SUCCESS) {
		fprintf(stderr, "%s\n", err);
		return r;
	}

	return ERR_OK;
}

//
//int kee_ledger_parse_item(kee_ledger_item_t *item, const char *data, size_t data_len) {
//	int r;
//	char err[1024];
//	asn1_node root;
//	asn1_node item;
//	int alice;
//	int bob;
//	int credit;
//	int collateral;
//	int c;
//	char flag;
//	int v;
//	char *p;
//
//	memset(&root, 0, sizeof(root));
//	memset(&item, 0, sizeof(item));
//	r = asn1_array2tree(schema_entry_asn1_tab, &root, err);
//	if (r != ASN1_SUCCESS) {
//		debug_log(DEBUG_ERROR, err);
//		return ERR_FAIL;
//	}
//
//	r = asn1_create_element(root, "Kee.KeeEntry", &item);
//	if (r != ASN1_SUCCESS) {
//		fprintf(stderr, "%s\n", err);
//		return r;
//	}
//	
//	c = (int)data_len - 1;
//	r = asn1_der_decoding(&item, data, c, err);
//	if (r != ASN1_SUCCESS) {
//		fprintf(stderr, "%s\n", err);
//		return r;
//	}
//	flag = *(data+data_len-1);
//	
////	c = 1;
////	flags = 0;
////	r = asn1_read_value(item, "flags", &flags, &c);
////	if (r != ASN1_SUCCESS) {
////		fprintf(stderr, "%s\n", err);
////		return r;
////	}
//
//	credit = 0;
//	p = (char*)&v;
//	c = sizeof(v);
//	v = 0;
//	r = asn1_read_value(item, "creditDelta", p, &c);
//	if (r != ASN1_SUCCESS) {
//		fprintf(stderr, "%s\n", err);
//		return r;
//	}
//
//	strap_be(p, c, (char*)&credit, sizeof(credit));
//	if (is_le()) {
//		flip_endian(sizeof(credit), (void*)&credit);
//	}
//
//	collateral = 0;
//	c = sizeof(v);
//	v = 0;
//	r = asn1_read_value(item, "collateralDelta", p, &c);
//	if (r != ASN1_SUCCESS) {
//		fprintf(stderr, "%s\n", err);
//		return r;
//	}
//
//	strap_be(p, c, (char*)&collateral, sizeof(collateral));
//	if (is_le()) {
//		flip_endian(sizeof(collateral), (void*)&collateral);
//	}
//
//
//	alice = 0;
//	bob = 0;
//	if (flag) { // bit string is left to right
//		bob = credit;	
//	} else {
//		alice = credit;	
//	}
//
//	return ERR_OK;	
//}

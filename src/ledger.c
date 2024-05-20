#include <stddef.h>
#include <string.h>
#include <libtasn1.h>
#include <gcrypt.h>

#include <rerr.h>

#include "ledger.h"
#include "cadiz.h"
#include "debug.h"
#include "digest.h"
#include "strip.h"
#include "content.h"
#include "endian.h"
#include "gpg.h"
#include "wire.h"


char zero_content[64];

/// \todo consolidate with get_message_data
static char *get_message_asn(struct kee_ledger_t *ledger, asn1_node item, char *out_digest, char *out_data, size_t *out_len, enum kee_ledger_state_e mode) {
	int r;
	size_t c;
	asn1_node root;
	char err[1024];
	char buf[64];

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

	r = asn1_copy_node(root, "Kee.KeeEntry.response", item, "response");
	if (r != ASN1_SUCCESS) {
		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
		return NULL;
	}


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

	if (mode == KEE_LEDGER_STATE_FINAL) {
		r = asn1_copy_node(root, "Kee.KeeEntry.signatureResponse", item, "signatureResponse");
		if (r != ASN1_SUCCESS) {
			printf("%d (%s) %s\n", r, err, asn1_strerror(r));
			return NULL;
		}
	} else {
		buf[0] = 0;
		c = 0;
		r = asn1_write_value(root, "Kee.KeeEntry.signatureResponse", buf, c);
		if (r != ASN1_SUCCESS) {
			return NULL;
		}
	}

	if (mode > KEE_LEDGER_STATE_REQUEST) {
		r = asn1_copy_node(root, "Kee.KeeEntry.signatureRequest", item, "signatureRequest");
		if (r != ASN1_SUCCESS) {
			printf("%d (%s) %s\n", r, err, asn1_strerror(r));
			return NULL;
		}
	} else {
		buf[0] = 0;
		c = 0;
		r = asn1_write_value(root, "Kee.KeeEntry.signatureRequest", buf, c);
		if (r != ASN1_SUCCESS) {
			return NULL;
		}
	}

	r = asn1_der_coding(root, "Kee.KeeEntry", out_data+64, (int*)out_len, err);
	if (r != ASN1_SUCCESS) {
		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
		return NULL;
	}

	memcpy(out_data, ledger->digest, 64);
	*out_len += DIGEST_LENGTH;

	return out_data;

//	r = calculate_digest_algo(out_data, *out_len, out_digest, GCRY_MD_SHA512);
//	if (r) {
//		return NULL;
//	}
//
//	return out_digest;
}
//
//static char *get_message_data(struct kee_ledger_t *ledger, const char *item_data, size_t item_data_len, char *out_digest) {
//	char *p;
//	char msg_data[1024 + 64];
//	size_t c;
//
//	p = (char*)msg_data;
//	memcpy(p, ledger->digest, DIGEST_LENGTH);
//	p += DIGEST_LENGTH;
//	memcpy(p, item_data, item_data_len);
//	c = DIGEST_LENGTH + item_data_len;
//	r = calculate_digest_algo(msg_data, &c, out_digest, GCRY_MD_SHA512);
//	if (r) {
//		return NULL;
//	}
//	
//	return out_digest;
//}


/// \todo consolidate with verify_item_asn
static int verify_item_data(struct kee_ledger_t *ledger, const char* item_data, size_t item_data_len, const char *sig_data, const char *pubkey_data) {
	char b[DIGEST_LENGTH];
	int r;

	if (item_data_len) {
		r = calculate_digest_algo(item_data, item_data_len, b, GCRY_MD_SHA512);
		if (r) {
			return 1;
		}

		r = gpg_store_verify(sig_data, b, pubkey_data);
		if (r) {
			return 1;
		}
		debug_log(DEBUG_DEBUG, "ledger item verified");
	}

//	c = 0;
//	err = gcry_mpi_scan(&sr, GCRYMPI_FMT_STD, sig_data, 32, &c);
//	if (err != GPG_ERR_NO_ERROR) {
//		return 1;
//	}
//	if (c != 32) {
//		return 1;
//	}
//
//	c = 0;
//	err = gcry_mpi_scan(&ss, GCRYMPI_FMT_STD, sig_data+32, 32, &c);
//	if (err != GPG_ERR_NO_ERROR) {
//		return 1;
//	}
//	if (c != 32) {
//		return 1;
//	}

//	c = 0;
//	err = gcry_sexp_build(&sig, &c, "(sig-val(eddsa(r %m)(s %m)))", sr, ss);
//	if (err != GPG_ERR_NO_ERROR) {
//		return 1;
//	}
//
//	c = 0;
//	err = gcry_sexp_build(&msg, &c, "(data(flags eddsa)(hash-algo sha512)(value %b))", 64, p);
//	if (err != GPG_ERR_NO_ERROR) {
//		return 1;
//	}

/// \todo "string too long" error when build same string as can "new" - bug in gcrypt?
//	c = 0;
//	err = gcry_sexp_build(&pubkey, &c, "(8:key-data(10:public-key(3:ecc(5:curve7:Ed25519)(1:q32:%b))))", 32, pubkey_last_data);
//	if (err != GPG_ERR_NO_ERROR) {
//		return 1;
//	}

//	strcpy(pubkey_sexp_data, "(8:key-data(10:public-key(3:ecc(5:curve7:Ed25519)(1:q32:");
//	c = strlen(pubkey_sexp_data);
//	memcpy(pubkey_sexp_data + c, pubkey_last_data, 32);
//	strcpy(pubkey_sexp_data + c + 32, "))))");
//
//	pubkey_sexp_len = c + 32 + 4;
//	c = 0;
//	err = gcry_sexp_new(&pubkey, pubkey_sexp_data, pubkey_sexp_len, 1);
//	if (err != GPG_ERR_NO_ERROR) {
//		return 1;
//	}

//	err = gcry_pk_verify(sig, msg, pubkey);
//	if (err != GPG_ERR_NO_ERROR) {
//		return 1;
//	}

	return 0;
}


static int verify_item_asn(struct kee_ledger_t *ledger, asn1_node item, const char *pubkey_first_data, const char *pubkey_last_data) {
	char sig_data[64];
	char msg_data[1024 + 64];
	int r;
	size_t c;
	char *p;

	p = (char*)msg_data;
	c = 64;
	r = asn1_read_value(item, "signatureResponse", sig_data, (int*)&c);
	if (r != ASN1_SUCCESS) {
		return 1;
	}

	if (c) {
		c = 1024;
		p = get_message_asn(ledger, item, p, p+64, &c, KEE_LEDGER_STATE_RESPONSE);
		if (p == NULL) {
			return 1;
		}

		r = verify_item_data(ledger, p, c, sig_data, pubkey_last_data);
		if (r) {
			return 1;
		}
	}

	return 0;
}

static int kee_ledger_digest(struct kee_ledger_t *ledger, char *out) {
	int r;
	char out_data[4096];
	size_t c;

	c = 4096;
	r = kee_ledger_serialize(ledger, out_data, &c);
	if (r) {
		return r;
	}

	r = calculate_digest_algo(out_data, c, out, GCRY_MD_SHA512);
	if (r) {
		return ERR_FAIL;
	}

	return ERR_OK;
}

//static int kee_ledger_item_digest(struct kee_ledger_t *ledger, struct kee_ledger_item_t *item, enum kee_ledger_state_e mode, char *out) {
//	char *p;
//	int r;
//	char b[1024];
//	size_t c;
//
//	p = (char*)b;
////	r = kee_ledger_digest(ledger, p);
////	if (r) {
////		return ERR_FAIL;
////	}
//	memcpy(p, ledger->digest, DIGEST_LENGTH);
//	p += DIGEST_LENGTH;
//
//	c = 1024;
//	r = kee_ledger_item_serialize(item, p, &c, mode);
//	if (r) {
//		return ERR_FAIL;
//	}
//
//	r = calculate_digest_algo(b, c + DIGEST_LENGTH, out, GCRY_MD_SHA512);
//	if (r) {
//		return ERR_FAIL;
//	}
//
//	return ERR_OK;
//}
//


void kee_ledger_item_apply_cache(struct kee_ledger_t *ledger, struct kee_ledger_item_t *item) {
	if (ledger->cache == NULL) {
		return;
	}

	ledger->cache->alice_credit_balance += item->alice_credit_delta;
	ledger->cache->alice_collateral_balance += item->alice_collateral_delta;
	ledger->cache->bob_credit_balance += item->bob_credit_delta;
	ledger->cache->bob_collateral_balance += item->bob_collateral_delta;

	ledger->cache->count++;
}

void kee_ledger_reset_cache(struct kee_ledger_t *ledger) {
	if (ledger->cache == NULL) {
		ledger->cache = calloc(sizeof(struct kee_ledger_cache_t), 1);
	} else {
		memset(ledger->cache, 0, sizeof(struct kee_ledger_cache_t));
	}
}

struct kee_ledger_item_t *kee_ledger_add_item(struct kee_ledger_t *ledger) {
	struct kee_ledger_item_t *prev;

	prev = ledger->last_item;
	ledger->last_item = calloc(sizeof(struct kee_ledger_item_t), 1);
	kee_ledger_item_init(ledger->last_item);
	ledger->last_item->prev_item = prev;
	
	return ledger->last_item;
}

struct kee_ledger_item_t *kee_ledger_parse_item_db(struct kee_ledger_t *ledger, const char *data, size_t data_len) {
	size_t c;

	enum kee_initiator_e initiator;

	if (*(data+(data_len-1))) {
		initiator = BOB;
	} else {
		initiator = ALICE;
	}
	c = data_len - 1;
	return kee_ledger_parse_item(ledger, data, c, initiator);
}

struct kee_ledger_item_t *kee_ledger_parse_item(struct kee_ledger_t *ledger, const char *data, size_t data_len, enum kee_initiator_e initiator) {
	int r;
	int c;
	char err[1024];
	asn1_node root;
	asn1_node item;
	struct kee_ledger_item_t *cur;
	int *credit_delta;
	int *collateral_delta;
	const char *pubkey_first;
	const char *pubkey_last;
	char *signature_request;
	char *signature_response;
	char tmp[64];
	int v;

	memset(&root, 0, sizeof(root));
	memset(&item, 0, sizeof(item));
	r = asn1_array2tree(schema_entry_asn1_tab, &root, err);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}

	r = asn1_create_element(root, "Kee.KeeEntry", &item);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}

	cur = kee_ledger_add_item(ledger);
	if (initiator == BOB) {
		cur->initiator = BOB;
		credit_delta = &cur->bob_credit_delta;
		collateral_delta = &cur->bob_collateral_delta;
		pubkey_first = (const char*)ledger->pubkey_bob;
		pubkey_last = (const char*)ledger->pubkey_alice; // alice countersigns bobs
		signature_request = cur->bob_signature;
		signature_response = cur->alice_signature;
	} else {
		cur->initiator = ALICE;
		credit_delta = &cur->alice_credit_delta;
		collateral_delta = &cur->alice_collateral_delta;
		pubkey_first = (const char*)ledger->pubkey_alice;
		pubkey_last = (const char*)ledger->pubkey_bob;
		signature_request = cur->alice_signature;
		signature_response = cur->bob_signature;
	}

	r = asn1_der_decoding(&item, data, data_len, err);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}

	r = verify_item_asn(ledger, item, pubkey_first, pubkey_last);
	if (r) {
		return NULL;
	}

	c = sizeof(v);
	r = asn1_read_value(item, "creditDelta", &v, &c);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}

	strap_be((char*)&v, c, (char*)credit_delta, sizeof(v));
	if (is_le()) {
		flip_endian(sizeof(v), (void*)credit_delta);
	}

	c = sizeof(v);
	r = asn1_read_value(item, "collateralDelta", &v, &c);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}

	strap_be((char*)&v, c, (char*)collateral_delta, sizeof(v));
	if (is_le()) {
		flip_endian(sizeof(v), (void*)collateral_delta);
	}

	c = 8;
	r = asn1_read_value(item, "response", tmp, &c);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}
	if (tmp[0] == 'T') { // "TRUE"
		cur->response = 1;	
	}

	/// \todo document timestamp size
	c = 8;
	r = asn1_read_value(item, "timestamp", tmp, &c);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}
	if (is_le()) {
		flip_endian(4, (char*)tmp);
		flip_endian(4, ((char*)tmp)+4);
	}
	memcpy(&cur->time.tv_sec, tmp, 4);
	memcpy(&cur->time.tv_nsec, ((char*)tmp)+4, 4);

	c = 64;
	r = asn1_read_value(item, "body", tmp, &c);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}


	r = kee_content_init(&(cur->content), tmp, 0);
	if (r) {
		return NULL;
	}

	c = SIGNATURE_LENGTH;
	r = asn1_read_value(item, "signatureRequest", tmp, &c);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}
	if (c > 0) {
		memcpy(signature_request, tmp, c);
	}

	c = SIGNATURE_LENGTH;
	r = asn1_read_value(item, "signatureResponse", tmp, &c);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}
	if (c > 0) {
		memcpy(signature_response, tmp, c);
	}

	kee_ledger_item_apply_cache(ledger, cur);

	return cur;
}

void kee_ledger_item_free(struct kee_ledger_item_t *item) {
	if (item == NULL) {
		return;
	}
	if (item->prev_item != NULL) {
		kee_ledger_item_free(item->prev_item);
	}
	free(item);
}

void kee_ledger_free(struct kee_ledger_t *ledger) {
	if (ledger->cache) {
		free(ledger->cache);
	}
	kee_ledger_item_free(ledger->last_item);
}

void kee_ledger_init(struct kee_ledger_t *ledger) {
	memset(ledger, 0, sizeof(struct kee_ledger_t));
}

void kee_ledger_item_init(struct kee_ledger_item_t *item) {
	memset(item, 0, sizeof(struct kee_ledger_item_t));
}

int kee_ledger_parse(struct kee_ledger_t *ledger, const char *data, size_t data_len) {
	int r;
	char err[1024];
	asn1_node root;
	asn1_node item;
	int c;
	char content_key[64];
	//char decimals;

	memset(&root, 0, sizeof(root));
	memset(&item, 0, sizeof(item));
	r = asn1_array2tree(schema_entry_asn1_tab, &root, err);
	if (r != ASN1_SUCCESS) {
		return ERR_FAIL;
	}

	r = asn1_create_element(root, "Kee.KeeEntryHead", &item);
	if (r != ASN1_SUCCESS) {
		return r;
	}
	
	c = (int)data_len;
	r = asn1_der_decoding(&item, data, c, err);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = 64;
	r = asn1_read_value(item, "uoa", ledger->uoa, &c);
	if (r != ASN1_SUCCESS) {
		return r;
	}
	
	c = 1;
	r = asn1_read_value(item, "uoaDecimals", &ledger->uoa_decimals, &c);
	if (r != ASN1_SUCCESS) {
		return r;
	}
	//ledger->uoa_decimals = (char)decimals;

	c = 32;
	r = asn1_read_value(item, "alicePubKey", ledger->pubkey_alice, &c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = 32;
	r = asn1_read_value(item, "bobPubKey", ledger->pubkey_bob, &c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = 64;
	r = asn1_read_value(item, "body", content_key, &c);
	if (r != ASN1_SUCCESS) {
		return r;
	}
	r = kee_content_init(&(ledger->content), content_key, 0);
	if (r) {
		return 1;
	}

	r = kee_ledger_digest(ledger, (char*)ledger->digest);
	//r = calculate_digest_algo(data, data_len, (char*)ledger->digest, GCRY_MD_SHA512);
	if (r) {
		return 1;
	}

	return ERR_OK;
}

void kee_ledger_resolve(struct kee_ledger_t *ledger, Cadiz *cadiz) {
	struct kee_ledger_item_t *item;

	kee_content_resolve(&ledger->content, cadiz);
	item = ledger->last_item;
	while (item != NULL) {
		kee_content_resolve(&item->content, cadiz);
		item = item->prev_item;
	}
}

int kee_ledger_serialize(struct kee_ledger_t *ledger, char *out, size_t *out_len) {
	int r;
	char err[1024];
	asn1_node node;
	int c;

	//memset(&root, 0, sizeof(root));
	memset(&node, 0, sizeof(node));
	//r = asn1_array2tree(schema_entry_asn1_tab, &root, err);
	r = asn1_array2tree(schema_entry_asn1_tab, &node, err);
	if (r != ASN1_SUCCESS) {
		return ERR_FAIL;
	}

	//c = strlen(ledger->uoa) + 1;
	c = strlen(ledger->uoa);
	r = asn1_write_value(node, "Kee.KeeEntryHead.uoa", ledger->uoa, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = 1;
	r = asn1_write_value(node, "Kee.KeeEntryHead.uoaDecimals", &ledger->uoa_decimals, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = PUBKEY_LENGTH;
	r = asn1_write_value(node, "Kee.KeeEntryHead.alicePubKey", ledger->pubkey_alice, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = PUBKEY_LENGTH;
	r = asn1_write_value(node, "Kee.KeeEntryHead.bobPubKey", ledger->pubkey_bob, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = DIGEST_LENGTH;
	r = asn1_write_value(node, "Kee.KeeEntryHead.body", ledger->content.key, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	r = asn1_der_coding(node, "Kee.KeeEntryHead", out, (int*)out_len, err);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	return 0;
}

int kee_ledger_item_serialize(struct kee_ledger_item_t *item, char *out, size_t *out_len, enum kee_ledger_state_e mode) {
	int r;
	char err[1024];
	asn1_node node;
	char timedata[8];
//	long long nanotime;
	int c;
	int credit_delta;
	int collateral_delta;
	char *signature_request;
	char *signature_response;
	char *response_s;

	//memset(&root, 0, sizeof(root));
	memset(&node, 0, sizeof(node));
	//r = asn1_array2tree(schema_entry_asn1_tab, &root, err);
	r = asn1_array2tree(schema_entry_asn1_tab, &node, err);
	if (r != ASN1_SUCCESS) {
		return ERR_FAIL;
	}

	c = 64;
	if (item->prev_item == NULL) {
		r = asn1_write_value(node, "Kee.KeeEntry.parent", zero_content, c);
	} else {
		r = asn1_write_value(node, "Kee.KeeEntry.parent", item->prev_item, c);
	}
	if (r != ASN1_SUCCESS) {
		return r;
	}

	//nanotime = item->time.tv_sec * 1000000000;
	//nanotime += item->time.tv_nsec;
	//r = to_endian(TO_ENDIAN_BIG, 8, &nanotime);
	//if (r) {
	//	return 1;
	//}
	memcpy(timedata, &item->time.tv_sec, 4);
	memcpy(((char*)timedata)+4, &item->time.tv_nsec, 4);
	r = to_endian(TO_ENDIAN_BIG, 4, timedata);
	if (r) {
		return ERR_FAIL;
	}
	r = to_endian(TO_ENDIAN_BIG, 4, ((char*)timedata)+4);
	if (r) {
		return ERR_FAIL;
	}

	c = 8;
	//r = asn1_write_value(node, "Kee.KeeEntry.timestamp", &nanotime, c);
	r = asn1_write_value(node, "Kee.KeeEntry.timestamp", timedata, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	if (item->initiator == BOB) {
		credit_delta = item->bob_credit_delta;
		collateral_delta = item->bob_collateral_delta;
		signature_request = item->bob_signature;
		signature_response = item->alice_signature;

	} else {
		credit_delta = item->alice_credit_delta;
		collateral_delta = item->alice_collateral_delta;
		signature_request = item->alice_signature;
		signature_response = item->bob_signature;
	}
	r = to_endian(TO_ENDIAN_BIG, sizeof(int), &credit_delta);
	if (r) {
		return ERR_FAIL;
	}
	r = to_endian(TO_ENDIAN_BIG, sizeof(int), &collateral_delta);
	if (r) {
		return ERR_FAIL;
	}

	c = 4;
	r = asn1_write_value(node, "Kee.KeeEntry.creditDelta", &credit_delta, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = 4;
	r = asn1_write_value(node, "Kee.KeeEntry.collateralDelta", &collateral_delta, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = 64;
	r = asn1_write_value(node, "Kee.KeeEntry.body", item->content.key, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	if (mode == KEE_LEDGER_STATE_REQUEST) {
		signature_request = zero_content;
		c = 0;
	} else {
		c = 64;
	}
	r = asn1_write_value(node, "Kee.KeeEntry.signatureRequest", signature_request, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = 5;
	if (item->response && mode > KEE_LEDGER_STATE_REQUEST) {
		response_s = "TRUE";
	} else {
		response_s = "FALSE";
		c++;
	}
	r = asn1_write_value(node, "Kee.KeeEntry.response", response_s, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	if (mode < KEE_LEDGER_STATE_FINAL) {
		signature_response = zero_content;
		c = 0;
	} else {
		c = 64;
	}
	r = asn1_write_value(node, "Kee.KeeEntry.signatureResponse", signature_response, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	r = asn1_der_coding(node, "Kee.KeeEntry", out, (int*)out_len, err);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	return 0;
}

/// \todo verify final state
int kee_ledger_item_serialize_db(struct kee_ledger_item_t *item, char *out, size_t *out_len) {
	int r;

	r = kee_ledger_item_serialize(item, out, out_len, KEE_LEDGER_STATE_FINAL);
	if (r) {
		return ERR_FAIL;
	}

	if (item->initiator == BOB) {
		*(out+*out_len) = 1;
	} else {
		*(out+*out_len) = 0;
	}
	*out_len += 1;

	return ERR_OK;	
}

int kee_ledger_sign(struct kee_ledger_t *ledger, struct kee_ledger_item_t *item, struct gpg_store *gpg, const char *passphrase) {
	int r;
	char *p;
	size_t c;
	size_t l;
	char out[1024];
	size_t out_len;
	enum kee_ledger_state_e mode;

	out_len = 1024;
	p = (char*)out;
	c = out_len;
	l = out_len;
	out_len = 0;

	mode = KEE_LEDGER_STATE_REQUEST;
	if (item->initiator == BOB) {
		mode = KEE_LEDGER_STATE_RESPONSE;
	}

	if (memcmp(item->alice_signature, zero_content, SIGNATURE_LENGTH)) {
		return ERR_ALREADY_SIGNED;
	}

	r = kee_ledger_digest(ledger, p);
	if (r) {
		return ERR_FAIL;
	}
	c = DIGEST_LENGTH;
	p = out + c;
	l -= c;
	out_len += c;

	c = l;
	r = kee_ledger_item_serialize(item, p, &c, mode);
	if (r) {
		return ERR_FAIL;
	}
	out_len += c;

	r = gpg_store_sign_with(gpg, (char*)out, out_len, passphrase, gpg->fingerprint);
	if (r) {
		return ERR_FAIL;
	}
	memcpy(item->alice_signature, gpg->last_signature, SIGNATURE_LENGTH);

	return ERR_OK;
}

int kee_ledger_serialize_open(struct kee_ledger_t *ledger, char *out, size_t *out_len) {
	int r;
	char err[1024];
	char b[1024];
	size_t c;
	asn1_node root;

	memset(&root, 0, sizeof(root));
	r = asn1_array2tree(schema_entry_asn1_tab, &root, err);
	if (r != ASN1_SUCCESS) {
		debug_log(DEBUG_ERROR, err);
		return ERR_FAIL;	
	}

	c = 1024;
	r = kee_ledger_serialize(ledger, b, &c);
	if (r) {
		return ERR_FAIL;	
	}
	r = asn1_write_value(root, "Kee.KeeTransport", "NEW", 1);
	if (r) {
		return ERR_FAIL;	
	}
	r = asn1_write_value(root, "Kee.KeeTransport.?1", b, c);
	if (r) {
		return ERR_FAIL;	
	}

	c = 1024;
	r = kee_ledger_item_serialize(ledger->last_item, b, &c, KEE_LEDGER_STATE_RESPONSE);
	if (r) {
		return ERR_FAIL;	
	}
	r = asn1_write_value(root, "Kee.KeeTransport", "NEW", 1);
	if (r) {
		return ERR_FAIL;	
	}
	r = asn1_write_value(root, "Kee.KeeTransport.?2", b, c);
	if (r) {
		return ERR_FAIL;	
	}

	r = asn1_der_coding(root, "Kee.KeeTransport", out, (int*)out_len, err);
	if (r) {
		debug_log(DEBUG_ERROR, err);
		return ERR_FAIL;	
	}
//	*(out+*out_len) = 0;
//	if (ledger->last_item->initiator == BOB) {
//		*(out+*out_len) = 1;
//	}
//	*out_len += 1;

	return ERR_OK;
}

int kee_ledger_parse_open(struct kee_ledger_t *ledger, struct gpg_store *gpg, const char *in, size_t in_len) {
	int r;
	char err[1024];
	char b[1024];
	size_t c;
	asn1_node root;
	asn1_node pair;
	struct kee_ledger_item_t *item;
	enum kee_initiator_e initiator;

	kee_ledger_init(ledger);

	memset(&root, 0, sizeof(root));
	memset(&pair, 0, sizeof(root));
	r = asn1_array2tree(schema_entry_asn1_tab, &root, err);
	if (r != ASN1_SUCCESS) {
		debug_log(DEBUG_ERROR, err);
		return ERR_FAIL;	
	}

	r = asn1_create_element(root, "Kee.KeeTransport", &pair);
	if (r) {
		debug_log(DEBUG_ERROR, asn1_strerror(r));
		return ERR_FAIL;	
	}

	r = asn1_der_decoding(&pair, in, in_len, err);
	if (r != ASN1_SUCCESS) {
		debug_log(DEBUG_ERROR, err);
		return ERR_FAIL;	
	}

//	r = asn1_copy_node(root, "Kee.KeeEntryHead", pair, "?1");
//	if (r != ASN1_SUCCESS) {
//		debug_log(DEBUG_ERROR, asn1_strerror(r));
//		return ERR_FAIL;	
//	}
//
//	r = asn1_copy_node(root, "Kee.KeeEntry", pair, "?2");
//	if (r != ASN1_SUCCESS) {
//		debug_log(DEBUG_ERROR, asn1_strerror(r));
//		return ERR_FAIL;	
//	}

	c = 1024;
	//r = asn1_der_coding(root, "Kee.KeeEntryHead", out, &c, err);
	r = asn1_der_coding(pair, "?1", b, (int*)&c, err);
	if (r != ASN1_SUCCESS) {
		debug_log(DEBUG_ERROR, err);
		return ERR_FAIL;	
	}

	r = kee_ledger_parse(ledger, b, c);
	if (r) {
		return ERR_FAIL;
	}

	c = 1024;
	//r = asn1_der_coding(root, "Kee.KeeEntry", out, &c, err);
	r = asn1_der_coding(pair, "?2", b, (int*)&c, err);
	if (r != ASN1_SUCCESS) {
		debug_log(DEBUG_ERROR, err);
		return ERR_FAIL;	
	}

	initiator = kee_ledger_item_initiator(ledger, gpg, NULL);
	item = kee_ledger_parse_item(ledger, b, c, initiator);
	if (item == NULL) {
		return ERR_FAIL;
	}

	return ERR_OK;
}

static size_t timespec_to_keypart(struct timespec *ts, char *out)  {
	unsigned int sec;
	unsigned int nsec;

	sec = (unsigned int)ts->tv_sec;
	nsec = (unsigned int)ts->tv_nsec;

	memcpy(out, &sec, sizeof(sec));
	memcpy(out + sizeof(sec), &nsec, sizeof(nsec));
	to_endian(TO_ENDIAN_BIG, sizeof(sec), out);
	to_endian(TO_ENDIAN_BIG, sizeof(nsec), out + sizeof(sec));
	
	return sizeof(sec) + sizeof(nsec);
}

// will return (offset + sizeof(unsigned int) * 2) bytes
static size_t db_key(enum DbKey pfx, struct timespec *ts, char *out, size_t offset) {
	size_t c;
	int r;
	char *p;
	struct timespec ts_fallback;

	if (ts == NULL) {
		ts = &ts_fallback;
		r = clock_gettime(CLOCK_REALTIME, ts);
		if (r) {
			return 0;
		}
	}

	*out = (char)pfx;
	p = out + 1;

	c = timespec_to_keypart(ts, p+offset);

	return offset + 1 + c;
}

// idx is reverse chronological order
static struct kee_ledger_item_t* get_item_by_idx(struct kee_ledger_t *ledger, int idx) {
	int i;
	struct kee_ledger_item_t *item;
	
	item = ledger->last_item;

	for (i = 0; i < idx; i++) {
		if (item == NULL) {
			return NULL;
		}
		item = item->prev_item;
	}
	return item;
}

static int kee_ledger_item_put_buf(struct kee_ledger_t *ledger, struct db_ctx *db, int idx, char *k, char *v) {
	int r;
	size_t c;
	size_t l;
	//char mem[4096];
	//char *k;
	//char *v;
	struct kee_ledger_item_t *item;

	//k = (char*)mem;
	//k = mem;
	//v = k + 2048;

	item = get_item_by_idx(ledger, idx);
	if (item == NULL) {
		return ERR_FAIL;
	}

	memcpy(k+1, ledger->digest, DIGEST_LENGTH);

	l = db_key(DbKeyLedgerEntry, &item->time, k, DIGEST_LENGTH+1);
	if (l == 0) {
		return ERR_FAIL;
	}

//	db_rewind(db);
//	r = db_next(db, DbKeyLedgerEntry, &k, &l, &v, &c);
//	if (!r) {
//		return ERR_DB_EXISTS;
//	}

	//c = 928;
	c = 2048;
	r = kee_ledger_item_serialize_db(item, v, &c);
	if (r) {
		return ERR_FAIL;
	}
	//r = db_put(db, k, l, v, c);
	r = db_add(db, k, l, v, c);
	if (r) {
		return ERR_FAIL;
	}

	return ERR_OK;	
}

int kee_ledger_item_put(struct kee_ledger_t *ledger, struct db_ctx *db, int idx) {
	char k[2048];
	char v[2048];

	return kee_ledger_item_put_buf(ledger, db, idx, (char*)k, (char*)v);
}

/// \todo atomic put for ledger and items!!
/// \todo guard local k/v buffer overflow
int kee_ledger_put(struct kee_ledger_t *ledger, struct db_ctx *db) {
	int r;
	int i;
	size_t c;
	size_t l;
	//char *mem;
	char mem[4096];
	char *k;
	char *v;

	//mem = malloc(4096);
	k = (char*)mem;
	v = k + 2048;
	//k = mem;
	//v = k + 2048;	

	*k = DbKeyReverse;
	memcpy(k+1, ledger->digest, DIGEST_LENGTH); 
	l = DIGEST_LENGTH + 1;
	//c = 928; // 1024 - 96
	c = 2048;
	//db_rewind(db);

	
	r = db_next(db, DbKeyReverse, &k, &l, &v, &c);
	if (!r) {
		memcpy(k, v, c);
		l = c;
		c = 2048;
		db_rewind(db);
		r = db_next(db, DbKeyLedgerHead, &k, &l, &v, &c);
		if (!r) {
			free(k);
			free(v);
			return ERR_DB_EXISTS;
		}
	}

	db_rewind(db);

	l = db_key(DbKeyLedgerHead, NULL, k, 0);
	if (l == 0) {
		return ERR_FAIL;
	}

	//c = 928;
	c = 2048;
	r = kee_ledger_serialize(ledger, v, &c);
	if (r) {
		return ERR_DB_FAIL;
	}

	r = db_start(db);
	if (r) {
		return ERR_DB_FAIL;
	}

	//r = db_put(db, k, l, v, c);
	r = db_add(db, k, l, v, c);
	if (r) {
		return ERR_DB_FAIL;
	}

	memcpy(v, k, l);
	c = l;
	l = DIGEST_LENGTH + 1;
	*k = DbKeyReverse;
	memcpy(k+1, ledger->digest, DIGEST_LENGTH);
	//r = db_put(db, k, l, v+1, c-1);
	r = db_add(db, k, l, v+1, c-1);
	if (r) {
		return ERR_DB_FAIL;
	}

	i = 0;
	r = 0;
	while (r == 0) {
		r = kee_ledger_item_put_buf(ledger, db, i, k, v);
		i++;
	}

	r = db_finish(db);
	if (r) {
		return ERR_DB_FAIL;
	}

	return ERR_OK;
}


static int get_authentication_params(struct kee_ledger_t *ledger, struct kee_ledger_item_t *item, char **pubkey_request, char **sig_request, char **pubkey_response, char **sig_response, enum kee_initiator_e initiator) {
	if (item->initiator == NOONE) {
		item->initiator = initiator;
	}
	if (item->initiator == NOONE) {
		return ERR_FAIL;
	}
	if (item->initiator == BOB) {
		*pubkey_request = ledger->pubkey_bob;
		*pubkey_response = ledger->pubkey_alice;
		*sig_request = item->bob_signature;
		*sig_response = item->alice_signature;
	} else {
		*pubkey_request = ledger->pubkey_alice;
		*pubkey_response = ledger->pubkey_bob;
		*sig_request = item->alice_signature;
		*sig_response = item->bob_signature;
	}
	if (!memcmp(*sig_request, zero_content, SIGNATURE_LENGTH)) {
		*sig_request = NULL;
	}
	if (!memcmp(*sig_response, zero_content, SIGNATURE_LENGTH)) {
		*sig_response = NULL;
	}
	return ERR_OK;
}

static int verify_item(struct kee_ledger_t *ledger, struct kee_ledger_item_t *item, enum kee_ledger_state_e mode, const char *sig, const char *pubkey) {
	int r;
	char b[1024];
	size_t c;
	
	c = 960;
	r = kee_ledger_item_serialize(item, ((char*)b)+DIGEST_LENGTH, &c, mode);
	if (r) {
		return ERR_FAIL;
	}
	r = kee_ledger_digest(ledger, b);
	if (r) {
		return ERR_FAIL;
	}
	r = verify_item_data(ledger, b, c + DIGEST_LENGTH, sig, pubkey);
	if (r) {
		return ERR_FAIL;
	}
	return ERR_OK;
}

// idx shows which item in ledger execution terminated
int kee_ledger_verify(struct kee_ledger_t *ledger, int *idx) {
	int r;
	struct kee_ledger_item_t *item;
	char *pubkey_request;
	char *pubkey_response;
	char *sig_request;
	char *sig_response;

	*idx = 0;
	item = ledger->last_item;
	if (item == NULL) {
		return ERR_FAIL;
	}

	r = 0;
	while (1) {
		r = get_authentication_params(ledger, item, &pubkey_request, &sig_request, &pubkey_response, &sig_response, NOONE);
		if (r) {
			return ERR_FAIL;
		}
		if (sig_response == NULL) {
			if (*idx > 0) {
				return ERR_FAIL;
			}
		} else {
			r = verify_item(ledger, item, KEE_LEDGER_STATE_RESPONSE, sig_response, pubkey_response);
		}
			
		if (sig_request != NULL) {
			r = verify_item(ledger, item, KEE_LEDGER_STATE_REQUEST, sig_request, pubkey_request);
//			c = 960;
//			r = kee_ledger_item_serialize(item, ((char*)b)+DIGEST_LENGTH, &c, KEE_LEDGER_STATE_REQUEST);
//			if (r) {
//				return ERR_FAIL;
//			}
//			r = kee_ledger_digest(ledger, b);
//			if (r) {
//				return ERR_FAIL;
//			}
//			r = verify_item_data(ledger, b, c + DIGEST_LENGTH, sig_request, pubkey_request);
//			if (r) {
//				return ERR_FAIL;
//			}
		}
		if (r) {
			return ERR_FAIL;
		}
		item = item->prev_item;
		if (item == NULL) {
			break;
		}
		*idx += 1;
	}
	return ERR_OK;
}



/// \todo zero initiator need detect
enum kee_ledger_state_e kee_ledger_item_state(struct kee_ledger_item_t *item) {
	char *sig_request;
	char *sig_response;

	enum kee_ledger_state_e state;

	if (item->initiator == BOB) {
		sig_request = item->bob_signature;
		sig_response = item->alice_signature;
	} else {
		sig_request = item->alice_signature;
		sig_response = item->bob_signature;
	}

	state = KEE_LEDGER_STATE_REQUEST;
	if (memcmp(sig_request, zero_content, SIGNATURE_LENGTH)) {
		if (memcmp(sig_response, zero_content, SIGNATURE_LENGTH)) {
			state = KEE_LEDGER_STATE_FINAL;	
		} else {
			state = KEE_LEDGER_STATE_RESPONSE;	
		}
	}

	return state;
}

/// \todo consider optional verify with item signature
/// \todo don't get confused; ledger alice is ALWAYS the requester, but when SIGNING alice is always the current keystore private key holder - consider renaming the latter to carol/dave...
enum kee_initiator_e kee_ledger_item_initiator(struct kee_ledger_t *ledger, struct gpg_store *gpg, struct kee_ledger_item_t *item) {
	enum kee_initiator_e initiator;

	initiator = NOONE;
	if (!memcmp(ledger->pubkey_alice, gpg->public_key, PUBKEY_LENGTH)) {
		initiator = ALICE;
	} else if (memcmp(ledger->pubkey_bob, zero_content, PUBKEY_LENGTH)) {
		initiator = BOB;
	}
	if (item != NULL) {
		item->initiator = initiator;
	}
	return initiator;
}

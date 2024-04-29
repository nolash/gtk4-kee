#include <stddef.h>
#include <string.h>
#include <libtasn1.h>
#include <gcrypt.h>

#include "ledger.h"
#include "cadiz.h"
#include "err.h"
#include "debug.h"
#include "digest.h"
#include "strip.h"
#include "content.h"
#include "endian.h"
#include "gpg.h"
#include "wire.h"


char zero_content[64];

static char *get_message(struct kee_ledger_t *ledger, asn1_node item, char *out_digest, char *out_data, size_t *out_len) {
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
//
//	c = 64;
//	r = asn1_read_value(item, "signatureResponse", sig, (int*)&c);
//	if (r != ASN1_SUCCESS) {
//		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
//		return NULL;
//	}

	buf[0] = 0;
	c = 0;
	r = asn1_write_value(root, "Kee.KeeEntry.signatureResponse", buf, c);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}

	memcpy(out_data, ledger->digest, 64);

	r = asn1_der_coding(root, "Kee.KeeEntry", out_data+64, (int*)out_len, err);
	if (r != ASN1_SUCCESS) {
		printf("%d (%s) %s\n", r, err, asn1_strerror(r));
		return NULL;
	}

	*out_len += 64;

	r = calculate_digest_algo(out_data, *out_len, out_digest, GCRY_MD_SHA512);
	if (r) {
		return NULL;
	}

	return out_digest;
}


static int verify_item(struct kee_ledger_t *ledger, asn1_node item, const char *pubkey_first_data, const char *pubkey_last_data) {
	int r;
	size_t c;
	char sig_data[64];
	char msg_data[1024 + 64];
	char *p = (char*)msg_data;

	c = 1024;
	p = get_message(ledger, item, p, p+64, &c);
	if (p == NULL) {
		return 1;
	}

	c = 64;
	r = asn1_read_value(item, "signatureResponse", sig_data, (int*)&c);
	if (r != ASN1_SUCCESS) {
		return 1;
	}

	if (c) {
		r = gpg_store_verify(sig_data, p, pubkey_last_data);
		if (r) {
			return 1;
		}
		debug_log(DEBUG_DEBUG, "ledger item verified\n");
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

void kee_ledger_item_apply_cache(struct kee_ledger_t *ledger, struct kee_ledger_item_t *item) {
	if (ledger->cache == NULL) {
		return;
	}

	ledger->cache->alice_credit_balance += item->alice_credit_delta;
	ledger->cache->bob_credit_balance += item->bob_credit_delta;
	ledger->cache->alice_collateral_balance += item->alice_collateral_delta;
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
	ledger->last_item = malloc(sizeof(struct kee_ledger_item_t));
	kee_ledger_item_init(ledger->last_item);
	ledger->last_item->prev_item = prev;
	
	return ledger->last_item;
}

struct kee_ledger_item_t *kee_ledger_parse_item(struct kee_ledger_t *ledger, const char *data, size_t data_len) {
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
	char tmp[64];
	int v;

	cur = kee_ledger_add_item(ledger);

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

	c = (int)data_len - 1;
	if (*(data+c)) {
		cur->initiator = BOB;
		credit_delta = &cur->bob_credit_delta;
		collateral_delta = &cur->bob_collateral_delta;
		pubkey_first = (const char*)ledger->pubkey_bob;
		pubkey_last = (const char*)ledger->pubkey_alice; // alice countersigns bobs
	} else {
		credit_delta = &cur->alice_credit_delta;
		collateral_delta = &cur->alice_collateral_delta;
		pubkey_first = (const char*)ledger->pubkey_alice;
		pubkey_last = (const char*)ledger->pubkey_bob;
	}

	r = asn1_der_decoding(&item, data, c, err);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}

	r = verify_item(ledger, item, pubkey_first, pubkey_last);
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

	c = 8;
	r = asn1_read_value(item, "timestamp", tmp, &c);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}

	c = 64;
	r = asn1_read_value(item, "body", tmp, &c);
	if (r != ASN1_SUCCESS) {
		return NULL;
	}

	r = kee_content_init(&(cur->content), tmp, 0);
	if (r) {
		return NULL;
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
	int decimals;

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
	
	c = 8;
	r = asn1_read_value(item, "uoaDecimals", &decimals, &c);
	if (r != ASN1_SUCCESS) {
		return r;
	}
	ledger->uoa_decimals = (char)decimals;

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

	r = calculate_digest_algo(data, data_len, (char*)ledger->digest, GCRY_MD_SHA512);
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

	c = strlen(ledger->uoa);
	r = asn1_write_value(node, "Kee.KeeEntryHead.uoa", ledger->uoa, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = 4;
	r = asn1_write_value(node, "Kee.KeeEntryHead.uoaDecimals", &ledger->uoa_decimals, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = 32;
	r = asn1_write_value(node, "Kee.KeeEntryHead.alicePubKey", ledger->pubkey_alice, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = 32;
	r = asn1_write_value(node, "Kee.KeeEntryHead.bobPubKey", ledger->pubkey_bob, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	c = 64;
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

int kee_ledger_item_serialize(struct kee_ledger_item_t *item, char *out, size_t *out_len, enum kee_item_serialize_mode_e mode) {
	int r;
	char err[1024];
	asn1_node node;
//	char timedata[8];
	long long nanotime;
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

//	memcpy(timedata, item->time, 4);
//	r = to_endian(TO_ENDIAN_BIG, 4, timedata);
//	if (r) {
//		return 1;
//	}
//
//	memcpy(timedata+4, item->time+4, 4);
//	r = to_endian(TO_ENDIAN_BIG, 4, timedata+4);
//	if (r) {
//		return 1;
//	}

	nanotime = item->time.tv_sec * 1000000000;
	nanotime += item->time.tv_nsec;
	r = to_endian(TO_ENDIAN_BIG, 8, &nanotime);
	if (r) {
		return 1;
	}

	c = 8;
	r = asn1_write_value(node, "Kee.KeeEntry.timestamp", &nanotime, c);
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

	if (mode == KEE_LEDGER_ITEM_SERIALIZE_REQUEST) {
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
	if (item->response) {
		response_s = "TRUE";
	} else {
		response_s = "FALSE";
		c++;
	}
	r = asn1_write_value(node, "Kee.KeeEntry.response", response_s, c);
	if (r != ASN1_SUCCESS) {
		return r;
	}

	if (mode < KEE_LEDGER_ITEM_SERIALIZE_FINAL) {
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

static int kee_ledger_digest(struct kee_ledger_t *ledger, char *out) {
	int r;
	char out_data[1024];
	size_t c;

	c = 1024;
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

int kee_ledger_sign(struct kee_ledger_t *ledger, struct kee_ledger_item_t *item, struct gpg_store *gpg, char *out, size_t *out_len, const char *passphrase) {
	int r;
	char *p;
	size_t c;
	size_t l;
	enum kee_item_serialize_mode_e mode;

	p = out;
	c = *out_len;
	l = *out_len;
	*out_len = 0;

	mode = KEE_LEDGER_ITEM_SERIALIZE_REQUEST;
	if (item->initiator == BOB) {
		mode = KEE_LEDGER_ITEM_SERIALIZE_RESPONSE;
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
	*out_len += c;

//	r = kee_ledger_serialize(ledger, p, &c);
//	if (r) {
//		return ERR_FAIL;
//	}
//	p = out + c;
//	l -= c;
//	c = l;

	c = l;
	r = kee_ledger_item_serialize(item, p, &c, mode);
	if (r) {
		return ERR_FAIL;
	}
	*out_len += c;

	r = gpg_store_sign_with(gpg, out, *out_len, passphrase, gpg->fingerprint);
	if (r) {
		return ERR_FAIL;
	}
	memcpy(item->alice_signature, gpg->last_signature, SIGNATURE_LENGTH);

	return ERR_OK;
}

//int kee_ledger_serialize_pair() {
//	int r;
//	asn1_node root;
//	asn1_node item;
//	asn1_node entry;
//	asn1_node pair;
//	char err[1024];
//
//	memset(&root, 0, sizeof(root));
//	r = asn1_array2tree(schema_entry_asn1_tab, &root, err);
//	if (r != ASN1_SUCCESS) {
//		debug_log(DEBUG_ERROR, err);
//		return ERR_FAIL;
//	}
//
//	r = asn1_create_element(root, "Kee.KeeEntryInit.KeeEntryHead", &entry);
//	if (r != ASN1_SUCCESS) {
//		return ERR_FAIL;
//	}
//
//	r = asn1_create_element(root, "Kee.KeeEntryInit.KeeEntryHead", &entry);
//	if (r != ASN1_SUCCESS) {
//		return ERR_FAIL;
//	}
//
//	r = asn1_create_element(root, "Kee.KeeEntryInit.KeeEntry", &item);
//	if (r != ASN1_SUCCESS) {
//		return ERR_FAIL;
//	}
//
//	c = strlen(ledger->uoa);
//	r = asn1_write_value(node, "Kee.KeeEntryHead.uoa", ledger->uoa, c);
//	if (r != ASN1_SUCCESS) {
//		return ERR_FAIL;
//	}
//
//	return ERR_OK;
//}

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
	r = kee_ledger_item_serialize(ledger->last_item, b, &c, KEE_LEDGER_ITEM_SERIALIZE_RESPONSE);
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
	*(out+*out_len) = 0;
	if (ledger->last_item->initiator == BOB) {
		*(out+*out_len) = 1;
	}
	*out_len += 1;

	return ERR_OK;
}

int kee_ledger_parse_open(struct kee_ledger_t *ledger, char *in, size_t in_len) {
	int r;
	char err[1024];
	char b[1024];
	size_t c;
	asn1_node root;
	asn1_node pair;
	//asn1_node item;
	//asn1_node entry;
	struct kee_ledger_item_t *item;
	char is_bob;

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

	is_bob = (char)*(in+in_len-1);
	in_len--;
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

	*(b+c) = is_bob;
	c++;
	item = kee_ledger_parse_item(ledger, b, c);
	if (item == NULL) {
		return ERR_FAIL;
	}

	return ERR_OK;
}

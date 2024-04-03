#include <string.h>
#include <fcntl.h>
#include <lmdb.h>
#include <gcrypt.h>
#include <time.h>

#include "db.h"
#include "digest.h"
#include "err.h"
#include "endian.h"
#include "debug.h"

int db_connect(struct db_ctx *ctx, char *conn) {
	int r;

	db_reset(ctx);
	ctx->connstr = conn;
	r = mdb_env_create(&ctx->env);
	if (r) {
		return ERR_FAIL;
	}
	r = mdb_env_open(ctx->env, ctx->connstr, MDB_NOLOCK, S_IRWXU);
	if (r) {
		return ERR_FAIL;
	}

	debug_log(DEBUG_INFO, "db connected");

	return ERR_OK;
}

/**
 * \todo split up and optimize
 */
int db_put(struct db_ctx *ctx, enum DbKey pfx, char *data, size_t data_len) {
	int r;
	char *buf;
	char buf_reverse[33];
	unsigned char *rv;
	char kv;
	struct timespec ts;
	char rts[sizeof(struct timespec)];
	gcry_error_t e;
	gcry_md_hd_t h;
	MDB_txn *tx;
	MDB_dbi dbi;
	MDB_val k;
	MDB_val v;

	buf = (char*)malloc(1 + 32 + sizeof(struct timespec)); // length should be lookup in sizes array for each key

	r = clock_gettime(CLOCK_REALTIME, &ts);
	if (r) {
		free(buf);
		return ERR_FAIL;
	}
	memcpy(rts, &ts.tv_sec, sizeof(ts.tv_sec));
	memcpy(rts + sizeof(ts.tv_sec), &ts.tv_nsec, sizeof(ts.tv_nsec));
	to_endian(0, sizeof(ts.tv_sec), rts);
	to_endian(0, sizeof(ts.tv_nsec), rts + sizeof(ts.tv_sec));

	e = gcry_md_open(&h, GCRY_MD_SHA256, 0);
	if (e) {
		free(buf);
		return ERR_DIGESTFAIL;
	}
	gcry_md_write(h, data, data_len);
	rv = gcry_md_read(h, 0);
	kv = (char)pfx;
	memcpy(buf, &kv, 1);
	memcpy(buf + 1, rts, sizeof(struct timespec));
	memcpy(buf + 1 + sizeof(struct timespec), rv, 32);

	r = mdb_txn_begin(ctx->env, NULL, 0, &tx);
	if (r) {
		free(buf);
		return ERR_FAIL;
	}

	r = mdb_dbi_open(tx, NULL, MDB_CREATE, &dbi);
	if (r) {
		free(buf);
		return ERR_FAIL;
	}

	k.mv_data = buf;
	k.mv_size = 1 + 32 + sizeof(struct timespec);
	v.mv_data = data;
	v.mv_size = data_len;

	r = mdb_put(tx, dbi, &k, &v, 0);
	if (r) {
		free(buf);
		return ERR_FAIL;
	}

	// put reverse lookup
	buf_reverse[0] = (char)DbKeyReverse;
	memcpy(buf_reverse+1, rv, 32);
	k.mv_data = buf_reverse; 
	k.mv_size = 33;
	v.mv_data = buf;
	v.mv_size = 1 + 32 + sizeof(struct timespec);
	gcry_md_close(h); // keep the handle open until here because we use its digest value again for the key

	r = mdb_put(tx, dbi, &k, &v, 0);
	if (r) {
		free(buf);
		return ERR_FAIL;
	}

	r = mdb_txn_commit(tx);
	if (r) {
		free(buf);
		return ERR_FAIL;
	}
	free(buf);

	return ERR_OK;

}

/**
 *
 * \todo change cursor to jump to new search match when current (last) prefix does not match lookup prefix.
 *
 */
int db_next(struct db_ctx *ctx, enum DbKey pfx, char **key, size_t *key_len, char **value, size_t *value_len) {
	int r;
	unsigned char start[DB_KEY_SIZE_LIMIT];

	memset(start, 0, DB_KEY_SIZE_LIMIT);;
	if ((char)pfx == 0) {
		return ERR_DB_INVALID;
	}

	//if (ctx->current_key == DbNoKey) {
	if (!ctx->browsing) {
		if (ctx->started) {
			mdb_cursor_close(ctx->crsr);
			mdb_dbi_close(ctx->env, ctx->dbi);
			mdb_txn_abort(ctx->tx);
		}

		r = mdb_txn_begin(ctx->env, NULL, MDB_RDONLY, &ctx->tx);
		if (r) {
			return ERR_DB_FAIL;
		}
		r = mdb_dbi_open(ctx->tx, NULL, 0, &ctx->dbi);
		if (r) {
			return ERR_DB_FAIL;
		}
		
		r = mdb_cursor_open(ctx->tx, ctx->dbi, &ctx->crsr);	
		if (r) {
			return ERR_DB_FAIL;
		}
		ctx->current_key = pfx;

		/// \todo add to else case below
		start[0] = (char)pfx;
		ctx->k.mv_size = 1;
//		if (!ctx->browsing) {
//			if (*key != 0) {
//				memcpy(start+1, *key, *key_len);
//				ctx->k.mv_size += *key_len;
//			}
//		}
		ctx->k.mv_data = start;

		ctx->browsing = 1;
		if (*key != 0) {
			memcpy(ctx->k.mv_data, *key, *key_len);
			ctx->k.mv_size += *key_len;
		}
		r = mdb_cursor_get(ctx->crsr, &ctx->k, &ctx->v, MDB_SET_RANGE);
	} else {
		r = mdb_cursor_get(ctx->crsr, &ctx->k, &ctx->v, MDB_NEXT_NODUP);
	}
	if (r) {
		return ERR_DB_FAIL;
	}
	ctx->started = 1;
	start[0] = (char)*((char*)ctx->k.mv_data);
	if (start[0] != ctx->current_key) {
		//db_reset(ctx);
		ctx->browsing = 0;
		return ERR_DB_NOMATCH;
	}

	*key_len = ctx->k.mv_size;
	memcpy(*key, (char*)ctx->k.mv_data, *key_len);
	*value_len = ctx->v.mv_size;
	memcpy(*value, (char*)ctx->v.mv_data, *value_len);

	return ERR_OK;

}


void db_rewind(struct db_ctx *ctx) {
	ctx->browsing = 0;
}


void db_reset(struct db_ctx *ctx) {
	mdb_cursor_close(ctx->crsr);
	mdb_dbi_close(ctx->env, ctx->dbi);
	mdb_txn_abort(ctx->tx);
	memset(ctx, 0, sizeof(struct db_ctx));
}

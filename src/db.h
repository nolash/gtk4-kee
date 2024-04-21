#ifndef _DB_H
#define _DB_H

#include <lmdb.h>

#ifndef DB_KEY_SIZE_LIMIT
#define DB_KEY_SIZE_LIMIT 128
#endif

#ifndef DB_VALUE_SIZE_LIMIT 
//#define DB_VALUE_SIZE_LIMIT 1048576
#define DB_VALUE_SIZE_LIMIT 1024
#endif

enum DbErr {
	ERR_DB_FAIL = 1,
	ERR_DB_NOMATCH,
	ERR_DB_INVALID,
};

/**
 * \brief Key prefixes used for database storage.
 *
 */
enum DbKey {
	/// Noop value, used for default value of a DbKey variable
	DbNoKey = 0x00,
	/// A credit item record
	DbKeyLedgerHead = 0x01,
	/// A credit item record
	DbKeyLedgerEntry = 0x02,
	/// A DN record
	DbKeyDN = 0x03,
	/// A reverse lookup record; resolves the content hash to the content entry in the database.
	DbKeyReverse = 0xff,
};

/**
 *
 * \brief Interface to persistent storage of data items used in application.
 *
 */
struct db_ctx {
	char *connstr;
	MDB_env *env;
	MDB_dbi dbi;
	MDB_txn *tx;
	MDB_cursor *crsr;
	MDB_val k;
	MDB_val v;
	enum DbKey current_key;
	int started;
	int browsing;
};

int db_connect(struct db_ctx *ctx, char *conn);
int db_put(struct db_ctx *ctx, enum DbKey pfx, char *data, size_t data_len);
int db_next(struct db_ctx *ctx, enum DbKey pfx, char **key, size_t *key_len, char **value, size_t *value_len);
void db_rewind(struct db_ctx *ctx);
void db_reset(struct db_ctx *ctx);

#endif // _DB_H

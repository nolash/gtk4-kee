#include <string.h>

#include "ledger.h"
#include "hex.h"

const char *test_ledger_data = "30818e0c035553440201020420c042f26197b312fef5def17e8c7f978c3219f74981f8430cd3bd57116014d33904201613476c3986b1317fbd831a07665210a1c271f1d88a04e8aa804bb032b44f6b04403aba8490187f543270b48e770bb272021f8fdb22f584080fc9d4d02553f4504624122d99ffc9254cdb8a2ef3826e47f60e1e7b5ec28b635fbba1141e3f486aad";

const char *test_item_data_a = "3082011d044000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000020817c7aecdfb6a9cbd020212d8020219600440c333b18dfa822c3ce875de07d54d932ed631aaa7f996153a120bda81e2f0717f4f5e67547019bcd8af0ba6ecba95313bbdc5e43385670d67c24ef76879514ad6044010b17b8c2bc18d7c040f8a4927d20714b78f48f6c3be23867f0ba4aac96b61081f86157942620852c2a47d12a3fb066604018d4228908cf5b0c29568fd67bf0c0101ff0440d6ea4b6501aa6c87cc67bc3297a999a53d8d756873656a0ad68ce9a3b5f9be3d974acf17f4d54d27f9ecf8e854fed32b6c672fa1378142f8898ebd9d68a8540f00";

const char *test_item_data_b = "3082011d0440534f94460b11203ec7272d5e5ec67c7d5c2669d0db4b732ce987f7f9a94e6a00d34622d466a1839feedc4e0b690af900baa631ce4cafd67305f5b9c0f8f8faaa020817c7aecdfb94675e02021d4a02020cd0044035c13f852ed6a20385959560c61e780dafebffa44d1e96f9293db1e75bf32b7cee53f3e96beeaeeebdf49e4467727f101b35a3e4a536c8545ec84b2880ce9b0d04403107700183297208a870aa15dcf1ca5da4b59adbc9388eceeb1a859259cd1b0ad7510c67b92a51fc9fc60b1cf7657976b99839ef7bd26134a4d5fed4cf2b820d0101ff044083ba7fd4d08541dee2a3f13189194454d429440fe7799d5e8536d2e18e6a5006487746b68f16fb2534a23372814bce45455e6269d187741217e505ff6f73d70a01";


int main() {
	int r;
	size_t c;
	struct kee_ledger_t ledger;
	struct kee_ledger_item_t *ledger_item_a;
	struct kee_ledger_item_t *ledger_item_b;
	Cadiz cadiz;
	char data[1024];
	
	cadiz.locator = "./testdata_resource";

	kee_ledger_init(&ledger);
	kee_ledger_reset_cache(&ledger);

	c = hex2bin(test_ledger_data, (unsigned char*)data);
	r = kee_ledger_parse(&ledger, data, c);
	if (r) {
		return 1;
	}

	c = hex2bin(test_item_data_a, (unsigned char*)data);
	ledger_item_a = kee_ledger_parse_item(&ledger, data, c);
	if (ledger_item_a == NULL) {
		return 1;
	}

	c = hex2bin(test_item_data_b, (unsigned char*)data);
	ledger_item_b = kee_ledger_parse_item(&ledger, data, c);
	if (ledger_item_b == NULL) {
		return 1;
	}

	kee_ledger_resolve(&ledger, &cadiz);

	if (ledger.cache->alice_credit_balance == 0) {
		return 1;
	}
	if (ledger.cache->bob_credit_balance == 0) {
		return 1;
	}
	if (ledger.cache->alice_collateral_balance == 0) {
		return 1;
	}
	if (ledger.cache->bob_collateral_balance == 0) {
		return 1;
	}

	kee_ledger_free(&ledger);

	return 0;
}

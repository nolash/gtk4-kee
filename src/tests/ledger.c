#include <string.h>

#include "ledger.h"
#include "hex.h"

const char *test_ledger_data = "30818e0c035553440201020420c042f26197b312fef5def17e8c7f978c3219f74981f8430cd3bd57116014d33904201613476c3986b1317fbd831a07665210a1c271f1d88a04e8aa804bb032b44f6b04403aba8490187f543270b48e770bb272021f8fdb22f584080fc9d4d02553f4504624122d99ffc9254cdb8a2ef3826e47f60e1e7b5ec28b635fbba1141e3f486aad";

const char *test_item_data_a = "3082011d044000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000020817c7aecdfb6a9cbd020212d8020219600440c333b18dfa822c3ce875de07d54d932ed631aaa7f996153a120bda81e2f0717f4f5e67547019bcd8af0ba6ecba95313bbdc5e43385670d67c24ef76879514ad6044010b17b8c2bc18d7c040f8a4927d20714b78f48f6c3be23867f0ba4aac96b61081f86157942620852c2a47d12a3fb066604018d4228908cf5b0c29568fd67bf0c0101ff0440d6ea4b6501aa6c87cc67bc3297a999a53d8d756873656a0ad68ce9a3b5f9be3d974acf17f4d54d27f9ecf8e854fed32b6c672fa1378142f8898ebd9d68a8540f00";


int main() {
	int r;
	size_t c;
	struct kee_ledger_t ledger;
	struct kee_ledger_item_t *ledger_item_a;
	char data[1024];
	
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

	kee_ledger_free(&ledger);

	return 0;
}

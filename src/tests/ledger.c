#include <string.h>

#include "ledger.h"
#include "hex.h"

const char *test_ledger_data = "30818e0c035553440201020420c67ee54f93d63d00f4b8c9a7e1c11b39657b55c525704bb32e15ec85bc140d140420adcaf6474132ac36e97d3dbee693d3b186cd8399d402dc505073069c46b5bd780440878102c19c032fd0d06f6b054a01e969b823ccfe7d5ba37a37beef3e64feb5f9b38e1a0f7413b781a4626b884f89bb3052f662692c53578453dc7c7d911d8609";

const char *test_item_data_a = "3082011d044000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000020817c94f8dec0e485d0202150e0202107a04404919455218d5bfb6ae9f0de9af37c23c140f976d4a0cb8652fc6897e6f043bc454021128b8daa18eb36c28687cfc33c3e1aa9b4e37059822ae14a0cefd1087a704409bd071c737b9342ab87de3f3e43ccba508366d09b28a6e331f1255d5668b211694fe011c78b6bba376590a5dae47e2ff880facad68e9ab4fca15309c00a6bd0a0101ff04402a3eac6ff818857883fb26d052fb17f2384f9fdb60f5bbae7d849bf621dfc65e68c2b82359b6c54b041732f11919ab0c1ae1a68504870c872f30cc74f9b9ae0400";

const char *test_item_data_b = "3082011d0440c2b795d9d3183bcc9d6ae1ae2960c302d7364a04996013dd9f31be628c46d2ee87b0cba51db67cd851a64dba04cc3e191dd48e7d7f3e063b0c850fd7b9b82218020817c94f8dec3e67aa02020ce20202049504401f78629f3015afa72f443005fc6711f7a7e2e20072eac86c98874c1dbe42095de3408d5711fb8fca56428461139992e8ff0452dc2092d2ba6ddb9658607f90ac0440d5d6cd6d905d0eb104ff3ab825cfc1be27f69a5377a3c84c33b3c5a0e6902e2af74d9024db58e1b90375be316e687a928edb881f8b6b3795682c20e533f9ed040101ff04409e8ffbbd5684b75aed7bf42a044914ea5813b1fccd9645462664317fa92dd9766c9ede39ea381e9648ef88bad220d0808660be63c94bf9954cf00daddad1150e01";


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

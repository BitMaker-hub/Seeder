# First Address QR checks

Requires Python 3.12 or newer and a C compiler. From the repository root:

```sh
python3 -m venv tests/address_qr/.venv
tests/address_qr/.venv/bin/python -m pip install -r tests/address_qr/requirements.txt
tests/address_qr/.venv/bin/python tests/address_qr/verify.py
python3 scripts/audit_ui.py
```

The test compiles the existing production `src/qrcoded.c`, generates two public
BIP84 receiving-address QRs at the 3px/4px module sizes used on the two displays,
and decodes the PGM images with independent ZXing-C++. It checks version 2-L,
alphanumeric mode, the exact uppercase address bytes, and the unchanged witness
program/scriptPubKey with embit. One address is the published [BIP84 test vector](https://github.com/bitcoin/bips/blob/master/bip-0084.mediawiki#test-vectors).
These are public test seeds and must never be used for funds.

[BIP173 recommends uppercase inside QR codes](https://github.com/bitcoin/bips/blob/master/bip-0173.mediawiki#uppercaselowercase)
to enable alphanumeric encoding; the readable address remains lowercase.
[SeedSigner normalizes scanned Bech32 addresses to lowercase](https://github.com/SeedSigner/seedsigner/blob/0.8.7/src/seedsigner/models/decode_qr.py#L1016-L1066)
before handing them onward. The UI audit checks that all 42 visible characters
fit beside the QR and that its four-module quiet zone stays below the header
and within the screen on both boards.

This verifies encoding and layout constraints, not a physical display/camera.
Temporary images are deleted after the check; host build files are ignored.

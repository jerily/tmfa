# tmfa

HOTP, TOTP, and Base32 for Tcl 8.6 and 9.x, using OpenSSL 3.

```tcl
package require tmfa 1.1
set secret [tmfa::base32_encode "12345678901234567890"]
tmfa::get_hotp $secret 0 6 SHA1
tmfa::get_totp $secret 6 30 SHA1
tmfa::get_totp $secret 8 30 SHA1 59
tmfa::base32_decode $secret
```

- `get_hotp base32_secret counter digits algo`
- `get_totp base32_secret digits period algo ?timestamp?`
- `base32_encode bytes` returns padded Base32 text.
- `base32_decode text` returns a byte array; padding is optional and either letter case is accepted.

Both OTP commands require a nonempty Base32 secret, 4–10 digits, and `SHA1`,
`SHA256`, or `SHA512`. The period is 1–120 seconds. Counters and Unix timestamps
are unsigned 64-bit decimal integers; TOTP defaults to the current time.
Tokens are strings with leading zeroes preserved. Encode text as UTF-8 bytes
with `encoding convertto utf-8` before Base32 encoding it.

## Development

Install a C11 compiler, CMake 3.22+, and [ttrek](https://github.com/jerily/ttrek), then:

```sh
ttrek install -y
ttrek run -script test
ttrek run -script demo
```

The manifest builds and installs the extension into `ttrek-venv`.
With Tcl development files and OpenSSL 3 already installed:

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX="$PWD/install"
cmake --build build --parallel
ctest --test-dir build --output-on-failure
cmake --install build
```

Use `TCL_INCLUDE_DIR`, `TCL_LIBRARY_DIR`, and `OPENSSL_ROOT_DIR` to select dependencies.
Set `BUILD_SHARED_LIBS=OFF` for embedding; link Tcl and OpenSSL Crypto and register
`Tmfa_Init`. For NaviServer, run `make NAVISERVER=/path/to/ns` and
`make NAVISERVER=/path/to/ns install` with OpenSSL's `libcrypto.pc` on `PKG_CONFIG_PATH`.

Tests cover [HOTP](https://www.rfc-editor.org/rfc/rfc4226),
[TOTP](https://www.rfc-editor.org/rfc/rfc6238), and
[Base32](https://www.rfc-editor.org/rfc/rfc4648) vectors, binary data, and invalid inputs.

`tmfa::keyuri secret type account ?-issuer name? ?-digits 6|8?
?-algorithm SHA1|SHA256|SHA512? ?-step seconds? ?-counter integer?` (1.1.2)
produces a standard `otpauth://` provisioning URI. `type` is `totp` (default
step 30 seconds) or `hotp` (requires counter). Labels and issuer are UTF-8
percent encoded; secrets are canonical unpadded Base32. Colons/control characters
in labels are rejected. The generic Tcl manager has no framework dependencies.
Static/NaviServer embeddings should source `library/manager.tcl` after registering
the native commands. QR rendering belongs to the caller, e.g. `tqrcodegen`.

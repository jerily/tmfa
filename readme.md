# tmfa

TCL/C extension for Multi-Factor Authentication (MFA) using TOTP and HOTP.

## Tcl Commands
```
# Generate a TOTP token using the secret.
# tmfa::get_totp base32_encoded_secret digits period algo 
tmfa::get_totp abcdef 6 45 SHA256

# Generate a HOTP token using the secret.
# tmfa::get_hotp secret counter digits algo
tmfa::get_hotp abcdef 0 6 SHA1

# Base32 encode a string.
# tmfa::base32_encode text
tmfa::base32_encode something

# Base32 decode a string.
# tmfa::base32_decode base32_encoded_text
tmfa::base32_decode [tmfa::base32_encode something]
```

## Build and install dependencies

On Linux:
```bash
sudo apt install pkg-config libgcrypt-dev
```

On MacOS:
```bash
brew install pkg-config libgcrypt
```

Build and install libcotp:
```bash
wget https://github.com/paolostivanin/libcotp/archive/refs/tags/v2.0.1.tar.gz
tar -xzf v2.0.1.tar.gz
cd libcotp-2.0.1
# if on MacOS, run the following line:
# sed -i.orig 's/^add_link_options/#add_link_options/' CMakeLists.txt
mkdir build
cd build
cmake ..
make
make install
```

## Build the library
For TCL
```
wget https://github.com/jerily/tjson/archive/refs/tags/v1.0.0.tar.gz
tar -xzf v1.0.0.tar.gz
cd tmfa-1.0.0
mkdir build
cd build
cmake .. \
  -DTCL_LIBRARY_DIR=/usr/local/lib \
  -DTCL_INCLUDE_DIR=/usr/local/include
make
make install
```

For Naviserver (using Makefile)
```bash
make
make install
```

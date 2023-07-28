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

## Install dependencies
```
sudo apt install libgcrypt-dev
```

## Build the library
For TCL
```
git clone --recurse-submodules https://github.com/jerily/tmfa.git
cd tmfa
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/usr
make
make install
```

For Naviserver (using cmake)
```
git clone --recurse-submodules https://github.com/jerily/tmfa.git
cd tmfa
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/usr -DNAVISERVER=/usr/local/ns
make
make install
```

For Naviserver (using Makefile)
```
git clone --recurse-submodules https://github.com/jerily/tmfa.git
cd tmfa
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/usr -DNAVISERVER=/usr/local/ns
make
make install

cd ..
make
make install
```
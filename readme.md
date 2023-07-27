# TCL/C extension for Multi-Factor Authentication (MFA) using TOTP and HOTP.

## Tcl Commands
```
# Generate a TOTP token using the secret.
# tmfa::get_totp base32_encoded_secret digits period algo 
tmfa::get_totp abcdef 6 45 SHA256

# Generate a HOTP token using the secret.
# tmfa::get_hotp secret counter digits algo
tmfa::get_hotp abcdef 0 6 SHA1
```

## Install dependencies
```
sudo apt install libgcrypt-dev
```

## Build the library 
```
git clone --recurse-submodules https://github.com/jerily/tmfa.git
cd tmfa
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/usr
make
make install
```
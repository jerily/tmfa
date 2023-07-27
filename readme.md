# TCL/C extension for Multi-Factor Authentication (MFA) using TOTP and HOTP.

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
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ../
make
make install
```
#!/bin/bash
set -e
CURL_VERSION=8.3.0
cd /tmp
wget https://curl.se/download/curl-$CURL_VERSION.tar.gz
tar -xzvf curl-$CURL_VERSION.tar.gz
cd curl-$CURL_VERSION

autoconf

./configure --with-openssl
make
make install
ldconfig

cd ..
rm -rf curl-$CURL_VERSION*

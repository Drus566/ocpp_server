#!/bin/sh

CERT_PATH=/ocppclient/certificates

cd $CERT_PATH 

# Create CA
openssl req -new -x509 -nodes -sha256 -days 365000 -extensions v3_ca -keyout ca.key -out ca.crt -subj "/CN=ocpp-example"

# Generate self-signed charge-point certificate
openssl genrsa -out $CERT_PATH/charge-point.key 4096
openssl req -new -out $CERT_PATH/charge-point.csr -key $CERT_PATH/charge-point.key -config $CERT_PATH/openssl-cp.conf -sha256

openssl x509 -req -in $CERT_PATH/charge-point.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out $CERT_PATH/charge-point.crt -days 365000 -extensions req_ext -extfile $CERT_PATH/openssl-cp.conf -sha256

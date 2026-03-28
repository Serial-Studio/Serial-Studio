# OpenSSL

OpenSSL is a robust, commercial-grade, full-featured Open Source Toolkit for the Transport Layer Security (TLS) protocol formerly known as the Secure Sockets Layer (SSL) protocol. The protocol implementation is based on a full-strength general purpose cryptographic library, which can also be used stand-alone.

OpenSSL is descended from the SSLeay library developed by Eric A. Young and Tim J. Hudson.

The official Home Page of the OpenSSL Project is [www.openssl.org](www.openssl.org).

This repo uses prebuilt binaries from [https://github.com/alex-spataru/OpenSSL-Builder](https://github.com/alex-spataru/OpenSSL-Builder), which automates fetching, building, and packaging OpenSSL via CI. 

## Overview

The OpenSSL toolkit includes:

- **libssl**
  an implementation of all TLS protocol versions up to TLSv1.3 ([RFC 8446](https://www.rfc-editor.org/rfc/rfc8446)).

- **libcrypto**
  a full-strength general purpose cryptographic library. It constitutes the basis of the TLS implementation, but can also be used independently.

- **openssl**
  the OpenSSL command line tool, a swiss army knife for cryptographic tasks,
  testing and analyzing. It can be used for
  - creation of key parameters
  - creation of X.509 certificates, CSRs and CRLs
  - calculation of message digests
  - encryption and decryption
  - SSL/TLS client and server tests
  - handling of S/MIME signed or encrypted mail
  - and more...

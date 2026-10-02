#!/usr/bin/env python3
"""The release key, derived from the passphrase secret (A15.5, PLT-06). The only reader of KINDLING_SIGNING_PASSPHRASE.

Usage:  pk8 <out>     derive the key and write it as PKCS#8 DER to <out> (a temporary file the caller deletes);
                      the first time, also make android/keys/release-cert.der and release-cert.sha256 and print the
                      fingerprint for the owner's developer account (RSK-18); later, fail unless the derived key
                      matches that certificate, so a mistyped passphrase fails the build
        fingerprint   print the committed certificate's SHA-256 fingerprint
        selftest      derive from a fixed test phrase and compare with the recorded public key hash
Never prints the passphrase or the key.
"""
import datetime
import hashlib
import os
import sys

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import NameOID

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CERT = os.path.join(ROOT, "android", "keys", "release-cert.der")
CERT_SHA = os.path.join(ROOT, "android", "keys", "release-cert.sha256")
SECRET = "KINDLING_SIGNING_PASSPHRASE"
SALT = b"kindling-release-v1"
# P-256's group order q (FIPS 186-4, D.1.2.3).
Q = 0xFFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551
TEST_PHRASE = "kindling test phrase only"
# SHA-256 of the test phrase's public key (SubjectPublicKeyInfo DER), recorded on the first run, 2 October 2026.
TEST_PUBLIC_SHA256 = "66465c03d17177d18603c338bd48beb67f6cb9b596583f9ccc8516cec28cd455"


def derive(passphrase):
    """The P-256 private key from a passphrase: scrypt (n = 2^17, r = 8, p = 1) to 48 bytes, d = int mod (q − 1) + 1.
    Implements PLT-06, see A15.5."""
    raw = hashlib.scrypt(passphrase.encode("utf-8"), salt=SALT, n=2**17, r=8, p=1, maxmem=256 * 1024 * 1024, dklen=48)
    d = int.from_bytes(raw, "big") % (Q - 1) + 1
    return ec.derive_private_key(d, ec.SECP256R1())


def public_der(public_key):
    return public_key.public_bytes(serialization.Encoding.DER, serialization.PublicFormat.SubjectPublicKeyInfo)


def make_cert(key):
    """The self-signed release certificate: CN=Kindling, serial 1, valid 2026-10-01 to 2126-10-01, SHA-256."""
    name = x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, "Kindling")])
    utc = datetime.timezone.utc
    return (
        x509.CertificateBuilder()
        .subject_name(name)
        .issuer_name(name)
        .public_key(key.public_key())
        .serial_number(1)
        .not_valid_before(datetime.datetime(2026, 10, 1, tzinfo=utc))
        .not_valid_after(datetime.datetime(2126, 10, 1, tzinfo=utc))
        .sign(key, hashes.SHA256())
    )


def fingerprint(der):
    """The certificate's SHA-256 as the developer console shows it: upper-case hex pairs joined by colons."""
    h = hashlib.sha256(der).hexdigest().upper()
    return ":".join(h[i:i + 2] for i in range(0, len(h), 2))


def pk8(out):
    passphrase = os.environ.get(SECRET, "")
    if not passphrase:
        print(f"signing-key: {SECRET} is not set")
        return 1
    key = derive(passphrase)
    del passphrase
    if os.path.exists(CERT):
        with open(CERT, "rb") as f:
            cert = x509.load_der_x509_certificate(f.read())
        if public_der(cert.public_key()) != public_der(key.public_key()):
            print("signing-key: passphrase does not match android/keys/release-cert.der")
            return 1
    else:
        der = make_cert(key).public_bytes(serialization.Encoding.DER)
        with open(CERT, "wb") as f:
            f.write(der)
        with open(CERT_SHA, "w") as f:
            f.write(f"{hashlib.sha256(der).hexdigest()}  release-cert.der\n")
        print("signing-key: made android/keys/release-cert.der; commit it with release-cert.sha256")
        print(f"signing-key: fingerprint (SHA-256) {fingerprint(der)}")
    fd = os.open(out, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o600)
    with os.fdopen(fd, "wb") as f:
        f.write(key.private_bytes(serialization.Encoding.DER, serialization.PrivateFormat.PKCS8,
                                  serialization.NoEncryption()))
    return 0


def main(argv):
    if argv[:1] == ["pk8"] and len(argv) == 2:
        return pk8(argv[1])
    if argv == ["fingerprint"]:
        if not os.path.exists(CERT):
            print("signing-key: no android/keys/release-cert.der yet")
            return 1
        with open(CERT, "rb") as f:
            print(f"SHA-256 {fingerprint(f.read())}")
        return 0
    if argv == ["selftest"]:
        # checks: PLT-06
        got = hashlib.sha256(public_der(derive(TEST_PHRASE).public_key())).hexdigest()
        if got != TEST_PUBLIC_SHA256:
            print(f"Signing key selftest: FAIL (public key hash {got}, recorded {TEST_PUBLIC_SHA256})")
            return 1
        print("Signing key selftest: OK")
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

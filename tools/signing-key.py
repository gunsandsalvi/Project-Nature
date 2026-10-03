#!/usr/bin/env python3
"""The release key, derived from the passphrase secret (A15.5, PLT-06).

The key is never stored: scrypt turns KINDLING_SIGNING_PASSPHRASE into 48 bytes, reduced to a P-256 key as FIPS
186-4's "extra random bits" method does (d = c mod (n - 1) + 1), and every use first checks that its public key is
the one in android/keys/release-cert.der, so a mistyped passphrase stops the build before anything is signed.
Only this script reads the secret, and it never prints it.

    python3 tools/signing-key.py pk8 <file>             the release key as PKCS#8, for apksigner (deleted after use)
    python3 tools/signing-key.py check                  the passphrase matches the certificate
    python3 tools/signing-key.py fingerprint            the certificate's SHA-256, as registered (RSK-18)
    python3 tools/signing-key.py throwaway <pk8> <der>  a key and certificate made for one check build, then dropped
    python3 tools/signing-key.py selftest               the derivation and the check, on a test phrase
"""
import datetime
import hashlib
import os
import sys
import tempfile

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import NameOID

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CERT = os.path.join(ROOT, "android", "keys", "release-cert.der")
FINGERPRINT = os.path.join(ROOT, "android", "keys", "release-cert.sha256")
SALT = b"kindling-release-v1"
P256_ORDER = 0xFFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551
TEST_PHRASE = "kindling test phrase only"


def derive(passphrase):
    """The P-256 key for a passphrase: scrypt with n = 2^17, r = 8, p = 1 (128 MiB, about a second)."""
    c = hashlib.scrypt(passphrase.encode("utf-8"), salt=SALT, n=2**17, r=8, p=1, maxmem=256 * 1024 * 1024, dklen=48)
    d = int.from_bytes(c, "big") % (P256_ORDER - 1) + 1
    return ec.derive_private_key(d, ec.SECP256R1())


def public_der(key):
    return key.public_key().public_bytes(serialization.Encoding.DER, serialization.PublicFormat.SubjectPublicKeyInfo)


def matches(key, cert_der):
    cert = x509.load_der_x509_certificate(cert_der)
    return cert.public_key().public_bytes(
        serialization.Encoding.DER, serialization.PublicFormat.SubjectPublicKeyInfo) == public_der(key)


def certificate(key, name):
    """A self-signed certificate valid for a century, for a test or a throwaway key."""
    subject = x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, name)])
    start = datetime.datetime(2026, 10, 1, tzinfo=datetime.timezone.utc)
    return (x509.CertificateBuilder().subject_name(subject).issuer_name(subject).public_key(key.public_key())
            .serial_number(1).not_valid_before(start).not_valid_after(start.replace(year=2126))
            .sign(key, hashes.SHA256())).public_bytes(serialization.Encoding.DER)


def write_private(key, path):
    data = key.private_bytes(serialization.Encoding.DER, serialization.PrivateFormat.PKCS8,
                             serialization.NoEncryption())
    fd = os.open(path, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o600)
    with os.fdopen(fd, "wb") as f:
        f.write(data)


def release_key():
    phrase = os.environ.get("KINDLING_SIGNING_PASSPHRASE", "")
    if not phrase:
        sys.exit("Signing key: KINDLING_SIGNING_PASSPHRASE is not set; a release build needs it (A15.5)")
    key = derive(phrase)
    with open(CERT, "rb") as f:
        if not matches(key, f.read()):
            sys.exit("Signing key: the passphrase does not match android/keys/release-cert.der; nothing was signed")
    return key


def cert_fingerprint():
    with open(CERT, "rb") as f:
        return hashlib.sha256(f.read()).hexdigest()


def selftest():
    a, b = derive(TEST_PHRASE), derive(TEST_PHRASE)
    assert public_der(a) == public_der(b), "the derivation is not repeatable"
    cert = certificate(a, "Kindling test")
    assert matches(b, cert), "a key does not match its own certificate"
    assert not matches(derive(TEST_PHRASE + "!"), cert), "another passphrase matched the certificate"
    with tempfile.TemporaryDirectory() as tmp:
        path = os.path.join(tmp, "k.pk8")
        write_private(a, path)
        assert os.stat(path).st_mode & 0o077 == 0, "the key file is readable by others"
        loaded = serialization.load_der_private_key(open(path, "rb").read(), password=None)
        assert public_der(loaded) == public_der(a), "the PKCS#8 file does not hold the key"
    with open(FINGERPRINT) as f:
        assert f.read().split()[0] == cert_fingerprint(), "release-cert.sha256 is not the certificate's SHA-256"
    print("Signing key selftest: OK")


def main(argv):
    cmd = argv[1] if len(argv) > 1 else ""
    if cmd == "pk8" and len(argv) == 3:
        write_private(release_key(), argv[2])
    elif cmd == "check":
        release_key()
        print(f"Signing key: matches android/keys/release-cert.der (SHA-256 {cert_fingerprint()})")
    elif cmd == "fingerprint":
        print(cert_fingerprint())
    elif cmd == "throwaway" and len(argv) == 4:
        key = ec.generate_private_key(ec.SECP256R1())
        write_private(key, argv[2])
        with open(argv[3], "wb") as f:
            f.write(certificate(key, "Kindling check build"))
    elif cmd == "selftest":
        selftest()
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main(sys.argv)

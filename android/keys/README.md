# Signing keys (A15.5, PLT-06)

## The release key

From α00b every delivered APK is signed with the release key, derived from the owner's passphrase secret
`KINDLING_SIGNING_PASSPHRASE` by `tools/signing-key.py` (scrypt, then a P-256 key). The key itself is never stored:
each build derives it into a temporary file and deletes it. Only `tools/signing-key.py` reads the secret.

- `release-cert.der`: the public, self-signed certificate (`CN=Kindling`, serial 1, valid 2026-10-01 to 2126-10-01,
  ECDSA with SHA-256), made on the first derivation. Each build checks the derived key against it, so a mistyped
  passphrase fails the build.
- `release-cert.sha256`: its SHA-256. The fingerprint the owner registers in the developer account (`RSK-18`):
  `python3 tools/signing-key.py fingerprint`.

## The throwaway key

`throwaway.p12` is public and throwaway: it signed α00, and signs builds made without the secret.

- Store type PKCS12, alias `throwaway`, password `kindling-throwaway`, EC P-256, made with:
  `keytool -genkeypair -keystore android/keys/throwaway.p12 -storetype PKCS12 -storepass kindling-throwaway -alias throwaway -keyalg EC -groupname secp256r1 -sigalg SHA256withECDSA -validity 36500 -dname "CN=Kindling throwaway key"`
- APKs signed with it install over each other.
- The first APK signed with the release key (α00b) needs one uninstall; after that, worlds come back only by export
  and import (`PLT-08`). Before α07a there are no worlds to lose.

# Signing keys (A15.5, PLT-06)

`throwaway.p12` is public and throwaway: it signs α00 and later builds until the owner adds the
`KINDLING_SIGNING_PASSPHRASE` secret, after which α00b's release key, derived from that passphrase, takes over.

- Store type PKCS12, alias `throwaway`, password `kindling-throwaway`, EC P-256, made with:
  `keytool -genkeypair -keystore android/keys/throwaway.p12 -storetype PKCS12 -storepass kindling-throwaway -alias throwaway -keyalg EC -groupname secp256r1 -sigalg SHA256withECDSA -validity 36500 -dname "CN=Kindling throwaway key"`
- APKs signed with it install over each other.
- The first APK signed with the release key needs one uninstall; after that, worlds come back only by export and
  import (`PLT-08`). Before α07a there are no worlds to lose.

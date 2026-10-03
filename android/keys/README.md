# The release certificate (A15.5, PLT-06)

`release-cert.der` is the release key's public certificate, and `release-cert.sha256` its SHA-256, the fingerprint
registered in the owner's developer account (`RSK-18`). Both were kept when the codebase was deleted on 3 October
2026 to be rebuilt from scratch, so every new build still installs as the same app. The key itself is never stored:
the build derives it from the owner's passphrase secret, as A15.5 says, and checks it against this certificate.

# Fixture project

The self-test's clean project file (tools/filecheck.py selftest, PRC-12).

## How this file works

### Kinds of item

- **Context:** `CTX`.
- **Rules:** `RUL`, plus `ABC-04`.
- **Features:** every other area.

Examples such as `WLD-01` are not checked here.

## 1. Things

- `ABC-01` **Feature one** *(Decided)*: cites `ABC-02`.
  - **What:** plain.
- `ABC-02` **Feature two** *(Decided)*
- `ABC-03` **Gone** *(Dropped)*
  - **Dropped because:** merged into `ABC-01`.
- `ABC-04` **A rule** *(Decided)*
- `CTX-01` **Context** *(Decided)*
- `RUL-01` **Kept rule** *(Decided)*
- `ABC-02` **Feature two again** *(Decided)*

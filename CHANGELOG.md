# Intel(R) EPID SDK ChangeLog                                   {#ChangeLog}

## [7.0.1] - 2019-03-08

### Changed

- Updated Intel(R) IPP Cryptography library to version 2019 Update 1.

### Known Issues

- Only the SHA-256 hash algorithm is supported when using the SDK with
  the IBM TPM simulator due to a defect in version 1119 of the
  simulator.

- Basenames are limited to 124 bytes in TPM mode.

- Scons build will not work natively on ARM. You can still build using
  `make` or cross compile.

- Compressed private keys are not supported.


## [7.0.0] - 2018-09-21

### Added

- The SDK now includes a command-line tool to create join requests
  used for join based provisioning.

- The SDK now includes a command-line tool to create member pairing
  pre-computation blobs from issuer material so that device
  manufacturers can more easily pre-provision devices.

- The `EpidMemberWritePrecomp` API was reintroduced to allow the
  pairing pre-computation blob to be serialized. The serialized blob
  can be used speed up future sessions.

- Full signature pre-computation was added to non-split member to speed up
  signing for constrained devices.

### Changed

- The SDK has been updated to support split signatures aligned with
  TPM 2.0 v1.38. Signatures created in split-signature mode can only
  be verified with a split-signature aware verifier. The SDK builds in
  non-split mode by default.

- Verifier API updated to support split and non-split signatures.

    - Parameters changed
        - `EpidVerify`
    - Removed or made private
        - `EpidVerifyBasicSig`
        - `EpidNrVerify`
        - `EpidCheckPrivRlEntry`

- SDK functions now use the hash algorithm encoded in the group ID by
  default. You can still override the default hash algorithm using
  `EpidMemberSetHashAlg` or `EpidVerifierSetHashAlg`.

### Known Issues

- Only the SHA-256 hash algorithm is supported when using the SDK with
  the IBM TPM simulator due to a defect in version 1119 of the
  simulator.

- Basenames are limited to 124 bytes in TPM mode.

- Scons build will not work natively on ARM. You can still build using
  `make` or cross compile.

- Compressed private keys are not supported.


## [6.0.1] - 2018-05-04

### Changed

- Updated Intel(R) IPP Cryptography library to version 2018 (Update
  2.1).

### Fixed

- The member library now includes the tpm2 subcomponent when built
  using SCons, instead of requiring a separate member.tpm2 library.

### Known Issues

- Only the SHA-256 hash algorithm is supported when using the SDK with
  the IBM TPM simulator due to a defect in version 532 of the
  simulator.

- Basenames are limited to 124 bytes in TPM mode.

- Scons build will not work natively on ARM. You can still build using
  `make` or cross compile.


## [6.0.0] - 2017-12-15

### Added

- The member can now be built with a substantially reduced code size
  using a compilation option.

- New context lifetime management APIs have been added to member to
  give callers more control of memory allocation.

- New member API `EpidClearRegisteredBasenames` has been added to
  clear registered basenames without recreating the member.

### Changed

- `EpidRegisterBaseName` was renamed to `EpidRegisterBasename` because
  basename is a single word.

- Command-line parsing library used by samples and tools has been
  replaced by Argtable3.


### Deprecated

- `EpidMemberCreate` has been deprecated. This API has been superseded
  by `EpidMemberGetSize` and `EpidMemberInit`.

- `EpidMemberDelete` has been deprecated. This API has been superseded
  by `EpidMemberDeinit`.

### Removed

- `size_optimized_release` build configuration has been removed.
  Use the compilation option to build member with reduced code size.

### Known Issues

- Only the SHA-256 hash algorithm is supported when using the SDK with
  the IBM TPM simulator due to a defect in version 532 of the
  simulator.

- Basenames are limited to 124 bytes in TPM mode.

- Scons build will not work natively on ARM. You can still build using
  `make` or cross compile.


## [5.0.0] - 2017-09-15

### Added

- The member implementation now has the option to support signing
  using a TPM, using the ECDAA capabilities of TPM 2.0.


### Changed

- Member API updated to unify HW and SW use cases.
    - Added
        - `ProvisionKey`
        - `ProvisionCompressed`
        - `ProvisionCredential`
        - `Startup`
    - Parameters changed
        - `MemberCreate`
        - `RequestJoin`
    - Removed or made private
        - `WritePrecomp`
        - `SignBasic`
        - `NrProve`
        - `AssemblePrivKey`

- `EpidRequestJoin` was renamed to `EpidCreateJoinRequest` to make it
  clear that it is not directly communicating with the issuer.


### Fixed

- `EpidCreateJoinRequest` creates valid join requests. This fixes a
  regression in `EpidRequestJoin` introduced in 4.0.0.


### Known Issues

- Only the SHA-256 hash algorithm is supported when using the
  SDK with the IBM TPM simulator due to a defect in version
  532 of the simulator.

- Basenames are limited to 124 bytes in TPM mode.


## [4.0.0] - 2017-04-25

### Added

- The member implementation now provides an internal interface that
  gives guidance on partitioning member operations between highly
  sensitive ones that use f value of the private key, and less
  sensitive operations that can be performed in a host environment.

- New member API `EpidAssemblePrivKey` was added to help assemble and
  validate the new member private key that is created when a member
  either joins a group (using the join protocol) or switches to a new
  group (as the result of a performance rekey).


### Changed

- Updated Intel(R) IPP Cryptography library to version 2017 (Update 2).

- The mechanism to set the signature based revocation list (SigRL)
  used for signing was changed. `EpidMemberSetSigRl` must be used to
  set the SigRL. The SigRL is no longer a parameter to `EpidSign`.
  This better models typical use case where a device stores a
  revocation list and updates it independently of signing operations.


### Removed

- Removed `EpidWritePreSigs` API. Serialization of pre-computed
  signatures is a risky capability to provide, and simply expanding
  the internal pool via `EpidAddPreSigs` still provides most of the
  optimization benefits.

- The `EpidIsPrivKeyInGroup` API is no longer exposed to clients. It
  is no longer needed because the new member API `EpidAssemblePrivKey`
  performs this check.


### Fixed

- When building with commercial version of the Intel(R) IPP
  Cryptography library, optimized functions are now properly invoked,
  making signing and verification operations ~2 times faster

- SHA-512/256 hash algorithm is now supported.

- README for compressed data now correctly documents the number of
  entries in revocation lists.

- The `verifysig` sample now reports a more clear error message for
  mismatched SigRLs.

- The default scons build will now build for a 32-bit target on a
  32-bit platform.


### Known Issues

- Scons build will not work natively on ARM. You can still build using
  `make` or cross compile.


## [3.0.0] - 2016-11-22

### Added

- Support for verification of Intel(R) EPID 1.1 members.

- Make-based build system support.

- Sample material includes compressed keys.

- Enhanced documentation, including step-by-step walkthroughs of
  example applications.

- Validated on additional IoT platforms.


### Changes

- A new verifier API has been added to set the basename to be used for
  verification. Verifier APIs that used to accept basenames now use
  the basename set via `EpidVerifierSetBasename`.

- The verifier pre-computation structure has been changed to include
  the group ID to allow detection of errors that result from providing
  a pre-computation blob from a different group to
  `EpidVerifierCreate`.


### Fixes

- The kEpidxxxRevoked enums have been renamed to be consistent with
  other result return values.


### Known Issues

- SHA-512/256 hash algorithm is not supported.


## [2.0.0] - 2016-07-20

### Added

- Signed binary issuer material support.

  - Binary issuer material validation APIs.

  - Updated sample issuer material.

  - Updated samples that parse signed binary issuer material.

- Compressed member private key support.

- Validated on additional IoT platforms.

  - Windows 10 IoT Core

  - WindRiver IDP


### Changed

- The default hash algorithm has changed. It is now SHA-512.

- Functions that returned `EpidNullPtrErr` now return `EpidBadArgErr`
  instead.


### Fixed

- Updated build flags to work around GCC 4.8.5 defect.


## [1.0.0] - 2016-03-03

### Added

- Basic sign and verify functionality

- Dynamic join support for member

- Apache 2.0 License

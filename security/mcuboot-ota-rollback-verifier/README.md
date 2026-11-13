# MCUboot Cryptographic Secure Boot & Dual-Slot A/B Anti-Rollback OTA Verifier

C++17 embedded bootloader policy engine modeling **MCUboot (`0x96F3B83D`)** dual-slot (`Slot A / Slot B`) firmware authentication, **eFuse/OTP Security Version Number (SVN) anti-rollback protection**, and **fail-safe watchdog rollback** on unconfirmed OTA updates.

## 1. Verified Bootloader Decision Matrix

| OTA Update Scenario | Slot B SVN | ECDSA P-256 Sig | `image_ok` Self-Test | Bootloader Verdict |
| :--- | :---: | :---: | :---: | :---: |
| **1. Signed Upgrade (`SVN 4 -> 5`)** | `5` (`>= OTP 4`) | Valid | Confirmed (`true`) | **`SWAP_TO_SLOT_B`** |
| **2. Corrupted / Forged Binary** | `5` | Invalid (`false`) | `false` | **`REJECTED [BAD_ECDSA_SIG]`** |
| **3. Version Downgrade Attack** | `2` (`< OTP 4`) | Valid | `true` | **`REJECTED [ANTI_ROLLBACK_OTP]`** |
| **4. Post-OTA Boot Loop / Watchdog Reset** | `5` | Valid | Unconfirmed (`false`) | **`REVERTED [SAFE_FALLBACK_SLOT_A]`** |

## 2. Resume / CV Bullet
* **Implemented a C++17 dual-slot A/B Secure Boot and OTA update policy engine modeled on MCUboot, enforcing ECDSA P-256 signature validation, monotonic OTP security version anti-rollback counters, and hardware-watchdog fallback recovery.**

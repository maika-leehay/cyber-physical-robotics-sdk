#pragma once
#include <cstdint>

namespace mechatronics {

constexpr uint32_t MCUBOOT_MAGIC = 0x96F3B83DU;

enum class BootloaderVerdict : uint8_t {
    BOOT_PRIMARY_SLOT_A        = 0,
    SWAP_TO_UPDATED_SLOT_B     = 1,
    REJECT_INVALID_MAGIC_SIG   = 2,
    REJECT_SVN_ROLLBACK_ATTACK = 3,
    REVERT_FAILED_SELF_TEST_B  = 4
};

struct FirmwareSlotHeader {
    uint32_t magic;            // Must equal 0x96F3B83D
    uint32_t security_version; // Monotonic anti-rollback counter (SVN)
    bool     ecdsa_sig_valid;  // ECDSA P-256 signature verification result
    bool     image_ok_flag;    // Runtime self-test confirmation flag
};

class McubootDualSlotVerifier {
public:
    explicit McubootDualSlotVerifier(uint32_t otp_min_svn = 4)
        : otp_min_svn_(otp_min_svn) {}

    BootloaderVerdict selectBootSlot(const FirmwareSlotHeader& slot_a, const FirmwareSlotHeader& slot_b, bool trial_boot_failed) const {
        if (slot_b.magic != MCUBOOT_MAGIC || !slot_b.ecdsa_sig_valid) {
            return BootloaderVerdict::REJECT_INVALID_MAGIC_SIG;
        }
        if (slot_b.security_version < otp_min_svn_) {
            return BootloaderVerdict::REJECT_SVN_ROLLBACK_ATTACK;
        }
        if (trial_boot_failed && !slot_b.image_ok_flag) {
            return BootloaderVerdict::REVERT_FAILED_SELF_TEST_B;
        }
        if (slot_b.security_version >= slot_a.security_version) {
            return BootloaderVerdict::SWAP_TO_UPDATED_SLOT_B;
        }
        return BootloaderVerdict::BOOT_PRIMARY_SLOT_A;
    }

private:
    uint32_t otp_min_svn_;
};

} // namespace mechatronics

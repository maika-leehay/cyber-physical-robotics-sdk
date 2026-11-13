#include "../include/mcuboot_ota_verifier.hpp"
#include <iostream>

int main() {
    using namespace mechatronics;
    McubootDualSlotVerifier bootloader(/*otp_min_svn=*/ 4);

    FirmwareSlotHeader slot_a_active = { MCUBOOT_MAGIC, 4, true, true };
    FirmwareSlotHeader slot_b_valid  = { MCUBOOT_MAGIC, 5, true, true };
    FirmwareSlotHeader slot_b_tamper = { MCUBOOT_MAGIC, 5, false, false };
    FirmwareSlotHeader slot_b_downgd = { MCUBOOT_MAGIC, 2, true, true };
    FirmwareSlotHeader slot_b_brick  = { MCUBOOT_MAGIC, 5, true, false };

    auto v1 = bootloader.selectBootSlot(slot_a_active, slot_b_valid,  false);
    auto v2 = bootloader.selectBootSlot(slot_a_active, slot_b_tamper, false);
    auto v3 = bootloader.selectBootSlot(slot_a_active, slot_b_downgd, false);
    auto v4 = bootloader.selectBootSlot(slot_a_active, slot_b_brick,  true);

    std::cout << "=========================================================================\n";
    std::cout << " MCUboot Secure Boot & Dual-Slot A/B Anti-Rollback OTA Verifier\n";
    std::cout << "=========================================================================\n"
              << "  * Test 1 (Valid Signed SVN=5 OTA Update)   : " << (v1 == BootloaderVerdict::SWAP_TO_UPDATED_SLOT_B ? "SWAP_TO_SLOT_B" : "FAIL") << "\n"
              << "  * Test 2 (Tampered ECDSA Signature)        : " << (v2 == BootloaderVerdict::REJECT_INVALID_MAGIC_SIG ? "REJECTED [BAD_ECDSA_SIG]" : "FAIL") << "\n"
              << "  * Test 3 (SVN=2 Version Downgrade Attack)  : " << (v3 == BootloaderVerdict::REJECT_SVN_ROLLBACK_ATTACK ? "REJECTED [ANTI_ROLLBACK_OTP]" : "FAIL") << "\n"
              << "  * Test 4 (Unconfirmed Trial Image Watchdog): " << (v4 == BootloaderVerdict::REVERT_FAILED_SELF_TEST_B ? "REVERTED [SAFE_FALLBACK_SLOT_A]" : "FAIL") << "\n";

    return (v1 == BootloaderVerdict::SWAP_TO_UPDATED_SLOT_B &&
            v2 == BootloaderVerdict::REJECT_INVALID_MAGIC_SIG &&
            v3 == BootloaderVerdict::REJECT_SVN_ROLLBACK_ATTACK &&
            v4 == BootloaderVerdict::REVERT_FAILED_SELF_TEST_B) ? 0 : 1;
}

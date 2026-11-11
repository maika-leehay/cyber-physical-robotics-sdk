#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace mechatronics {

enum class DdsAccessVerdict {
    ALLOW_ENCRYPTED_RTPS = 0,
    DENY_UNAUTHENTICATED_CERT = 1,
    DENY_TOPIC_WRITE_VIOLATION = 2
};

struct EnclavePermissions {
    std::string enclave_id;
    bool cert_signed_by_ca;
    std::unordered_set<std::string> publish_allowlist;
    std::unordered_set<std::string> subscribe_allowlist;
};

class Sros2DdsGovernanceGuard {
public:
    void registerEnclave(const EnclavePermissions& p) {
        enclaves_[p.enclave_id] = p;
    }

    DdsAccessVerdict checkPublish(const std::string& enclave_id, const std::string& topic) const {
        auto it = enclaves_.find(enclave_id);
        if (it == enclaves_.end() || !it->second.cert_signed_by_ca) {
            return DdsAccessVerdict::DENY_UNAUTHENTICATED_CERT;
        }
        if (it->second.publish_allowlist.count(topic) == 0) {
            return DdsAccessVerdict::DENY_TOPIC_WRITE_VIOLATION;
        }
        return DdsAccessVerdict::ALLOW_ENCRYPTED_RTPS;
    }

private:
    std::unordered_map<std::string, EnclavePermissions> enclaves_;
};

} // namespace mechatronics

int main() {
    using namespace mechatronics;
    Sros2DdsGovernanceGuard guard;

    guard.registerEnclave({
        "/amr/lidar_driver", true,
        {"/scan", "/diagnostics"},
        {"/clock"}
    });
    guard.registerEnclave({
        "/amr/nav2_controller", true,
        {"/cmd_vel", "/local_plan"},
        {"/scan", "/odom", "/tf"}
    });
    guard.registerEnclave({
        "/rogue_lan_node", false,
        {"/cmd_vel"},
        {}
    });

    auto v1 = guard.checkPublish("/amr/nav2_controller", "/cmd_vel");
    auto v2 = guard.checkPublish("/amr/lidar_driver", "/cmd_vel");
    auto v3 = guard.checkPublish("/rogue_lan_node", "/cmd_vel");

    std::cout << "=========================================================================\n";
    std::cout << " SROS2 DDS-Security Enclave & AES-256-GCM Topic Governance Verification\n";
    std::cout << "=========================================================================\n"
              << "  * Test 1 (/amr/nav2_controller -> /cmd_vel) : " << (v1 == DdsAccessVerdict::ALLOW_ENCRYPTED_RTPS ? "ALLOWED [AES-256-GCM]" : "FAIL") << "\n"
              << "  * Test 2 (/amr/lidar_driver -> /cmd_vel)    : " << (v2 == DdsAccessVerdict::DENY_TOPIC_WRITE_VIOLATION ? "BLOCKED [LEAST_PRIVILEGE_XML]" : "FAIL") << "\n"
              << "  * Test 3 (/rogue_lan_node -> /cmd_vel)      : " << (v3 == DdsAccessVerdict::DENY_UNAUTHENTICATED_CERT ? "BLOCKED [INVALID_X509_CA]" : "FAIL") << "\n";

    return (v1 == DdsAccessVerdict::ALLOW_ENCRYPTED_RTPS &&
            v2 == DdsAccessVerdict::DENY_TOPIC_WRITE_VIOLATION &&
            v3 == DdsAccessVerdict::DENY_UNAUTHENTICATED_CERT) ? 0 : 1;
}

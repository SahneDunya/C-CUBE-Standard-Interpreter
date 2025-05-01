#include "method_info.hpp"
#include <sstream> // toString için

namespace CCube {
namespace Runtime {

// MethodInfo string gösterimi (hata ayıklama için)
std::string MethodInfo::toString() const {
    std::stringstream ss;
    ss << "MethodInfo(Name: '" << name << "'";
    if (signature) {
        ss << ", Signature: " << signature->toString(); // FunctionType::toString gerektirir
    } else {
        ss << ", Signature: <null>";
    }
    ss << ", EntryPoint: " << entry_point;
    // TODO: Diğer alanları da ekle (ID, access)
    ss << ")";
    return ss.str();
}

// Kurucu implementasyonu başlıkta inline yapıldı.
// Diğer metotlar buraya eklenecektir.

} namespace Runtime
} namespace CCube
#include "target_info.hpp"

namespace CCube {

// Architecture enum değerini stringe çevirir
std::string architectureToString(Architecture arch) {
    switch (arch) {
        case Architecture::UNKNOWN: return "unknown";
        case Architecture::X86_32: return "x86_32";
        case Architecture::X86_64: return "x86_64";
        case Architecture::ARM32: return "arm32";
        case Architecture::ARM64: return "arm64";
        case Architecture::POWERPC: return "powerpc";
        case Architecture::M68K: return "m68k";
        case Architecture::SPARC: return "sparc";
        case Architecture::WASM: return "wasm";
        default: {
            std::stringstream ss;
            ss << "arch_" << static_cast<int>(arch);
            return ss.str();
        }
    }
}

// OperatingSystem enum değerini stringe çevirir
std::string operatingSystemToString(OperatingSystem os) {
     switch (os) {
        case OperatingSystem::UNKNOWN: return "unknown";
        case OperatingSystem::WINDOWS: return "windows";
        case OperatingSystem::LINUX: return "linux"; // Raspberry Pi OS, Stadia dahil
        case OperatingSystem::MACOS: return "macos"; // OS X dahil
        case OperatingSystem::IOS: return "ios";
        case OperatingSystem::ANDROID: return "android"; // Android BlackBerry dahil
        case OperatingSystem::FREEBSD: return "freebsd";
        case OperatingSystem::NETBSD: return "netbsd";
        case OperatingSystem::OPENBSD: return "openbsd";
        case OperatingSystem::DRAGONFLYBSD: return "dragonflybsd";
        case OperatingSystem::SOLARIS: return "solaris";
        case OperatingSystem::HAIKU: return "haiku";
        case OperatingSystem::TIZEN: return "tizen";
        case OperatingSystem::VXWORKS: return "vxworks";
        case OperatingSystem::QNX: return "qnx"; // BlackBerry PlayBook, BB10 dahil
        case OperatingSystem::HARMONYOS: return "harmonyos"; // OpenHarmony dahil
        case OperatingSystem::FUCHSIA: return "fuchsia";
        case OperatingSystem::NINTENDO_SWITCH: return "nintendo_switch";
        case OperatingSystem::PLAYSTATION_3: return "playstation_3"; // CellOS
        case OperatingSystem::AMIGAOS: return "amigaos";
        default: {
            std::stringstream ss;
            ss << "os_" << static_cast<int>(os);
            return ss.str();
        }
    }
}


// TargetPlatform::toString implementasyonu
std::string TargetPlatform::toString() const {
    std::stringstream ss;
    ss << architectureToString(arch) << "-" << operatingSystemToString(os);
    return ss.str();
}


} namespace CCube
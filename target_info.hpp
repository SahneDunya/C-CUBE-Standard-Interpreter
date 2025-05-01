#ifndef CUBE_TARGET_INFORMATION_TARGET_INFO_HPP
#define CUBE_TARGET_INFORMATION_TARGET_INFO_HPP

#include <string>
#include <sstream> // toString için

namespace CCube {

// Hedef CPU Mimarisi
enum class Architecture {
    UNKNOWN,
    X86_32,     // Intel/AMD 32-bit
    X86_64,     // Intel/AMD 64-bit
    ARM32,      // ARM 32-bit (ARMv7, vb.)
    ARM64,      // ARM 64-bit (AArch64)
    POWERPC,    // PowerPC (PS3 Cell mimarisi dahil)
    M68K,       // Motorola 68k (AmigaOS gibi eski sistemler)
    SPARC,      // SPARC (Solaris)
    WASM,       // WebAssembly
    // TODO: Diğer mimariler eklenecek: RISCV, MIPS, vb.
};

// Hedef İşletim Sistemi veya Ortam
enum class OperatingSystem {
    UNKNOWN,
    WINDOWS,        // Windows (95 ve üzeri sürümler dahil: 95, 98, ME, NT, 2000, XP, Vista, 7, 8, 10, 11)
    LINUX,          // Linux (Çeşitli dağıtımlar, Raspberry Pi OS, Stadia dahil)
    MACOS,          // macOS (OS X dahil)
    IOS,            // iOS
    ANDROID,        // Android (Android BlackBerry'ler dahil)
    FREEBSD,        // FreeBSD
    NETBSD,         // NetBSD
    OPENBSD,        // OpenBSD
    DRAGONFLYBSD,   // DragonFly BSD
    SOLARIS,        // Solaris
    HAIKU,          // Haiku
    TIZEN,          // Tizen
    VXWORKS,        // vxWorks
    QNX,            // QNX (BlackBerry PlayBook ve BB10 OS dahil)
    HARMONYOS,      // HarmonyOS (OpenHarmony dahil)
    FUCHSIA,        // Fuchsia
    NINTENDO_SWITCH,// Nintendo Switch
    PLAYSTATION_3,  // PlayStation 3 (CellOS)
    AMIGAOS,        // AmigaOS
    // TODO: Diğer OS/Ortamlar eklenecek: tvOS, watchOS, ...
};

// Derleme veya çalışma zamanı hedef platformunu temsil eden yapı
struct TargetPlatform {
    Architecture arch = Architecture::UNKNOWN;
    OperatingSystem os = OperatingSystem::UNKNOWN;

    // Platformun geçerli olup olmadığını kontrol eder
    bool isValid() const { return arch != Architecture::UNKNOWN && os != OperatingSystem::UNKNOWN; }

    // Hedef platformun string gösterimini döndürür (örn: "x86_64-linux")
    std::string toString() const; // Implementasyonu .cpp'de

    // TODO: İleride bytecode formatı versiyonu, ABI (Application Binary Interface) bilgisi gibi detaylar eklenebilir.
};

// Architecture enum değerini stringe çevirir
std::string architectureToString(Architecture arch);

// OperatingSystem enum değerini stringe çevirir
std::string operatingSystemToString(OperatingSystem os);


} namespace CCube

#endif // CUBE_TARGET_INFORMATION_TARGET_INFO_HPP
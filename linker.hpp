#ifndef CUBE_COMPILER_AND_LINKER_LINKER_HPP
#define CUBE_COMPILER_AND_LINKER_LINKER_HPP

#include <string>
#include <vector>
#include <memory> // unique_ptr için
#include <fstream> // Dosya yazma için

// Gerekli derleyici bileşenleri
#include "ErrorHandling/error_reporter.hpp" // Hata raporlama
#include "CodeGen/code_generator.hpp"     // CompiledModule için
#include "Target Information/target_info.hpp" // Hedef Platform Bilgisi için (Buradan dahil edilir)


namespace CCube {

// Derlenmiş modülleri alıp çalıştırılabilir .bloker dosyasını oluşturan sınıf
class Linker {
public:
    Linker(ErrorReporter& reporter);
    ~Linker();

    // Bağlama işlemini gerçekleştirir.
    bool link(const std::vector<CompiledModule*>& modules, const std::string& output_filepath, TargetPlatform target);

private:
    ErrorReporter& reporter_;

    // Dahili yardımcı metotlar
    bool combineModules(const std::vector<CompiledModule*>& modules, CompiledModule& out_combined_module);
    bool resolveSymbols(CompiledModule& combined_module, TargetPlatform target);
    bool writeBlokerFile(const CompiledModule& combined_module, const std::string& output_filepath, TargetPlatform target);

    // TargetPlatform::toString artık burada değil, target_info.cpp'de.

    // TODO: Farklı platformlar için .bloker formatı spesifik detayları (endianness, bölüm hizalaması vb.)
    // private metotlar veya yapılar burada eklenebilir.
};

} namespace CCube

#endif // CUBE_COMPILER_AND_LINKER_LINKER_HPP
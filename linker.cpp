#include "linker.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>
#include <algorithm>

// TODO: Standart Kütüphane Çalışma Zamanı (Runtime) referansları veya sembol tabloları buraya dahil edilmelidir.
// Linker, CALL instruction'larındaki SymbolIndex'leri runtime'daki gerçek adreslere/ID'lere eşleyecektir.

// TODO: Farklı platformlar için .bloker formatı detayları (Byte order, section alignment vb.)
// Platforma özel include'lar veya mantık burada yer alacaktır.
// `#ifdef CUBE_TARGET_OS_WINDOWS` gibi makrolar kullanılabilir.


namespace CCube {

// Linker Kurucu
Linker::Linker(ErrorReporter& reporter)
    : reporter_(reporter) {
    std::cout << "Linker created." << std::endl;
}

// Linker Yıkıcı
Linker::~Linker() {
    std::cout << "Linker destroyed." << std::endl;
}

// Bağlama işlemini gerçekleştirir.
bool Linker::link(const std::vector<CompiledModule*>& modules, const std::string& output_filepath, TargetPlatform target) {
    if (!target.isValid()) {
        reporter_.reportError({0,0}, ErrorCode::INTERNAL_ERROR, "Invalid target platform specified for linking.");
        return false;
    }
     if (modules.empty()) {
         reporter_.reportError({0,0}, ErrorCode::INTERNAL_ERROR, "No compiled modules provided for linking.");
         return false;
     }

    std::cout << "Starting linking for target: " << target.toString() << std::endl;


    // --- 1. Modülleri Birleştir ---
    CompiledModule combined_module;
    if (!combineModules(modules, combined_module)) {
        std::cerr << "Linking failed during module combination." << std::endl;
        return false;
    }
    std::cout << "Modules combined." << std::endl;


    // --- 2. Sembol Referanslarını Çözümle ---
    if (!resolveSymbols(combined_module, target)) {
        std::cerr << "Linking failed due to unresolved symbols." << std::endl;
        return false;
    }
    std::cout << "Symbols resolved." << std::endl;


    // --- 3. .bloker Dosyasını Yaz ---
    if (!writeBlokerFile(combined_module, output_filepath, target)) {
        std::cerr << "Linking failed during file writing." << std::endl;
        return false;
    }
    std::cout << "Output file '" << output_filepath << "' generated." << std::endl;


    // --- 4. Sonuç ---
    if (reporter_.hasErrors()) {
        std::cerr << "Linking finished with errors." << std::endl;
        return false;
    } else {
        std::cout << "Linking successful!" << std::endl;
        return true;
    }
}

// Dahili yardımcı metot implementasyonları (Önceki implementasyonlar buraya taşınır)

// Modülleri birleştirir (Placeholder - platforma göre farklılıklar olabilir)
bool Linker::combineModules(const std::vector<CompiledModule*>& modules, CompiledModule& out_combined_module) {
     std::cout << "Combining modules (placeholder, platform nuances ignored)..." << std::endl;

     // String literal havuzlarını birleştirme (genellikle platformdan bağımsız)
     std::unordered_map<std::string, size_t> combined_string_literal_map;
     for (const auto& module : modules) {
         if (!module) continue;
         for (const auto& str : module->string_literals) {
             if (combined_string_literal_map.find(str) == combined_string_literal_map.end()) {
                 size_t new_index = out_combined_module.string_literals.size();
                 out_combined_module.string_literals.push_back(str);
                 combined_string_literal_map[str] = new_index;
             }
         }
     }

     // Fonksiyonları birleştirme ve Basic Block indexlerini güncelleme
     // BU KISIM PLATFORMA ÖZEL ABİ DETAYLARINA GİRMEYECEK (SADECE GENEL YAPI)
     int current_block_offset = 0;
     for (const auto& module : modules) {
         if (!module) continue;
         // Function'ları kopyala veya taşı
         for (const auto& func_uptr : module->functions) {
             auto new_func = std::make_unique<Function>();
             new_func->symbol = func_uptr->symbol; // Symbol shared_ptr
             new_func->basic_blocks.reserve(func_uptr->basic_blocks.size()); // Yer ayır

             // Basic Block'ları taşı ve geçici olarak yeniden indexle
             int func_block_offset = 0;
             for (const auto& block_uptr : func_uptr->basic_blocks) {
                  auto new_block = std::make_unique<BasicBlock>(block_uptr->id != -1 ? "block_" + std::to_string(block_uptr->id) : "block");
                  new_block->id = current_block_offset + func_block_offset; // Yeni genel ID
                  // Komutları kopyala (Operand indexleri hala eski olabilir)
                  new_block->instructions = block_uptr->instructions;
                  new_func->basic_blocks.push_back(std::move(new_block));
                  func_block_offset++;
             }
             out_combined_module.functions.push_back(std::move(new_func));
             current_block_offset += func_block_offset;
         }
     }

    // TODO: Birleştirilmiş komutlardaki eski Basic Block ve String Index referanslarını düzeltme.
    // Bu, orijinal modülün yapısı ve kombine modüldeki yeni konumlar arasındaki eşlemeyi kullanarak yapılır.
    // Bu oldukça detaylı ve hataya açıktır.

     return !reporter_.hasErrors(); // Placeholder başarı durumu
}


// Sembol referanslarını çözer (Placeholder - platforma ve Runtime'a bağımlı)
bool Linker::resolveSymbols(CompiledModule& combined_module, TargetPlatform target) {
     std::cout << "Resolving symbols (placeholder, platform/runtime specifics ignored)..." << std::endl;

     // Sembol çözme, CALL (SymbolIndex) ve LOAD/STORE_VAR (VarIndex) gibi komutlardaki operandları günceller.
     // SymbolIndex'ler: Birleştirilmiş modül içi fonksiyon çağrıları için Function/Block Indexlerine,
     // Harici (stdlib) çağrılar için Runtime External ID'lere çevrilir.
     // VarIndex'ler: Global değişkenler için Data Section offsetlerine çevrilir.

     // Örnek Runtime External ID eşleme (target'a göre değişebilir)
     std::unordered_map<std::string, int> runtime_external_symbol_ids_example = {
         // print, input, vb. stdlib fonksiyonlarının runtime ID'leri
         {"__builtin_print", 1},
         {"__builtin_input", 2},
         // TODO: Diğer stdlib fonksiyonları ve harici C/sistem fonksiyonları
         // Her platform için farklı ID'ler veya isimler olabilir.
     };

     // TODO: CombinedModule içindeki SymbolIndex'lerin karşılık geldiği Symbol nesnelerini bulma mekanizması.
     // CompiledModule'ün SymbolTable bilgisi taşıması gerekebilir.

     for (const auto& func_uptr : combined_module.functions) {
         for (const auto& block_uptr : func_uptr->basic_blocks) {
             for (auto& instruction : block_uptr->instructions) {
                 for (auto& operand : instruction.operands) {
                     if (operand.type == OperandType::SYMBOL_INDEX) {
                         // TODO: SymbolIndex'in karşılık geldiği Symbol nesnesini bul.
                          std::shared_ptr<Symbol> symbol = findSymbolByCompiledIndex(operand.value.index); // Placeholder

                         // TODO: Sembolün adını al (symbol->name)
                          std::string symbol_name = symbol->name;

                         // TODO: Sembol kombine modül içinde tanımlı bir fonksiyon mu (inter-module call)?
                         // TODO: Veya harici (stdlib/sistem) bir fonksiyon mu?

                         // Harici ise runtime ID'sine çevir.
                          if (runtime_external_symbol_ids_example.count(symbol_name)) {
                             operand.type = OperandType::INT; // veya yeni bir RUNTIME_EXT_ID tipi
                             operand.value.i = runtime_external_symbol_ids_example[symbol_name];
                          } else {
                             reporter_.reportError({0,0}, ErrorCode::SEMANTIC_UNDEFINED_FUNCTION, "Unresolved external symbol: '" + symbol_name + "'.");
                             // Hata durumunda operandı geçersiz yap veya özel bir hata ID'si ata.
                          }

                         // Modül içi ise, hedef fonksiyonun giriş Basic Block'unun indexine çevir.
                         // Bu, fonksiyon listesini birleştirdikten sonra yapılmalıdır.
                     }
                      // TODO: VarIndex operandları (global değişkenler için) Data Section offsetlerine çevrilir.
                      // TODO: StringIndex operandları (eğer CombineModules'ta düzeltilmediyse) düzeltilir.
                 }
             }
         }
     }

     // TODO: Çözümlenemeyen sembol olup olmadığını kontrol et.

     return !reporter_.hasErrors(); // Placeholder başarı durumu
}


// Birleştirilmiş modülü ve meta veriyi .bloker formatına seri hale getirir ve dosyaya yazar.
bool Linker::writeBlokerFile(const CompiledModule& combined_module, const std::string& output_filepath, TargetPlatform target) {
     std::cout << "Writing .bloker file (placeholder, format details ignored)..." << std::endl;

    std::ofstream outfile(output_filepath, std::ios::binary);
    if (!outfile) {
        reporter_.reportError({0,0}, ErrorCode::INTERNAL_ERROR, "Failed to open output file for writing: " + output_filepath);
        return false;
    }

    // --- .bloker Dosya Formatı (Placeholder) ---
    // Gerçek format platforma göre değişebilir (endianness, hizalama, bölüm yapıları vb.)

    // Sihirli sayı ve versiyon (Endianness'e dikkat!)
    uint32_t magic_number = 0x43554245; // "CUBE"
    uint32_t version = 1;
    // TODO: Hedef platformu binary olarak yaz (Architecture ve OperatingSystem enum değerleri)
     uint32_t target_arch = static_cast<uint32_t>(target.arch);
     uint32_t target_os = static_cast<uint32_t>(target.os);

    outfile.write(reinterpret_cast<const char*>(&magic_number), sizeof(magic_number));
    outfile.write(reinterpret_cast<const char*>(&version), sizeof(version));
     outfile.write(reinterpret_cast<const char*>(&target_arch), sizeof(target_arch));
     outfile.write(reinterpret_cast<const char*>(&target_os), sizeof(target_os));

    // TODO: Entry point (örn: "__global_main" fonksiyonunun ilk Basic Block indexi veya runtime ID'si) yazılacak.
     int entry_point_block_index = findEntryPointBlock(combined_module); // Placeholder
     outfile.write(reinterpret_cast<const char*>(&entry_point_block_index), sizeof(entry_point_block_index));


    // String Literal Havuzunu Yaz
    uint32_t string_pool_size = combined_module.string_literals.size();
    outfile.write(reinterpret_cast<const char*>(&string_pool_size), sizeof(string_pool_size));
    for (const auto& str : combined_module.string_literals) {
        uint32_t str_len = str.length();
        outfile.write(reinterpret_cast<const char*>(&str_len), sizeof(str_len));
        outfile.write(str.c_str(), str_len);
        // TODO: Hizalama (padding) gerekebilir
    }


    // Fonksiyonları ve Basic Block'ları (IR komutlarını) Yaz
    uint32_t function_count = combined_module.functions.size();
    outfile.write(reinterpret_cast<const char*>(&function_count), sizeof(function_count));

    for (const auto& func_uptr : combined_module.functions) {
        const auto& func = *func_uptr;
        // TODO: Fonksiyon bilgisini yaz (Symbol ID veya ismi, parametre sayısı vb.)
        // Fonksiyonun sembolünü nasıl temsil edeceğiniz runtime'a bağlı.
        uint32_t block_count = func.basic_blocks.size();
        outfile.write(reinterpret_cast<const char*>(&block_count), sizeof(block_count));

        for (const auto& block_uptr : func.basic_blocks) {
            const auto& block = *block_uptr;
             // TODO: Basic Block bilgisini yaz (ID, bağlantılar - eğer format gerektiriyorsa)
            uint32_t instruction_count = block.instructions.size();
            outfile.write(reinterpret_cast<const char*>(&instruction_count), sizeof(instruction_count));

            for (const auto& instruction : block.instructions) {
                // Opcode'u yaz
                uint32_t opcode_val = static_cast<uint32_t>(instruction.opcode);
                outfile.write(reinterpret_cast<const char*>(&opcode_val), sizeof(opcode_val));

                // Operand sayısını yaz
                uint32_t operand_count = instruction.operands.size();
                outfile.write(reinterpret_cast<const char*>(&operand_count), sizeof(operand_count));

                // Operandları yaz
                for (const auto& operand : instruction.operands) {
                    uint32_t operand_type_val = static_cast<uint32_t>(operand.type);
                    outfile.write(reinterpret_cast<const char*>(&operand_type_val), sizeof(operand_type_val));

                    // Operand değerini tipe göre yaz (Endianness'e dikkat!)
                    switch (operand.type) {
                        case OperandType::INT: outfile.write(reinterpret_cast<const char*>(&operand.value.i), sizeof(operand.value.i)); break;
                        case OperandType::FLOAT: outfile.write(reinterpret_cast<const char*>(&operand.value.f), sizeof(operand.value.f)); break;
                        case OperandType::BOOL: { uint8_t b = operand.value.b ? 1 : 0; outfile.write(reinterpret_cast<const char*>(&b), sizeof(b)); break; }
                        case OperandType::STRING_INDEX: outfile.write(reinterpret_cast<const char*>(&operand.value.index), sizeof(operand.value.index)); break;
                        case OperandType::VAR_INDEX: outfile.write(reinterpret_cast<const char*>(&operand.value.index), sizeof(operand.value.index)); break;
                        case OperandType::BLOCK_INDEX: outfile.write(reinterpret_cast<const char*>(&operand.value.index), sizeof(operand.value.index)); break;
                        case OperandType::SYMBOL_INDEX: outfile.write(reinterpret_cast<const char*>(&operand.value.index), sizeof(operand.value.index)); break; // Bu zaten çözümlenmiş olmalıydı (RUNTIME_EXT_ID veya FUNCTION_INDEX)
                        // TODO: Çözümlenmiş semboller için farklı operand tipleri ve yazma mantığı
                        case OperandType::NONE: break; // Değer yok
                    }
                    // TODO: Operandlar için hizalama (padding) gerekebilir.
                }
            }
            // TODO: Basic Block sonu için hizalama (padding) gerekebilir.
        }
        // TODO: Fonksiyon sonu için hizalama (padding) gerekebilir.
    }

    // TODO: Diğer veri bölümlerini yaz (global değişken değerleri vb.)
    // TODO: Sembol tablosu veya hata ayıklama bilgileri (isteğe bağlı)
    // TODO: Dosya sonu belirteci veya padding

    outfile.close();

    if (outfile.good()) {
        return true; // Dosya yazma başarılı
    } else {
        reporter_.reportError({0,0}, ErrorCode::INTERNAL_ERROR, "Error writing to output file: " + output_filepath);
        return false;
    }
}


// TargetPlatform::toString implementasyonu zaten target_info.cpp'de.


} namespace CCube
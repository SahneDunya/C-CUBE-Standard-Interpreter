#include "compiler.hpp"
#include <iostream> // Hata ayıklama çıktıları için
#include <fstream>  // Dosyadan okuma/yazma için (gelecekte kullanılabilir)

// TODO: Gerçek CodeGenerator implementasyonunu dahil edin.
#include "CodeGen/MyConcreteCodeGenerator.hpp" // Örneğin, LLVM kullanıyorsa veya kendi generator'ınız.

namespace CCube {

// Compiler Kurucu
Compiler::Compiler() {
    // Hata raporlayıcıyı başlat
    error_reporter_ = std::make_unique<ErrorReporter>();
    // Diğer bileşenler genellikle compile metodu içinde, kaynak koda göre başlatılır.
    std::cout << "Compiler created." << std::endl; // Hata ayıklama
}

// Compiler Yıkıcı
Compiler::~Compiler() {
    // unique_ptr'lar scope dışına çıkınca otomatik temizlenir.
    std::cout << "Compiler destroyed." << std::endl; // Hata ayıklama
}

// Kaynak kodu derler.
bool Compiler::compile(const std::string& source_code, const std::string& filename_hint) {
    // --- 1. Hazırlık ---
    // Önceki derlemeden kalan hataları temizle ve hata raporlayıcıyı sıfırla (gerekirse)
    error_reporter_ = std::make_unique<ErrorReporter>(); // Yeni bir hata raporlayıcı oluştur

    std::cout << "Starting compilation..." << std::endl; // Hata ayıklama
    if (!filename_hint.empty()) {
        std::cout << "Source file: " << filename_hint << std::endl;
    }


    // --- 2. Sözdizimsel Analiz (Lexing) ---
    std::cout << "Lexing..." << std::endl; // Hata ayıklama
    try {
        lexer_ = std::make_unique<Lexer>(source_code);
        // Lexer hataları genellikle Parser tarafından consume edilirken yakalanır veya lexer doğrudan ErrorReporter kullanır.
        // Eğer lexer constructor'da veya token okurken hata raporluyorsa, error_reporter_ kontrol edilebilir.
    } catch (const std::exception& e) {
        // Lexer başlatma hatası
        error_reporter_->reportError({1, 1}, ErrorCode::INTERNAL_ERROR, "Lexer initialization failed: " + std::string(e.what()));
        return false; // Fatal hata
    }


    // --- 3. Ayrıştırma (Parsing) ---
    std::cout << "Parsing..." << std::endl; // Hata ayıklama
    parser_ = std::make_unique<Parser>(*lexer_); // Parser lexer'a referans alır

    try {
        ast_ = parser_->parseProgram(); // Program AST'sini oluştur
        // Parser hataları Parser sınıfı içinde ParsingError istisnası fırlatabilir veya ErrorReporter kullanabilir.
        // Eğer ParsingError fırlatıyorsa burada yakalanır.
    } catch (const ParsingError& e) {
        // Parsing sırasında yakalanan ilk fatal hata
        error_reporter_->reportError(e.code, e.token.location, e.what()); // Hata kodunu ve konumu ParsingError'dan al
        // Senkronizasyon mekanizması yoksa fatal kabul edilebilir.
        // Parser senkronize oluyorsa, parseProgram tamamlanabilir ancak error_reporter_ içinde hatalar birikmiş olur.
        std::cerr << e.what_with_location() << std::endl; // ParsingError'un detaylı mesajını yazdır
    } catch (const std::exception& e) {
         // Diğer parser hataları
         error_reporter_->reportError({1, 1}, ErrorCode::INTERNAL_ERROR, "Parser failed: " + std::string(e.what()));
         return false; // Fatal hata
    }

    // Parsing hataları olduysa (ParsingError yakalandı veya ErrorReporter'da birikti)
    if (error_reporter_->hasErrors()) {
        std::cerr << "Compilation failed due to parsing errors." << std::endl; // Hata ayıklama
        // Eğer fatal kabul ediliyorsa veya AST geçerli değilse burada durulabilir.
         if (!ast_ || is_fatal_parsing_error) return false;
    }

    // --- 4. Anlamsal Analiz (Semantic Analysis) ---
    // Eğer parsing başarılı olduysa (ast_ geçerli ise) semantik analize geç
    if (ast_ && !error_reporter_->hasErrors()) { // Basit kontrol: Fatal parsing hatası yoksa devam et
        std::cout << "Semantic Analysis..." << std::endl; // Hata ayıklama
        semantic_analyzer_ = std::make_unique<SemanticAnalyzer>(*ast_, *error_reporter_); // Semantik analizci AST ve ErrorReporter'a referans alır

        try {
             semantic_analyzer_->analyze(); // Analizi yap
        } catch (const std::exception& e) {
             // Semantik analiz sırasında beklenmedik hata
             error_reporter_->reportError({1, 1}, ErrorCode::INTERNAL_ERROR, "Semantic analysis failed: " + std::string(e.what()));
             return false; // Fatal hata
        }

        // Semantik analiz hataları olduysa
        if (error_reporter_->hasErrors()) {
            std::cerr << "Compilation failed due to semantic errors." << std::endl; // Hata ayıklama
            // Eğer fatal semantik hatalar olduysa burada durulabilir.
             if (has_fatal_semantic_errors) return false;
        }
    } else {
         std::cerr << "Skipping semantic analysis due to parsing errors." << std::endl; // Hata ayıklama
         return false; // Parsing hataları nedeniyle dur
    }


    // --- 5. Kod Üretimi (Code Generation) ---
    // Eğer semantik analiz başarılı olduysa (veya sadece uyarılar varsa) kod üretimine geç
    // (SemanticAnalyzer hata raporlayıcıyı doldurur, hasErrors() kontrol edilir)
    if (ast_ && !error_reporter_->hasErrors()) { // Basit kontrol: Fatal semantik hatası yoksa devam et
        std::cout << "Code Generation..." << std::endl; // Hata ayıklama
        // TODO: Gerçek CodeGenerator sınıfını başlatın ve semantic analysis sonuçlarını (AST, SymbolTable, TypeSystem) ona paslayın.
        // SymbolTable SemanticAnalyzer'ın içinde, TypeSystem singleton.
        // CodeGenerator'ın kurucusunun bu bilgilere erişmesi gerekecek.
         code_generator_ = std::make_unique<MyConcreteCodeGenerator>(*ast_, semantic_analyzer_->getSymbolTable(), TypeSystem::getInstance(), *error_reporter_);

        // Şimdilik sadece bir placeholder oluşturuyoruz:
        // CodeGenerator arayüzü (control_flow_generator.hpp içinde tanımlı)
        // Bu satır derlenmeyecektir çünkü CodeGenerator soyut arayüzdür veya CodeGen modülünüzden somut bir sınıf gerektirir.
         code_generator_ = std::make_unique<CodeGenerator>(*ast_, *error_reporter_); // HATA: Soyut sınıf

        // Gerçek implementasyonda:
         code_generator_ = std::make_unique<MyConcreteCodeGenerator>(... gerekli bağımlılıklar ...);
         code_generator_->generate(*ast_); // CodeGenerator'a kod üretme işlemini başlatmasını söyle.
         reporter_.reportWarning({0,0}, ErrorCode::INTERNAL_ERROR, "Code generation step is a placeholder.");


        // Kod üretimi sırasında hatalar oluşabilir (örn: ulaşılamayan kod, geçersiz IR).
        if (error_reporter_->hasErrors()) {
            std::cerr << "Compilation finished with errors during code generation." << std::endl; // Hata ayıklama
             return false; // Kod üretimi hataları genellikle fataldır
        }

    } else {
         std::cerr << "Skipping code generation due to previous errors." << std::endl; // Hata ayıklama
          return false; // Önceki aşama hataları nedeniyle dur
    }


    // --- 6. Bağlama (Linking) ---
    // Eğer kod üretimi başarılı olduysa (veya sadece uyarılar varsa) bağlama aşamasına geçilir.
    // Linker genellikle CodeGenerator'ın çıktısını alır.
    // TODO: Linker sınıfını başlatın ve kod üreticisinin çıktısını ona verin.
    // Linker sınıfı ayrı bir dosyada (Standard CUBE Compiler and Linker/linker.hpp/cpp) olacaktır.
     auto linker = std::make_unique<Linker>(*error_reporter_);
     bool linking_success = linker->link(code_generator_->getOutput(), "output.bloker", ... hedef platform bilgisi ...);
     if (!linking_success) {
         std::cerr << "Linking failed." << std::endl;
         return false;
     }
     std::cout << "Linking step is a placeholder." << std::endl; // Hata ayıklama


    // --- 7. Sonuç ---
    // Tüm aşamalar tamamlandıktan sonra hata olup olmadığını kontrol et.
    if (error_reporter_->hasErrors()) {
        std::cerr << "Compilation finished with errors." << std::endl; // Hata ayıklama
        return false;
    } else {
        std::cout << "Compilation successful!" << std::endl; // Hata ayıklama
        // TODO: Oluşturulan .bloker dosyasını veya çıktıyı kaydet/işle.
        return true;
    }
}

// Derleme sırasında hata oluşup oluşmadığını kontrol eder.
bool Compiler::hasErrors() const {
    // Hata raporlayıcı NULL değilse ve hata içeriyorsa true
    return error_reporter_ && error_reporter_->hasErrors();
}

// Raporlanan hata ve uyarıların listesini döndürür.
const std::vector<Error>& Compiler::getErrors() const {
    // Hata raporlayıcı NULL ise boş bir liste döndür
    if (!error_reporter_) {
        static const std::vector<Error> empty_errors;
        return empty_errors;
    }
    return error_reporter_->getErrors();
}

// TODO: getGeneratedCode gibi çıktı metotları buraya eklenebilir.


} namespace CCube
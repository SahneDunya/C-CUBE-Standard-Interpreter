#ifndef CUBE_ERROR_HANDLING_ERROR_REPORTER_HPP
#define CUBE_ERROR_HANDLING_ERROR_REPORTER_HPP

#include <string>
#include <vector>
#include <iostream> // Konsola hata yazdırmak için

#include "error_types.hpp" // Yeni ErrorCode tanımı için

namespace CCube {

// Kaynak kodundaki konum bilgisi (Daha önce tanımlanmıştı)
struct SourceLocation {
    int line;
    int column;
    // İsteğe bağlı: std::string file_name;
};

// Bir hatayı veya uyarıyı temsil eden yapı (ErrorCode eklendi)
struct Error {
    ErrorCode code;         // Hatayı/Uyarıyı kategorize eden kod
    SourceLocation location;
    std::string message;    // Detaylı hata mesajı (konum ve bağlam ile oluşturulur)
    bool is_warning;

    // Kurucu
    Error(ErrorCode code, SourceLocation loc, std::string msg, bool is_warning)
        : code(code), location(loc), message(std::move(msg)), is_warning(is_warning) {}
};

// Hata ve uyarıları toplayıp raporlayan sınıf (Metotlar .cpp'de implemente edilecek)
class ErrorReporter {
public:
    ErrorReporter(); // Kurucu .cpp'de

    // Hata raporla (ErrorCode ve konum alır)
    void reportError(ErrorCode code, SourceLocation loc, const std::string& details = "");

    // Uyarı raporla (ErrorCode ve konum alır)
    void reportWarning(ErrorCode code, SourceLocation loc, const std::string& details = "");

    // Hata olup olmadığını kontrol et
    bool hasErrors() const;

    // Raporlanan hata ve uyarı listesini döndür
    const std::vector<Error>& getErrors() const;

    // Raporlanan tüm hata ve uyarıları belirtilen akışa yazdırır
    void printReports(std::ostream& os) const;

private:
    std::vector<Error> errors_;
    bool has_errors_; // Hata oluştuysa true
};

} namespace CCube

#endif // CUBE_ERROR_HANDLING_ERROR_REPORTER_HPP
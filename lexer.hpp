#ifndef CUBE_SYNTAX_LEXER_HPP
#define CUBE_SYNTAX_LEXER_HPP

#include <string>
#include <vector>
#include <unordered_map> // Anahtar kelimeleri hızlı aramak için
#include "token.hpp"     // Token tanımını dahil et

namespace CCube {

// Kaynak kodu karakter karakter okuyup Token'lara ayıran sınıf
class Lexer {
public:
    // Kurucu: Kaynak kodu string olarak alır.
    Lexer(const std::string& source_code);

    // Yıkıcı
    ~Lexer() = default; // Varsayılan yıkıcı yeterli

    // Kaynak kodundan bir sonraki token'ı döndürür ve ilerler.
    Token getNextToken();

    // Bir sonraki token'a bakar ancak ilerlemez (parser için kullanışlı).
    Token peekToken();

private:
    // Kaynak kodu stringi
    const std::string& source_code_;

    // Şu anki okuma pozisyonu
    size_t current_position_;

    // Şu anki satır ve sütun numaraları (hata raporlama için)
    int current_line_;
    int current_column_;

    // peekToken için kullanılan tampon token (ileri bakış)
    Token buffered_token_;
    bool has_buffered_token_;

    // Anahtar kelimelerin hızlı lookup'ı için harita
    std::unordered_map<std::string, TokenType> keywords_;

    // Yardımcı Fonksiyonlar
    // Şu anki pozisyondaki karakteri döndürür (dosya sonunu kontrol eder).
    char peekChar() const;

    // Okuma pozisyonunu bir karakter ilerletir, satır/sütun günceller.
    void advance();

    // Belirli bir karakterin okuma pozisyonunda olup olmadığını kontrol eder.
    bool matchChar(char expected);

    // Boşlukları ve yorumları atlar.
    void skipWhitespaceAndComments();

    // Harf veya alt çizgi ile başlayan bir tanımlayıcıyı veya anahtar kelimeyi okur.
    Token readIdentifierOrKeyword();

    // Sayı (tam sayı veya ondalıklı sayı) okur.
    Token readNumber();

    // Tırnak içindeki string ifadeyi okur.
    Token readString();

    // Operatör veya noktalama işaretini okur (çok karakterli olanları da).
    Token readOperatorOrPunctuation();

    // Dosya sonuna ulaşıldı mı kontrolü
    bool isAtEnd() const;

    // Token okuma mantığının çekirdeği (buffered_token_ için kullanılır)
    Token readNextTokenInternal();
};

} namespace CCube

#endif // CUBE_SYNTAX_LEXER_HPP
#include "lexer.h"
#include <iostream>
#include <unordered_map>

// Hata raporlama fonksiyonu (main.cpp'de tanımlanacak, burada prototipini ekleyelim)
// Gerçek uygulamada bunu ayrı bir modül veya hata işleme sınıfı ile yönetmek daha iyi olabilir.
 void reportError(int line, const std::string& where, const std::string& message);
 void error(int line, const std::string& message); // Lexer hataları için

// Anahtar kelimeleri tutan map
std::unordered_map<std::string, TokenType> keywords;

// Anahtar kelime map'ini başlatan fonksiyon
void initKeywords() {
    keywords["and"] = TokenType::AND;
    keywords["class"] = TokenType::CLASS;
    keywords["else"] = TokenType::ELSE;
    keywords["false"] = TokenType::FALSE;
    keywords["fun"] = TokenType::FUN; // Fonksiyonlar için
    keywords["for"] = TokenType::FOR;
    keywords["if"] = TokenType::IF;
    keywords["none"] = TokenType::NONE; // Python'daki None gibi
    keywords["or"] = TokenType::OR;
    keywords["print"] = TokenType::PRINT; // Geçici, sonra kaldırılabilir veya standart kütüphanede olabilir
    keywords["return"] = TokenType::RETURN;
    keywords["super"] = TokenType::SUPER;
    keywords["this"] = TokenType::THIS;
    keywords["true"] = TokenType::TRUE;
    keywords["var"] = TokenType::VAR; // Değişken tanımlama için
    keywords["while"] = TokenType::WHILE;
    keywords["match"] = TokenType::MATCH; // match ifadesi
    keywords["import"] = TokenType::IMPORT; // import deyimi
    // Oyun geliştirme odaklı ek anahtar kelimeler buraya eklenebilir
     keywords["entity"] = TokenType::ENTITY;
     keywords["component"] = TokenType::COMPONENT;
     keywords["scene"] = TokenType::SCENE;
     keywords["async"] = TokenType::ASYNC; // Eğer asenkron destek olacaksa
     keywords["await"] = TokenType::AWAIT;
     keywords["graphic"] = TokenType::GRAPHIC; // Grafik ile ilgili bir keyword olabilir
}

// Lexer sınıfı implementasyonu

std::vector<Token> Lexer::scanTokens() {
    // Anahtar kelimeleri sadece bir kere başlat
    static bool keywords_initialized = false;
    if (!keywords_initialized) {
        initKeywords();
        keywords_initialized = true;
    }

    while (!isAtEnd()) {
        // Döngünün başında bir sonraki lexeme'in başlangıcını kaydet
        start = current;
        scanToken();
    }

    // Dosya sonu token'ını ekle
    tokens.emplace_back(TokenType::EOF_TOKEN, "", line);
    return tokens;
}

void Lexer::scanToken() {
    char c = advance();

    switch (c) {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case '-': addToken(TokenType::MINUS); break;
        case '+': addToken(TokenType::PLUS); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '*': addToken(TokenType::STAR); break;
        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;
        case '/':
            if (match('/')) {
                // Yorum satırı: satır sonuna kadar veya dosya sonuna kadar ilerle
                while (peek() != '\n' && !isAtEnd()) {
                    advance();
                }
            } else {
                addToken(TokenType::SLASH);
            }
            break;

        // Whitespace karakterlerini yok say
        case ' ':
        case '\r':
        case '\t':
            break; // Sadece ilerle

        case '\n':
            line++; // Yeni satıra geç
            break;

        // String literalleri
        case '"': string(); break;

        default:
            if (isDigit(c)) {
                number(); // Sayı literalini işle
            } else if (isAlpha(c)) {
                identifier(); // Tanımlayıcı veya anahtar kelimeyi işle
            }
            else {
                // Tanınmayan karakter hatası
                 error(line, "Unexpected character.");
                // Geçici olarak konsola yazdıralım:
                 std::cerr << "[Line " << line << "] Error: Unexpected character '" << c << "'." << std::endl;
            }
            break;
    }
}

// Yardımcı Metodlar Implementasyonu

bool Lexer::isAtEnd() {
    return current >= source.length();
}

char Lexer::advance() {
    current++;
    return source[current - 1];
}

void Lexer::addToken(TokenType type) {
    std::string text = source.substr(start, current - start);
    tokens.emplace_back(type, text, line);
}

// String veya sayı gibi literal değeri olan tokenlar için,
// lexeme zaten addToken metodunda alınıyor. Eğer isterseniz,
// sayıları burada parse edip literal olarak saklayabilirsiniz,
// ancak bunu Parser'da yapmak da mümkündür. Basitlik için lexeme stringini saklıyoruz.

// char beklenen karakter mi? Eğer evet ise karakteri tüket ve true döndür.
bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;

    current++;
    return true;
}

// Şu anki karaktere bak (tüketme)
char Lexer::peek() {
    if (isAtEnd()) return '\0'; // Dosya sonu için null karakter döndür
    return source[current];
}

// Bir sonraki karaktere bak (tüketme)
char Lexer::peekNext() {
     if (current + 1 >= source.length()) return '\0';
     return source[current + 1];
}


void Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++; // String içinde yeni satır desteği
        advance();
    }

    if (isAtEnd()) {
         error(line, "Unterminated string.");
        std::cerr << "[Line " << line << "] Error: Unterminated string." << std::endl;
        return;
    }

    // Kapanış tırnağını tüket
    advance();

    // Tırnaklar arasındaki değeri (lexeme) al
    // start + 1 ile ilk tırnağı atla
    // current - start - 2 ile tırnakların uzunluğunu çıkar
    std::string value = source.substr(start + 1, current - start - 2);
     addToken(TokenType::STRING, value); // Literal değeri ekleyebilirsiniz, şimdilik lexeme yeterli
    tokens.emplace_back(TokenType::STRING, value, line);
}

void Lexer::number() {
    // Tam sayı kısmını oku
    while (isDigit(peek())) {
        advance();
    }

    // Ondalıklı kısım var mı? (örn: 123.45)
    if (peek() == '.' && isDigit(peekNext())) {
        // '.' karakterini tüket
        advance();
        // Ondalıklı kısmı oku
        while (isDigit(peek())) {
            advance();
        }
    }

    // Lexeme'yi al ve NUMBER token'ı olarak ekle
    std::string value = source.substr(start, current - start);
     addToken(TokenType::NUMBER, std::stod(value)); // String'i double'a çevirip literal olarak saklayabilirsiniz
    tokens.emplace_back(TokenType::NUMBER, value, line);
}

void Lexer::identifier() {
    while (isAlphaNumeric(peek())) {
        advance();
    }

    // Lexeme'yi al (tanımlayıcı veya anahtar kelime olabilir)
    std::string text = source.substr(start, current - start);

    // Anahtar kelime map'inde ara
    auto it = keywords.find(text);
    TokenType type = TokenType::IDENTIFIER; // Varsayılan olarak tanımlayıcı

    if (it != keywords.end()) {
        // Anahtar kelime bulundu
        type = it->second;
    }

    addToken(type); // Token'ı ekle (lexeme zaten metin içinde)
}

bool Lexer::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_'; // Python'da _ karakteri de geçerli
}

bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

// Hata Yönetimi (Basit Konsol Çıktısı)
// Daha gelişmiş bir sistem için main.cpp'de tanımlanan veya ayrı bir sınıf/fonksiyon kullanılmalıdır.

void Lexer::error(int line, const std::string& message) {
    // hadError flag'ini set et
    // Gerçek hata raporlama fonksiyonunu çağır (reportError)
    std::cerr << "[Line " << line << "] Error: " << message << std::endl;
}

#include "lexer.hpp"
#include <cctype> // isalpha, isdigit, isspace gibi fonksiyonlar için
#include <iostream> // Hata ayıklama veya basit hata mesajları için (geçici olabilir)
#include <stdexcept> // Hata durumları için exception kullanımı

namespace CCube {

// Token yapısı için toString implementasyonu
std::string Token::toString() const {
    return "Token(" + tokenTypeToString(type) + ", \"" + lexeme + "\", Line: " + std::to_string(line) + ", Column: " + std::to_string(column) + ")";
}

// TokenType enum değerini string olarak döndüren yardımcı fonksiyon
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::PERCENT: return "PERCENT";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::EQ: return "EQ";
        case TokenType::NEQ: return "NEQ";
        case TokenType::LT: return "LT";
        case TokenType::GT: return "GT";
        case TokenType::LTE: return "LTE";
        case TokenType::GTE: return "GTE";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::NOT: return "NOT";
        case TokenType::DOT: return "DOT";
        case TokenType::COMMA: return "COMMA";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COLON: return "COLON";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::LSQUARE: return "LSQUARE";
        case TokenType::RSQUARE: return "RSQUARE";
        case TokenType::IMPORT: return "IMPORT";
        case TokenType::MATCH: return "MATCH";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::WHILE: return "WHILE";
        case TokenType::FOR: return "FOR";
        case TokenType::DEF: return "DEF";
        case TokenType::CLASS: return "CLASS";
        case TokenType::RETURN: return "RETURN";
        case TokenType::BREAK: return "BREAK";
        case TokenType::CONTINUE: return "CONTINUE";
        case TokenType::NONE_KW: return "NONE_KW";
        case TokenType::TRUE_KW: return "TRUE_KW";
        case TokenType::FALSE_KW: return "FALSE_KW";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::COMMENT: return "COMMENT";
        case TokenType::UNKNOWN: return "UNKNOWN";
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        default: return "UNKNOWN_TYPE"; // Beklenmedik bir durum
    }
}


// Lexer Kurucu
Lexer::Lexer(const std::string& source_code)
    : source_code_(source_code),
      current_position_(0),
      current_line_(1),
      current_column_(1),
      has_buffered_token_(false)
{
    // Anahtar kelime haritasını doldur
    keywords_["import"] = TokenType::IMPORT;
    keywords_["match"] = TokenType::MATCH;
    keywords_["if"] = TokenType::IF;
    keywords_["else"] = TokenType::ELSE;
    keywords_["while"] = TokenType::WHILE;
    keywords_["for"] = TokenType::FOR;
    keywords_["def"] = TokenType::DEF;
    keywords_["class"] = TokenType::CLASS;
    keywords_["return"] = TokenType::RETURN;
    keywords_["break"] = TokenType::BREAK;
    keywords_["continue"] = TokenType::CONTINUE;
    keywords_["None"] = TokenType::NONE_KW;
    keywords_["True"] = TokenType::TRUE_KW;
    keywords_["False"] = TokenType::FALSE_KW;

    // Başlangıçta tampon token'ı oku
    buffered_token_ = readNextTokenInternal();
    has_buffered_token_ = true;
}

// Bir sonraki token'ı döndürür ve ilerler
Token Lexer::getNextToken() {
    if (!has_buffered_token_) {
        // Bu duruma gelinmemeli eğer peekToken doğru kullanılıyorsa
        // veya ilk token constructor'da okunmuşsa
        // Ancak güvenlik için tekrar okuma ekleyebiliriz veya hata fırlatabiliriz.
        // Basitlik için şimdilik doğrudan okuyalım:
        buffered_token_ = readNextTokenInternal();
    }

    Token current_token = buffered_token_;
    has_buffered_token_ = false; // Tampon token tüketildi
    buffered_token_ = readNextTokenInternal(); // Bir sonraki token'ı tampona oku
    return current_token;
}

// Bir sonraki token'a bakar ancak ilerlemez
Token Lexer::peekToken() {
    // Tampon token her zaman bir sonraki tokenı tutar (getNextToken tarafından güncellenir)
    // Bu yüzden sadece tampondakini döndürmek yeterli.
    // has_buffered_token_ kontrolü getNextToken'ın ilk çağrısında buffer'ı doldurmasını sağlar.
     if (!has_buffered_token_) {
         // Eğer buffer boşsa (bu durum normalde olmaz), yine de okuyup tampona koy.
         buffered_token_ = readNextTokenInternal();
         has_buffered_token_ = true;
     }
    return buffered_token_;
}

// Yardımcı Fonksiyon Implementasyonları

// Şu anki pozisyondaki karakteri döndürür
char Lexer::peekChar() const {
    if (isAtEnd()) return '\0';
    return source_code_[current_position_];
}

// Okuma pozisyonunu bir karakter ilerletir, satır/sütun günceller
void Lexer::advance() {
    if (!isAtEnd()) {
        if (source_code_[current_position_] == '\n') {
            current_line_++;
            current_column_ = 1; // Yeni satır, sütun 1'den başlar
        } else {
            current_column_++;
        }
        current_position_++;
    }
}

// Belirli bir karakterin okuma pozisyonunda olup olmadığını kontrol eder ve eşleşirse ilerler.
bool Lexer::matchChar(char expected) {
    if (isAtEnd()) return false;
    if (source_code_[current_position_] != expected) return false;
    advance();
    return true;
}

// Boşlukları ve yorumları atlar.
void Lexer::skipWhitespaceAndComments() {
    while (true) {
        char c = peekChar();
        if (c == ' ' || c == '\r' || c == '\t' || c == '\n') {
            advance();
        } else if (c == '#') { // Yorumlar # ile başlar
            // Satır sonuna veya dosya sonuna kadar oku
            while (peekChar() != '\n' && !isAtEnd()) {
                advance();
            }
            // Satır sonunu da atla (eğer dosya sonu değilse)
            if (peekChar() == '\n') advance();
        } else {
            break; // Boşluk veya yorum değilse döngüden çık
        }
    }
}

// Harf veya alt çizgi ile başlayan bir tanımlayıcıyı veya anahtar kelimeyi okur.
Token Lexer::readIdentifierOrKeyword() {
    int start_col = current_column_; // Token'ın başladığı sütun
    size_t start_pos = current_position_; // Token'ın başladığı pozisyon

    // İlk karakter harf veya alt çizgi olmalı (çağrıdan önce kontrol edilmeli)
    advance(); // İlk karakteri tüket

    // Geri kalan karakterler harf, rakam veya alt çizgi olabilir
    while (std::isalnum(peekChar()) || peekChar() == '_') {
        advance();
    }

    std::string lexeme = source_code_.substr(start_pos, current_position_ - start_pos);
    TokenType type;

    // Anahtar kelime mi kontrol et
    auto it = keywords_.find(lexeme);
    if (it != keywords_.end()) {
        type = it->second; // Anahtar kelime ise ilgili TokenType'ı al
    } else {
        type = TokenType::IDENTIFIER; // Değilse tanımlayıcı (identifier)
    }

    return {type, lexeme, current_line_, start_col};
}

// Sayı (tam sayı veya ondalıklı sayı) okur.
Token Lexer::readNumber() {
     int start_col = current_column_;
     size_t start_pos = current_position_;

     // Tam sayı kısmını oku
     while (std::isdigit(peekChar())) {
         advance();
     }

     // Ondalıklı kısım var mı kontrol et
     bool is_float = false;
     if (peekChar() == '.' && std::isdigit(source_code_[current_position_ + 1])) {
         is_float = true;
         advance(); // '.' karakterini tüket
         // Ondalıklı kısmı oku
         while (std::isdigit(peekChar())) {
             advance();
         }
     }

     // E veya e ile başlayan üstel kısmı (exponent) kontrol et (örn: 1.2e+5)
     if (peekChar() == 'e' || peekChar() == 'E') {
          char next_char = source_code_[current_position_ + 1];
          if (std::isdigit(next_char) || ((next_char == '+' || next_char == '-') && std::isdigit(source_code_[current_position_ + 2]))) {
               is_float = true; // Üstel kısım varsa float olmalı
               advance(); // 'e' veya 'E' karakterini tüket
               if (peekChar() == '+' || peekChar() == '-') {
                    advance(); // '+' veya '-' işaretini tüket
               }
               // Üstel sayının rakamlarını oku
               while (std::isdigit(peekChar())) {
                    advance();
               }
          }
     }


     std::string lexeme = source_code_.substr(start_pos, current_position_ - start_pos);
     TokenType type = is_float ? TokenType::FLOAT : TokenType::INTEGER;

     return {type, lexeme, current_line_, start_col};
}


// Tırnak içindeki string ifadeyi okur.
Token Lexer::readString() {
     int start_col = current_column_;
     size_t start_pos = current_position_;
     char quote_char = peekChar(); // String ' veya " ile başlayabilir

     advance(); // Açılış tırnağını tüket

     std::string value;
     while (peekChar() != quote_char && !isAtEnd()) {
         // Kaçış karakterlerini (escaped characters) ele alabilirsiniz, örneğin \n, \\, \"
         if (peekChar() == '\\') {
              // Basit bir kaçış karakteri işleme örneği:
              advance(); // '\' karakterini tüket
              if (!isAtEnd()) {
                  char escaped_char = peekChar();
                  switch (escaped_char) {
                      case 'n': value += '\n'; break;
                      case 't': value += '\t'; break;
                      case '\\': value += '\\'; break;
                      case '"': value += '"'; break;
                      case '\'': value += '\''; break;
                      // Daha fazla kaçış karakteri buraya eklenebilir
                      default:
                           // Tanımsız kaçış karakteri hatası
                           // Geçici olarak hata raporu yazdırıp ilerleyelim
                           std::cerr << "Error: Undefined escape sequence \\" << escaped_char
                                     << " at line " << current_line_ << ", column " << current_column_ << std::endl;
                           value += escaped_char; // Hatalı karakteri eklemeye devam et
                           break;
                  }
                  advance();
              } else {
                   // Dosya sonu kaçış karakterinden sonra gelirse hata
                    std::cerr << "Error: Unexpected end of file after escape character at line "
                              << current_line_ << ", column " << current_column_ << std::endl;
              }

         } else {
             value += peekChar();
             advance();
         }
     }

     if (isAtEnd()) {
          // Kapanış tırnağı olmadan dosya sonu
          std::cerr << "Error: Unterminated string literal starting at line "
                    << current_line_ << ", column " << start_col << std::endl;
          // Token'ı UNKNOWN veya hata tokenı olarak döndürebilirsiniz
          return {TokenType::UNKNOWN, source_code_.substr(start_pos, current_position_ - start_pos), current_line_, start_col};
     }

     advance(); // Kapanış tırnağını tüket

     // Lexeme, tırnaklar dahil tüm stringi içerir
     std::string lexeme = source_code_.substr(start_pos, current_position_ - start_pos);

     // Token değeri olarak sadece tırnaklar arasındaki içeriği kullanabilirsiniz
     return {TokenType::STRING, value, current_line_, start_col}; // Token değeri olarak 'value' stringini kullanıyoruz
}


// Operatör veya noktalama işaretini okur (çok karakterli olanları da).
Token Lexer::readOperatorOrPunctuation() {
     int start_col = current_column_;
     size_t start_pos = current_position_;
     char c = peekChar();
     advance(); // İlk karakteri tüket

     TokenType type = TokenType::UNKNOWN; // Varsayılan UNKNOWN

     switch (c) {
         case '+': type = TokenType::PLUS; break;
         case '-': type = TokenType::MINUS; break;
         case '*': type = TokenType::STAR; break;
         case '/': type = TokenType::SLASH; break;
         case '%': type = TokenType::PERCENT; break;
         case '=': type = matchChar('=') ? TokenType::EQ : TokenType::ASSIGN; break; // == veya =
         case '!': type = matchChar('=') ? TokenType::NEQ : TokenType::UNKNOWN; break; // !=
         case '<': type = matchChar('=') ? TokenType::LTE : TokenType::LT; break; // <= veya <
         case '>': type = matchChar('=') ? TokenType::GTE : TokenType::GT; break; // >= veya >
         case '&': // C-CUBE 'and' kullanıyor, '&' veya '&&' operatör olarak düşünülürse eklenebilir.
                   // Şimdilik sadece tek karakterli operatörler/noktalamalarla sınırlı tutalım.
                   // Eğer '&' bir operatörse: type = TokenType::AND_BITWISE; break;
                   // Eğer '&&' bir operatörse: if(matchChar('&')) type = TokenType::AND_LOGICAL_SYMBOL; break;
                   // Şimdilik UNKNOWN bırakalım veya hata fırlatalım.
                   std::cerr << "Error: Unexpected character '&' at line " << current_line_ << ", column " << current_column_ -1 << std::endl;
                   break; // Hata işleme
         case '|': // C-CUBE 'or' kullanıyor, '|' veya '||' operatör olarak düşünülürse eklenebilir.
                   // Eğer '|' bir operatörse: type = TokenType::OR_BITWISE; break;
                   // Eğer '||' bir operatörse: if(matchChar('|')) type = TokenType::OR_LOGICAL_SYMBOL; break;
                   // Şimdilik UNKNOWN bırakalım veya hata fırlatalım.
                   std::cerr << "Error: Unexpected character '|' at line " << current_line_ << ", column " << current_column_ -1 << std::endl;
                   break; // Hata işleme
         case '.': type = TokenType::DOT; break;
         case ',': type = TokenType::COMMA; break;
         case ';': type = TokenType::SEMICOLON; break;
         case ':': type = TokenType::COLON; break;
         case '(': type = TokenType::LPAREN; break;
         case ')': type = TokenType::RPAREN; break;
         case '{': type = TokenType::LBRACE; break;
         case '}': type = TokenType::RBRACE; break;
         case '[': type = TokenType::LSQUARE; break;
         case ']': type = TokenType::RSQUARE; break;
         default:
             // Tanımlanamayan karakter
             type = TokenType::UNKNOWN;
             std::cerr << "Error: Unexpected character '" << c
                       << "' at line " << current_line_ << ", column " << current_column_ - 1 << std::endl;
             break; // Hata işleme
     }

     std::string lexeme = source_code_.substr(start_pos, current_position_ - start_pos);
     return {type, lexeme, current_line_, start_col};
}


// Dosya sonuna ulaşıldı mı kontrolü
bool Lexer::isAtEnd() const {
    return current_position_ >= source_code_.length();
}

// Token okuma mantığının çekirdeği
Token Lexer::readNextTokenInternal() {
    skipWhitespaceAndComments(); // Token okumadan önce boşlukları ve yorumları atla

    // Dosya sonuna ulaşıldıysa EOF token'ı döndür
    if (isAtEnd()) {
        return {TokenType::END_OF_FILE, "", current_line_, current_column_};
    }

    // Şu anki karakteri al ve ileriki pozisyonu kaydet (token lexeme'ini almak için)
    char c = peekChar();
    int start_col = current_column_;

    // Hangi tür token olabileceğini belirle
    if (std::isalpha(c) || c == '_') {
        return readIdentifierOrKeyword();
    }
    if (std::isdigit(c)) {
        return readNumber();
    }
    if (c == '"' || c == '\'') {
        return readString();
    }
    // Tek veya çok karakterli operatörler/noktalama işaretleri
    // Bu kısım, readOperatorOrPunctuation içinde ilk karakteri tüketir.
    return readOperatorOrPunctuation();

    // Eğer yukarıdaki durumlardan hiçbiri eşleşmezse, UNKNOWN token'ı döndürülmüş olur.
}


} namespace CCube
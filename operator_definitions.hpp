#ifndef CUBE_OPERATORS_OPERATOR_DEFINITIONS_HPP
#define CUBE_OPERATORS_OPERATOR_DEFINITIONS_HPP

#include <string>
#include <unordered_map>
#include "Syntax/token.hpp" // TokenType için

namespace CCube {

// Operatörün ilişkili olma yönü
enum class Associativity {
    LEFT,       // Sol ilişkili (örn: a - b - c = (a - b) - c)
    RIGHT,      // Sağ ilişkili (örn: a = b = c = a = (b = c))
    NON_ASSOC   // İlişkili değil (örn: a < b > c gibi zincirleme karşılaştırmalar geçersiz)
};

// Operatörün konumu
enum class Fixity {
    PREFIX,     // Önek (örn: -a, not b)
    INFIX,      // Orta (örn: a + b)
    POSTFIX     // Sonek (örn: a++, b--) - C-CUBE'da yoksa kullanılmaz
};

// Bir operatörün özelliklerini tutan yapı
struct OperatorInfo {
    TokenType type;
    Fixity fixity;
    int precedence;     // Öncelik seviyesi (daha yüksek sayı daha yüksek öncelik)
    Associativity associativity; // İlişkili olma yönü

    // Varsayılan kurucu
    OperatorInfo(TokenType type = TokenType::UNKNOWN, Fixity fixity = Fixity::INFIX, int precedence = -1, Associativity associativity = Associativity::LEFT)
        : type(type), fixity(fixity), precedence(precedence), associativity(associativity) {}
};

// TokenType'a karşılık gelen OperatorInfo'yu döndüren fonksiyon.
// Bu fonksiyonun implementasyonu operator_definitions.cpp dosyasında olacaktır.
const OperatorInfo& getOperatorInfo(TokenType type);

// TODO: İleride operatörlerin string karşılıklarını (lexeme) TokenType'a eşleyen harita da buraya eklenebilir.

} namespace CCube

#endif // CUBE_OPERATORS_OPERATOR_DEFINITIONS_HPP
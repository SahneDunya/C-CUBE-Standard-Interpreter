#include "operator_definitions.hpp"

namespace CCube {

// Tüm operatörlerin özelliklerini tutan statik harita
// Buradaki öncelik ve ilişkili olma kuralları C-CUBE'un gramerine göre belirlenir.
// Python'a yakın bir öncelik sırası örnek alınmıştır (düşük öncelikten yükseğe doğru sıralanmıştır).
static const std::unordered_map<TokenType, OperatorInfo> operator_info_map = {
    // Öncelik seviyeleri temsili olarak verilmiştir, gramer tasarımına göre ayarlanmalıdır.
    // Daha yüksek sayı daha yüksek öncelik demektir.

    // Atama operatörü (genellikle en düşük ifade önceliği)
    {TokenType::ASSIGN, {TokenType::ASSIGN, Fixity::INFIX, 10, Associativity::RIGHT}},

    // Mantıksal OR
    {TokenType::OR,     {TokenType::OR, Fixity::INFIX, 20, Associativity::LEFT}},
    // Mantıksal AND
    {TokenType::AND,    {TokenType::AND, Fixity::INFIX, 30, Associativity::LEFT}},

    // Eşitlik operatörleri
    {TokenType::EQ,     {TokenType::EQ, Fixity::INFIX, 40, Associativity::NON_ASSOC}},
    {TokenType::NEQ,    {TokenType::NEQ, Fixity::INFIX, 40, Associativity::NON_ASSOC}},

    // Karşılaştırma operatörleri
    {TokenType::LT,     {TokenType::LT, Fixity::INFIX, 50, Associativity::NON_ASSOC}}, // <
    {TokenType::GT,     {TokenType::GT, Fixity::INFIX, 50, Associativity::NON_ASSOC}}, // >
    {TokenType::LTE,    {TokenType::LTE, Fixity::INFIX, 50, Associativity::NON_ASSOC}}, // <=
    {TokenType::GTE,    {TokenType::GTE, Fixity::INFIX, 50, Associativity::NON_ASSOC}}, // >=

    // Toplama / Çıkarma
    {TokenType::PLUS,   {TokenType::PLUS, Fixity::INFIX, 60, Associativity::LEFT}},
    {TokenType::MINUS,  {TokenType::MINUS, Fixity::INFIX, 60, Associativity::LEFT}},

    // Çarpma / Bölme / Modulo
    {TokenType::STAR,   {TokenType::STAR, Fixity::INFIX, 70, Associativity::LEFT}},
    {TokenType::SLASH,  {TokenType::SLASH, Fixity::INFIX, 70, Associativity::LEFT}},
    {TokenType::PERCENT,{TokenType::PERCENT, Fixity::INFIX, 70, Associativity::LEFT}}, // %

    // Birli operatörler (genellikle yüksek öncelik, sağ ilişkili olabilir)
    {TokenType::MINUS,  {TokenType::MINUS, Fixity::PREFIX, 80, Associativity::RIGHT}}, // Birli eksi (Çift tanımlı TokenType ama Fixity farklı)
    {TokenType::NOT,    {TokenType::NOT, Fixity::PREFIX, 80, Associativity::RIGHT}}, // Mantıksal NOT

    // TODO: Diğer operatörler eklenecek:
    // Nokta erişimi (.), Çağrı (()), Indexleme ([]), Üs alma (**), Bitwise operatörler (&, |, ^, ~, <<, >>) vb.
    // Çağrı ve nokta erişimi genellikle en yüksek önceliğe sahiptir ve sol ilişkilidir.

};

// TokenType'a karşılık gelen OperatorInfo'yu döndüren fonksiyon.
// Bulamazsa varsayılan UNKNOWN bilgisi döner.
const OperatorInfo& getOperatorInfo(TokenType type) {
    auto it = operator_info_map.find(type);
    if (it != operator_info_map.end()) {
        return it->second;
    }
    // Bilinmeyen operatör için varsayılan bilgi döndür
    static const OperatorInfo unknown_op_info(TokenType::UNKNOWN, Fixity::INFIX, -1, Associativity::LEFT);
    return unknown_op_info;
}

} namespace CCube
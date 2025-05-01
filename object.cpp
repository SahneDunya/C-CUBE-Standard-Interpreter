#include "object.hpp"
#include <sstream> // toString için
#include <typeinfo> // typeid için (hata ayıklamada faydalı olabilir)

namespace CCube {

// Sanal Yıkıcı Implementasyonu (temel sınıf için)
Object::~Object() {
    // Temel sınıfta özel bir yıkım mantığı genellikle olmaz,
    // türetilmiş sınıflar kendi kaynaklarını (örn. string içeriği, liste elemanları) temizler.
     std::cout << "[DEBUG] Object of type '" << type_->toString() << "' at " << this << " destroyed." << std::endl; // Hata ayıklama
}

// Varsayılan toString Implementasyonu
// Nesnenin tipini ve adresini içeren bir string döndürür.
std::string Object::toString() const {
    std::stringstream ss;
    ss << "<object of type '" << type_->toString() << "' at " << this << ">";
    return ss.str();
}

// Varsayılan equals Implementasyonu
// İki nesnenin pointer eşitliğini kontrol eder.
bool Object::equals(const Object* other) const {
    // Eğer her ikisi de nullptr ise eşittir (olmamalı ama güvenlik)
    if (this == nullptr && other == nullptr) return true;
    // Biri nullptr diğeri değilse eşit değildir
    if (this == nullptr || other == nullptr) return false;

    // Varsayılan olarak pointer eşitliği
    return this == other;

    // Gerçek eşitlik kontrolü (derinlemesine değer karşılaştırması)
    // türetilmiş sınıflarda override edilmelidir (örn. int değerleri, string içeriği).
}


// TODO: hashCode implementasyonu

// TODO: getMember / setMember varsayılan implementasyonları (belki hata fırlatır)

} namespace CCube
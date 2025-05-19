#ifndef C_CUBE_VALUE_H
#define C_CUBE_VALUE_H

#include <string>
#include <vector>
#include <memory>
#include <variant> // C++17 ve sonrası için

// İleri bildirimler
 class C_CUBE_Object; // Nesne yönelimli kısım için
 class C_CUBE_Function; // Fonksiyonlar için

// C-CUBE runtime değerlerini temsil eden sınıf/yapı
 std::variant kullanmak farklı tipleri tutmak için iyi bir yoldur
using Value = std::variant<
    std::monostate, // None / null
    bool,
    double, // Sayılar
    std::string
     std::shared_ptr<C_CUBE_Object>, // Obje referansları
     std::shared_ptr<C_CUBE_Function> // Fonksiyon referansları
    // ... diğer tipler (listeler, dictler vb.)
>;

using ValuePtr = std::shared_ptr<Value>; // Değerlere işaretçiler

// Değerleri yazdırmak veya karşılaştırmak gibi helper fonksiyonları burada olabilir
 std::string valueToString(ValuePtr value);
 bool isTruthy(ValuePtr value); // Koşullu ifadeler için (Python'daki gibi true/false değerlendirme)
 bool isEqual(ValuePtr v1, ValuePtr v2);
// ...

#endif // C_CUBE_VALUE_H

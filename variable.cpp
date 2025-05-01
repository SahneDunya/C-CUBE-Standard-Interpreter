#include "variable.hpp"
#include <sstream> // toString için

namespace CCube {

// toString Implementasyonu
std::string Variable::toString() const {
    std::stringstream ss;
    ss << "Variable (Symbol: '" << symbol->name << "', Type: '" << symbol->type->toString() << "')";
    if (value) {
        ss << ", Value: " << value->toString(); // Object'in toString'ini kullan
    } else {
        ss << ", Value: <nullptr>"; // Değer atanmamışsa veya null ise
    }
    return ss.str();
}

// Kurucu implementasyonu zaten başlık dosyasında inline yapıldı.
// Başka metodlar eklenirse buraya implementasyonları gelecektir.

} namespace CCube
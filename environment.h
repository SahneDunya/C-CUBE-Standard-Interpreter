#ifndef C_CUBE_ENVIRONMENT_H
#define C_CUBE_ENVIRONMENT_H

#include <string>
#include <unordered_map>
#include <memory>
#include "value.h" // Değişken değerleri ValuePtr olacak

class Environment {
private:
    std::unordered_map<std::string, ValuePtr> values;
    std::shared_ptr<Environment> enclosing; // Üst skop

public:
    // Global skop için constructor
    Environment();

    // İç içe skoplar için constructor (üst skopu alır)
    Environment(std::shared_ptr<Environment> enclosing);

    // Değişken tanımlama
    void define(const std::string& name, ValuePtr value);

    // Değişken değeri alma
    ValuePtr get(const Token& name);

    // Değişken değeri atama (mevcut skopta veya üst skoplarda)
    void assign(const Token& name, ValuePtr value);

    // Belirli bir isim için hangi skopta tanımlandığını bulma (performans optimizasyonu için Resolution aşaması eklendiğinde kullanılır)
     EnvironmentPtr ancestor(int distance);

    // Değişken değeri alma (Resolution kullanıldığında)
     ValuePtr getAt(int distance, const std::string& name);

    // Değişken değeri atama (Resolution kullanıldığında)
     void assignAt(int distance, const std::string& name, ValuePtr value);
};

using EnvironmentPtr = std::shared_ptr<Environment>; // Paylaşılan işaretçi alias'ı

#endif // C_CUBE_ENVIRONMENT_H

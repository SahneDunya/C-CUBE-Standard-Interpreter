#include "std_core.hpp"
#include <iostream> // cout, cin için
#include <sstream>  // String manipülasyonları için
#include <stdexcept> // Hata durumları için
#include <limits> // Sayısal dönüşüm sınırları için
#include <cctype>   // isspace gibi karakter fonksiyonları için

// TODO: Çalışma zamanı somut Object tiplerini dahil etmeniz GEREKİYOR.
// Bu implementasyonlar bu somut tiplerin varlığını ve metotlarını varsayar.
// Örnek olarak eklenmiştir, kendi dosya yollarınızla ve sınıf isimlerinizle değiştirin.
 #include "Data Types/IntegerObject.hpp"
 #include "Data Types/FloatObject.hpp"
 #include "Data Types/StringObject.hpp"
 #include "Data Types/BooleanObject.hpp"
 #include "Data Types/NoneObject.hpp"
 #include "Data Types/ListObject.hpp" // print fonksiyonu için (eğer argüman vector olarak temsil edilmiyorsa)

#include "Data Types/type_system.hpp" // TypeSystem singleton'ına erişim için (nesne oluştururken tip lazım)

namespace CCube {
namespace StandardLibrary {
namespace Core {

// Konsola çıktı yazdırma fonksiyonu (Python'daki print gibi)
Object* print(const std::vector<Object*>& args, ErrorReporter& reporter, SourceLocation loc) {
    bool first = true;
    for (const auto& arg : args) {
        if (!first) {
            std::cout << " "; // Argümanlar arasına boşluk koy
        }
        if (arg) {
            // Object'in toString metodunu kullanarak çıktıyı al
            std::cout << arg->toString();
        } else {
            // Null Object pointer durumu (olmamalı)
             reporter.reportWarning(loc, "Null object passed to print.");
             std::cout << "<null>";
        }
        first = false;
    }
    std::cout << std::endl; // Sonuna yeni satır ekle

    // Python print fonksiyonu None döndürür
    // TODO: TypeSystem singleton'ından NoneObject* nesnesini döndür.
    // NoneObject* none_instance = static_cast<NoneObject*>(TypeSystem::getInstance().getNoneType()); // Bu cast doğru olmaz!
    // TypeSystem sadece Type* verir. NoneObject'in bir singleton instance'ı olmalı.
    // Veya her print çağrısında yeni bir NoneObject oluşturulabilir (GC ile yönetilir).
    // Basitlik için nullptr döndürelim, bu genellikle "void" veya "None" anlamına gelir runtime'da.
    return nullptr; // Geçici olarak nullptr döndür
}

// Kullanıcıdan girdi okuma fonksiyonu (Python'daki input gibi)
Object* input(const std::vector<Object*>& args, ErrorReporter& reporter, SourceLocation loc) {
    // Prompt (istek mesajı) varsa yazdır
    if (!args.empty()) {
        if (args[0]) {
             std::cout << args[0]->toString(); // İlk argümanı prompt olarak kullan
        } else {
            reporter.reportWarning(loc, "Null object passed as prompt to input.");
        }
    }

    // Girdi oku
    std::string line;
    std::getline(std::cin, line);

    // Okunan stringi bir C-CUBE StringObject'ine dönüştür
    // TODO: Yeni bir StringObject nesnesi oluştur ve GC tarafından yönetildiğinden emin ol.
     return new StringObject(line, TypeSystem::getInstance().getStringType()); // Örnek: new ile oluşturma
     reporter.reportWarning(loc, "Input function not fully implemented (returns nullptr).");
    return nullptr; // Geçici olarak nullptr döndür
}

// objeyi Integer'a dönüştürmeye çalışır.
Object* int_conversion(Object* obj, ErrorReporter& reporter, SourceLocation loc) {
    if (!obj) {
         reporter.reportError(loc, "Cannot convert null to integer.");
         // TODO: NoneObject* veya hata nesnesi döndür
         return nullptr;
    }

    Type* obj_type = obj->getRuntimeType();

    // Integer'a dönüştürülebilir tipler (örn: int, float, string, bool, None)
    if (obj_type->id == TypeId::INTEGER) {
        // Zaten integer ise kendisini döndür (veya kopyasını)
        // TODO: Eğer IntegerObject ise, pointer'ı döndür veya kopyasını oluştur.
         return obj; // veya new IntegerObject(static_cast<IntegerObject*>(obj)->getValue(), obj_type);
         reporter.reportWarning(loc, "Integer conversion from integer not fully implemented (returns nullptr).");
        return nullptr;
    }
    if (obj_type->id == TypeId::FLOAT) {
         // Float'tan integer'a dönüşüm (ondalıklı kısım atılır)
         // TODO: FloatObject*'a cast et, int'e çevir, yeni IntegerObject oluştur.
          int value = static_cast<int>(static_cast<FloatObject*>(obj)->getValue());
          return new IntegerObject(value, TypeSystem::getInstance().getIntegerType());
          reporter.reportWarning(loc, "Integer conversion from float not fully implemented (returns nullptr).");
         return nullptr;
    }
    if (obj_type->id == TypeId::STRING) {
        // String'ten integer'a dönüşüm (parsing)
        // TODO: StringObject*'a cast et, stringi parse et. Geçersiz format ise hata raporla.
         std::string str_value = static_cast<StringObject*>(obj)->getValue();
         try {
             int value = std::stoi(str_value);
             return new IntegerObject(value, TypeSystem::getInstance().getIntegerType());
         } catch (const std::invalid_argument& e) {
              reporter.reportError(loc, "Invalid literal for int() with base 10: '" + str_value + "'.");
              // TODO: NoneObject* veya hata nesnesi döndür
              return nullptr;
         } catch (const std::out_of_range& e) {
              reporter.reportError(loc, "Integer value out of range: '" + str_value + "'.");
               // TODO: NoneObject* veya hata nesnesi döndür
               return nullptr;
         }
         reporter.reportWarning(loc, "Integer conversion from string not fully implemented (returns nullptr).");
        return nullptr;
    }
     if (obj_type->id == TypeId::BOOLEAN) {
         // Boolean'dan integer'a dönüşüm (True=1, False=0)
         // TODO: BooleanObject*'a cast et, değeri al (true/false), 1 veya 0 içeren yeni IntegerObject oluştur.
          int value = static_cast<BooleanObject*>(obj)->getValue() ? 1 : 0;
          return new IntegerObject(value, TypeSystem::getInstance().getIntegerType());
          reporter.reportWarning(loc, "Integer conversion from boolean not fully implemented (returns nullptr).");
         return nullptr;
     }
    if (obj_type->id == TypeId::NONE) {
         // None'dan integer'a dönüşüm (Python'da TypeError)
         reporter.reportError(loc, "Cannot convert None to integer.");
         // TODO: NoneObject* veya hata nesnesi döndür
         return nullptr;
    }

    // Desteklenmeyen tip dönüşümü
    reporter.reportError(loc, "Cannot convert type '" + obj_type->toString() + "' to integer.");
    // TODO: NoneObject* veya hata nesnesi döndür
    return nullptr;
}

// objeyi Float'a dönüştürmeye çalışır.
Object* float_conversion(Object* obj, ErrorReporter& reporter, SourceLocation loc) {
     if (!obj) {
         reporter.reportError(loc, "Cannot convert null to float.");
         return nullptr;
     }

     Type* obj_type = obj->getRuntimeType();

     if (obj_type->id == TypeId::FLOAT) {
         // Zaten float ise
          return obj; // veya kopyası
          reporter.reportWarning(loc, "Float conversion from float not fully implemented (returns nullptr).");
         return nullptr;
     }
      if (obj_type->id == TypeId::INTEGER) {
           Integer'dan float'a dönüşüm
         // TODO: IntegerObject*'a cast et, değeri double'a çevir, yeni FloatObject oluştur.
          double value = static_cast<double>(static_cast<IntegerObject*>(obj)->getValue());
          return new FloatObject(value, TypeSystem::getInstance().getFloatType());
           reporter.reportWarning(loc, "Float conversion from integer not fully implemented (returns nullptr).");
         return nullptr;
      }
     if (obj_type->id == TypeId::STRING) {
          String'ten float'a dönüşüm (parsing)
         // TODO: StringObject*'a cast et, stringi parse et (std::stod). Geçersiz format/aralık ise hata.
          std::string str_value = static_cast<StringObject*>(obj)->getValue();
          try {
              double value = std::stod(str_value);
              return new FloatObject(value, TypeSystem::getInstance().getFloatType());
          } catch (const std::invalid_argument& e) {
               reporter.reportError(loc, "Invalid literal for float(): '" + str_value + "'.");
               return nullptr;
          } catch (const std::out_of_range& e) {
               reporter.reportError(loc, "Float value out of range: '" + str_value + "'.");
               return nullptr;
          }
          reporter.reportWarning(loc, "Float conversion from string not fully implemented (returns nullptr).");
         return nullptr;
     }
      if (obj_type->id == TypeId::BOOLEAN) {
         // Boolean'dan float'a dönüşüm (True=1.0, False=0.0)
         // TODO: BooleanObject*'a cast et, değeri double'a çevir, yeni FloatObject oluştur.
          double value = static_cast<BooleanObject*>(obj)->getValue() ? 1.0 : 0.0;
          return new FloatObject(value, TypeSystem::getInstance().getFloatType());
           reporter.reportWarning(loc, "Float conversion from boolean not fully implemented (returns nullptr).");
         return nullptr;
     }
     if (obj_type->id == TypeId::NONE) {
         reporter.reportError(loc, "Cannot convert None to float.");
         return nullptr;
     }

     reporter.reportError(loc, "Cannot convert type '" + obj_type->toString() + "' to float.");
     return nullptr;
}

// objeyi String'e dönüştürür
Object* str_conversion(Object* obj, ErrorReporter& reporter, SourceLocation loc) {
    if (!obj) {
        // Python'da str(None) -> "None" stringini verir.
        // Bu yüzden null olsa bile özel bir "None" stringi döndürebiliriz.
        // TODO: NoneObject*'ın toString() çıktısını kullanın veya özel "None" stringi oluşturun.
         return new StringObject("None", TypeSystem::getInstance().getStringType());
        reporter.reportWarning(loc, "String conversion from null not fully implemented (returns nullptr).");
        return nullptr;
    }

    // Her Object'in toString metodu zaten string temsilini verir.
    // Bu temsilin C-CUBE StringObject'i olması gerekir.
    // TODO: obj->toString() çıktısını alıp yeni bir StringObject oluşturun.
     std::string result_string = obj->toString();
     return new StringObject(result_string, TypeSystem::getInstance().getStringType());
     reporter.reportWarning(loc, "String conversion handler not fully implemented (returns nullptr).");
    return nullptr;
}

// objeyi Boolean'a dönüştürür
Object* bool_conversion(Object* obj, ErrorReporter& reporter, SourceLocation loc) {
     // Python'daki "truthiness" kuralları uygulanır.
     // Sayılar (0 hariç True), Stringler (boş string hariç True), Listeler (boş liste hariç True), None (False) vb.

     if (!obj || obj->getRuntimeType()->id == TypeId::NONE) {
         // None her zaman False'tur
         // TODO: False değerli yeni BooleanObject oluşturun.
          return new BooleanObject(false, TypeSystem::getInstance().getBooleanType());
         reporter.reportWarning(loc, "Boolean conversion for null/None not fully implemented (returns nullptr).");
         return nullptr;
     }

    Type* obj_type = obj->getRuntimeType();

    if (obj_type->id == TypeId::BOOLEAN) {
        // Zaten boolean ise kendisini döndür
         return obj; // veya kopyası
         reporter.reportWarning(loc, "Boolean conversion from boolean not fully implemented (returns nullptr).");
        return nullptr;
    }
    if (obj_type->id == TypeId::INTEGER) {
         Integer: 0 False, diğerleri True
        // TODO: IntegerObject*'a cast et, değeri 0 mı kontrol et, yeni BooleanObject oluştur.
         bool value = static_cast<IntegerObject*>(obj)->getValue() != 0;
         return new BooleanObject(value, TypeSystem::getInstance().getBooleanType());
         reporter.reportWarning(loc, "Boolean conversion from integer not fully implemented (returns nullptr).");
        return nullptr;
    }
    if (obj_type->id == TypeId::FLOAT) {
          Float: 0.0 False, diğerleri True
         // TODO: FloatObject*'a cast et, değeri 0.0 mı kontrol et, yeni BooleanObject oluştur.
          bool value = static_cast<FloatObject*>(obj)->getValue() != 0.0;
          return new BooleanObject(value, TypeSystem::getInstance().getBooleanType());
          reporter.reportWarning(loc, "Boolean conversion from float not fully implemented (returns nullptr).");
         return nullptr;
    }
     if (obj_type->id == TypeId::STRING) {
          String: Boş string ("") False, diğerleri True
         // TODO: StringObject*'a cast et, string boş mu kontrol et, yeni BooleanObject oluştur.
          bool value = !static_cast<StringObject*>(obj)->getValue().empty();
          return new BooleanObject(value, TypeSystem::getInstance().getBooleanType());
          reporter.reportWarning(loc, "Boolean conversion from string not fully implemented (returns nullptr).");
         return nullptr;
     }
     // TODO: List, Dictionary gibi koleksiyonlar için (boşsa False, değilse True)

    // Diğer tipler için varsayılan (genellikle True)
    // TODO: Diğer Object türevleri için varsayılan "truthiness" değeri.
     return new BooleanObject(true, TypeSystem::getInstance().getBooleanType());
     reporter.reportWarning(loc, "Boolean conversion for unsupported type '" + obj_type->toString() + "' not fully implemented (returns nullptr).");
     return nullptr; // Geçici

}


// TODO: Diğer çekirdek fonksiyonların implementasyonları buraya gelecek.

} namespace Core
} namespace StandardLibrary
} namespace CCube
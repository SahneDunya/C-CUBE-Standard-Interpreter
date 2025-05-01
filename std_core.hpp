#ifndef CUBE_STANDARD_LIBRARY_CORE_STD_CORE_HPP
#define CUBE_STANDARD_LIBRARY_CORE_STD_CORE_HPP

#include <vector>
#include <string>
#include <memory> // shared_ptr veya unique_ptr

#include "object.hpp" // Object türü için
#include "error_reporter.hpp" // Runtime hata raporlama için

// TODO: Çalışma zamanı somut Object tiplerini dahil etmeniz gerekebilir (örn: StringObject)
// std::string fonksiyon imzalarında geçiyorsa StringObject gerekli olabilir.
 #include "StringObject.hpp"


namespace CCube {
namespace StandardLibrary {
namespace Core { // Standart kütüphane çekirdek fonksiyonları için iç içe namespace

// Konsola çıktı yazdırma fonksiyonu (Python'daki print gibi)
// Birden fazla argüman alabilir (std::vector<Object*> ile temsil edelim)
// Genellikle None döndürür (void gibi)
// ErrorReporter, runtime hataları için kullanılır (örn: objelerin toString yapamaması)
Object* print(const std::vector<Object*>& args, ErrorReporter& reporter, SourceLocation loc);

// Kullanıcıdan girdi okuma fonksiyonu (Python'daki input gibi)
// İsteğe bağlı olarak bir prompt (istek mesajı) alabilir.
// Her zaman bir StringObject* döndürür.
Object* input(const std::vector<Object*>& args, ErrorReporter& reporter, SourceLocation loc); // Tek veya sıfır argüman (prompt) beklenir

// ---------- Temel Tip Dönüşüm Fonksiyonları ----------
// Örn: int(obj), float(obj), str(obj), bool(obj)

// objeyi Integer'a dönüştürmeye çalışır. Başarılı olursa IntegerObject*, aksi halde hata ve NoneObject*
Object* int_conversion(Object* obj, ErrorReporter& reporter, SourceLocation loc);

// objeyi Float'a dönüştürmeye çalışır. Başarılı olursa FloatObject*, aksi halde hata ve NoneObject*
Object* float_conversion(Object* obj, ErrorReporter& reporter, SourceLocation loc);

// objeyi String'e dönüştürür (obj->toString() kullanır). StringObject* döndürür.
Object* str_conversion(Object* obj, ErrorReporter& reporter, SourceLocation loc);

// objeyi Boolean'a dönüştürür (objenin "truthiness" değerini hesaplar). BooleanObject* döndürür.
Object* bool_conversion(Object* obj, ErrorReporter& reporter, SourceLocation loc);

// TODO: Diğer çekirdek fonksiyonlar eklenecek (örn: len, range gibi built-inler)


} namespace Core
} namespace StandardLibrary
} namespace CCube

#endif // CUBE_STANDARD_LIBRARY_CORE_STD_CORE_HPP
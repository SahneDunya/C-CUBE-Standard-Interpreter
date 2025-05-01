#ifndef CUBE_DATA_TYPES_OBJECT_HPP
#define CUBE_DATA_TYPES_OBJECT_HPP

#include <string>
#include <memory>
#include "type.hpp" // Runtime tip bilgisi için
// GC sınıfına ileri bildirim
namespace CCube { namespace MemoryManagement { class GarbageCollector; } }


namespace CCube {

// C-CUBE çalışma zamanındaki tüm değerlerin (nesnelerin) temel sınıfı
class Object {
    // GC'nin private/protected üyelere erişmesi gerekebilir
    friend class MemoryManagement::GarbageCollector;

public:
    // Kurucu
    Object(Type* runtime_type); // Implementasyonu .cpp'de

    // Sanal yıkıcı
    virtual ~Object(); // Implementasyonu .cpp'de

    // ---------- Ortak Nesne Metotları (Sanal) ----------
    virtual std::string toString() const;
    virtual bool equals(const Object* other) const;
    Type* getRuntimeType() const { return type_; }
     virtual size_t hashCode() const;
     virtual Object* getMember(const std::string& name);
     virtual void setMember(const std::string& name, Object* value);


    // ---------- Çöp Toplama (GC) Metotları ve Alanları ----------

    // GC'nin işaretleme aşamasında pointer referanslarını ziyaret eden sanal metot.
    // Türetilmiş sınıflar, kendi içlerinde tuttukları diğer Object* pointerlarını
    // (örn: liste elemanları, dictionary anahtarları/değerleri, sınıf üyeleri)
    // GC'ye bildirmek için bu metodu override etmelidir.
    virtual void visitPointers(MemoryManagement::GarbageCollector& gc);


protected:
    Type* type_; // Nesnenin çalışma zamanı tipi

    // GC için işaretleme bayrağı (Mark-and-Sweep algoritması için)
    bool gc_marked_;

    // TODO: Diğer GC algoritmaları için ek alanlar gerekebilir (örn: referans sayacı, next pointer)
     int ref_count_; // Referans sayma GC için

private:
    // Kopya oluşturmayı ve atamayı yasakla
    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
};

} namespace CCube

#endif // CUBE_DATA_TYPES_OBJECT_HPP
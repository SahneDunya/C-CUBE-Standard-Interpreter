#ifndef C_CUBE_GC_H
#define C_CUBE_GC_H

#include <vector>
#include <memory>   // std::shared_ptr için
#include <string>
#include <unordered_set> // Marked objeleri takip etmek için
#include <algorithm> // std::remove_if için

// İleri bildirimler (circular dependency'i önlemek için)
// Bu sınıfların tam tanımları daha sonra veya başka header'larda olmalıdır.
class Object;
class Value;
class Environment;
class CCubeString; // String objeleri için
class CCubeList;   // Liste objeleri için
class CCubeInstance; // Class instance objeleri için
class CCubeClass;    // Class objeleri için
class CCubeFunction; // Function objeleri için
class BoundMethod;   // Bound method objeleri için
class CCubeModule;   // Module objeleri için

#endif // C_CUBE_GC_H

#ifndef C_CUBE_MODULE_H
#define C_CUBE_MODULE_H

#include "gc.h"        // GcObject'ten türemek için
#include "environment.h" // Modülün ortam bilgisi için (EnvironmentPtr için gerekli)
#include "ast.h"         // C-CUBE (.cube) modüllerinin AST'si için (StmtPtr için gerekli)
#include "value.h"       // ValuePtr için, eğer modül içinde ValuePtr'lar saklanıyorsa

#include <string>
#include <vector>
#include <variant>       // Farklı modül içeriği için
#include <memory>        // std::shared_ptr için

// İleri bildirimler (eğer C_CUBE_Object/Function/Class ayrı dosyalarda ise)
 class C_CUBE_Object;
 class C_CUBE_Function;
 class C_CUBE_Class;

// --- Diğer Dil Modül Handle'ları ---
// Bu sınıflar, C-CUBE'un dışındaki dillerden yüklenen modülleri temsil eder.
// Gerçek implementasyonlar, ilgili dillerin C API'leri veya FFI kütüphaneleriyle entegre olur.

// Python modülü için yer tutucu handle
// Gerçek implementasyonda PyObject* gibi Python C API tipleri tutulabilir.
class PythonModuleHandle {
public:
    std::string filePath;
     PyObject* pyModule = nullptr; // Gerçek Python modül objesi buraya gelecek

    PythonModuleHandle(const std::string& path) : filePath(path) {}
    std::string toString() const { return "<Python module: " + filePath + ">"; }
     PyObject* getPyModule() const { return pyModule; } // Getter eklenebilir
};

// Native (C/C++/Shader) modülü için yer tutucu handle
// Bu tür dosyalar doğrudan yorumlanamaz, genellikle FFI veya özel derleme/yükleme mekanizmaları ile kullanılır.
class NativeModuleHandle {
public:
    std::string filePath;
     void* nativeHandle = nullptr; // Derlenmiş kütüphane veya shader programı handle'ı

    NativeModuleHandle(const std::string& path) : filePath(path) {}
    std::string toString() const { return "<Native module: " + filePath + ">"; }
     void* getNativeHandle() const { return nativeHandle; } // Getter eklenebilir
};

// Fortran modülü için yer tutucu handle
class FortranModuleHandle {
public:
    std::string filePath;
    FortranModuleHandle(const std::string& path) : filePath(path) {}
    std::string toString() const { return "<Fortran module: " + filePath + ">"; }
};

// Julia modülü için yer tutucu handle
class JuliaModuleHandle {
public:
    std::string filePath;
    JuliaModuleHandle(const std::string& path) : filePath(path) {}
    std::string toString() const { return "<Julia module: " + filePath + ">"; }
};


// --- C_CUBE_Module: Modülün Kendisi (GcObject) ---
// C-CUBE dilinde 'import' ifadesiyle yüklenen bir modülü temsil eder.
// GcObject'ten türediği için çöp toplayıcı tarafından yönetilir.
class C_CUBE_Module : public GcObject {
public:
    std::string name; // Modülün adı (örn. "math", "game.utils")

    // Modülün gerçek içeriğini tutan variant.
    // Her farklı dil için uygun handle veya doğrudan AST/Environment tutulur.
    std::variant<
        std::monostate,             // Boş veya bilinmeyen tip (varsayılan)
        std::pair<std::vector<StmtPtr>, EnvironmentPtr>, // C-CUBE (.cube) modüllerinin AST ve Ortamı
        PythonModuleHandle,         // Python (.py) modülü
        NativeModuleHandle,         // C/C++/Shader (.h, .cuh, .glsl, .hlsl, .metal, .spv) modülü
        FortranModuleHandle,        // Fortran (.mod) modülü
        JuliaModuleHandle           // Julia (.jl) modülü
        // Diğer dil modülleri buraya eklenebilir
    > content;

    // --- Constructor'lar ---
    // C-CUBE (.cube) modülleri için constructor
    C_CUBE_Module(const std::string& name, std::vector<StmtPtr> ast, EnvironmentPtr env)
        : name(name), content(std::make_pair(std::move(ast), env)) {}

    // Python modülleri için constructor
    C_CUBE_Module(const std::string& name, PythonModuleHandle pyHandle)
        : name(name), content(std::move(pyHandle)) {}

    // Native (C/C++/Shader) modülleri için constructor
    C_CUBE_Module(const std::string& name, NativeModuleHandle nativeHandle)
        : name(name), content(std::move(nativeHandle)) {}

    // Fortran modülleri için constructor
    C_CUBE_Module(const std::string& name, FortranModuleHandle fortranHandle)
        : name(name), content(std::move(fortranHandle)) {}

    // Julia modülleri için constructor
    C_CUBE_Module(const std::string& name, JuliaModuleHandle juliaHandle)
        : name(name), content(std::move(juliaHandle)) {}

    // GcObject arayüzü: Modülün çocuklarını (başka GcObject'lere referansları) işaretler.
    std::string toString() const override; // GcObject'ten türediği için gerekli
    void markChildren(GarbageCollector& gc) override; // GcObject'ten türediği için gerekli
};

#endif // C_CUBE_MODULE_H

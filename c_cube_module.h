// c_cube_module.h
#ifndef C_CUBE_MODULE_H
#define C_CUBE_MODULE_H

#include "gc.h"        // GcObject'ten türemek için
#include "environment.h" // Ortam bilgisi için
#include "ast.h"         // C-CUBE AST'si için (yalnızca .cube)
#include <string>
#include <vector>
#include <variant>       // Farklı modül içeriği için

// Python modülü için bir placeholder sınıfı
// Gerçek implementasyon PyObject* tutmalı ve Python C API ile etkileşmeli
class PythonModuleHandle {
public:
    std::string filePath;
    // PyObject* pyModule; // Gerçek Python modül objesi
    PythonModuleHandle(const std::string& path) : filePath(path) {}
    std::string toString() const { return "<Python module: " + filePath + ">"; }
};

// Native (C/C++/Shader) modülü için bir placeholder sınıfı
// Gerçek implementasyon FFI veya GPU API etkileşimi için gerekli verileri tutmalı
class NativeModuleHandle {
public:
    std::string filePath;
    // void* native_handle; // Derlenmiş kütüphane veya shader programı handle'ı
    NativeModuleHandle(const std::string& path) : filePath(path) {}
    std::string toString() const { return "<Native module: " + filePath + ">"; }
};

// C_CUBE_Module, farklı dillerden yüklenen modülleri temsil eder
class C_CUBE_Module : public GcObject {
public:
    std::string name; // Modülün adı (örn. "math", "game.utils")

    // Modülün içeriğini tutmak için variant
    std::variant<
        std::monostate, // Henüz yüklenmemiş veya bilinmeyen tip
        std::vector<StmtPtr>, // C-CUBE (.cube) modüllerinin AST'si
        std::shared_ptr<Environment>, // C-CUBE (.cube) modüllerinin kendi ortamı (globals)
                                     // (AST ve Environment birlikte tutulabilir)
        PythonModuleHandle, // Python (.py) modülü
        NativeModuleHandle  // C/C++/Shader (.h, .cuh, .glsl, vb.) modülü
        // ... Diğer dil modülleri için buraya ekle
    > content;

    // Constructor for .cube modules
    C_CUBE_Module(const std::string& name, std::vector<StmtPtr> ast, EnvironmentPtr env)
        : name(name), content(std::make_pair(ast, env)) {} // pair olarak tutmak AST ve Env için uygun

    // Constructor for Python modules
    C_CUBE_Module(const std::string& name, PythonModuleHandle pyHandle)
        : name(name), content(pyHandle) {}

    // Constructor for Native modules
    C_CUBE_Module(const std::string& name, NativeModuleHandle nativeHandle)
        : name(name), content(nativeHandle) {}

    // GcObject'ten gelen sanal metotlar
    std::string toString() const override; // GcObject'te yoksa GcObject'e eklenir
    void markChildren() override; // GcObject'ten türeyen çocukları işaretler
    void markChildren(GarbageCollector& gc) override { /* ... */ }
};

#endif // C_CUBE_MODULE_H

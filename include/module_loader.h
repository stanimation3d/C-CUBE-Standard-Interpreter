#ifndef C_CUBE_MODULE_LOADER_H
#define C_CUBE_MODULE_LOADER_H

#include "environment.h"   // Module's root environment
#include "ast.h"           // Module's AST (primarily for .cube files)
#include "value.h"         // ValuePtr for module objects

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>          // std::shared_ptr, std::unique_ptr
#include <functional>      // std::function

// İleri bildirimler
class Interpreter;
class Lexer;
class Parser;

// --- ModuleReader Arayüzü ---
// Farklı dosya uzantılarından modül içeriğini okuma ve işleme sorumluluğunu üstlenir.
class ModuleReader {
public:
    virtual ~ModuleReader() = default;

    // Modül dosyasını okur ve yorumlayıcı tarafından işlenebilecek bir "modül değeri" döndürür.
    // Bu değer, modülün ortamını, AST'sini veya başka bir özel modül objesini içerebilir.
    // ErrorReporter ile hata raporlamalıdır.
    virtual ModulePtr readModule(const std::string& filePath,
                                 const std::string& moduleName,
                                 Interpreter& interpreter) = 0;
};

// --- C-CUBE Modül Okuyucusu (.cube) ---
// .cube uzantılı dosyaları okur, lexer ve parser kullanarak AST'ye dönüştürür.
class CubeModuleReader : public ModuleReader {
public:
    ModulePtr readModule(const std::string& filePath,
                         const std::string& moduleName,
                         Interpreter& interpreter) override;
};

// --- Python Modül Okuyucusu (.py) ---
// Python modüllerini yüklemek için bir yer tutucu. Gerçek implementasyon
// Python C API'si veya pybind11 gibi kütüphaneler gerektirir.
class PythonModuleReader : public ModuleReader {
public:
    ModulePtr readModule(const std::string& filePath,
                         const std::string& moduleName,
                         Interpreter& interpreter) override;
};

// --- C/C++/Shader Modül Okuyucusu (.h, .hpp, .cuh, .cl, .glsl, .hlsl, .metal, .spv) ---
// Bu tür dosyalar doğrudan yürütülemez. Genellikle FFI (Foreign Function Interface)
// veya özel shader/kernel derleme/yükleme mekanizmaları ile entegre olurlar.
// Buradaki implementasyonları sadece dosya yolunu veya içeriğini bir 'modül objesi'
// olarak döndürmek ve gerçek derleme/yükleme işini başka bir yere bırakmak üzerine kuruludur.
class NativeModuleReader : public ModuleReader {
public:
    ModulePtr readModule(const std::string& filePath,
                         const std::string& moduleName,
                         Interpreter& interpreter) override;
};

// --- Fortran Modül Okuyucusu (.mod) ---
// Fortran modül dosyalarını işlemek için yer tutucu.
// Genellikle derlenmiş kütüphanelerle etkileşim için kullanılır.
class FortranModuleReader : public ModuleReader {
public:
    ModulePtr readModule(const std::string& filePath,
                         const std::string& moduleName,
                         Interpreter& interpreter) override;
};

// --- Julia Modül Okuyucusu (.jl) ---
// Julia modüllerini yüklemek için yer tutucu.
// Julia'nın C API'si ile entegrasyon gerektirir.
class JuliaModuleReader : public ModuleReader {
public:
    ModulePtr readModule(const std::string& filePath,
                         const std::string& moduleName,
                         Interpreter& interpreter) override;
};


// --- ModuleLoader: Çekirdek Yükleyici ---
class ModuleLoader {
private:
    // Modül önbelleği: Yüklenen modülleri (modül adı -> modül objesi) saklar
    std::unordered_map<std::string, ModulePtr> moduleCache;

    // Modül kaynak dosyalarının aranacağı yollar
    std::vector<std::string> searchPaths;

    // Uzantıdan ModuleReader'a eşleme
    std::unordered_map<std::string, std::unique_ptr<ModuleReader>> readers;

    // Yardımcı fonksiyon: Dosya yolunu uzantısına göre parçalar
    std::string getFileExtension(const std::string& filePath) const;

    // Yardımcı fonksiyon: Modül dosyasını arama yollarında bulur
    std::string findModuleFile(const std::string& modulePath,
                               const std::vector<std::string>& possibleExtensions) const;

public:
    // Constructor: Interpreter'a referans alır ve arama yollarını ayarlar.
    // Farklı ModuleReader implementasyonlarını burada kaydederiz.
    ModuleLoader(Interpreter& interpreter, const std::vector<std::string>& searchPaths);

    // Modülü yükler ve çalıştırır. Modül objesini döndürür.
    // Eğer modül önbellekte varsa, önbellekten döner.
    // `modulePath` "game.utils" gibi noktalı veya doğrudan "shader.glsl" gibi olabilir.
    ModulePtr loadModule(const std::string& modulePath);

    // Dışarıdan yeni bir ModuleReader eklemek için (eğer dinamik uzantı eklemek istenirse)
    void registerModuleReader(const std::string& extension, std::unique_ptr<ModuleReader> reader);
};

#endif // C_CUBE_MODULE_LOADER_H

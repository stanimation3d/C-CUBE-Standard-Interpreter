#include "module_loader.h"
#include "lexer.h"           // C-CUBE lexing
#include "parser.h"          // C-CUBE parsing
#include "interpreter.h"     // Interpreter'ın global ortamına erişim için
#include "error_reporter.h"  // Hata raporlama için
#include "value.h"           // ValuePtr ve ModulePtr için
#include "c_cube_module.h"   // C_CUBE_Module sınıfı için

#include <fstream>           // File I/O
#include <sstream>           // String stream
#include <iostream>          // Debugging/Error output
#include <filesystem>        // C++17 for path manipulation

// Use C++17 filesystem for path manipulation if available
#ifdef __cpp_lib_filesystem
namespace fs = std::filesystem;
#endif

// Harici bir ErrorReporter instance'ı olduğunu varsayalım
// extern ErrorReporter globalErrorReporter; // Eğer singleton veya global ise

// Yardımcı fonksiyon: Dosya içeriğini okur
static std::string readFileContent(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// --- CubeModuleReader Implementasyonu ---
ModulePtr CubeModuleReader::readModule(const std::string& filePath,
                                       const std::string& moduleName,
                                       Interpreter& interpreter) {
    try {
        std::string source = readFileContent(filePath);

        // Lexing
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.scanTokens();
        // if (globalErrorReporter.hadLexError()) return nullptr; // Lexer hatalarını kontrol et

        // Parsing
        Parser parser(tokens);
        std::vector<StmtPtr> ast;
        try {
            ast = parser.parse();
        } catch (const ParseError& e) {
            // globalErrorReporter.parseError(e.token, e.what());
            std::cerr << "[Module " << moduleName << " @ " << e.token.line << "] Parse Error: " << e.what() << std::endl;
            return nullptr;
        }

        // Modül için yeni bir ortam oluştur (built-in'lere erişebilmeli)
        // Eğer Interpreter'da built-in'leri tutan ayrı bir ortam varsa, onu parent yapın.
        // Aksi takdirde, main Interpreter'ın global ortamını kullanabiliriz.
        // Burada Interpreter'ın global ortamını parent olarak kullanıyoruz.
        EnvironmentPtr moduleEnv = std::make_shared<Environment>(interpreter.globals);

        // Modül objesini oluştur
        ModulePtr loadedModule = std::make_shared<Module>(std::move(ast), moduleEnv);

        // Modülün AST'sini yorumlayıcı aracılığıyla, kendi ortamında yürüt
        // Bu, C-CUBE modüllerinin `import` edildiğinde otomatik olarak çalıştırıldığı anlamına gelir.
        // Interpreter'ın ortamını geçici olarak modül ortamına ayarla
        EnvironmentPtr originalEnv = interpreter.environment;
        interpreter.environment = moduleEnv;
        try {
            interpreter.interpret(loadedModule->ast); // Modülün kodunu çalıştır
        } catch (const std::runtime_error& e) {
            // globalErrorReporter.runtimeError(?, "Runtime error in module " + moduleName + ": " + e.what());
            std::cerr << "Runtime Error in module '" << moduleName << "': " << e.what() << std::endl;
            interpreter.environment = originalEnv; // Ortamı geri yükle
            return nullptr;
        }
        interpreter.environment = originalEnv; // Ortamı geri yükle

        return loadedModule;

    } catch (const std::runtime_error& e) {
        std::cerr << "Error loading C-CUBE module '" << moduleName << "': " << e.what() << std::endl;
        return nullptr;
    }
}

// --- PythonModuleReader Implementasyonu ---
// Bu, Python C API'si veya pybind11 gibi bir entegrasyon gerektirir.
// Şimdilik sadece bir yer tutucu ve dosya yolunu içeren basit bir modül objesi döndürür.
ModulePtr PythonModuleReader::readModule(const std::string& filePath,
                                         const std::string& moduleName,
                                         Interpreter& interpreter) {
    std::cerr << "Warning: Python module loading is not fully implemented. File: " << filePath << std::endl;
    // Gerçek bir implementasyonda:
    // 1. Python yorumlayıcısını başlat (Py_Initialize()).
    // 2. PyObject* pModule = PyImport_ImportModule("module_name");
    // 3. Modülün sözlüğünü al: PyObject* pDict = PyModule_GetDict(pModule);
    // 4. Bu Python objesini C-CUBE'un ValuePtr sistemine saracak özel bir C_CUBE_PythonModule objesi oluştur.
    // 5. Bu özel objenin 'get' metodu, Python modülündeki attribute'lara erişimi sağlar.

    // Geçici olarak, sadece bir "native" modül gibi dosya yolunu içeren bir modül objesi döndürelim.
    EnvironmentPtr moduleEnv = std::make_shared<Environment>(); // Boş bir ortam
    // Bir C_CUBE_NativeModule objesi oluşturulabilir (C_CUBE_Object'ten türemiş)
    // Bu objenin field'ları, Python modülündeki öğeleri taklit edebilir.
    // Örneğin, 'path' adında bir field ekleyelim:
    // moduleEnv->define("path", std::make_shared<Value>(filePath));
    // moduleEnv->define("name", std::make_shared<Value>(moduleName));
    return std::make_shared<Module>(std::vector<StmtPtr>{}, moduleEnv);
}

// --- NativeModuleReader Implementasyonu ---
// Bu okuyucu, .h, .hpp, .cuh, .cl, .glsl, .hlsl, .metal, .spv uzantılarını işler.
// Bu dosyalar doğrudan yorumlanamaz, bu yüzden sadece dosya yolunu tutan
// bir "native modül" objesi döndürür. Gerçek entegrasyon (örn. FFI, shader derleme)
// daha sonra Interpreter'da veya özel bir built-in fonksiyonda gerçekleşir.
ModulePtr NativeModuleReader::readModule(const std::string& filePath,
                                         const std::string& moduleName,
                                         Interpreter& interpreter) {
    std::cout << "Debug: Loading native/header module (path only): " << filePath << std::endl;

    // Gerçek implementasyonda:
    // - .h/.hpp/.cuh için: C++ FFI (Foreign Function Interface) mekanizması ile C++ fonksiyonlarına
    //   erişimi sağlayan bir proxy objesi döndürülebilir.
    // - .cl/.glsl/.hlsl/.metal için: Shader/Kernel kodunu doğrudan dosya yolunu tutan bir objeye sarın.
    //   Bu objenin metotları, shader'ı derleme, OpenGL/Vulkan/DirectX API'sine gönderme gibi işlevleri sağlar.
    // - .spv (SPIR-V): Derlenmiş shader kodunu içerir. Doğrudan GPU'ya gönderilebilir.

    EnvironmentPtr moduleEnv = std::make_shared<Environment>();
    // C_CUBE_NativeModule adlı özel bir ValuePtr türü tanımlayabiliriz.
    // Şimdilik, sadece dosya yolunu içeren bir string değeri olarak tutalım veya
    // özel bir Object'e saralım.
    moduleEnv->define("path", std::make_shared<Value>(filePath));
    moduleEnv->define("name", std::make_shared<Value>(moduleName));

    // Örneğin, bir "load_shader_program(module.path)" built-in fonksiyonu olabilir.
    return std::make_shared<Module>(std::vector<StmtPtr>{}, moduleEnv);
}

// --- FortranModuleReader Implementasyonu ---
ModulePtr FortranModuleReader::readModule(const std::string& filePath,
                                          const std::string& moduleName,
                                          Interpreter& interpreter) {
    std::cerr << "Warning: Fortran module loading is not implemented. File: " << filePath << std::endl;
    // Gerçek implementasyonda: Fortran derlenmiş modüllerle FFI veya özel araçlarla entegrasyon.
    EnvironmentPtr moduleEnv = std::make_shared<Environment>();
    moduleEnv->define("path", std::make_shared<Value>(filePath));
    moduleEnv->define("name", std::make_shared<Value>(moduleName));
    return std::make_shared<Module>(std::vector<StmtPtr>{}, moduleEnv);
}

// --- JuliaModuleReader Implementasyonu ---
ModulePtr JuliaModuleReader::readModule(const std::string& filePath,
                                        const std::string& moduleName,
                                        Interpreter& interpreter) {
    std::cerr << "Warning: Julia module loading is not implemented. File: " << filePath << std::endl;
    // Gerçek implementasyonda: Julia'nın C API'si ile entegrasyon.
    EnvironmentPtr moduleEnv = std::make_shared<Environment>();
    moduleEnv->define("path", std::make_shared<Value>(filePath));
    moduleEnv->define("name", std::make_shared<Value>(moduleName));
    return std::make_shared<Module>(std::vector<StmtPtr>{}, moduleEnv);
}


// --- ModuleLoader Core Implementasyonu ---

// Constructor: Interpreter ve arama yollarını ayarla, okuyucuları kaydet
ModuleLoader::ModuleLoader(Interpreter& interpreter, const std::vector<std::string>& searchPaths)
    : searchPaths(searchPaths) {
    // Burada her dosya uzantısı için ilgili ModuleReader'ı kaydediyoruz
    registerModuleReader(".cube", std::make_unique<CubeModuleReader>());
    registerModuleReader(".py", std::make_unique<PythonModuleReader>());
    registerModuleReader(".h", std::make_unique<NativeModuleReader>());
    registerModuleReader(".hpp", std::make_unique<NativeModuleReader>());
    registerModuleReader(".cuh", std::make_unique<NativeModuleReader>());
    registerModuleReader(".cl", std::make_unique<NativeModuleReader>());
    registerModuleReader(".glsl", std::make_unique<NativeModuleReader>());
    registerModuleReader(".hlsl", std::make_unique<NativeModuleReader>());
    registerModuleReader(".spv", std::make_unique<NativeModuleReader>()); // SPV doğrudan ikili format olduğundan işlenmesi farklıdır.
                                                                           // Burada yine de dosya yolunu tutan bir obje döndürülebilir.
    registerModuleReader(".metal", std::make_unique<NativeModuleReader>());
    registerModuleReader(".jl", std::make_unique<JuliaModuleReader>());
    registerModuleReader(".mod", std::make_unique<FortranModuleReader>()); // Fortran modül dosyası uzantısı
}

// Yeni bir okuyucu kaydet
void ModuleLoader::registerModuleReader(const std::string& extension, std::unique_ptr<ModuleReader> reader) {
    if (extension.empty() || extension[0] != '.') {
        std::cerr << "Error: Module reader extension must start with a dot (e.g., '.cube')." << std::endl;
        return;
    }
    readers[extension] = std::move(reader);
}

// Dosya uzantısını alma yardımcı fonksiyonu
std::string ModuleLoader::getFileExtension(const std::string& filePath) const {
    size_t dotPos = filePath.rfind('.');
    if (dotPos == std::string::npos) {
        return ""; // Uzantı yok
    }
    return filePath.substr(dotPos);
}

// Modül dosyasını arama yollarında bulur
std::string ModuleLoader::findModuleFile(const std::string& modulePath,
                                       const std::vector<std::string>& possibleExtensions) const {
    // 1. Modül yolu zaten bir uzantı içeriyor mu kontrol et (örn: "shader.glsl")
    std::string explicitExtension = getFileExtension(modulePath);
    std::string baseModulePath = modulePath;
    std::vector<std::string> extensionsToTry = possibleExtensions;

    if (!explicitExtension.empty() && std::find(possibleExtensions.begin(), possibleExtensions.end(), explicitExtension) != possibleExtensions.end()) {
        // Path already contains a valid explicit extension, use it directly
        baseModulePath = modulePath.substr(0, modulePath.length() - explicitExtension.length());
        extensionsToTry = {explicitExtension}; // Sadece bu uzantıyı dene
    }

    // "game.utils" -> "game/utils"
    for (char& c : baseModulePath) {
        if (c == '.') {
            #ifdef _WIN32
            c = '\\';
            #else
            c = '/';
            #endif
        }
    }

    // Arama yollarında ve olası uzantılarla dosyayı ara
    for (const auto& searchPath : searchPaths) {
        for (const auto& ext : extensionsToTry) {
            #ifdef __cpp_lib_filesystem
            fs::path fullPath = fs::path(searchPath) / (baseModulePath + ext);
            if (fs::exists(fullPath) && fs::is_regular_file(fullPath)) {
                return fullPath.string(); // Dosya bulundu
            }
            #else
            // C++17 filesystem olmadan basit kontrol
            std::string currentFullPath = searchPath;
            if (!currentFullPath.empty() && currentFullPath.back() != '/' && currentFullPath.back() != '\\') {
                 #ifdef _WIN32
                 currentFullPath += '\\';
                 #else
                 currentFullPath += '/';
                 #endif
            }
            currentFullPath += (baseModulePath + ext);
            std::ifstream file(currentFullPath);
            if (file.good()) {
                return currentFullPath;
            }
            #endif
        }
    }
    return ""; // Bulunamadı
}

// Modülü yükle ve çalıştır
ModulePtr ModuleLoader::loadModule(const std::string& modulePath) {
    // 1. Önbellekte var mı kontrol et
    if (moduleCache.count(modulePath)) {
        std::cout << "Debug: Module '" << modulePath << "' found in cache." << std::endl;
        return moduleCache.at(modulePath);
    }

    std::cout << "Debug: Attempting to load module '" << modulePath << "' from file." << std::endl;

    // Olası tüm uzantıları topla
    std::vector<std::string> allExtensions;
    for (const auto& pair : readers) {
        allExtensions.push_back(pair.first);
    }

    // 2. Modül dosyasını bul (tüm olası uzantılarla)
    std::string filePath = findModuleFile(modulePath, allExtensions);
    if (filePath.empty()) {
        std::cerr << "Runtime Error: Module '" << modulePath << "' not found in search paths with any known extension." << std::endl;
        return nullptr;
    }

    // 3. Dosya uzantısını al
    std::string extension = getFileExtension(filePath);
    if (extension.empty()) {
        std::cerr << "Runtime Error: Module file '" << filePath << "' has no recognized extension." << std::endl;
        return nullptr;
    }

    // 4. Uzantıya göre uygun ModuleReader'ı seç
    if (readers.count(extension) == 0) {
        std::cerr << "Runtime Error: No module reader registered for extension '" << extension << "'." << std::endl;
        return nullptr;
    }

    ModuleReader* reader = readers.at(extension).get();

    // 5. Modül adını belirle (genellikle uzantısız kısım)
    std::string moduleName = modulePath;
    if (modulePath.length() > extension.length() && modulePath.substr(modulePath.length() - extension.length()) == extension) {
        moduleName = modulePath.substr(0, modulePath.length() - extension.length());
    }

    // 6. Reader ile modülü oku ve işle
    ModulePtr loadedModule = reader->readModule(filePath, moduleName, interpreter);

    // 7. Hata varsa dön
    if (!loadedModule) {
        std::cerr << "Error: Module '" << modulePath << "' failed to load via reader for '" << extension << "'." << std::endl;
        return nullptr;
    }

    // 8. Modülü önbelleğe al
    moduleCache[modulePath] = loadedModule;

    std::cout << "Debug: Module '" << modulePath << "' loaded and cached successfully." << std::endl;

    return loadedModule;
}

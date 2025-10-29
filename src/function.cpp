#include "function.h"
#include "interpreter.h" // Interpreter sınıfını kullanıyoruz
#include "environment.h" // Environment sınıfını kullanıyoruz
#include "ast.h"       // AST düğümlerini kullanıyoruz (BlockStmt)
#include "value.h"     // ValuePtr kullanıyoruz
#include <iostream>    // Hata ayıklama için

// Runtime'da return deyiminden çıkmak için özel bir istisna sınıfı
// Bu, interpreter'ın call metodunda yakalanır.
struct ReturnException : public std::runtime_error {
    ValuePtr value; // Döndürülen değer
    ReturnException(ValuePtr val) : std::runtime_error("Return"), value(val) {}
};


// C_CUBE_Function sınıfı implementasyonu

// Constructor zaten .h dosyasında tanımlandı.

// Fonksiyonu çağırma metodunun implementasyonu
Value CCubeFunction::call(Interpreter& interpreter, const std::vector<Value>& arguments, std::shared_ptr<CCubeInstance> this_instance) {
    std::shared_ptr<Environment> function_environment = std::make_shared<Environment>(closure);
    if (this_instance != nullptr) {
        function_environment->define("this", this_instance);
    }
    // Parametreleri yeni ortama tanımla
    for (size_t i = 0; i < declaration->params.size(); ++i) {
        function_environment->define(declaration->params[i].lexeme, arguments[i]);
    }
    // ... (geri kalan fonksiyon gövdesini yürütme)
    try {
        interpreter.executeBlock(declaration->body->statements, function_environment);
    } catch (const ReturnException& result) {
        if (isInitializer) return this_instance; // Kurucular her zaman instance'ı döndürür
        return result.value;
    }
    if (isInitializer) return this_instance; // Kurucular her zaman instance'ı döndürür
    return std::monostate{};
}
    EnvironmentPtr environment = std::make_shared<Environment>(closure);

    // Eğer bu bir metot ise, 'this' referansını yeni ortama bağla
    
    if (instance) {
        environment->define("this", std::make_shared<Value>(instance));
    }
    

    // 2. Parametreleri argüman değerlerine bağla (yeni ortamda tanımla)
    for (size_t i = 0; i < parameters.size(); ++i) {
        // Parametre adı: parameters[i].lexeme
        // Argüman değeri: arguments[i]
        environment->define(parameters[i].lexeme, arguments[i]);
    }

    // 3. Fonksiyon gövdesini çalıştır (genellikle bir BlockStmt)
    // Interpreter'ın executeBlock metodunu kullanarak yeni ortamda çalıştır
    try {
        // body, genellikle bir BlockStmt düğümüne işaret eden StmtPtr olmalıdır
        if (auto block_stmt = std::dynamic_pointer_cast<BlockStmt>(body)) {
             interpreter.executeBlock(block_stmt->statements, environment);
        } else {
             // Eğer gövde BlockStmt değilse, doğrudan execute edilebilir (tek deyimli fonksiyonlar?)
             interpreter.execute(body);
        }
        // Normalde fonksiyonun sonuna ulaşılırsa 'none' döndürülür
         // Eğer bu bir initializer ise, 'this' objesini döndür (yapıcılar objeyi döndürür)
        
        if (isInitializer) {
            return std::make_shared<Value>(instance);
        }
        
        return std::make_shared<Value>(); // Varsayılan dönüş değeri (none)

    } catch (const ReturnException& returnValue) {
        // Return deyiminden fırlatılan özel istisnayı yakala
        // Eğer bu bir initializer ise, daima 'this' objesini döndür (return değeri yok sayılır)
        
        if (isInitializer) {
            return std::make_shared<Value>(instance);
        }
        
        return returnValue.value; // Döndürülen değeri al ve fonksiyondan çık
    } catch (const std::runtime_error& e) {
         // Diğer runtime hatalarını burada yakalayabilir veya interpreter'ın üst seviye yakalamasına bırakabilirsiniz.
         // Şimdilik yeniden fırlatalım:
         throw e;
    }
}

// Metot binding'i (Object sınıfı implement edildiğinde kullanılacak)
C_CUBE_FunctionPtr C_CUBE_Function::bind(std::shared_ptr<C_CUBE_Object> instance) {
    // Fonksiyonun closure ortamını genişleterek yeni bir ortam oluştur
    // Bu yeni ortam, 'this' adını instance objesine bağlar.
    EnvironmentPtr bound_closure = std::make_shared<Environment>(closure);
    bound_closure->define("this", std::make_shared<Value>(instance)); // instance ValuePtr olmalı

    // Bağlanmış fonksiyonun yeni bir kopyasını oluştur
    // Bu kopya aynı parametreleri, gövdeyi kullanır ama bağlı closure'a sahiptir.
    // Initializer durumu da kopyalanır.
    return std::make_shared<C_CUBE_Function>(parameters, body, bound_closure, isInitializer);
}
size_t CCubeFunction::getSize() const {
    // Sadece Function objesinin kendisinin ve pointerlarının boyutunu düşünün.
    // Closure ortamının boyutu ayrı olarak GC tarafından yönetilir.
    return sizeof(CCubeFunction); // + declaration.size() + closure.size() (rough estimation)
}

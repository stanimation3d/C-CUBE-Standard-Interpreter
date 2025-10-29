#include "class.h"
#include "interpreter.h" // Interpreter sınıfını kullanıyoruz (call metodunda)
#include "object.h"      // C_CUBE_Object nesneleri oluşturuyoruz
#include "function.h"    // Metotları kullanıyoruz (C_CUBE_Function)
#include "value.h"       // ValuePtr kullanıyoruz

#include <iostream>
#include <stdexcept> // std::runtime_error için

// C_CUBE_Class sınıfı implementasyonu

// Constructor zaten .h dosyasında tanımlandı.

// Metot arama
C_CUBE_FunctionPtr C_CUBE_Class::findMethod(const std::string& name) {
    // Önce kendi metotlarımızda ara
    if (methods.count(name)) {
        return methods.at(name);
    }

    // Kendi metotlarımızda yoksa, üst sınıfta ara (miras)
    if (superclass) {
        return superclass->findMethod(name); // Rekürsif arama
    }

    // Ne kendi metotlarımızda ne de üst sınıflarda bulundu
    return nullptr;
}

// Arity (Yapıcı metodun arity'si)
int C_CUBE_Class::arity() const {
    // "init" metodunu ara (yapıcı)
    C_CUBE_FunctionPtr initializer = findMethod("init");
    if (initializer) {
        // Eğer init metodu varsa, onun arity'sini döndür
        return initializer->arity();
    }
    // Init metodu yoksa, 0 argüman alır (varsayılan yapıcı gibi)
    return 0;
}

// Sınıfı çağırma (Nesne oluşturma)
ValuePtr C_CUBE_Class::call(Interpreter& interpreter, const std::vector<ValuePtr>& arguments) {
    // 1. Yeni bir nesne örneği (instance) oluştur
    // Nesne, kendisine ait olduğu sınıfın shared_ptr'ını tutmalı.
     std::enable_shared_from_this<C_CUBE_Class> sayesinde this objesine shared_ptr alabiliriz.
    C_CUBE_ObjectPtr instance = std::make_shared<C_CUBE_Object>(shared_from_this());
    ValuePtr instance_value = std::make_shared<Value>(instance); // Nesneyi ValuePtr içine sar

    // 2. Init metodunu (yapıcı) ara
    C_CUBE_FunctionPtr initializer = findMethod("init");

    // 3. Init metodu varsa, nesneye bağla ve çağır
    if (initializer) {
        // Init metodunu yeni oluşturulan nesne örneğine bağla
        // C_CUBE_Function'a bind metodu eklenmişti.
        // isInitializer flag'i init metotlar için true olmalı (Function constructor'da set edilebilir)
         C_CUBE_FunctionPtr boundInitializer = initializer->bind(instance, true); // isInitializer=true
        // Geçici olarak bind kullanmadan (initializer constructor'ında closure'a instance ekleyerek yapılabilir)
         EnvironmentPtr init_env = std::make_shared<Environment>(initializer->closure);
         init_env->define("this", instance_value); // 'this'i instance'a bağla
         C_CUBE_Function temp_init_func(initializer->parameters, initializer->body, init_env/*, true*/);


        // Bağlanmış init metodunu çağır
        try {
             // temp_init_func.call(interpreter, arguments);
             // Normalde Function::call içindeki ReturnException, init metotlarda
             // daima instance'ı döndürecek şekilde ayarlanmalıdır.
             interpreter.callFunction(std::make_shared<C_CUBE_Function>(initializer->parameters, initializer->body, init_env/*, true*/), arguments); // Interpreter'a fonksiyon çağırma metodu eklenmeli
        } catch (const ReturnException& returnValue) {
             // Init metodundan return gelirse (init'lerde return değeri genellikle yok sayılır)
             // Init metotlar daima instance'ı döndürmeli.
             // Bu catch bloğu, eğer Function::call içinde init için özel dönüş mantığı yoksa
             // gerekebilir. Genellikle Function::call init için doğru değeri döner.
        } catch (const std::runtime_error& e) {
             // Diğer hatalar için
             throw e;
        }

    } else {
        // Init metodu yoksa, 0 argümanla çağrılmalıydı.
        if (!arguments.empty()) {
              runtimeError(?, "Expected 0 arguments but got " + std::to_string(arguments.size()) + ".");
              std::cerr << "Runtime Error: Expected 0 arguments but got " << arguments.size() << "." << std::endl;
              throw std::runtime_error("Expected 0 arguments but got " + std::to_string(arguments.size()) + ".");
        }
    }


    // 4. Yeni oluşturulan ve init metodu (varsa) çalıştırılmış nesneyi döndür.
    return instance_value; // ValuePtr olarak sarılmış nesne
}

// String temsilini döndürür
 const std::string& C_CUBE_Class::getName() const { return name; } // Getter metodu (isteğe bağlı)

size_t CCubeClass::getSize() const {
    // Sınıfın kendi boyutu + name string boyutu + metotların harita boyutu
    size_t total_size = sizeof(CCubeClass) + name.capacity(); // name.length() + null terminator
    // Metotların kendileri ayrı ObjPtr'lar olarak yönetildiğinden burada sadece pointer maliyetini hesaplayın.
    total_size += methods.size() * (sizeof(std::string) + sizeof(std::shared_ptr<CCubeFunction>)); // Rough estimation
    return total_size;
}

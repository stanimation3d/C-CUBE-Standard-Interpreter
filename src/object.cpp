#include "object.h"
#include "class.h" // C_CUBE_Class tanımı için gerekli
#include "function.h" // Metotlar C_CUBE_Function nesneleridir
#include "value.h"
#include "token.h"
#include <iostream>
#include <stdexcept> // std::runtime_error için


// C_CUBE_Object sınıfı implementasyonu

// Constructor
C_CUBE_Object::C_CUBE_Object(C_CUBE_ClassPtr klass) : klass(klass) {
    // Alanlar map'i otomatik olarak boş başlar.
    // Sınıfın init metodu çalıştığında alanlar doldurulur.
}

// Alan değerini alma veya metot bulma
ValuePtr C_CUBE_Object::get(const Token& name) {
    // Önce alanlarda ara
    if (fields.count(name.lexeme)) {
        return fields.at(name.lexeme);
    }

    // Alanlarda yoksa, sınıfın metotlarında ara
    // Bu kısım C_CUBE_Class sınıfına bir getMethod metodu eklenmesini gerektirir.
     C_CUBE_FunctionPtr method = klass->findMethod(name.lexeme); // C_CUBE_Class'a bu metot eklenecek

    
    if (method) {
        // Metot bulunduysa, bu nesneye (this) bağlı bir kopya döndür (metot binding)
        // C_CUBE_Function'a bind metodu eklenmişti.
        // this'e shared_ptr almak için std::enable_shared_from_this gereklidir.
        return std::make_shared<Value>(method->bind(shared_from_this())); // ValuePtr olarak sar
    }
    

    // Ne alan ne de metot bulunamadıysa runtime hatası
     runtimeError(name, "Undefined property '" + name.lexeme + "'.");
    std::cerr << "[Line " << name.line << "] Runtime Error: Undefined property '" << name.lexeme << "'." << std::endl;
     hadRuntimeError = true;
     throw std::runtime_error("Undefined property '" + name.lexeme + "'"); // İstisna fırlatma seçeneği

    return nullptr; // Hata durumunda veya bulunamadığında nullptr döndür (veya özel None değeri)
}

// Alan değeri atama veya yeni alan ekleme
void C_CUBE_Object::set(const Token& name, ValuePtr value) {
    // Alan mevcutsa güncellenir, yoksa yeni alan eklenir.
    fields[name.lexeme] = value;
}

// Nesnenin string temsili
std::string C_CUBE_Object::toString() const {
    // Ait olduğu sınıfın adını kullanarak string oluştur
    // Bu kısım C_CUBE_Class sınıfına bir getName metodu eklenmesini gerektirir.
     return "<instance of " + klass->getName() + ">"; // C_CUBE_Class'a getName eklenecek
     return "<instance>"; // Şimdilik jenerik isim
}

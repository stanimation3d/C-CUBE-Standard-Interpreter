#ifndef C_CUBE_BUILTIN_FUNCTIONS_H
#define C_CUBE_BUILTIN_FUNCTIONS_H

#include "callable.h" // Callable arayüzünü kullanıyoruz
#include "value.h"    // ValuePtr kullanıyoruz

#include <vector>
#include <string>
#include <memory> // std::shared_ptr için

// Interpreter sınıfını kullanacağımız için ileri bildirim
class Interpreter;

// Built-in 'print' fonksiyonu
class BuiltinPrint : public Callable {
public:
    // Constructor (eğer gerekiyorsa, şimdilik boş)
    BuiltinPrint() {}

    // print fonksiyonu tek bir argüman alır
    int arity() const override { return 1; }

    // print fonksiyonunu çağırır
    ValuePtr call(Interpreter& interpreter, const std::vector<ValuePtr>& arguments) override;

    // String temsilini döndürür
    std::string toString() const override {
        return "<fn print>";
    }
};

// Built-in 'clock' fonksiyonu (Saniye cinsinden geçen zamanı döndürür)
class BuiltinClock : public Callable {
public:
    // Constructor
    BuiltinClock() {}

    // clock fonksiyonu hiçbir argüman almaz
    int arity() const override { return 0; }

    // clock fonksiyonunu çağırır
    ValuePtr call(Interpreter& interpreter, const std::vector<ValuePtr>& arguments) override;

    // String temsilini döndürür
    std::string toString() const override {
        return "<fn clock>";
    }
};

// Ek built-in fonksiyonlar buraya eklenebilir (örn: input, len, tip dönüşümleri vb.)
// Oyun geliştirme odaklı built-in'ler (örn: get_entity, spawn_actor, load_scene) daha sonra buraya eklenecektir.


#endif // C_CUBE_BUILTIN_FUNCTIONS_H

#include "builtin_functions.h"
#include "interpreter.h" // Interpreter sınıfını kullanıyoruz (call metodunda)
#include "value.h"       // ValuePtr kullanıyoruz
#include <iostream>      // std::cout, std::cerr için
#include <vector>
#include <string>
#include <chrono>        // clock fonksiyonu için

// BuiltinPrint implementasyonu

ValuePtr BuiltinPrint::call(Interpreter& interpreter, const std::vector<ValuePtr>& arguments) {
    // Argüman sayısını kontrol et (arity 1 olmalı)
    if (arguments.size() != arity()) {
        // Hata raporlama (lexer/parser hatalarından farklı olabilir)
        std::cerr << "Runtime Error: Function 'print' expected " << arity()
                  << " arguments but got " << arguments.size() << "." << std::endl;
        // Hata durumunda nullptr veya özel bir hata değeri döndürün
         interpreter.hadRuntimeError = true; // Interpreter'da bir flag varsa
        return nullptr; // Hata durumunda
    }

    // Argümanı Value'dan çıkar ve ekrana yazdır
    // valueToString yardımcı fonksiyonunu kullanmak idealdir (value.h veya utils.h'de olmalı)
    // Şimdilik Value variantına doğrudan erişelim (eksik tip desteği olabilir)
    ValuePtr arg = arguments[0];
    if (arg) {
        if (std::holds_alternative<std::string>(*arg)) {
            std::cout << std::get<std::string>(*arg);
        } else if (std::holds_alternative<double>(*arg)) {
            // Sayıları farklı formatlarda yazdırabilirsiniz
            std::cout << std::get<double>(*arg);
        } else if (std::holds_alternative<bool>(*arg)) {
            std::cout << (std::get<bool>(*arg) ? "true" : "false");
        } else if (std::holds_alternative<std::monostate>(*arg)) {
            std::cout << "none"; // None değerini yazdırma
        }
        // Diğer Value tipleri için (Object, Function, List, Dict vb.) buraya durum ekleyin
         else if (std::holds_alternative<C_CUBE_ObjectPtr>(*arg)) {
            std::cout << std::get<C_CUBE_ObjectPtr>(*arg)->toString();
         }
         else {
             // Bilinmeyen veya desteklenmeyen tip
             std::cout << "[unprintable value]";
         }
    } else {
         std::cout << "null"; // Eğer ValuePtr nullptr ise
    }
    std::cout << std::endl; // Her print'ten sonra yeni satır ekle (Python'daki gibi)

    // Print fonksiyonu genellikle bir değer döndürmez (void), bu yüzden 'none' döndürülür
    return std::make_shared<Value>(); // std::monostate (None) içeren Value
}

// BuiltinClock implementasyonu

ValuePtr BuiltinClock::call(Interpreter& interpreter, const std::vector<ValuePtr>& arguments) {
    // Argüman sayısını kontrol et (arity 0 olmalı)
    if (arguments.size() != arity()) {
         std::cerr << "Runtime Error: Function 'clock' expected " << arity()
                  << " arguments but got " << arguments.size() << "." << std::endl;
         interpreter.hadRuntimeError = true;
        return nullptr; // Hata durumunda
    }

    // Geçen zamanı saniye cinsinden al (double olarak döndürelim)
    // std::chrono kullanarak daha modern ve hassas zaman alabiliriz
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();

    // Saniye değerini bir ValuePtr içinde sarıp döndür
    return std::make_shared<Value>(seconds);
}

// Ek built-in fonksiyonların implementasyonları buraya eklenecektir.

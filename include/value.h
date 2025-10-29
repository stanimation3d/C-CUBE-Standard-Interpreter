#ifndef C_CUBE_VALUE_H
#define C_CUBE_VALUE_H

#include <variant>
#include <string>
#include <memory> // std::shared_ptr için
#include <vector> // Eğer listeler de Value'da tutuluyorsa
#include <iostream> // Debug için

// İleri bildirim: Object sınıfını tanıtırız
class Object;

// ObjPtr: Object sınıfının paylaşılan akıllı işaretçisi.
// Tüm GC tarafından yönetilen nesneler bu tipte tutulacaktır.
using ObjPtr = std::shared_ptr<Object>;

// Value: C-CUBE dilindeki farklı değer tiplerini tutan bir varyant.
// ObjPtr'ı da içerecek şekilde güncellendi.
using Value = std::variant<
    std::monostate,   // Karşılığı 'none'
    bool,             // Boolean değerler
    double,           // Sayısal değerler
    std::string,      // String değerler
    ObjPtr            // Çöp toplayıcı tarafından yönetilen objeler (Function, Class, Instance, List, Module, BoundMethod vb.)
>;

// Value'yu string'e dönüştürmek için yardımcı fonksiyon (utils.h veya burada bulunabilir)
// Bu fonksiyonun implementasyonunu utils.cpp'de veya burada yapabiliriz.
// Şimdilik sadece imzayı bırakıyorum.

 std::string valueToString(const Value& value); // utils.h'de olduğu varsayılır.

#endif // C_CUBE_VALUE_H

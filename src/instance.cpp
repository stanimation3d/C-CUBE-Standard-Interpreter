#include "instance.h"
#include "class.h"      // CCubeClass tanımı için
#include "error_reporter.h" // RuntimeException için
#include "utils.h"      // valueToString için

// Constructor
CCubeInstance::CCubeInstance(std::shared_ptr<CCubeClass> klass) : klass(klass) {}

// Bir özelliğin değerini alır
Value CCubeInstance::get(const Token& name) {
    // Önce instance'ın kendi özelliklerinde ara
    if (properties.count(name.lexeme)) {
        return properties.at(name.lexeme);
    }

    // Instance'da bulunamazsa, sınıfın metotlarında ara
    std::shared_ptr<CCubeFunction> method = klass->findMethod(name.lexeme);
    if (method != nullptr) {
        // Metodu mevcut instance'a bağla ve döndür.
        // Bu BoundMethod nesnesini GC'ye kaydetmek gerekir,
        // ancak bu metot çağrısı Interpreter'dan yapıldığı için,
        // Interpreter'ın döndürülen BoundMethod'u GC'ye kaydetmesi gerekir.
        // Interpreter'da visitGetExpr kısmında bu zaten yapılıyor.
        return std::make_shared<BoundMethod>(shared_from_this(), method);
    }

    throw RuntimeException(name, "'" + name.lexeme + "' adlı özellik bulunamadı.");
}

// Bir özelliğe değer atar
void CCubeInstance::set(const Token& name, Value value) {
    properties[name.lexeme] = value;
}

// Object arayüzünden toString implementasyonu
std::string CCubeInstance::toString() const {
    return "<instance of " + klass->toString() + ">";
}

// GC için boyut hesaplama
size_t CCubeInstance::getSize() const {
    // CCubeInstance'ın kendi boyutu
    size_t total_size = sizeof(CCubeInstance);
    // Özellikler haritasının boyutu
    for (const auto& pair : properties) {
        total_size += pair.first.capacity(); // Anahtar string boyutu
        // Value'nun boyutu, içinde ObjPtr varsa onu ayrı hesaplayacağız.
        // Burada sadece Value nesnesinin kendisinin boyutu (variant'ın boyutu)
        total_size += sizeof(Value);
    }
    return total_size;
}

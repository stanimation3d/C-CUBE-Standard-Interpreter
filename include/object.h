#ifndef C_CUBE_OBJECT_H
#define C_CUBE_OBJECT_H

#include <string>
#include <memory> // std::shared_ptr için

// İleri bildirimler (circular dependency'den kaçınmak için)
// Eğer ObjPtr içinde Object tanımı varsa, bu döngüsel bağımlılık oluşabilir.
// Bu yüzden ObjPtr için bir typedef veya ayrı bir forward declaration kullanmak önemlidir.
// ObjPtr'ı burada tanımlamayacağız, value.h'de tanımlayacağız.

class Object {
public:
    // GC tarafından yönetilen obje tipleri
    enum class ObjectType {
        FUNCTION,
        CLASS,
        INSTANCE,
        LIST,
        C_CUBE_MODULE,
        BOUND_METHOD,
        // Diğer obje tipleri buraya eklenebilir (örn. DICTIONARY, TUPLE vb.)
    };

    virtual ~Object() = default; // Sanal yıkıcı

    // Her objenin tipini döndürmesi gerekir (GC ve runtime type checking için)
    virtual ObjectType getType() const = 0;

    // Her objenin string temsilini döndürmesi gerekir
    virtual std::string toString() const = 0;

    // GC için objenin yaklaşık bellek boyutunu döndürür.
    // Bu, GC'nin eşiklerini yönetmesine yardımcı olur.
    virtual size_t getSize() const = 0;

    // Bu objenin çağrılabilir olup olmadığını kontrol eder (Callable arayüzünü uyguluyorsa true)
    bool isCallable() const {
        return getType() == ObjectType::FUNCTION ||
               getType() == ObjectType::CLASS ||
               getType() == ObjectType::BOUND_METHOD;
    }
};

// ObjPtr, C-CUBE objeleri için akıllı işaretçi.
// Bu typedef, object.h veya value.h'de tanımlanabilir.
// value.h'de tanımlanması, Value variant'ında kullanılacağı için daha mantıklı.
// Buraya sadece forward declaration ekleyelim:
 class Object; // Zaten tanımlı
 using ObjPtr = std::shared_ptr<Object>; // Bu value.h'de olacak

#endif // C_CUBE_OBJECT_H

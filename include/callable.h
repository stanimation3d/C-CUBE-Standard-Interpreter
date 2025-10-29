#ifndef C_CUBE_CALLABLE_H
#define C_CUBE_CALLABLE_H

#include <vector>
#include <string>
#include <memory>

#include "value.h" // Çağrılabilir nesneler argüman ve dönüş değerleri olarak Value kullanır

// İleri bildirim
class Interpreter;

class Callable {
public:
    virtual ~Callable() = default; // Sanal yıkıcı

    // Çağrılabilir nesnenin beklediği argüman sayısını döndürür
    virtual size_t arity() const = 0;

    // Çağrılabilir nesneyi yürütür
    virtual Value call(Interpreter& interpreter, const std::vector<Value>& arguments) = 0;
};

// BoundMethod sınıfı (genellikle ayrı bir dosyada tanımlanır, örneğin bound_method.h)
// Bir sınıf metodunu bir instance'a bağlar.
// Bu sınıf da ObjPtr olarak yönetildiği için Object'ten türemelidir.
#ifndef C_CUBE_BOUND_METHOD_H
#define C_CUBE_BOUND_METHOD_H

#include "object.h"      // Temel Object sınıfı
#include "callable.h"    // Callable arayüzü
#include "instance.h"    // Bağlanılacak instance için
#include "function.h"    // Bağlanılacak fonksiyon için

class BoundMethod : public Object, public Callable {
public:
    std::shared_ptr<CCubeInstance> instance;
    std::shared_ptr<CCubeFunction> function;

    BoundMethod(std::shared_ptr<CCubeInstance> instance, std::shared_ptr<CCubeFunction> function);

    // Callable arayüzünden
    virtual Value call(Interpreter& interpreter, const std::vector<Value>& arguments) override;
    virtual size_t arity() const override;

    // Object arayüzünden
    virtual ObjectType getType() const override { return ObjectType::BOUND_METHOD; }
    virtual std::string toString() const override;
    virtual size_t getSize() const override; // GC için boyut hesaplama
};

#endif // C_CUBE_BOUND_METHOD_H

#endif // C_CUBE_CALLABLE_H

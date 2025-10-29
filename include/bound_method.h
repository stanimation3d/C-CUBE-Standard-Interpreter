#ifndef C_CUBE_BOUND_METHOD_H
#define C_CUBE_BOUND_METHOD_H

#include <string>
#include <memory> // std::shared_ptr için
#include <vector>

#include "object.h"      // Temel Object sınıfı
#include "callable.h"    // Callable arayüzü
#include "instance.h"    // Bağlanılacak instance için (CCubeInstance'ın ileri bildirimi)
#include "function.h"    // Bağlanılacak fonksiyon için (CCubeFunction'ın ileri bildirimi)
#include "value.h"       // Argüman ve dönüş değerleri için Value

// İleri bildirimler
class Interpreter;
class CCubeInstance; // BoundMethod içinde kullanılacağı için
class CCubeFunction; // BoundMethod içinde kullanılacağı için

class BoundMethod : public Object, public Callable {
public:
    std::shared_ptr<CCubeInstance> instance; // Metodun bağlı olduğu instance
    std::shared_ptr<CCubeFunction> function; // Bağlı olan fonksiyon (CCubeFunction)

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

#ifndef C_CUBE_FUNCTION_H
#define C_CUBE_FUNCTION_H

#include <vector>
#include <string>
#include <memory> // std::shared_ptr için

#include "object.h"      // Temel Object sınıfı
#include "callable.h"    // Callable arayüzü
#include "environment.h" // Fonksiyonun closure ortamı için
#include "ast.h"         // FunStmt için
#include "value.h"       // Argüman ve dönüş değerleri için Value

// İleri bildirimler
class Interpreter;

class CCubeFunction : public Object, public Callable {
private:
    std::shared_ptr<FunStmt> declaration;
    std::shared_ptr<Environment> closure; // Fonksiyonun tanımlandığı ortam (closure)
    bool isInitializer; // Eğer bu bir sınıfın 'init' metoduysa

public:
    CCubeFunction(std::shared_ptr<FunStmt> declaration, std::shared_ptr<Environment> closure, bool isInitializer);

    // Callable arayüzünden
    virtual Value call(Interpreter& interpreter, const std::vector<Value>& arguments, std::shared_ptr<CCubeInstance> this_instance = nullptr) override;
    virtual size_t arity() const override;

    // Object arayüzünden
    virtual ObjectType getType() const override { return ObjectType::FUNCTION; }
    virtual std::string toString() const override;
    virtual size_t getSize() const override; // GC için boyut hesaplama

    // CCubeFunction'ı belirli bir instance'a bağlar (metotlar için)
    std::shared_ptr<CCubeFunction> bind(std::shared_ptr<CCubeInstance> instance);

    // GC'nin closure ortamına erişebilmesi için
    std::shared_ptr<Environment> getClosure() const { return closure; }
};

#endif // C_CUBE_FUNCTION_H

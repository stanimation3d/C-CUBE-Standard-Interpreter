#ifndef C_CUBE_CLASS_H
#define C_CUBE_CLASS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory> // std::shared_ptr için

#include "object.h"   // Temel Object sınıfı
#include "callable.h" // Sınıfın kurucu metodu çağrılabilir olduğu için
#include "function.h" // Metotlar CCubeFunction olduğu için
#include "instance.h" // Kurucunun bir instance döndürmesi için
#include "value.h"    // Value içinde ObjPtr var

// İleri bildirimler
class Interpreter;

class CCubeClass : public Object, public Callable {
private:
    std::string name;
    std::shared_ptr<CCubeClass> superclass; // Üst sınıf referansı
    std::unordered_map<std::string, std::shared_ptr<CCubeFunction>> methods; // Sınıf metotları

public:
    CCubeClass(const std::string& name, std::shared_ptr<CCubeClass> superclass,
               std::unordered_map<std::string, std::shared_ptr<CCubeFunction>> methods);

    // Bir metodu ismine göre bulur
    std::shared_ptr<CCubeFunction> findMethod(const std::string& name);

    // Callable arayüzünden (constructor çağrısı için)
    virtual Value call(Interpreter& interpreter, const std::vector<Value>& arguments) override;
    virtual size_t arity() const override;

    // Object arayüzünden
    virtual ObjectType getType() const override { return ObjectType::CLASS; }
    virtual std::string toString() const override;
    virtual size_t getSize() const override; // GC için boyut hesaplama

    // Üst sınıfa erişim için
    std::shared_ptr<CCubeClass> getSuperclass() const { return superclass; }
    // Metotlara erişim için (GC tarafından taranacak)
    const std::unordered_map<std::string, std::shared_ptr<CCubeFunction>>& getMethods() const { return methods; }
};

#endif // C_CUBE_CLASS_H

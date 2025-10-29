#ifndef C_CUBE_MODULE_H
#define C_CUBE_MODULE_H

#include <string>
#include <memory> // std::shared_ptr için

#include "object.h"      // Temel Object sınıfı
#include "environment.h" // Modülün kendi ortamı için
#include "value.h"       // Modül üyeleri için Value

class CCubeModule : public Object {
private:
    std::string name;
    // Modülün dışa aktarılan değerlerini tutan ortam
    std::shared_ptr<Environment> moduleEnvironment;

public:
    CCubeModule(const std::string& name, std::shared_ptr<Environment> env);

    // Modülün bir üyesini ismine göre döndürür
    Value getMember(const Token& name);

    // Object arayüzünden
    virtual ObjectType getType() const override { return ObjectType::C_CUBE_MODULE; }
    virtual std::string toString() const override;
    virtual size_t getSize() const override; // GC için boyut hesaplama

    // GC'nin modül ortamına erişebilmesi için
    std::shared_ptr<Environment> getEnvironment() const { return moduleEnvironment; }
};

#endif // C_CUBE_MODULE_H

#ifndef C_CUBE_INSTANCE_H
#define C_CUBE_INSTANCE_H

#include <string>
#include <unordered_map>
#include <memory> // std::shared_ptr için

#include "object.h" // Temel Object sınıfı
#include "value.h"  // Instance özelliklerinin değerleri için Value

// İleri bildirimler
class CCubeClass; // Sınıfı temsil eden CCubeClass'a referans için
class Interpreter; // Metot çağrıları için

class CCubeInstance : public Object, public std::enable_shared_from_this<CCubeInstance> {
private:
    std::shared_ptr<CCubeClass> klass; // Bu instance'ın ait olduğu sınıf
    std::unordered_map<std::string, Value> properties; // Instance'a özgü özellikler

public:
    CCubeInstance(std::shared_ptr<CCubeClass> klass);

    // Bir özelliğin değerini alır
    Value get(const Token& name);
    // Bir özelliğe değer atar
    void set(const Token& name, Value value);

    // Object arayüzünden
    virtual ObjectType getType() const override { return ObjectType::INSTANCE; }
    virtual std::string toString() const override;
    virtual size_t getSize() const override; // GC için boyut hesaplama

    // GC'nin sınıfına ve özelliklerine erişebilmesi için
    std::shared_ptr<CCubeClass> get_class() const { return klass; }
    const std::unordered_map<std::string, Value>& getProperties() const { return properties; }
};

#endif // C_CUBE_INSTANCE_H

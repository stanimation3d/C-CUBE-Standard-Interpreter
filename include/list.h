#ifndef C_CUBE_LIST_H
#define C_CUBE_LIST_H

#include <vector>
#include <string>
#include <memory> // std::shared_ptr için

#include "object.h" // Temel Object sınıfı
#include "value.h"  // Liste elemanlarının değerleri için Value

class CCubeList : public Object {
private:
    std::vector<Value> elements;

public:
    CCubeList(const std::vector<Value>& initialElements);

    // Liste elemanlarına erişim
    const std::vector<Value>& getElements() const { return elements; }
    std::vector<Value>& getElementsMutable() { return elements; } // Değiştirilebilir erişim için

    // Object arayüzünden
    virtual ObjectType getType() const override { return ObjectType::LIST; }
    virtual std::string toString() const override;
    virtual size_t getSize() const override; // GC için boyut hesaplama

    // Liste operasyonları (ekleyebiliriz: add, remove, get_at, set_at, size)
    void add(Value val);
    Value get_at(size_t index) const;
    void set_at(size_t index, Value val);
    size_t size() const;
};

#endif // C_CUBE_LIST_H

#ifndef C_CUBE_ENVIRONMENT_H
#define C_CUBE_ENVIRONMENT_H

#include <string>
#include <unordered_map>
#include <memory> // std::shared_ptr için
#include <stdexcept> // std::runtime_error için

#include "token.h" // Token sınıfı için (hata raporlama ve isim almak için)
#include "value.h" // Value sınıfı için (değişken değerleri - ObjPtr içerir)
#include "error_reporter.h" // RuntimeException için (Environment hataları)

class Environment : public std::enable_shared_from_this<Environment> {
private:
    // Bu ortamın kapsadığı üst ortam. Global ortamın parent'ı nullptr'dır.
    std::shared_ptr<Environment> enclosing;
    // Değişken isimlerini değerlere eşleştiren harita
    std::unordered_map<std::string, Value> values;

public:
    // Global ortam için constructor (parent'ı yok)
    Environment();
    // İç içe geçmiş ortamlar için constructor (bir parent'ı var)
    Environment(std::shared_ptr<Environment> enclosing);

    // Yeni bir değişken tanımlar
    void define(const std::string& name, Value value);

    // Mevcut bir değişkene değer atar
    // Atama işlemi, değişkeni mevcut ortamdan başlayarak üst ortamlarda arar.
    void assign(const Token& name, Value value);

    // Bir değişkenin değerini döndürür
    // Değişkeni mevcut ortamdan başlayarak üst ortamlarda arar.
    Value get(const Token& name);

    // Belirli bir uzaklıktaki ortamda değişkenin değerini döndürür
    // (Resolver entegre edildiğinde kullanılır)
    Value getAt(int distance, const std::string& name);

    // Belirli bir uzaklıktaki ortamda değişkene değer atar
    // (Resolver entegre edildiğinde kullanılır)
    void assignAt(int distance, const Token& name, Value value);

    // Bir ortamın belirtilen değişkeni içerip içermediğini kontrol eder.
    bool contains(const std::string& name) const;

    // Ortamın üst ortamını döndürür (eğer varsa)
    std::shared_ptr<Environment> getEnclosing() const;

    // GC'nin bu ortamın içindeki ObjPtr'ları tarayabilmesi için
    // Haritanın değiştirilemez bir referansını döndürür.
    const std::unordered_map<std::string, Value>& getValues() const { return values; }

private:
    // Belirtilen uzaklıktaki ortamı bulmaya yardımcı metod
    std::shared_ptr<Environment> ancestor(int distance);
};

#endif // C_CUBE_ENVIRONMENT_H

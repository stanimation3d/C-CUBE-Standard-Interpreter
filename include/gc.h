#ifndef C_CUBE_GC_H
#define C_CUBE_GC_H

#include <vector>
#include <memory>
#include <unordered_set>
#include <map> // Nesilleri tutmak için
#include <algorithm> // std::remove_if için

#include "object.h"      // Temel obje sınıfı
#include "function.h"    // CCubeFunction
#include "class.h"       // CCubeClass
#include "instance.h"    // CCubeInstance
#include "list.h"        // CCubeList
#include "c_cube_module.h" // CCubeModule
#include "value.h"       // Value (içinde ObjPtr var)

// Nesnelerin hash'lenmesi için std::hash specialization'ı
namespace std {
    template<> struct hash<ObjPtr> {
        size_t operator()(const ObjPtr& p) const {
            return hash<Object*>()(p.get());
        }
    };
}


class Gc {
public:
    // GC tarafından yönetilen her nesne için metadata (yaş, nesil bilgisi)
    struct GcObjectMetadata {
        ObjPtr object;
        int generation;  // 0: Young, 1: Old
        int age;         // Young Generation'da kaç koleksiyondan sağ çıktı
        bool marked;     // Mark aşamasında işaretlenmiş mi?

        GcObjectMetadata(ObjPtr obj, int gen)
            : object(obj), generation(gen), age(0), marked(false) {}
    };

    // Global kökler: Interpreter'ın global ortamındaki değişkenler, vb.
    // Bunlar doğrudan Value olarak saklanabilir ve Value içinde ObjPtr varsa erişilebilir.
    std::vector<Value*> roots;

    // Nesiller için haritalar
    std::unordered_set<ObjPtr> youngGeneration; // Gen0
    std::unordered_set<ObjPtr> oldGeneration;   // Gen1

    // Nesillerin meta verilerini tutan harita (ObjPtr'dan GcObjectMetadata'ya)
    // Bu, her ObjPtr için GC ile ilgili bilgilere hızlı erişim sağlar.
    std::unordered_map<ObjPtr, GcObjectMetadata*> objectMetadata;

    // Koleksiyon eşikleri
    size_t youngGenCapacity = 1024 * 10; // Genç nesil için ilk kapasite (byte cinsinden, veya obje sayısı)
                                       // Basitlik için obje sayısı veya rough size tutalım.
    size_t oldGenCapacity = 1024 * 100; // Yaşlı nesil için kapasite
    int youngGenCollections = 0;       // Genç nesil koleksiyon sayısı (terfi için)
    const int PROMOTION_THRESHOLD = 3; // Genç nesilde bu kadar koleksiyondan sağ kalan terfi eder.

    // Genel boyut takibi (opsiyonel, hata ayıklama için)
    size_t bytesAllocated = 0;

private:
    // Mark aşaması için yardımcı: Bir nesneyi ve referanslarını işaretler
    void markObject(ObjPtr obj);
    void markValue(const Value& val);
    void markContainer(const std::vector<Value>& container); // Listeler, objeler için
    void markMap(const std::unordered_map<std::string, Value>& map); // Environment, Instance properties için
    void markMapObjects(const std::unordered_map<std::string, ObjPtr>& map); // Class methods için

    // Sweep aşaması için yardımcı: İşaretlenmemiş nesneleri toplar
    // Hangi nesli temizleyeceğini belirten bir parametre alır.
    void sweep(int generation_to_sweep);

    // Nesneleri genç nesilden eski nesile terfi ettirir
    void promoteObjects();

public:
    Gc();
    ~Gc(); // Yıkıcıda tüm kalan nesneleri temizle

    // C-CUBE değerlerini Heap'te oluşturmak için genel fabrika metodları
    // Bu metodlar, oluşturulan nesnelerin genç nesle eklendiğinden emin olur.
    ObjPtr createObject(std::shared_ptr<CCubeFunction> func);
    ObjPtr createObject(std::shared_ptr<CCubeClass> klass);
    ObjPtr createObject(std::shared_ptr<CCubeInstance> instance);
    ObjPtr createObject(std::shared_ptr<CCubeModule> module);
    ObjPtr createObject(std::shared_ptr<BoundMethod> boundMethod);

    ObjPtr createString(const std::string& str); // Stringler için özel bir durum, genellikle ObjPtr içinde
    ObjPtr createList(const std::vector<Value>& elements); // Listeler için

    // Kök ekleme ve çıkarma (Interpreter yığını, global değişkenler, vb.)
    void addRoot(Value* val);
    void removeRoot(Value* val); // Dikkatli kullanılmalı, pointer değişirse sorun olabilir.
    void addRoot(ObjPtr obj); // Nesne pointer'ını doğrudan root olarak ekle (Fonksiyonlar, Sınıflar, Modüller için)
    void removeRoot(ObjPtr obj); // AddRoot'un karşılığı

    // Manuel olarak çöp toplama tetikleme
    void collectGarbage(bool full_collection = false);

    // Debug amaçlı
    void printStats();
    size_t getTotalAllocatedBytes() const { return bytesAllocated; }

    // GC'ye kaydettiğimiz GcObjectMetadata nesnelerinin temizlenmesi için
    void cleanupMetadata();

private:
    // Tüm nesneler (genç ve yaşlı) için mark'ları sıfırla
    void resetMarks();
};

#endif // C_CUBE_GC_H

#include "gc.h"
#include <iostream>
#include <cassert> // Debug için

// Constructor
Gc::Gc() : youngGenCapacity(1024), oldGenCapacity(1024 * 10), youngGenCollections(0), bytesAllocated(0) {}

// Yıkıcı: Kalan tüm nesneleri ve meta verilerini temizle
Gc::~Gc() {
    collectGarbage(true); // Tam bir koleksiyon yap
    cleanupMetadata();    // Tüm meta verilerini temizle
}

// Yeni C-CUBE objeleri oluşturmak için genel fabrika metodları
// Bu metodlar, oluşturulan nesnelerin genç nesle eklendiğinden emin olur.
ObjPtr Gc::createObject(std::shared_ptr<CCubeFunction> func) {
    ObjPtr obj = std::static_pointer_cast<Object>(func);
    GcObjectMetadata* metadata = new GcObjectMetadata(obj, 0); // Genç nesle ekle
    youngGeneration.insert(obj);
    objectMetadata[obj] = metadata;
    bytesAllocated += sizeof(CCubeFunction); // Rough size, actual size might vary
    // Eğer genç nesil kapasitesini aştıysak, genç nesil koleksiyonu tetikle
    if (youngGeneration.size() >= youngGenCapacity) { // Basitçe obje sayısıyla kontrol
        collectGarbage(false); // Sadece genç nesil koleksiyonu
    }
    return obj;
}

ObjPtr Gc::createObject(std::shared_ptr<CCubeClass> klass) {
    ObjPtr obj = std::static_pointer_cast<Object>(klass);
    GcObjectMetadata* metadata = new GcObjectMetadata(obj, 0); // Genç nesle ekle
    youngGeneration.insert(obj);
    objectMetadata[obj] = metadata;
    bytesAllocated += sizeof(CCubeClass);
    if (youngGeneration.size() >= youngGenCapacity) {
        collectGarbage(false);
    }
    return obj;
}

ObjPtr Gc::createObject(std::shared_ptr<CCubeInstance> instance) {
    ObjPtr obj = std::static_pointer_cast<Object>(instance);
    GcObjectMetadata* metadata = new GcObjectMetadata(obj, 0); // Genç nesle ekle
    youngGeneration.insert(obj);
    objectMetadata[obj] = metadata;
    bytesAllocated += sizeof(CCubeInstance);
    if (youngGeneration.size() >= youngGenCapacity) {
        collectGarbage(false);
    }
    return obj;
}

ObjPtr Gc::createObject(std::shared_ptr<CCubeModule> module) {
    ObjPtr obj = std::static_pointer_cast<Object>(module);
    GcObjectMetadata* metadata = new GcObjectMetadata(obj, 0); // Genç nesle ekle
    youngGeneration.insert(obj);
    objectMetadata[obj] = metadata;
    bytesAllocated += sizeof(CCubeModule);
    if (youngGeneration.size() >= youngGenCapacity) {
        collectGarbage(false);
    }
    return obj;
}

ObjPtr Gc::createObject(std::shared_ptr<BoundMethod> boundMethod) {
    ObjPtr obj = std::static_pointer_cast<Object>(boundMethod);
    GcObjectMetadata* metadata = new GcObjectMetadata(obj, 0); // Genç nesle ekle
    youngGeneration.insert(obj);
    objectMetadata[obj] = metadata;
    bytesAllocated += sizeof(BoundMethod);
    if (youngGeneration.size() >= youngGenCapacity) {
        collectGarbage(false);
    }
    return obj;
}

ObjPtr Gc::createString(const std::string& str) {
    // Stringler genellikle Value içinde doğrudan tutulur.
    // Eğer stringleri de Object olarak yönetiyorsak, burada CCubeString gibi bir sınıf olmalı.
    // Şimdilik, stringleri Value'nun doğrudan parçası olarak varsayıyoruz, bu yüzden GC'ye ihtiyaç duymayabilirler.
    // Eğer stringler de ObjPtr ise:
     ObjPtr obj = std::make_shared<CCubeString>(str); // Veya benzeri
     GcObjectMetadata* metadata = new GcObjectMetadata(obj, 0);
     youngGeneration.insert(obj);
     objectMetadata[obj] = metadata;
     bytesAllocated += str.length(); // + sizeof(CCubeString)
     if (youngGeneration.size() >= youngGenCapacity) {
         collectGarbage(false);
     }
     return obj;
    // Geçici olarak nullptr döndürelim veya hata fırlatalım
    // Stringler şu anda Value içinde doğrudan tutulduğu için bu metot muhtemelen kullanılmayacak.
    throw std::runtime_error("Stringler şimdilik doğrudan Value içinde yönetiliyor, ObjPtr olarak değil.");
}


ObjPtr Gc::createList(const std::vector<Value>& elements) {
    std::shared_ptr<CCubeList> list_obj = std::make_shared<CCubeList>(elements);
    ObjPtr obj = std::static_pointer_cast<Object>(list_obj);
    GcObjectMetadata* metadata = new GcObjectMetadata(obj, 0); // Genç nesle ekle
    youngGeneration.insert(obj);
    objectMetadata[obj] = metadata;
    bytesAllocated += sizeof(CCubeList) + elements.size() * sizeof(Value); // Rough size
    if (youngGeneration.size() >= youngGenCapacity) {
        collectGarbage(false);
    }
    return obj;
}

// Kökleri ekleme (Interpreter yığını, global değişkenler, vb.)
void Gc::addRoot(Value* val) {
    roots.push_back(val);
}

void Gc::removeRoot(Value* val) {
    // std::remove_if ve erase idiomunu kullanarak kaldır
    roots.erase(std::remove_if(roots.begin(), roots.end(), [val](Value* p){ return p == val; }), roots.end());
}

void Gc::addRoot(ObjPtr obj) {
    // obj zaten bir ObjPtr olduğundan, bu direkt olarak izlenebilir.
    // Ancak roots vektörü Value* tuttuğu için bu durumda ObjPtr'ı Value'ya dönüştürmemiz gerekir.
    // Veya roots vektörünü ObjPtr da tutacak şekilde genişletmeliyiz.
    // Şimdilik, ObjPtr'ı root olarak eklemek için ObjectMetadata'yı işaretlemeyi kullanacağız.
    // Ancak bu, ObjPtr'ın zaten heap'te olması ve Gc tarafından bilinmesi gerektiği anlamına gelir.
    // Burası biraz karmaşık, root'ları ObjPtr olarak doğrudan tutmak daha uygun olabilir.
    // Örneğin, 'ObjPtr roots_obj_ptr;' gibi ayrı bir kök listesi.
    // Basitlik için, objeyi youngGeneration'a eklemişsek ve metadata'sı varsa, markObject ile kök olarak işaretleyebiliriz.
    // Eğer objeler henüz GC tarafından yönetilmiyorsa, burada bir hata oluşabilir.
    if (objectMetadata.count(obj)) {
        objectMetadata[obj]->marked = true; // Kök olarak işaretle
    }
    // Gc dışındaki akıllı işaretçilerin de kök olduğu varsayılabilir.
}

void Gc::removeRoot(ObjPtr obj) {
    if (objectMetadata.count(obj)) {
        objectMetadata[obj]->marked = false; // İşareti kaldır (yalnızca tek bir döngüde root ise)
    }
}


// Çöp toplama döngüsünü tetikler
void Gc::collectGarbage(bool full_collection) {
     std::cout << "GC Başladı (" << (full_collection ? "Tam Koleksiyon" : "Genç Nesil") << ")..." << std::endl;
     printStats();

    // 1. Mark Aşaması
    resetMarks(); // Tüm nesnelerin marklarını sıfırla

    // Tüm kökleri işaretle
    for (Value* root_val : roots) {
        markValue(*root_val);
    }

    // Ek kökler: Global ortam, interpreter'ın mevcut ortamı, sınıf metotları vb.
    // Environment'ın kendisi de kök olmalı (globals ve current environment)
    // Interpreter'daki globals ve current environment'ı buraya manuel olarak eklemeliyiz.
    // Veya Interpreter'ın Gc ile olan ilişkisini güçlendirmeliyiz.
    // Şimdilik, Interpreter'ın global ve mevcut ortamlarını burada varsayalım.
    // Interpreter'ın ortamlarını buraya root olarak eklemenin en iyi yolu, Interpreter'ın
    // `current` ve `globals` ortamlarını `Gc::addRoot` metoduna iletmek olacaktır.
    // Geçici olarak, Interpreter'daki environment'ları burada doğrudan marklamaya çalışalım:
    
    if (interpreter_globals != nullptr) markMap(interpreter_globals->getValues());
    if (interpreter_environment != nullptr) markMap(interpreter_environment->getValues());
    
    // Bu, Gc sınıfının Interpreter'a bağımlı olmasına neden olur, bu iyi bir tasarım değildir.
    // En iyi yol, Interpreter'ın tüm kökleri Gc'ye sağlamasıdır.

    // Sadece genç nesil koleksiyonu ise, yaşlı nesildeki nesnelerden genç nesile yapılan referansları da işaretle
    if (!full_collection) {
        for (const auto& entry : objectMetadata) {
            if (entry.second->generation == 1 && entry.second->marked) { // Yaşlı nesildeki işaretli nesnelerden
                 // Yaşlı nesilden genç nesile referansları bulup işaretle
                 // Bu kısım karmaşık, nesnelerin içindeki tüm Value'ları tekrar marklamamız gerekir.
                 // Örneğin, bir Class objesi içindeki metotları, bir Instance içindeki propertileri.
                 markObject(entry.first); // Object'in içindeki referansları markla
            }
        }
    }


    // 2. Sweep Aşaması
    // Terfi işlemi sweep'ten önce olmalı
    if (!full_collection) { // Sadece genç nesil koleksiyonu ise terfi et
        promoteObjects();
        youngGenCollections++;
    }

    // Hangi nesilleri temizleyeceğimizi belirle
    if (full_collection) {
        sweep(0); // Genç nesli temizle
        sweep(1); // Yaşlı nesli temizle
        youngGenCollections = 0; // Tam koleksiyondan sonra genç nesil koleksiyon sayacını sıfırla
    } else {
        sweep(0); // Sadece genç nesli temizle
        // Eğer genç nesil koleksiyonu belirli bir eşiğe ulaştıysa, tam koleksiyon tetikle
        if (youngGenCollections >= 5) { // Örneğin 5 genç nesil koleksiyonu sonra
            collectGarbage(true);
        }
    }

     std::cout << "GC Bitti. Kalan Nesneler: Genç=" << youngGeneration.size() << ", Yaşlı=" << oldGeneration.size() << std::endl;
     printStats();
}

// Tüm nesnelerin marklarını sıfırla
void Gc::resetMarks() {
    for (const auto& entry : objectMetadata) {
        entry.second->marked = false;
    }
}

// Mark aşaması için yardımcı: Bir nesneyi ve referanslarını işaretler
void Gc::markObject(ObjPtr obj) {
    if (obj == nullptr) return;

    // Nesnenin zaten işaretli olup olmadığını kontrol et
    if (!objectMetadata.count(obj) || objectMetadata[obj]->marked) {
        return; // Zaten işaretli veya GC tarafından yönetilmiyor
    }

    // Nesneyi işaretle
    objectMetadata[obj]->marked = true;

    // Nesnenin tipine göre içindeki referansları özyinelemeli olarak işaretle
    switch (obj->getType()) {
        case Object::ObjectType::FUNCTION: {
            std::shared_ptr<CCubeFunction> func = std::static_pointer_cast<CCubeFunction>(obj);
            // Fonksiyonun kapsayan ortamını işaretle
            // Environment'lar Value'lar tuttuğu için Environment::values haritasını işaretlemeliyiz.
            if (func->closure != nullptr) {
                // closure environment'ı da bir Environment objesidir, onu da marklamalıyız
                // Ancak Environment'lar doğrudan ObjPtr değil, bu karmaşık bir durum.
                // Basitlik için, Environment'ı işaretlemek yerine içindeki ObjPtr'ları işaretlemeliyiz.
                // (Bu, Interpreter'ın Environment'ları da Gc ile yönetmesi gerektiği anlamına gelebilir.)
                // Şimdilik, Environment'ın içindeki ObjPtr'ları markValue aracılığıyla işaretleyelim.
                markMap(func->closure->getValues());
                if (func->closure->getEnclosing() != nullptr) {
                    markMap(func->closure->getEnclosing()->getValues());
                }
            }
            break;
        }
        case Object::ObjectType::CLASS: {
            std::shared_ptr<CCubeClass> klass = std::static_pointer_cast<CCubeClass>(obj);
            // Metotları işaretle
            markMapObjects(klass->getMethods());
            // Üst sınıfı işaretle
            if (klass->superclass != nullptr) {
                markObject(std::static_pointer_cast<Object>(klass->superclass));
            }
            break;
        }
        case Object::ObjectType::INSTANCE: {
            std::shared_ptr<CCubeInstance> instance = std::static_pointer_cast<CCubeInstance>(obj);
            // Sınıfı işaretle
            markObject(std::static_pointer_cast<Object>(instance->get_class()));
            // Property'leri işaretle
            markMap(instance->getProperties());
            break;
        }
        case Object::ObjectType::LIST: {
            std::shared_ptr<CCubeList> list = std::static_pointer_cast<CCubeList>(obj);
            // Liste elemanlarını işaretle
            markContainer(list->getElements());
            break;
        }
        case Object::ObjectType::BOUND_METHOD: {
            std::shared_ptr<BoundMethod> boundMethod = std::static_pointer_cast<BoundMethod>(obj);
            // Instance ve fonksiyonu işaretle
            markObject(std::static_pointer_cast<Object>(boundMethod->instance));
            markObject(std::static_pointer_cast<Object>(boundMethod->function));
            break;
        }
        case Object::ObjectType::C_CUBE_MODULE: {
            std::shared_ptr<CCubeModule> module = std::static_pointer_cast<CCubeModule>(obj);
            // Modülün içerdiği üyeleri işaretle (genellikle environment'ıdır)
            markMap(module->getEnvironment()->getValues());
            break;
        }
        // Diğer obje tipleri için de benzer şekilde marklama yapılabilir
        default:
            break;
    }
}

// Bir Value'yu işaretle (eğer içinde ObjPtr varsa)
void Gc::markValue(const Value& val) {
    if (std::holds_alternative<ObjPtr>(val)) {
        markObject(std::get<ObjPtr>(val));
    }
}

// Bir Value vektörü içindeki ObjPtr'ları işaretle
void Gc::markContainer(const std::vector<Value>& container) {
    for (const Value& val : container) {
        markValue(val);
    }
}

// Bir string-Value haritası içindeki ObjPtr'ları işaretle
void Gc::markMap(const std::unordered_map<std::string, Value>& map) {
    for (const auto& pair : map) {
        markValue(pair.second);
    }
}

// Bir string-ObjPtr haritası içindeki ObjPtr'ları işaretle
void Gc::markMapObjects(const std::unordered_map<std::string, ObjPtr>& map) {
    for (const auto& pair : map) {
        markObject(pair.second);
    }
}


// Sweep aşaması: İşaretlenmemiş nesneleri toplar
void Gc::sweep(int generation_to_sweep) {
    std::unordered_set<ObjPtr>* target_generation = nullptr;
    if (generation_to_sweep == 0) {
        target_generation = &youngGeneration;
    } else if (generation_to_sweep == 1) {
        target_generation = &oldGeneration;
    } else {
        return; // Geçersiz nesil
    }

    std::vector<ObjPtr> to_delete;
    for (const auto& obj : *target_generation) {
        if (!objectMetadata.count(obj) || !objectMetadata[obj]->marked) { // İşaretli değilse
            to_delete.push_back(obj);
        }
    }

    for (const auto& obj : to_delete) {
        bytesAllocated -= obj->getSize(); // Rough size
        target_generation->erase(obj); // Nesli set'ten kaldır
        delete objectMetadata[obj]; // Meta veriyi sil
        objectMetadata.erase(obj);  // Haritadan kaldır
        // Nesnenin kendisi shared_ptr olduğu için, başka referans yoksa otomatik silinecektir.
        // Eğer shared_ptr yerine raw pointer yönetseydik burada delete obj.get() yapardık.
        // Shared_ptr kullanıldığı için referans sayısının sıfıra düşmesiyle otomatik yıkılacaklar.
    }
}


// Nesneleri genç nesilden eski nesile terfi ettirir
void Gc::promoteObjects() {
    std::vector<ObjPtr> to_promote;
    for (const auto& obj : youngGeneration) {
        if (objectMetadata.count(obj) && objectMetadata[obj]->marked) { // İşaretli ve canlıysa
            objectMetadata[obj]->age++;
            if (objectMetadata[obj]->age >= PROMOTION_THRESHOLD) {
                to_promote.push_back(obj);
            }
        }
    }

    for (const auto& obj : to_promote) {
        youngGeneration.erase(obj); // Genç nesilden kaldır
        oldGeneration.insert(obj);  // Eski nesle ekle
        objectMetadata[obj]->generation = 1; // Nesil bilgisini güncelle
        objectMetadata[obj]->age = 0;       // Yaşını sıfırla
    }
}

// GC'ye kaydettiğimiz GcObjectMetadata nesnelerinin temizlenmesi için
void Gc::cleanupMetadata() {
    for (const auto& entry : objectMetadata) {
        delete entry.second; // Heap'te oluşturulan GcObjectMetadata nesnelerini sil
    }
    objectMetadata.clear();
}

// Debug amaçlı istatistikleri yazdır
void Gc::printStats() {
    std::cout << "--- GC İstatistikleri ---" << std::endl;
    std::cout << "Toplam Ayrılan Bayt: " << bytesAllocated << std::endl;
    std::cout << "Genç Nesil Nesneler: " << youngGeneration.size() << std::endl;
    std::cout << "Yaşlı Nesil Nesneler: " << oldGeneration.size() << std::endl;
    std::cout << "Genç Nesil Koleksiyonları: " << youngGenCollections << std::endl;
    std::cout << "-------------------------" << std::endl;
}

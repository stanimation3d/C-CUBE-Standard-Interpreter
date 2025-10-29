#include "bound_method.h"
#include "interpreter.h" // Interpreter'ı kullanır
#include "environment.h" // Ortam yönetimi için

BoundMethod::BoundMethod(std::shared_ptr<CCubeInstance> instance, std::shared_ptr<CCubeFunction> function)
    : instance(instance), function(function) {}

Value BoundMethod::call(Interpreter& interpreter, const std::vector<Value>& arguments) {
    return function->call(interpreter, arguments, instance);
}
    std::shared_ptr<Environment> method_environment = std::make_shared<Environment>(function->getClosure());

    // 'this' anahtar kelimesini bu yeni ortama tanımla.
    // 'instance' zaten bir ObjPtr olduğu için doğrudan atanabilir.
    method_environment->define("this", instance);

    // Argümanları yeni ortama tanımla.
    // Bu kısım aslında fonksiyonun kendi çağrı mekanizmasında yapılmalı.
    // CCubeFunction::call metodu argümanları zaten kendi yeni ortamına yerleştirecektir.
    // Burada tekrar tanımlamak, CCubeFunction::call'ın içine bakmayı gerektirir.
    // Daha temiz bir yaklaşım: BoundMethod, sadece 'this'i ayarlar ve sonra
    // orijinal fonksiyonu çağırmak için Interpreter'ın uygun metodunu kullanır,
    // Interpreter ise argümanları kendi ortamına yerleştirir.

    // Ancak şu anki Interpreter tasarımında CCubeFunction::call,
    // Interpreter'a ve argümanlara ihtiyaç duyar.
    // CCubeFunction::call metodunu, 'this' için bir Environment alan bir şekilde
    // veya özel bir 'this' değeri alan şekilde güncelleyebiliriz.

    // Geçerli yaklaşım:
    // CCubeFunction::call zaten yeni bir ortam oluşturur ve parametreleri o ortama yerleştirir.
    // Bizim yapmamız gereken, bu ortamın 'this'i içermesini sağlamak.
    // En temiz yol, CCubeFunction'ın call metodunun, bağlandığı 'this' objesini de
    // parametre olarak alması ve bu 'this'i yeni ortamına eklemesidir.
    // Ancak bu, CCubeFunction::call'ın imzasını değiştirir.

    // Şu anki Interpreter yapısına en uygun yol:
    // Interpreter'daki fonksiyon çağrısı mekanizmasında 'this'i handle etmek.
    // BoundMethod, interpreter'a 'this'i içeren bir ortamla fonksiyonu çağırması talimatını vermeli.
    // Bu da aslında Interpreter'ın `executeBlock` gibi metodlarını daha esnek hale getirmeyi gerektirir.

    // Geriye dönüp Interpreter'a bakalım:
    // Interpreter::visitCallExpr() -> callable->call(interpreter, arguments);
    // Bu durumda callable'ın (yani BoundMethod'ın) kendi call metodu,
    // Interpreter'dan aldığı instance'ı kullanarak bir ortam oluşturmalı.
    // Yukarıdaki Environment oluşturma ve 'this'i tanımlama doğru.
    // Şimdi fonksiyonu bu yeni ortamda çağırma kısmı:
    // CCubeFunction'ın bir iç metodu olabilir: `callWithEnvironment(Interpreter&, Environment&, const std::vector<Value>&)`

    // Daha basit bir çözüm, CCubeFunction::call'ın kendisine bir 'this' değeri parametresi eklemek:
     virtual Value call(Interpreter& interpreter, const std::vector<Value>& arguments, Value this_value = std::monostate{}) override;
    // Bu durumda, CCubeFunction::call kendi içinde `this_value`'yu Environment'a tanımlar.

    // Şimdilik, BoundMethod'ın kendi içinde 'this'i tanımladığı ortamı oluşturup,
    // fonksiyonun gövdesini bu ortamda yürüten Interpreter metodunu çağıralım:

    // NOT: CCubeFunction::call metodunun imzası, fonksiyonun kendisi içinde
    // bir Environment oluşturup argümanları ve 'this'i bu Environment'a atamasını sağlar.
    // Dolayısıyla, BoundMethod'dan doğrudan bu yeni Environment'ı iletmek daha uygun olacaktır.

    // CCubeFunction'ın çağrı mekanizmasını kullanmak için:
    return function->callWithThis(interpreter, arguments, instance); // Yeni bir metod varsayalım
}

size_t BoundMethod::arity() const {
    return function->arity();
}

std::string BoundMethod::toString() const {
    return "<bound method " + function->toString() + " of " + instance->toString() + ">";
}

// GC için boyut hesaplama
size_t BoundMethod::getSize() const {
    // BoundMethod objesinin kendi boyutu ve tuttuğu shared_ptr'lerin boyutu
    return sizeof(BoundMethod);
}

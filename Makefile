# Derleyici ayarları
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g # -g hata ayıklama sembolleri için

# Kaynak dizinleri
SRC_DIR = . # Kaynak dosyalarının bulunduğu dizin (mevcut dizin)

# Nesne dosyalarının oluşturulacağı dizin
BUILD_DIR = build

# Tüm .cpp dosyaları
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Karşılık gelen .o dosyaları
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Çalıştırılabilir dosyanın adı
TARGET = c-cube

.PHONY: all clean run

# Varsayılan hedef: çalıştırılabilir dosyayı oluştur
all: $(BUILD_DIR) $(TARGET)

# Build dizini oluştur
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Çalıştırılabilir dosyayı bağla
$(TARGET): $(OBJS)
	@echo "Bağlanıyor: $@"
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

# .cpp dosyalarından .o dosyalarını derle
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Derleniyor: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Temizlik hedefi: nesne dosyalarını ve çalıştırılabilir dosyayı sil
clean:
	@echo "Temizleniyor..."
	@rm -rf $(BUILD_DIR) $(TARGET)

# Programı çalıştırma hedefi (sadece `make run` ile)
run: all
	@echo "C-CUBE Yorumlayıcısı Başlatılıyor..."
	@./$(TARGET) $(ARGS) # ARGS değişkeni ile komut satırı argümanları geçilebilir (örn: make run ARGS="program.ccb")

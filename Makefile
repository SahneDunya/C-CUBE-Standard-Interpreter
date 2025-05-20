# C-CUBE Yorumlayıcısı Makefile

# Derleyici
CXX = clang++

# Derleyici Bayrakları (Flags)
# -std=c++17: C++17 standardını kullan (std::variant, std::filesystem vb. için)
# -Wall: Tüm standart uyarıları etkinleştir
# -Wextra: Ek uyarıları etkinleştir
# -Wpedantic: Standartın katı gereksinimleri dışındaki yapıları uyar
# -g: Hata ayıklama bilgisini dahil et
# -I.: Mevcut dizini include yollarına ekle (başlık dosyaları için)
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -g -I.

# Kaynak Dosyaları (.cpp)
SRCS = \
	lexer.cpp \
	value.cpp \
	environment.cpp \
	function.cpp \
	object.cpp \
	class.cpp \
	builtin_functions.cpp \
	gc.cpp \
	c_cube_module.cpp \
	module_loader.cpp \
	error_reporter.cpp \
	utils.cpp \
	main.cpp

# Nesne Dosyaları (.o) - Kaynak dosyalarından otomatik olarak türetilir
OBJS = $(SRCS:.cpp=.o)

# Çalıştırılabilir Dosya Adı
EXEC = ccube

# Varsayılan Hedef: Çalıştırılabilir dosyayı oluştur
all: $(EXEC)

# Çalıştırılabilir dosyayı nesne dosyalarından bağlama kuralı
$(EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $(EXEC) $(CXXFLAGS)

# .cpp dosyalarını .o dosyalarına derleme kuralı (Desen Kuralı)
# $<: Kuralın önkoşulu (yani .cpp dosyası)
# $@: Kuralın hedefi (yani .o dosyası)
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Phony Hedefler (Dosya karşılığı olmayan hedefler)
.PHONY: all clean

# Clean Hedefi: Oluşturulan nesne ve çalıştırılabilir dosyaları kaldır
clean:
	rm -f $(OBJS) $(EXEC)

# --- Bağımlılıklar ---
# Başlık dosyası bağımlılıkları manuel olarak listelenmiştir.
# Daha büyük projeler için, bağımlılıkları otomatik olarak oluşturmak için
# g++ -MM veya makedepend gibi araçlar kullanılabilir.

lexer.o: lexer.cpp lexer.h token.h
value.o: value.cpp value.h gc.h object.h function.h class.h c_cube_module.h callable.h
environment.o: environment.cpp environment.h value.h gc.h token.h error_reporter.h
function.o: function.cpp function.h callable.h ast.h token.h environment.h value.h interpreter.h gc.h
object.o: object.cpp object.h value.h token.h class.h function.h gc.h
class.o: class.cpp class.h callable.h function.h object.h value.h interpreter.h gc.h
builtin_functions.o: builtin_functions.cpp builtin_functions.h callable.h value.h interpreter.h
gc.o: gc.cpp gc.h interpreter.h environment.h value.h object.h function.h class.h c_cube_module.h
c_cube_module.o: c_cube_module.cpp c_cube_module.h gc.h environment.h ast.h value.h
module_loader.o: module_loader.cpp module_loader.h lexer.h parser.h interpreter.h error_reporter.h value.h c_cube_module.h
error_reporter.o: error_reporter.cpp error_reporter.h token.h
utils.o: utils.cpp utils.h value.h
main.o: main.cpp lexer.h parser.h interpreter.h error_reporter.h

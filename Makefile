
OBJ_FILES := obj/culogic.o obj/cuelement.o obj/cudriver.o obj/cuproject.o

CXX_FLAGS := -std=c++17
#-O3


all: release clean

cuedit:
	mkdir -p obj
	make culogic.o cudriver.o cuelement.o cuproject.o

release: cuedit
	g++ $(OBJ_FILES) -o bin/culogic

culogic.o:
	g++ $(CXX_FLAGS) -c src/culogic.cpp -o obj/culogic.o
cudriver.o:
	g++ $(CXX_FLAGS) -c src/cudriver.cpp -o obj/cudriver.o
cuelement.o:
	g++ $(CXX_FLAGS) -c src/cuelement.cpp -o obj/cuelement.o
cuproject.o:
	g++ $(CXX_FLAGS) -c src/cuproject.cpp -o obj/cuproject.o

clean:
	rm -f obj/*.o

github:
	git add src/*.cpp
	git add src/*.h
	git add *.code-workspace
	git add *.md
	git add Makefile
	git add bin
	@read -p "Please enter a commit message:" msg; \
	git commit -m "$$msg"
	git push


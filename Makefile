INCLUDES = -I./../../opencv/build/include
# path for opencv/build.
# mingw32-make.exe in Windows.
default : displacement.cpp
	g++ $(INCLUDES) displacement.cpp -o displacement
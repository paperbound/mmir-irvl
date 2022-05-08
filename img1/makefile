compile:
	gcc main.c -lgraph -lm -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_calib3d

compile+:
	gcc main.cpp -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_calib3d -std=c++11 -stdlib=libc++ `pkg-config opencv --cflags --libs opencv`

compile2:
	clang++-3.8 -std=c++11 main.cpp -lgraph -lm -lSDL `pkg-config --cflags --libs opencv`

compilec:
	clang++-3.8 -std=c++11 main.cpp -lgraph -lm -lSDL `pkg-config --cflags --libs opencv` && ./a.out

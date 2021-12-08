CC = g++
CFLAGS = -g -std=c++11 -Wno-deprecated-register -Wno-deprecated-declarations -DGL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
INCFLAGS = -I./glm-0.9.7.1 -I./include/
LDFLAGS = -framework GLUT -framework OpenGL -L./lib/mac/ -lm  -lfreeimage

RM = /bin/rm -f
all: ModelViewer
ModelViewer: main.o Shader.o Camera.o Obj.o Mesh.o shaders/normal.frag shaders/projective.vert
	$(CC) -o ModelViewer main.o Shader.o Camera.o Obj.o $(LDFLAGS)
main.o: main.cpp include/hw2AutoScreenshots.h
	$(CC) $(CFLAGS) $(INCFLAGS) -c main.cpp 
Shader.o: src/Shader.cpp
	$(CC) $(CFLAGS) $(INCFLAGS) -c src/Shader.cpp
Camera.o: src/Camera.cpp include/Camera.h
	$(CC) $(CFLAGS) $(INCFLAGS) -c src/Camera.cpp
Obj.o: src/Obj.cpp include/Obj.h
	$(CC) $(CFLAGS) $(INCFLAGS) -c src/Obj.cpp
Mesh.o: src/Mesh.cpp include/Mesh.h
	$(CC) $(CFLAGS) $(INCFLAGS) -c src/Mesh.cpp
clean: 
	$(RM) *.o ModelViewer

 

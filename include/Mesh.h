#define GLM_FORCE_RADIANS
#include "Obj.h"
#include "Geometry.h"
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

struct HalfEdge {
    HalfEdge* next;
    HalfEdge* flip;
    Point* src;
    Face* face;
    Edge* parent;
};

struct Point {
    HalfEdge* he;
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 newPos;
    unsigned int index;
    bool isNew;
    
};

struct Face {
    HalfEdge* he;
};

struct Edge {
    HalfEdge* he;
    glm::vec3 newPos;
    bool isNew;
};

class Mesh : public Geometry {
    public:
        std::vector<Point*> pts;
        std::vector<HalfEdge*> hes;
        std::vector<Face*> faces;
        std::vector<Edge*> edges;

        //void init(const char* filename);

        static glm::vec3 movePoint(Point* p);

        void updatePointPos();
        Point* makePoint(Edge* edge);
        void flip(Edge* edge);
        std::vector<Edge*> split(Edge* edge);

        void calcNormals();
        void makeBuffers();
        
        Mesh(Obj* object);
};
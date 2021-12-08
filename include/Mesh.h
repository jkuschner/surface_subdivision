#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Geometry.h"
#include <iostream>
#include <vector>
#ifndef __MESH_H__
#define __MESH_H__

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

        Point* makePoint(Edge* edge);
        std::vector<Edge*> split(Edge* edge);
        void flip(Edge* edge);

        void updatePointPos();
        void subdivide();

        void calcNormals();
        void bindBuffers();
        
        Mesh();

        Mesh(std::vector<glm::vec3> vertexBuffer, std::vector<unsigned int> connectivityBuffer);
        ~Mesh();
};

#endif
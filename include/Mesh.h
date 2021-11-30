#define GLM_FORCE_RADIANS
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

class HalfEdge {
    public:
        HalfEdge* next;
        HalfEdge* flip;
        Point* src;
        Face* face;
        Edge* parent;
};

class Point {
    public:
        HalfEdge* he;
        glm::vec3 pos;
};

class Face {
    public:
        HalfEdge* he;
};

class Edge {
    public:
        HalfEdge* he;
};

class Mesh {
    public:
        std::vector<Point> pts;
        std::vector<HalfEdge> hes;
        std::vector<Face> faces;
        std::vector<Edge> edges;

        Mesh();

        Edge flip(Edge* edge);
        Edge split(Edge* edge);
};
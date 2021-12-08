#include "Mesh.h"
#include <unordered_map>
#include <utility>

/*
 * To display after subdividing:
 * 1. Calculate normals for all vertices using cross product(calcNormals)
 * 2. generate vertex and connectivity buffers(makeBuffers)
 * 3. do some OpenGL magic and display it
 * 
 */

typedef unsigned int uint;

/* Full Subdiv algo:
 * 1. Calculate new positions of old Points (movePoint)
 * 2. Calculate positions of new Points (split)
 * 3. split all edges and attatch to new Points (split)
 * 4. Meanwhile add all new edges to a new vector then swap vectors
 *      !! Remember to free old edge pointers !!
 * 5. flip all NEW edges that connect new and old point (flip)
 * 6. set all old Points to newPos (updatePointPos)
 * 
 * A single iteration of the subdivision algorithm
 */
void Mesh::subdivide() {
    // set newPos field for all existing points
    for(int i = 0; i < pts.size(); i++) {
        pts[i]->newPos = Mesh::movePoint(pts[i]);
    }
    
    // make new points and edges by splitting all edges
    std::vector<Edge*> newEdges, tmpEdges;
    for(int i = 0; i < edges.size(); i++) {
        // split creates new point and adds to pts
        // also creates new hedges and faces and adds to Mesh
        tmpEdges = this->split(edges[i]);

        // add all new edges to the new vector
        // TODO: probably a better way to do this, check std::vector docs
        for(int j = 0; j < tmpEdges.size(); j++) {
            newEdges.push_back(tmpEdges[j]);
        }
    }
    
    // free all of the now obsolete Edge pointers
    for (int i = 0; i < edges.size(); i++) {
        delete edges[i];
    }
    // set edges to our new vector
    edges = newEdges;

    // flip all NEW edges that connect a new and old point
    for(int i = 0; i < edges.size(); i++) {
        if(edges[i]->isNew) {
            // if this edge connects a new and old point
            if(edges[i]->he->src->isNew != edges[i]->he->flip->src->isNew) {
                this->flip(edges[i]);
            }
        }
    }

    // set all pos Point fields to newPos
    // and set all isNew Point fields to false
    this->updatePointPos();

}

/*
 * Given an edge in a Mesh object, replace the
 * edge with it's flipped version and return the
 * flipped edge
 * @param edge: the edge to be flipped
 */
void Mesh::flip(Edge* edge) {
    struct Face* f1, f2;
    f1 = edge->he->face;
    f2 = edge->he->flip->face;
    struct HalfEdge* m1, m2;
    m1 = edge->he;
    m2 = edge->he->flip;

    m1->src = m1->next->next->src; // set src
    m1->next = m2->next->next; // set dest
    m2->src = m2->next->next->src; // repeat for flipside
    m2->next = m1->next->next->next;
    m1->next->next->next = m1; //set prev->next to this
    m2->next->next->next = m2;

    f1->he = m1;
    f2->he = m2
    m1->face = f1;
    m2->face = f2;
    m1->next->face = f1;
    m2->next->face = f2;
    m1->next->next->face = f1;
    m2->next->next->face = f2;

}

/*
 * Calculate the new position of a Point based on the
 * surrounding points.
 * @param p: The original Point
 * @return: the vector location of the new position
 */
static glm::vec3 movePoint(Point* p) {
    std::vector<glm::vec3> valence;
    Halfedge* he, he0;
    he0 = p->he;
    // add all adjacent point positions to valence
    do {
        valence.push_back(he->flip->src->pos);
        he = he->flip->next;
    } while(he != he0);

    // k == valence.size()
    float beta = 3.0f / 16.0f;
    if (valence.size() > 3) {
        beta = 3.0f / (8.0f * valence.size());
    }

    glm::vec3 res = (1.0f - (valence.size()*beta)) * p->pos;
    for(int i = 0; i < valence.size(); i++) {
        res += beta * valence[i];
    }
    return res;
}

/*
 * Given an edge, find the "splitting point" and add 
 * that point to pts w/ isNew = true and set the newPos
 * value of edge to that location.
 * @param edge: the edge to find the "splitting point" of
 * @return: a pointer to the new Point
 */
Point* Mesh::makePoint(Edge* edge) {
    struct Point* p = new Point;
    p->isNew = true;
    //get coord of 4 adjacent points
    glm::vec3 P1, P2, p1, p2;
    P1 = edge->he->src->pos;
    P2 = edge->he->flip->src->pos;
    p1 = edge->he->next->next->src->pos;
    p2 = edge->he->flip->next->next->src->pos;
    //calculate newPos
    //p->pos = (0.375f*P1) + (0.375f*P2) + (0.125f*p1) + (0.125f*p2);
    p->newPos = (0.375f*P1) + (0.375f*P2) + (0.125f*p1) + (0.125f*p2);
    //add to Mesh
    pts.push_back(p);
    //set edge's newPos
    edge->newPos = p->pos;
}

/*
 * Given an edge, add another edge that splits
 * this one in half, increasing the number of triangles
 * in the mesh.
 * 
 * Should make 4 new edges and return them w/o adding to Mesh
 * Should make new hedges and DO add those to Mesh
 * Be sure to preserve all old hedge pointers
 * Should make new face and DO add to Mesh
 * Be sure to preserve old Face pointers
 * 
 * @param edge: the edge to be split
 * @return: All of the newly created edges
 */
std::vector<Edge*> Mesh::split(Edge* edge) {
    struct Point* p = makePoint(edge);

    // save face pointers for later
    struct Face* f1, f2, f3, f4;
    f1 = edge->he->face;
    f2 = edge->he->flip->face;
    f3 = new Face;
    f4 = new Face;

    //old hedge pointers
    struct HalfEdge* br, bl;
    br = edge->he; // bottom-right
    bl = edge->he->flip; // bottom-left
    //new hedge pointers
    struct HalfEdge* lb, lt, rb, rt, tr, tl;
    lb = new HalfEdge; // left-bottom
    lt = new HalfEdge; // left-top
    rb = new HalfEdge; // right-bottom
    rt = new HalfEdge; // right-top
    tr = new HalfEdge; // top-right
    tl = new HalfEdge; // top-left

    br->src = p; // set br->src to middle point
    lb->src = p; // set "" "" to middle point
    lb->next = bl->flip->next->next; // set lb->next to AB edge
    rb->src = br->next->next->src; // set rb->src to C
    rb->next = br;
    tl->src = p;
    tl->next = bl->next;
    bl->next = lb;
    rt->src = p;
    rt->next = br->next->next;
    br->next->next = rb;
    lt->src = lb->next->src;
    lt->next = tl;
    tr->src = tl->next->src;
    tr->next = rt;

    //set all flips
    br->flip = bl;
    bl->flip = br;
    lb->flip = lt;
    lt->flip = lb;
    rb->flip = rt;
    rt->flip = rb;
    tr->flip = tl;
    tl->flip = tr;

    //set all faces
    f1->he = br;
    f2->he = bl;
    f3->he = tl;
    f4->he = tr;
    
    br->face = f1;
    br->next->face = f1;
    br->next->next->face = f1;
    bl->face = f2;
    bl->next->face = f2;
    bl->next->next->face = f2;
    tl->face = f3;
    tl->next->face = f3;
    tl->next->next->face = f3;
    tr->face = f4;
    tr->next->face = f4;
    tr->next->next->face = f4;

    // make new edges
    std::vector<Edge*> newEdges;
    for(int i = 0; i < 4; i++) {
        newEdges[i] = new Edge;
        newEdges[i]->isNew = true; 
    }
    newEdges[0]->he = br;
    newEdges[1]->he = rb;
    newEdges[2]->he = tr;
    newEdges[3]->he = lb;

    newEdges[0]->isNew = false; //these used to be the old edge
    newEdges[2]->isNew = false; //from top to bottom
    
    //assign parent edges
    br->parent = newEdges[0];
    bl->parent = newEdges[0];
    rb->parent = newEdges[1];
    rt->parent = newEdges[1];
    tr->parent = newEdges[2];
    tl->parent = newEdges[3];
    lb->parent = newEdges[4];
    lt->parent = newEdges[4];

    // add hedges and faces to Mesh
    hes.push_back(br);
    hes.push_back(bl);
    hes.push_back(rb);
    hes.push_back(rt);
    hes.push_back(tr);
    hes.push_back(tl);
    hes.push_back(lb);
    hes.push_back(lt);

    faces.push_back(f1);
    faces.push_back(f2);
    faces.push_back(f3);
    faces.push_back(f4);

    return newEdges;
}

struct Triangle {
    struct Point* v1, v2, v3;
}
Mesh::Mesh(std::vector<glm::vec3> vertexBuffer, std::vector<uint> connectivityBuffer) {
    /* object contains list linear array of vertex
     * buffer[2] is a list of triangles in groups of 3
     * 
     * 1. Create list of Point from buffer[0]
     * 2. Create list of triangles from buffer[2]
     * 3. Create 3 halfedges for each triangle
     * 4. Create a map of pairs of vertices to halfedges
     * 5. For each halfedge, get the flip by searching map 
     *    for opposite points
     */

    struct Point* p;
    struct Triangle t;
    std::vector<Triangle> t_list;
    for(int i = 0; i < vertexBuffer.size(); i++) {
        // 1. create list of Point w/o hedges
        p = new Point;
        p->pos = object->buffer[0][i];
        p->newPos = NULL;
        p->isNew = false;
        pts.push_back(p);
    }
    for(int i = 0; i < connectivityBuffer.size(); i++) {
        // 2. create list of triangles
        if (i % 3 == 0) {
            t.v1 = pts[connectivityBuffer[i]];
        } else if (i % 3 == 1) {
            t.v2 = pts[connectivityBuffer[i]];
        } else {
            t.v3 = pts[connectivityBuffer[i]];
            t_list.push_back(t);
        }
    }
   
    struct HalfEdge* hedge1, hedge2, hedge3;
    struct Face* f;
    std::unordered_map< std::pair<uint, uint>, HalfEdge* > m;
    // 3. create 3 halfedges for each triange
    // 4. populate map w/ vertex pairs and hedges
    for(int i = 0; i < t_list.size(); i++) {
        hedge1 = new HalfEdge;
        hedge2 = new HalfEdge;
        hedge3 = new HalfEdge;
        // assign src ptr
        hedge1->src = t_list[i].v1;
        hedge2->src = t_list[i].v2;
        hedge3->src = t_list[i].v3;
        // assign Point.he ptr
        t_list[i].v1->he = hedge1;
        t_list[i].v2->he = hedge2;
        t_list[i].v3->he = hedge3;
        // assign next ptr
        hedge1->next = hedge2;
        hedge2->next = hedge3;
        hedge3->next = hedge1;
        //assign Face* to arbitrary hedge in triangle
        f = new Face;
        f->he = hedge1;
        // assign face ptr
        hedge1->face = f;
        hedge2->face = f;
        hedge3->face = f;
        // add hedges to Mesh's vector
        hes.push_back(hedge1);
        hes.push_back(hedge2);
        hes.push_back(hedge3);
        // populate map
        uint tmp1, tmp2, tmp3; //tmp vars to hold point indices
        tmp1 = t_list[i].v1->index;
        tmp2 = t_list[i].v2->index;
        tmp3 = t_list[i].v3->index;
        m.insert({
            {std::make_pair<uint, uint>(tmp1, tmp2), hedge1},
            {std::make_pair<uint, uint>(tmp2, tmp3), hedge2},
            {std::make_pair<uint, uint>(tmp3, tmp1), hedge3}
        });
        faces.push_back(f);
    }

    // 5. for each hedge get the flip using map
    std::pair<uint, uint> key;
    for(int i = 0; i < hes.size(); i++) {
        // hes[i] goes from src->dest, so the flip is dest->src
        // where dest := he->next->src
        key = std::make_pair<uint, uint>(hes[i]->next->src->index, 
                                            hes[i]->src->index);
        hes[i]->flip = m[key];
    }
        //TODO: Make an Edge for every PAIR of hedges
    struct Edge* e;
    for(int i = 0; i < hes.size(); i++) {
        if(hes[i]->parent == null) {
            e = new Edge;
            hes[i]->parent = e;
            hes[i]->flip->parent = e;
            e->he = hes[i];
            e->newPos = NULL;
            e->isNew = false;
            edges.push_back(e);
        }
    }
}

void Mesh::updatePointPos() {
    for (int i = 0; i < pts.size(); i++) {
        pts[i]->pos = pts[i]->newPos;
        pts[i]->isNew = false;
    }
}

/* For each vertex, find the area-weighted average
 * of all surrounding face normals to get the new 
 * normal for this vertex
 * Aglo:
 * 1. init glm::vec3 res
 * 2. For each adjacent face,
 *  2a. get 2 edge vectors and take cross product
 *  2b. add to res
 * 3. normalize res
 */
void Mesh::calcNormals() {
    glm::vec3 res, edge1, edge2;
    HalfEdge* he, he0;
    for (int i = 0; i < pts.size(); i++) {
        res = glm::vec3(0.0f, 0.0f, 0.0f);
        he0 = pts[i]->he;

        do {
            edge1 = he->src->pos - he->flip->src->pos;
            edge2 = he->next->src->pos - he->next->flip->src->pos;
            res += glm::cross(edge2, edge1);
            he = he->flip->next;
        } while(he != he0);

        pts[i]->normal = glm::normalize(res);
    }
}

void Mesh::bindBuffers() {
    //TODO
}

Mesh::~Mesh() {
    delete pts;
    delete hes;
    delete faces;
    delete edges;
}
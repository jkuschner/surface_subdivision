#include "Mesh.h"
#include <unordered_map>
#include <utility>

//TODO: After doing the loop subdivition, need to calculate normals
// w/ area weighted average

/* Full Subdiv algo:
 * 1. Calculate positions of new Points
 * 2. Calculate new positions of old Points
 * 3. split all old edges and attatch to new Points
 * 4. Meanwhile add all new edges to a new vector then swap vectors
 *      !! Remember to free old edge pointers !!
 * 5. flip all NEW edges that connect new and old point
 * 6. set all old Points to newPos
 * 
 *
 * 
 */

typedef unsigned int uint;

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
    p->pos = (0.375f*P1) + (0.375f*P2) + (0.125f*p1) + (0.125f*p2);
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
    //unsigned int v1, v2, v3;
    struct Point* v1, v2, v3;
}

Mesh::Mesh(Obj* object) {
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
    for(int i = 0; i < object->count; i++) {
        // 1. create list of Point w/o hedges
        p = new Point;
        p->pos = object->buffer[0][i];
        p->normal = object->buffer[1][i];
        p->index = object->buffer[2][i];
        p->newPos = NULL;
        p->isNew = false;
        // TODO: remove duplicates from pts
        pts.push_back(p);

        // 2. create list of triangles
        if (i % 3 == 0) {
            //t.v1 = object->buffer[2][i];
            t.v1 = p;
        } else if (i % 3 == 1) {
            //t.v2 = object->buffer[2][i];
            t.v2 = p;
        } else {
            //t.v3 = object->buffer[2][i];
            t.v3 = p;
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

/*
void Mesh::init(const char* filename) {
    std::vector< glm::vec3 > temp_vertices, vertices;
    std::vector< uint > temp_vertexIndices, indices;
    std::vector< Point > temp_points, points;
    std::vector< HalfEdge > temp_hedges, hedges;
    std::vector< Face > temp_faces, faces;
    std::vector< Edge > temp_edges, edge;
    
    // load obj file
    FILE * file = fopen( filename , "r" );
    if( file == NULL ){
        std::cerr << "Cannot open file: " << filename << std::endl;
        exit(-1);
    }
    std::cout << "Loading " << filename << "...";
    while (!feof(file)){
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader
        if ( strcmp( lineHeader, "v" ) == 0 ){
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);
        }else if ( strcmp( lineHeader, "vn" ) == 0 ){
            continue;
        }else if ( strcmp( lineHeader, "f" ) == 0 ){
            //std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], normalIndex[3];
            fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2] );
            temp_vertexIndices.push_back(vertexIndex[0]);
            temp_vertexIndices.push_back(vertexIndex[1]);
            temp_vertexIndices.push_back(vertexIndex[2]);
        }
    }
    std::cout << "done." << std::endl;
    
    // post processing
    std::cout << "Processing data...";
    unsigned int n = temp_vertexIndices.size(); // #(triangles)*3
    vertices.resize(n);
    normals.resize(n);
    indices.resize(n);
    for (unsigned int i = 0; i<n; i++){
        indices[i] = i;
        vertices[i] = temp_vertices[ temp_vertexIndices[i] - 1 ];
        normals[i] = temp_normals[ temp_normalIndices[i] - 1 ];
    }
    std::cout << "done." << std::endl;
    
    // setting up buffers
    std::cout << "Setting up buffers...";
    glGenVertexArrays(1, &vao );
    buffers.resize(3);
    glGenBuffers(3, buffers.data());
    glBindVertexArray(vao);
    
    // 0th attribute: position
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, n*sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    
    // 1st attribute: normal
    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, n*sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    
    // indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n*sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);
    
    count = n;
    glBindVertexArray(0);
    std::cout << "done." << std::endl;
}
*/
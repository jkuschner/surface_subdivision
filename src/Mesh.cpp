#include "Mesh.h"
#include <unordered_map>
#include <utility>

typedef unsigned int uint;

/*
 * Given an edge in a Mesh object, replace the
 * edge with it's flipped version and return the
 * flipped edge
 * @param edge: the edge to be flipped
 * @return: the flipped edge
 */
Edge Mesh::flip(Edge* edge) {

}

/*
 * Given an edge, add another edge that splits
 * this one in half, increasing the number of triangles
 * in the mesh.
 * @param edge: the edge to be split
 * @return: the new edge
 */
Edge Mesh::split(Edge* edge) {

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
    struct Edge* e;
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
        // assign next ptr
        hedge1->next = hedge2;
        hedge2->next = hedge3;
        hedge3->next = hedge1;
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
        //assign Face* and Edge* to arbitrary hedge in triangle
        f = new Face;
        f->he = hedge1;
        faces.push_back(f);
        e = new Edge;
        e->he = hedge1;
        edges.push_back(e);
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
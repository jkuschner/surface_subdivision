# Loop Subdivision

In this project, I created a program that should be able to parse a .obj file that describes a triangle mesh into a Halfedge mesh. Then, the program is able to subdivide the triangles in the mesh such that the surfaces appear smoother with each iteration of the subdivision algorithm. The subdivision algorithm has the following steps:
1. Update the position of all vertices based on it's surrounding vertices.
2. Create a new vertex, who's position is also based on surrounding vertices for every edge in the mesh
3. Connect each new vertex with a new edge, turning each triangle in the mesh into 4 subtriangles

![subtriangles](images/subtriangles.png)

## Halfedge Mesh

![halfedge](images/halfedge.png)

In order to implement the above algorithm, we need a convienient way to traverse the topology of the triangle mesh. Traditionally, the triangle mesh is described by one-dimensional vertex and connectivity buffers. The vertex buffer stores the geometric position of all vertices in the mesh, and the connectivity buffer stores the indices of the vertices that are connected as a triangle in sets of 3. The Halfedge Mesh is a data structure that is meant to store the topology of the mesh and make it easy to acess neighboring edges and vertices. The version of the Halfedge mesh that I implemented in this project stores a list of all Points, Faces, Edges, and Halfedges in the entire mesh. The Halfedges have many pointers and is the real heavy lifter of this data structure. Each Halfedge contains:
1. `next`, a pointer to the next Halfedge within the Face
2. `flip`, a pointer to the Halfedge that is along the same edge but in the oppposite direction
3. `src`, a pointer to the Point that this Halfedge starts at
4. `face`, a pointer to the Face this Halfedge resides in
5. `parent`, a pointer to the Edge that this Halfedge is a part of.

The rest of the structure in the Halfedge mesh serve as compliments to the HalfEdge. Each Face stores a pointer to an arbitrary Halfedge(`he`) that lies on that face. That way, one could access all Halfedges given a face with `myface->he`, `myface->he->next`, and `myface->he->next->next`. Each Edge similarly stores a pointer on one of the Halfedges that go along that Edge arbitrarily. The Edge part of the mesh is not totally necessary, but it makes the implementation of the subdivision algorithm considerably easier. Finally, each Point stores a pointer to one of the Halfedges that start at that point, but in addition to this topological information, Points also are responsible for storing the geometric information of the triangle mesh. Most importantly each point has a field for the vertex position and normal call `pos` and `normal`, respectively. `pos` simply stores the 3-D coordinates of that vertex as a `glm::vec3` and `normal` stores the vertex normal as a `glm::vec3`. I'll go into more detail about how those are calculated and updated later in the write-up. In the source code, there are a few other fields in the Halfedge Mesh's structures that I did not mention. Those are simply there to make the implementation easier.

## Geometry Calculations
The first two steps of the loop subdivision algorithm require us to calculate 2 things:
1. The *new* position of existing Points in the mesh
2. The position of Points to be added to the mesh

### The new position of existing Points
To calculate the new position of points that are already in the mesh, we take a weighted average of the position of all of the surrounding points based on the following formula:

![vertex_reposition](images/vertex_reposition.png)

After finding this new position, it gets stored in that Point's `newPos` field for later use. Since in the next step, we still need the Point's old position for the calculations, we do not want to overwrite the Point's `pos` field.

### The position of new Points
To calculate the position of new points, we take another weighted average of the vertices surrounding the edge that we are adding the new Point to, like so:

![new_vertex](images/new_vertex.png)

Similar to the previous calculation, this newly calculated position will get stored in the Edge's `newPos` field so that it does not affect subsequent vertex calculations.

## Topology Updates
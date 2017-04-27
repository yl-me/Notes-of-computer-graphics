#ifndef MODEL_H

#define MODEL_H

class Model {
public:
    struct Mesh{
        // The index of the material (texture and lighting coeffecients) to use for the mesh
        int m_materialIndex;
        // The numbers of triangle in the mesh 
        int m_numTriangles;
        //Storing the triangles in the mesh in the same way as the triangle stored indices to its vertices
        int* m_pTriangleIndices;
    };
    struct Material {
        float m_ambient[4], m_diffuse[4], m_specular[4], m_emissive[4];
        float m_shininess;
        GLuint m_texture;
        char* m_pTextureFilename;
    };
    struct Triangle {
        float m_vertexNormals[3][3];
        // The texture coordinates for each of the 3 vertices
        float m_s[3], m_t[3];
        // The 3 vertices
        int m_vertexIndices[3];
    };
    struct Vertex {
        char m_boneID;       // For skeletal animation
        float m_location[3];
    };

public:
    Model();
    virtual ~Model();
    virtual bool loadModelData(const char* filename) = 0;
    void draw();
    void reloadTextures();

protected:
    // Mesh used
    int m_numMeshes;
    Mesh* m_pMeshes;

    // Materials used
    int m_numMaterials;
    Material* m_pMaterials;

    // Triangles used
    int m_numTriangles;
    Triangle* m_pTriangles;

    // Vertices used
    int m_numVertices;
    Vertex* m_pVertices;
};

#endif
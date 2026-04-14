#include "GeometricModelLoader.h"

class GeometricModel;

class OBJLoader : public GeometricModelLoader {
public:
    OBJLoader();
    ~OBJLoader();
    virtual bool loadModel(std::string name, GeometricModel *model);

private:
    void setupForTextureCoordinates(GeometricModel *model);
    void computeNormals(GeometricModel *model);
    void computeTangents(GeometricModel *model);
};

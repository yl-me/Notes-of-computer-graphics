#ifndef MILKSHAPEMODEL_H

#define MILKSHAPEMODEL_H

#include "Model.h"

class MilkshapeModel : public Model {
public:
    MilkshapeModel() = default;
    virtual ~MilkshapeModel() = default;
    virtual bool loadModelData(const char* filename);
};

#endif
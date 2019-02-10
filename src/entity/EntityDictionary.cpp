#include "EntityDictionary.hpp"
#include "geo/Vec3.hpp"

template<>
void edictParseAttribute(const char *value, bool &outValue)
{
    if(strcmp(value, "true") == 0)
    {
        outValue = true;
    }
    else if(strcmp(value, "false") == 0)
    {
        outValue = false;
    }
    else
    {
        outValue = atoi(value);
    }
}

template<>
void edictParseAttribute(const char* value, int& outValue)
{
    outValue = atoi(value);
}

template<>
void edictParseAttribute(const char* value, BrushModelId& outValue)
{
    // Skip over '*' character
    const char* startOfModelId = value + 1;
    int id = atoi(startOfModelId);

    outValue = BrushModelId(id);
}

void edictParseAttribute(const char* value, char* outValue)
{
    strcpy(outValue, value);
}

template<>
void edictParseAttribute(const char* value, Vec3fp& dest)
{
    Vec3f v;
    sscanf(value, "%f %f %f", &v.x, &v.y, &v.z);

    dest = v
        .toVec3<fp>()
        .toX3dCoords();
}

X_EdictAttribute *X_Edict::getAttribute(const char *name) const
{
    for(int i = 0; i < totalAttributes; ++i)
    {
        if(strcmp(name, attributes[i].name) == 0)
        {
            return attributes + i;
        }
    }

    return nullptr;
}

bool X_Edict::hasAttribute(const char *name) const
{
    return getAttribute(name) != nullptr;
}



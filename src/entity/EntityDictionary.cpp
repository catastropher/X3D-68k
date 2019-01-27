#include "EntityDictionary.hpp"
#include "geo/Vec3.hpp"

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

void edictParseAttribute(const char* value, int& outValue)
{
    outValue = atoi(value);
}

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

void edictParseAttribute(const char* value, Vec3fp& dest)
{
    Vec3f v;
    sscanf(value, "%f %f %f", &v.x, &v.y, &v.z);

    dest = v.toVec3<fp>();
}

template<typename T>
bool X_Edict::getValue(const char *name, T &outValue)
{
    X_EdictAttribute* attribute = getAttribute(name);

    if(attribute == nullptr)
    {
        return false;
    }
    else
    {
        edictParseAttribute(attribute->value, outValue);

        return true;
    }
}


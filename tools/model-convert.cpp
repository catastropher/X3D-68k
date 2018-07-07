#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <cstdio>

float degToRadians(float deg) {
    return deg * 3.1415926535 / 180.0;
}

struct Vec3 {
    float x, y, z;
    
     Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) { }
     Vec3() : x(0), y(0), z(0) { }
    
     Vec3 operator+(const Vec3& v) const {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }
    
     Vec3 operator-(const Vec3& v) const {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }
    
     float lengthSquared() const {
        return x * x + y * y + z * z;
    }
    
     float length() const {
        return sqrt(lengthSquared());
    }
    
     Vec3 normalize() const {
        float len = length();
        
        return Vec3(x / len, y / len, z / len);
    }
    
     Vec3 operator*(float f) const {
        return Vec3(x * f, y * f, z * f);
    }
    
     Vec3 cross(const Vec3& v) const {
        const Vec3& u = *this;
        
        return Vec3(
            u.y * v.z - u.z * v.y,
            u.z * v.x - u.x * v.z,
            u.x * v.y - u.y * v.x
        );
    }
    
     Vec3 neg() const {
        return Vec3(-x, -y, -z);
    }
    
     Vec3 multiplyEach(const Vec3& v) const {
        return Vec3(x * v.x, y * v.y, z * v.z);
    }
    
     Vec3 maxValue(float maxValueAllowed) {
        #ifdef __WITH_CUDA__
        return Vec3(fminf(maxValueAllowed, x), fminf(maxValueAllowed, y), fminf(maxValueAllowed, z));
        #else
        return Vec3(std::min(maxValueAllowed, x), std::min(maxValueAllowed, y), std::min(maxValueAllowed, z));
        #endif
    }
    
     Vec3 reflectAboutNormal(const Vec3& normal) const {
        float ndot = -normal.dot(*this);
        return (*this + (normal * 2 * ndot)).normalize();
    }
    
     float dot(const Vec3& v) const {
        return x * v.x + y * v.y + z * v.z;
    }
    
     float distanceBetween(Vec3& v) {
        return (*this - v).length();
    }
    
    std::string toXString() const {
        char str[128];
        sprintf(str, "{ %f, %f, %f }", x, y, z);
        
        return std::string(str);
    }
};

struct Mat4 {
    float elements[4][4];
    
    Mat4(const float e[4][4]) {
        for(int i = 0; i < 4; ++i)
            for(int j = 0; j < 4; ++j)
                elements[i][j] = e[i][j];
    }
    
    Mat4() { }
    
    Mat4 multiply(Mat4 mat) const {
        Mat4 res;
        
        for(int i = 0; i < 4; ++i) {
            for(int j = 0; j < 4; ++j) {
                float sum = 0;
                
                for(int k = 0; k < 4; ++k) {
                    sum += elements[i][k] * mat.elements[k][j];
                }
                
                res.elements[i][j] = sum;
            }
        }
        
        return res;
    }
    
    Mat4 rotateAroundY(float angle) const {
        float sinAngle = sin(degToRadians(angle));
        float cosAngle = cos(degToRadians(angle));
        
        float mat[4][4] = {
            { cosAngle, 0,  -sinAngle,  0 },
            { 0,        1,  0,          0 },
            { sinAngle, 0,  cosAngle,   0 },
            { 0,        0,  0,          1 }
        };
        
        return multiply(Mat4(mat));
    }
    
    Mat4 rotateAroundX(float angle) const {
        float sinAngle = sin(degToRadians(angle));
        float cosAngle = cos(degToRadians(angle));
        
        float mat[4][4] = {
            { 1, 0,        0,           0 },
            { 0, cosAngle, -sinAngle,   0 },
            { 0, sinAngle, cosAngle,    0 },
            { 0, 0,        0,           1 }
        };
        
        return multiply(Mat4(mat));
    }
    
    Mat4 translate(const Vec3 v) const {
        float mat[4][4] = {
            { 1, 0, 0, v.x },
            { 0, 1, 0, v.y },
            { 0, 0, 1, v.z },
            { 0, 0, 0, 1   }
        };
        
        return multiply(Mat4(mat));
    }
    
    Vec3 transformVec4(const float* vv) const {
        float rot[4];
        
        for(int i = 0; i < 4; ++i) {
            rot[i] = 0;
            
            for(int j = 0; j < 4; ++j) {
                rot[i] += vv[j] * elements[i][j];
            }
        }
        
        float w = rot[3];
        
        if(w != 0)
            return Vec3(rot[0] / w, rot[1] / w, rot[2] / w);
        else
            return Vec3(rot[0], rot[1], rot[2]);
    }
    
    Vec3 rotateVec3(const Vec3 v) const {
        float vv[4] = { v.x, v.y, v.z, 1 };
        return transformVec4(vv);
    }
    
    Vec3 rotateVec3Normal(const Vec3 v) const {
        Mat4 mat = identity();
        
        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
                mat.elements[i][j] = elements[j][i];
            
            float vv[4] = { v.x, v.y, v.z, 0 };
        return transformVec4(vv);
    }
    
    static Mat4 identity() {
        const float mat[4][4] = {
            { 1, 0, 0, 0 },
            { 0, 1, 0, 0 },
            { 0, 0, 1, 0 },
            { 0, 0, 0, 1 }
        };
        
        return Mat4(mat);
    }
};

struct Triangle {
    int v[3];
    
    Triangle(int v0, int v1, int v2) {
        v[0] = v0;
        v[1] = v1;
        v[2] = v2;
    }
};

struct Model {
    std::vector<Vec3> vertices;
    std::vector<Triangle> triangles;
    
    void saveVertices(FILE* file) {
        fprintf(file, "%d\n", (int)vertices.size());
        
        for(int i = 0; i < vertices.size(); ++i) {
            fprintf(file, "%d %d %d\n", (int)vertices[i].x, (int)vertices[i].y, (int)vertices[i].z);
        }
    }
    
    void saveTriangles(FILE* file) {
        fprintf(file, "%d\n", (int)triangles.size());
        
        for(int i = 0; i < triangles.size(); ++i) {
            Triangle t = triangles[i];
            fprintf(file, "%d %d %d\n", t.v[0], t.v[1], t.v[2]);
        }
    }
    
    void save(std::string fileName) {
        FILE* file = fopen(fileName.c_str(), "wb");
        
        if(!file)
            throw "Failed to open file for writing";
        
        fprintf(file, "XMOD\n");
        saveVertices(file);
        saveTriangles(file);
        
        fclose(file);
    }
};

struct ModelLoader {
    struct LineArgument {
        std::vector<std::string> part;
    };
    
    struct Line {
        std::string type;
        std::vector<LineArgument> arguments;
    };
    
    std::vector<Vec3> vertices;
    std::vector<Triangle> triangles;
    std::vector<Vec3> normals;
    
    Vec3 calculateModelCenter() const {
        Vec3 center(0, 0, 0);
        
        for(Vec3 v : vertices) {
            center = center + v;
        }
        
        return center * (1.0 / vertices.size());
    }
    
    Model loadFile(std::string fileName) {
        printf("Loading model %s\n", fileName.c_str());
        
        FILE* file = fopen(fileName.c_str(), "rb");
        if(!file)
            throw "Failed to load file: " + fileName;
         
        std::string fileContents;
        int c;
        while((c = fgetc(file)) != EOF) {
            fileContents += (char)c;
        }
        
        fileContents += '\0';
        fclose(file);
        
        std::vector<Line> lines = parseFile(fileContents);
        processLines(lines);
        
        printf("Done loading file\n");
        
        Mat4 mat = Mat4::identity().translate(calculateModelCenter().neg());
        
        for(int i = 0; i < vertices.size(); ++i) {
            vertices[i] = mat.rotateVec3(vertices[i]);
        }
        
        Model model;
        model.triangles = triangles;
        model.vertices = vertices;
        
        return model;
    }
    
    void processLines(std::vector<Line>& lines) {
        for(Line& line : lines) {
            processLine(line);
        }
    }
    
    void processLine(Line& line) {
        if(line.type == "#" || line.type == "" || line.type == "s" || line.type == "g" || line.type == "usemtl" || line.type == "mtllib" || line.type == "vt") return;
        if(processVertex(line)) return;
        if(processFace(line)) return;
        if(processNormal(line)) return;
        
        throw "Unknown line type: " + line.type;
    }
    
    bool processVertex(Line& line) {
        if(line.type != "v")
            return false;
        
        assert(line.arguments.size() == 3);
        
        Vec3 v(
            atof(line.arguments[0].part[0].c_str()),
            atof(line.arguments[1].part[0].c_str()),
            atof(line.arguments[2].part[0].c_str())
        );
        
        vertices.push_back(v);
        
        return true;
    }
    
    void addTriangle(int v0, int v1, int v2) {        
        triangles.push_back(Triangle(v0, v1, v2));
    }
    
    bool processFace(Line& line) {
        if(line.type != "f")
            return false;
        
        assert(line.arguments.size() == 3 || line.arguments.size() == 4);
        
        int v0 = atoi(line.arguments[0].part[0].c_str()) - 1;
        int v1 = atoi(line.arguments[1].part[0].c_str()) - 1;
        int v2 = atoi(line.arguments[2].part[0].c_str()) - 1;
        int v3 = 0;
        
        if(line.arguments.size() == 4)
            v3 = atoi(line.arguments[3].part[0].c_str()) - 1;
        
        addTriangle(v0, v1, v2);
        
        if(line.arguments.size() == 4) {
            addTriangle(v2, v3, v0);
        }
        
        return true;
    }
    
    bool processNormal(Line& line) {
        if(line.type != "vn")
            return false;
        
        assert(line.arguments.size() == 3);
        
        Vec3 n(
            atof(line.arguments[0].part[0].c_str()),
            atof(line.arguments[1].part[0].c_str()),
            atof(line.arguments[2].part[0].c_str())
        );
        
        normals.push_back(n);
        
        return true;
    }
    
    char* findLineEnd(char* start) {
        while(*start && *start != '\n')
            ++start;
        
        return start;
    }
    
    char* consumeWhitespace(char* start, char* end) {
        while(start < end && (*start == ' ' || *start == '\t'))
            ++start;
        
        return start;
    }
    
    std::vector<Line> parseFile(std::string fileContents) {
        char* start = &fileContents[0];
        char* end;
        char* fileEnd = &fileContents[fileContents.size() - 1];
        std::vector<Line> lines;
        
        while(start < fileEnd) {
            end = findLineEnd(start);
            //std::cout << "Line: " << std::string(start, end) << std::endl;
            lines.push_back(parseLine(start, end));
            start = end + 1;
        }
        
        return lines;
    }
    
    Line parseLine(char* start, char* end) {
        char* startSave = start;
        
        start = consumeWhitespace(start, end);
        Line line;
        
        while(start < end && (*start == '#' || isalpha(*start))) {
            line.type += *start;
            ++start;
        }
        
        if(line.type == "#")
            return line;
        
        int count = 0;
        
        while(start < end) {
            start = consumeWhitespace(start, end);
            LineArgument lineArg;
            
            if(++count == 10000)
                throw "Too many iterations";
            
            while(start < end) {
                std::string arg;
                
                while(start < end && (*start == '.' || isdigit(*start) || *start == '-' || isalpha(*start) || *start == '_')) {
                    arg += *start;
                    ++start;
                }
                
                lineArg.part.push_back(arg);
                
                if(lineArg.part.size() > 10)
                    throw "Too many arguments for line: " + std::string(startSave, end);
                
                if(*start != '/')
                    break;
                
                ++start;
            }
            
            line.arguments.push_back(lineArg);
        }
        
        return line;
    }
};

int main(int argc, char* argv[]) {
    if(argc != 3) {
        printf("Converts 3D models in .obj format to X3D's model format\n");
        printf("Usage: %s [input model] [output file]l\n", argv[0]);
        return 0;
    }
    
    try {
        ModelLoader loader;
        Model model = loader.loadFile(argv[1]);
        model.save(argv[2]);
    }
    catch(std::string s) {
        printf("Error converting file: %s\n", s.c_str());
    }
}



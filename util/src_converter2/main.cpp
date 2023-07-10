#include <filesystem>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

struct vec2 {
    float x;
    float y;
};

struct vec3 {
    inline void operator+=(const vec3 &other) {
        x += other.x;
        y += other.y;
        z += other.z;
    }
    float x;
    float y;
    float z;
};

struct vec2d {
    double x;
    double y;
};

struct vec3d {
    inline operator vec3 () const {
        return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
    }
    double x;
    double y;
    double z;
};

struct VertexData {
    vec3 pos;
    vec2 tex;
    vec3 norm;
};

struct Triangle {
    uint32_t i[3];
};

struct OjmHeader {
    ssize_t sourceTimestamp = -1; // Last modification time of the source file, -1 if there is no source
    float radius = 1; // Radius of the ojm before normalization
    uint16_t nbShapes = 1; // Number of shapes in this OJM
    bool poorlyCentered = false;
};

struct ShapeAttributes {
    vec3 Ka;
    float Ns;
    vec3 Kd;
    float T;
    vec3 Ks;
};

struct ShapeHeader {
    uint32_t vertexCount;
    uint32_t indexCount;
    uint8_t len_map_Ka = 0;
    uint8_t len_map_Kd = 0; // Unused - always 0
    uint8_t len_map_Ks = 0; // Unused - always 0
    bool pushAttr = false; // True if attr differ from the previous shape
};

struct Obj {
    std::vector<VertexData> vertices;
    std::vector<Triangle> indices;
};

constexpr unsigned short FLAG(const char *str) {
    return str[0] | (str[1] << 8);
}

#define LOAD_INDEX \
if (norm.empty()) { \
    uid >>= 20; \
    uid -= 0x100001; \
    auto idx = indexer.insert(std::pair<int, int>(uid, ret.vertices.size())); \
    if (idx.second) { \
        ret.vertices.push_back({ \
            pos[uid >> 20], \
            tex[uid & 0xfffff], \
            {0, 0, 0} \
        }); \
    } \
    shape.push_back(idx.first->second); \
} else { \
    uid -= 0x10000100001; \
    auto idx = indexer.insert(std::pair<int, int>(uid, ret.vertices.size())); \
    if (idx.second) { \
        ret.vertices.push_back({ \
            pos[uid >> 40], \
            tex[(uid >> 20) & 0xfffff], \
            norm[uid & 0xfffff] \
        }); \
    } \
    shape.push_back(idx.first->second); \
}

Obj process(char *begin, char *end, const std::vector<vec3d> &pos, const std::vector<vec2> &tex, const std::vector<vec3> &norm)
{
    Obj ret;
    std::map<int, int> indexer;
    std::vector<int> shape;
    std::string tmp;
    // Reduce reallocations
    ret.vertices.reserve(65536);
    ret.indices.reserve(65536);
    tmp.reserve(32);
    shape.reserve(4);
    while (begin < end && *(begin++) == 'f') {
        uint64_t uid = 0;
        READ_UID:
        switch (*++begin) {
            case '\r':
                begin += 2;
                break;
            case '\n':
                begin += 1;
                break;
            case ' ':
                LOAD_INDEX;
                uid = 0;
                goto READ_UID;
            case '/':
                uid <<= 20;
                goto READ_UID;
            default:
                uid = (uid & 0xfffffffffff00000) | ((uid & 0xfffff) * 10 + (*begin) - '0');
                goto READ_UID;
        }
        LOAD_INDEX;
        switch (shape.size()) {
            case 4:
                ret.indices.push_back({shape[2], shape[3], shape[0]});
                [[fallthrough]];
            case 3:
                ret.indices.push_back({shape[0], shape[1], shape[2]});
                break;
        }
        shape.clear();
    }
    return ret;
}

void saveBinary(const char *srcFilename, const OjmHeader &mainHead, const Obj &object)
{
    std::string filename = srcFilename;
    filename[filename.size()-2] = 'j';
    filename.back() = 'm';
    filename += ".bin";
    std::ofstream file(filename, std::ofstream::trunc | std::ofstream::binary);
    file.write((const char*) &mainHead, sizeof(mainHead));
    ShapeHeader header {object.vertices.size(), object.indices.size() * 3};
    file.write((const char*) &header, sizeof(header));
    file.write((const char*) object.vertices.data(), object.vertices.size() * sizeof(VertexData));
    file.write((const char*) object.indices.data(), object.indices.size() * sizeof(Triangle));
}

void fixOrientationOf(Obj &object)
{
    for (auto &triangle : object.indices) {
        auto &v0 = object.vertices[triangle.i[0]];
        auto &v1 = object.vertices[triangle.i[1]];
        auto &v2 = object.vertices[triangle.i[2]];
        // Generate normal from vertices
        vec3 normal;
        {
            const vec3d v01{
                v1.pos.x - v0.pos.x,
                v1.pos.y - v0.pos.y,
                v1.pos.z - v0.pos.z
            };
            const vec3d v02{
                v2.pos.x - v0.pos.x,
                v2.pos.y - v0.pos.y,
                v2.pos.z - v0.pos.z
            };
            const vec3d tmp {
                v01.y * v02.z - v01.z * v02.y,
                v01.z * v02.x - v01.x * v02.z,
                v01.x * v02.y - v01.y * v02.x
            };
            const double radius = sqrt(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
            normal.x = tmp.x / radius;
            normal.y = tmp.y / radius;
            normal.z = tmp.z / radius;
        }
        if (normal.x * v0.norm.x + normal.y * v0.norm.y + normal.z * v0.norm.z < 0) {
            auto tmp = triangle.i[2];
            triangle.i[2] = triangle.i[1];
            triangle.i[1] = tmp;
        }
    }
}

void generateNormalsOf(Obj &object)
{
    for (auto &triangle : object.indices) {
        auto &v0 = object.vertices[triangle.i[0]];
        auto &v1 = object.vertices[triangle.i[1]];
        auto &v2 = object.vertices[triangle.i[2]];
        // Generate normal from vertices
        vec3 normal;
        {
            const vec3d v01{
                v1.pos.x - v0.pos.x,
                v1.pos.y - v0.pos.y,
                v1.pos.z - v0.pos.z
            };
            const vec3d v02{
                v2.pos.x - v0.pos.x,
                v2.pos.y - v0.pos.y,
                v2.pos.z - v0.pos.z
            };
            const vec3d tmp {
                v01.y * v02.z - v01.z * v02.y,
                v01.z * v02.x - v01.x * v02.z,
                v01.x * v02.y - v01.y * v02.x
            };
            const double radius = sqrt(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
            normal.x = tmp.x / radius;
            normal.y = tmp.y / radius;
            normal.z = tmp.z / radius;
        }
        v0.norm += normal;
        v1.norm += normal;
        v2.norm += normal;
    }
    for (auto &vertex : object.vertices) {
        float radius = sqrt(vertex.norm.x * vertex.norm.x + vertex.norm.y * vertex.norm.y + vertex.norm.z * vertex.norm.z);
        vertex.norm.x /= radius;
        vertex.norm.y /= radius;
        vertex.norm.z /= radius;
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        std::cout << argv[0] << " [option] <filename>\n";
        std::cout << "FLAGS :\n";
        std::cout << "-f\tfix the orientation of the triangles\n";
        std::cout << "-n\tgenerate normals from vertices and triangle orientation\n";
        std::cout << "-r\tkeep origin radius, for compatibility with older scripts\n";
        return 0;
    }
    bool fixOrientation = false;
    bool generateNormals = false;
    bool keepOriginRadius = false;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            const char *ptr = argv[i];
            while (*++ptr) {
                switch (*ptr) {
                    case 'f':
                        fixOrientation = true;
                        break;
                    case 'n':
                        generateNormals = true;
                        break;
                    case 'r':
                        keepOriginRadius = true;
                        break;
                }
            }
        }
    }
    std::cout << "Loading ojml source file \"" << argv[argc-1] << "\"...\n";
    auto size = std::filesystem::file_size(argv[argc-1]);
    int fd = open(argv[argc-1], O_RDONLY);
    char *ptr = (char *) mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

    char * const end = ptr + size;
    std::vector<vec3d> pos;
    std::vector<vec2> tex;
    std::vector<vec3> norm;
    double radius = 0;
    pos.reserve(65536); // To reduce reallocations
    tex.reserve(65536); // To reduce reallocations
    norm.reserve(65536); // To reduce reallocations

    do {
        char *line = ptr;
        while (*(ptr++) != '\n' && ptr < end);
        const size_t lineSize = (ptr[-2] == '\r') ? ((size_t) (ptr - line - 2)) : ((size_t) (ptr - line - 1));
        if (lineSize > 5) {
            switch (*reinterpret_cast<uint16_t *>(line)) {
                case FLAG("v "): {
                    const vec3d tmp = {
                        strtod(line+2, &line),
                        strtod(line+1, &line),
                        atof(line+1)
                    };
                    pos.push_back(tmp);
                    const double tmpRadius = tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z;
                    if (tmpRadius > radius)
                        radius = tmpRadius;
                    break;
                }
                case FLAG("vt"):
                    tex.push_back({strtod(line+3, &line), atof(line+1)});
                    break;
                case FLAG("vn"):
                    norm.push_back({strtod(line+3, &line), strtod(line+1, &line), atof(line+1)});
                    break;
                case FLAG("f "): {
                    std::cout << "Normalizing vertices...\n";
                    radius = sqrt(radius);
                    OjmHeader header {.radius = (keepOriginRadius ? static_cast<float>(radius) : 1.f)};
                    for (auto &p : pos) {
                        p.x /= radius;
                        p.y /= radius;
                        p.z /= radius;
                    }
                    if (generateNormals && !fixOrientation)
                        norm.clear();
                    std::cout << "Loading and optimizing indices...\n";
                    auto obj = process(ptr, end, pos, tex, norm);
                    pos.clear();
                    pos.shrink_to_fit();
                    tex.clear();
                    tex.shrink_to_fit();
                    norm.clear();
                    norm.shrink_to_fit();
                    if (fixOrientation) {
                        std::cout << "Fixing triangle orientation...\n";
                        fixOrientationOf(obj);
                    }
                    if (generateNormals) {
                        std::cout << "Generating normals...\n";
                        for (auto &vertex : obj.vertices) {
                            vertex.norm.x = 0;
                            vertex.norm.y = 0;
                            vertex.norm.z = 0;
                        }
                        generateNormalsOf(obj);
                    }
                    std::cout << "Saving to binary ojm...\n";
                    saveBinary(argv[argc-1], header, obj);
                    std::cout << "Done.\n";
                    ptr = end;
                }
            }
        }
    } while (ptr < end);
    munmap(end - size, size);
    close(fd);
    return 0;
}

#ifndef OJM_HEADER_HPP_
#define OJM_HEADER_HPP_

struct OjmHeader {
    ssize_t sourceTimestamp; // Last modification time of the source file
    float radius; // Radius of the ojm before normalization
    uint16_t nbShapes; // Number of shapes in this OJM
    bool poorlyCentered;
};

struct ShapeAttributes {
    Vec3f Ka;
    float Ns;
    Vec3f Kd;
    float T;
    Vec3f Ks;
};

struct ShapeHeader {
    uint32_t vertexCount;
    uint32_t indexCount;
    uint8_t len_map_Ka;
    uint8_t len_map_Kd; // Unused - always 0
    uint8_t len_map_Ks; // Unused - always 0
    bool pushAttr; // True if attr differ from the previous shape
};

#endif /* end of include guard: OJM_HEADER_HPP_ */

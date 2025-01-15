#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_(), t_verts_(), t_faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            iss >> trash;
            std::vector<int> f, t_f;
            int itrash, idx, t_idx;
            while (iss >> idx >> trash >> t_idx >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                t_idx--;
                f.push_back(idx);
                t_f.push_back(t_idx);
            }
            faces_.push_back(f);
            t_faces_.push_back(t_f);
        } else if (!line.compare(0, 4, "vt  ")) {
            iss >> trash >> trash;
            Vec2f vt;
            float ftrash;
            iss >> vt.u >> vt.v >> ftrash;
            t_verts_.push_back(vt);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
    std::cerr << "# t_v# " << t_verts_.size() << " t_f# "  << t_faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

int Model::nt_verts() {
    return (int)t_verts_.size();
}

int Model::nt_faces() {
    return (int)t_faces_.size();
}

Vec2f Model::t_vert(int i){
    return t_verts_[i];
}

std::vector<int> Model::t_face(int idx) {
    return t_faces_[idx];
}

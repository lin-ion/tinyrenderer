#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<Vec2f> t_verts_;
	std::vector<std::vector<int> > t_faces_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	std::vector<int> face(int idx);
	int nt_verts();
	int nt_faces();
	Vec2f t_vert(int i);
	std::vector<int> t_face(int idx);
};

#endif //__MODEL_H__

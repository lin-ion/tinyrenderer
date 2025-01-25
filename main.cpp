#include <vector>
#include <iostream>
#include <cmath>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

Model *model = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;

Vec3f barycentric(Vec3f *pts, Vec3f P){;
  Vec3f AB = Vec3f(pts[1]-pts[0]);
  Vec3f AC = Vec3f(pts[2]-pts[0]);
  Vec3f PA = Vec3f(pts[0]-P);

  Vec3f u = Vec3f(AB.x, AC.x, PA.x)^Vec3f(AB.y, AC.y, PA.y);

  // abs(u.z) means double of area of the triangle
  if (std::abs(u.z)<1) return Vec3f(-1,1,1);
  // A: 1.f-(u.x+u.y)/u.z
  // B: u.x/u.z
  // C: u.y/u.z
  return Vec3f(1.f-(u.x+u.y)/u.z, u.x/u.z, u.y/u.z);
}

void triangle(TGAImage &image, Vec3f *pts, float *zbuffer, float *intensities) {
  Vec2i bboxmin( std::numeric_limits<int>::max(),  std::numeric_limits<int>::max());
  Vec2i bboxmax(-std::numeric_limits<int>::max(), -std::numeric_limits<int>::max());
  Vec2i clamp(image.get_width()-1, image.get_height()-1);
  for (int i=0; i<3; i++){
    bboxmin.x = std::max(0, std::min(bboxmin.x, int(pts[i].x+.5f)));
    bboxmin.y = std::max(0, std::min(bboxmin.y, int(pts[i].y+.5f)));

    bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, int(pts[i].x+.5f)));
    bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, int(pts[i].y+.5f)));
  }
  Vec3f P;
  for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
    for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
      Vec3f bc_screen = barycentric(pts, P);
      if (bc_screen[0]<0 || bc_screen[1]<0 || bc_screen[2]<0) continue;

      // compute z-value
      // 삼각형의 정점들의 z-value와 각 정점에 대한 barycentric 가중치를 곱하여 더하면 P의 z-value가 계산된다.
      P.z = 0.f;
      for (int i=0; i<3; i++) {
        P.z += bc_screen[i] * pts[i].z;
      }

      if (zbuffer[int(P.x+P.y*width)]<P.z) {
        zbuffer[int(P.x+P.y*width)] = P.z;

        float intensity = 0.f;

        for (int i=0; i<3; i++) {
          intensity += bc_screen[i] * intensities[i];
        }

        if (intensity > 0) {
          intensity = std::pow(intensity, 2.2f) * 255;
          TGAColor color = TGAColor(intensity,intensity,intensity,255);
          image.set(P.x, P.y, color);
        }
      }
    }
  }
}

Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;
}

Matrix lookAt(Vec3f eye, Vec3f center, Vec3f up) {
  Vec3f z = (eye-center).normalize();
  Vec3f x = (up^z).normalize();
  Vec3f y = (z^x).normalize();
  Matrix Minv = Matrix::identity(4);
  Matrix Tr = Matrix::identity(4);
  for (int i=0; i<3; i++) {
    Minv[0][i] = x[i];
    Minv[1][i] = y[i];
    Minv[2][i] = z[i];
    Tr[i][3] = -center[i];
  }
  return Minv*Tr;
}

int main(int argc, char **argv) {
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  TGAImage image(width, height, TGAImage::RGB);

  float *zbuffer = new float[width*height];
  for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

  Vec3f light_dir = Vec3f(1.f, -1.f, 1.f).normalize();

  Vec3f eye(1.f, 1.f, 3.f);
  Vec3f center(0.f, 0.f, 0.f);
  Vec3f up(0.f, 1.f, 0.f);

  Matrix ModelView = lookAt(eye, center, up);
  Matrix Projection = Matrix::identity(4);
  Matrix ViewPort = viewport(width/8, height/8, width*3/4, height*3/4);
  Projection[3][2] = -1.f/(eye-center).z;

  for (int i=0; i<model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    Vec3f screen_coords[3];
    float intensities[3];

    for (int j=0; j<3; j++) {
        Vec3f v = model->vert(face[j]);
        screen_coords[j] = Vec3f(ViewPort*Projection*ModelView*Matrix(v));

        Vec3f vn = model->vert_norm(face[j]);
        intensities[j] = vn * light_dir;
    }

    triangle(image, screen_coords, zbuffer, intensities);
  }

  image.flip_vertically();
  image.write_tga_file("output.tga");

  delete model;
  delete[] zbuffer;
  return 0;
}

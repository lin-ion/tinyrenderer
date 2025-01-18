#include <vector>
#include <iostream>
#include <cmath>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

Model *model = NULL;
const int width = 800;
const int height = 800;

Vec3f barycentric(Vec3f *pts, Vec3f P){;
  Vec3f AB = Vec3f(pts[1]-pts[0]);
  Vec3f AC = Vec3f(pts[2]-pts[0]);
  Vec3f PA = Vec3f(pts[0]-P);

  Vec3f u = Vec3f(AB.x, AC.x, PA.x)^Vec3f(AB.y, AC.y, PA.y);

  // abs(u.z) means area of triangle
  if (std::abs(u.z)<1) return Vec3f(-1,1,1);
  // A: 1.f-(u.x+u.y)/u.z
  // B: u.x/u.z
  // C: u.y/u.z
  return Vec3f(1.f-(u.x+u.y)/u.z, u.x/u.z, u.y/u.z);
}

void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAImage &texture, Vec2f *t_pts, float intencity) {
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

        // compute texture color
        Vec2f tP = Vec2f(0., 0.);
        for (int i=0; i<3; i++) {
          tP.x += bc_screen[i] * t_pts[i].x;
          tP.y += bc_screen[i] * t_pts[i].y;
        }
        tP.x = tP.x*(texture.get_width()-1.)+.5;
        tP.y = tP.y*(texture.get_height()-1.)+.5;

        TGAColor color = texture.get(tP.x, tP.y);

        for (int i=0; i<3; i++) {
          color.raw[i] = color.raw[i] * intencity;
        }

        image.set(P.x, P.y, color);
      }
    }
  }
}

Matrix vec2mat(Vec3f v) {
  Matrix p = Matrix(4,1);
  p[0][0] = v.x;
  p[1][0] = v.y;
  p[2][0] = v.z;
  p[3][0] = 1.f;
  return p;
}

Vec3f mat2vec(Matrix p) {
  return Vec3f(
    p[0][0]/p[3][0],
    p[1][0]/p[3][0],
    p[2][0]/p[3][0]
  );
}

int main(int argc, char **argv) {
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  TGAImage image(width, height, TGAImage::RGB);

  TGAImage texture;
  texture.read_tga_file("obj/african_head_diffuse.tga");
  texture.flip_vertically();

  Vec3f light_dir(0,0,-1);

  float *zbuffer = new float[width*height];
  for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

  Vec3f camera = Vec3f(0,0,2);
  Matrix projection = Matrix::identity(4);
  projection[3][2] = -1.f/camera.z;
  Matrix viewport = Matrix::identity(4);
  // transpose 1, multiply d/2
  viewport[0][3] = width/2.f;
  viewport[1][3] = height/2.f;
  viewport[0][0] = width/2.f;
  viewport[1][1] = height/2.f;

  for (int i=0; i<model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    Vec3f screen_coords[3];
    Vec3f world_coords[3];
    for (int j=0; j<3; j++) {
        Vec3f v = model->vert(face[j]);
        screen_coords[j] = mat2vec(viewport*projection*vec2mat(v));
        world_coords[j] = v;
    }

    std::vector<int> t_face = model->t_face(i);
    Vec2f texture_coords[3];
    for (int j=0; j<3; j++) {
      texture_coords[j] = model->t_vert(t_face[j]);
    }

    Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
    n.normalize();
    float intencity = n*light_dir; // dot product
    
    if(intencity>0) {
      triangle(screen_coords, zbuffer, image, texture, texture_coords, intencity);
    }
  }

  image.flip_vertically();
  image.write_tga_file("output.tga");

  {
    TGAImage image_z(width, height, TGAImage::RGB);
    for (int x=0; x<width; x++){
      for (int y=0; y<height; y++){
        float z = zbuffer[x+y*width];
        z = std::max(-1.f, std::min(1.f, z)); // clamp
        z = (z+1.)/2.; // normalize
        z = std::pow(z, 2.2f); // gamma collection
        image_z.set(x,y,TGAColor(z*255,z*255,z*255,255));
      }
    }
    image_z.flip_vertically();
    image_z.write_tga_file("zbuffer.tga");
  }

  delete model;
  delete[] zbuffer;
  return 0;
}

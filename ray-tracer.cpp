#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include "Vector3.hpp"
#include "Triangle.hpp"
#include "file_util.h"

using namespace std;


const int w = 640;

const int h = 640;

unsigned char pixels[w * h * 3];

vector<Triangle> triangles;

void load_obj(const string &path, vector<Triangle> &out)
{
  vector<Vector3> vertices;

  for (string line : readLines(path))
  {
    line.erase(remove(line.begin(), line.end(), '\r'), line.end());
    replace(line.begin(), line.end(), '\t', ' ');

    vector<string> tokens;
    for (const string &token : split(line, ' '))
      if (!token.empty())
        tokens.push_back(token);

    if (tokens.empty() || tokens[0][0] == '#')
      continue;

    if (tokens[0] == "v" && tokens.size() >= 4)
    {
      vertices.push_back(Vector3{stof(tokens[1]), stof(tokens[2]), stof(tokens[3])});
    }
    else if (tokens[0] == "f" && tokens.size() >= 4)
    {
      vector<int> indices;
      for (size_t i = 1; i < tokens.size(); i++)
      {
        int index = stoi(split(tokens[i], '/')[0]);
        index = index > 0 ? index - 1 : (int)vertices.size() + index;
        indices.push_back(index);
      }

      for (size_t i = 1; i + 1 < indices.size(); i++)
        out.push_back(Triangle{vertices[indices[0]], vertices[indices[i]], vertices[indices[i + 1]]});
    }
  }
}

Vector3 background_color = Vector3{75, 0, 130};


float trace_ray_distance(Vector3 origin, Vector3 direction)
{
  float closest_t = INFINITY;

  for (const Triangle &triangle : triangles)
  {
    Vector3 edge_one = triangle.one - triangle.two;
    Vector3 edge_two = triangle.three - triangle.two;
    Vector3 p = edge_two.cross(edge_one);
    Vector3 abc = p.normalize();
    float d = -abc.dot(triangle.one);

    // if the ray is parallel to the triangle it will never hit it
    if (abc.dot(direction) == 0)
      continue;

    float t = -(abc.dot(origin) + d) / abc.dot(direction);

    // skip it if t is not a number
    if (isnan(t))
      continue;

    // skip triangles that are behind the camera
    if (t <= 0)
      continue;

    if (t > closest_t)
      continue;

    Vector3 collision_point = direction * t + origin;

    Vector3 minus_one = collision_point - triangle.one;
    Vector3 minus_two = collision_point - triangle.two;
    Vector3 minus_three = collision_point - triangle.three;

    float triangle_area = p.length() / 2;

    float area_one = minus_two.cross(minus_three).length() / 2;
    float area_two = minus_three.cross(minus_one).length() / 2;
    float area_three = minus_one.cross(minus_two).length() / 2;

    float u = area_one / triangle_area;
    float v = area_two / triangle_area;
    float a = area_three / triangle_area;

    float sum = u + v + a;

    float diff = abs(1 - sum);

    if (diff > .00001 || isnan(sum))
    {
      // I'm not in the triangle
    }
    else
    {
      // I'm in the triangle
      closest_t = t;

      
     
    }
  }
  return closest_t;
}


Vector3 trace_ray(Vector3 origin, Vector3 direction, int bounces = 10)
{
  // make the background a sky
  float sky = (direction.y + 1) / 2;
  float sky_r = 255 * (1 - sky) + 100 * sky;
  float sky_g = 255 * (1 - sky) + 150 * sky;
  float sky_b = 255 * (1 - sky) + 255 * sky;
  Vector3 sky_color = Vector3{sky_r, sky_g, sky_b};

  Vector3 color = sky_color;
  if(bounces <= 0)
    return sky_color;

  float closest_t = 1000000;

  for (const Triangle &triangle : triangles)
  {

    // Locate plane
    Vector3 edge_one = triangle.one - triangle.two;
    Vector3 edge_two = triangle.three - triangle.two;
    Vector3 p = edge_two.cross(edge_one);
    Vector3 abc = p.normalize();
    float d = -abc.dot(triangle.one);

    // if the ray is parallel to the triangle it will never hit it
    if (abc.dot(direction) == 0)
      continue;

    float t = -(abc.dot(origin) + d) / abc.dot(direction);

    // skip it if t is not a number
    if (isnan(t))
      continue;

    // skip triangles that are behind the camera
    if (t <= 0)
      continue;

    if (t > closest_t)
      continue;


    Vector3 collision_point = direction * t + origin;

    Vector3 minus_one = collision_point - triangle.one;
    Vector3 minus_two = collision_point - triangle.two;
    Vector3 minus_three = collision_point - triangle.three;

    float triangle_area = p.length() / 2;

    float area_one = minus_two.cross(minus_three).length() / 2;
    float area_two = minus_three.cross(minus_one).length() / 2;
    float area_three = minus_one.cross(minus_two).length() / 2;

    float u = area_one / triangle_area;
    float v = area_two / triangle_area;
    float a = area_three / triangle_area;

    float sum = u + v + a;

    float diff = abs(1 - sum);

    if (diff > .00001 || isnan(sum))
    {
      // Not in triangle
    }
    else
    {
      // In the triangle
      closest_t = t;

      Vector3 base_color = Vector3{180, 90, 255};
      Vector3 light_direction = Vector3{.5, 1, .5}.normalize();

      Vector3 ambient = base_color * .1;
      Vector3 normal = p.normalize();

      float strength = max(0.f, light_direction.dot(normal));

      Vector3 diffuse = base_color * strength;
      if(trace_ray_distance(collision_point, light_direction) != INFINITY){
        diffuse = Vector3{0,0,0};
      }


      Vector3 reflective = Vector3{0, 0, 0};

      if(true){
        Vector3 reflective_direction = direction - normal * 2 * normal.dot(direction);
        reflective = trace_ray(collision_point, reflective_direction, bounces - 1) * .5;
        diffuse *=.5;
        ambient *= .5;

      }

      Vector3 _color = ambient + diffuse + reflective;

      color = _color;
     
    }
  }
  return color;
}

int main()
{
  load_obj("box.obj", triangles);
  load_obj("simple.obj", triangles);
  cout << "Loaded " << triangles.size() << " triangles" << endl;

  Vector3 camera_origin = Vector3{0, 0, 1};
  Vector3 camera_look_at = Vector3{0, 0, 0};
  Vector3 camera_up = Vector3{0, 1, 0};

  // loop over every line going down 
  for (int y = 0; y < h; y++)
  {
    // While going down, loop over every pixel going right
    for (int x = 0; x < w; x++)
    {
      int index = y * w * 3 + x * 3;
      unsigned char r = background_color.x;
      unsigned char g = background_color.y;
      unsigned char b = background_color.z;


      Vector3 target {
          x / (float)w * 2 - 1, -(y / (float)h * 2 - 1), 0};

      Vector3 temp = (target - camera_origin);
      Vector3 ray = temp.normalize();

      Vector3 color = trace_ray(camera_origin, ray);

      // prevents colors from being larger than 255
      if (color.x > 255)
      {
        color.x = 255;
      }
      if (color.y > 255)
      {
        color.y = 255;
      }
      if (color.z > 255)
      {
        color.z = 255;
      }

      // prevents from being smaller than 0
      if (color.x < 0)
      {
        color.x = 0;
      }
      if (color.y < 0)
      {
        color.y = 0;
      }
      if (color.z < 0)
      {
        color.z = 0;
      }

      r = color.x;
      g = color.y;
      b = color.z;

      // Assign the r, g, and b values to the pixel at index
      pixels[index] = r;
      pixels[index + 1] = g;
      pixels[index + 2] = b;
    }
  }

  // create final image
  stbi_write_png("image.png", w, h, 3, pixels, w * 3);
  return 0;
}

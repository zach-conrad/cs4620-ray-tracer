# Notes for weekly sprints:

- 11/10/26:
  - I added checks to the triangle intersection code so the ray tracer skips triangles it can’t actually hit. If the ray is parallel to a triangle’s plane, the dot product of the normal and the ray direction is 0, so I skip that triangle before dividing by zero. After calculating t, I skip the triangle if t is NaN, and also if t is 0 or negative, meaning the triangle is behind the camera. I added these checks to both trace_ray and trace_ray_distance (the shadow-ray function).

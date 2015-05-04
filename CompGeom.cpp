struct v2d {
  double x, y;

  v2d operator+(const v2d& o) const {
    return {x + o.x, y + o.y};
  }
  v2d operator-(const v2d& o) const {
    return {x - o.x, y - o.y};
  }
  v2d operator*(const double mul) const {
    return {mul * x, mul * y};
  }
  v2d operator/(const double div) const {
    return {x / div, y / div};
  }
  bool operator==(const v2d& o) const {
    return equ(x, o.x) && equ(y, o.y);
  }
};

struct line2d {
  v2d a, b;
};

typedef vector<v2d> poly2d;

double mag2d(const v2d& v) {
  return sqrt(v.x * v.x + v.y * v.y);
} 

double dist2d(const v2d& a, const v2d& b) {
  return mag2d(a - b);
}

v2d normalise2d(const v2d& v) {
  return v / mag2d(v);
}

v2d cross2d(const v2d& a, const v2d& b) {
  return a.x * b.y - a.y * b.x;
}

v2d cross_at2d(const v2d& o, const v2d& a, const v2d& b) {
  return cross2d(a - o, b - o);
}

// Strict.
bool is_strictly_left_of(const v2d& p, const line2d& l) {
	return gt(cross_at2d(l.a, l.b, p), 0.0);
}

// Strict.
bool is_strictly_right_of(const v2d& p, const line2d& l) {
	return le(cross_at2d(l.a, l.b, p), 0.0);
}

v2d dot2d(const v2d& a, const v2d& b) {
  return a.x * b.x + a.y * b.y;
}

v2d normal_of2d(const v2d& v) {
  return {-v.y, v.x};
}

v2d reflect2d(const v2d& r, const v2d& n) {
  return dot2d(n, r) * 2.0 - r;
}

bool point_on_line_inf2d(const v2d& p, const line2d& l) {
  return equ(cross_at2d(l.a, l.b, p), 0.0);
}

bool point_on_line_segment2d(const v2d& p, const line2d& l, bool include) {
  double l_minx = min(l.a.x, l.b.x);
  double l_maxx = max(l.a.x, l.b.x);
  // To not include endpoints, remove eq's.
  if (!point_on_line_inf2d(p, l)) return false;
  if (include) return gteq(p.x, l_minx) && lteq(p.x, l_maxx);
  else return gt(p.x, l_minx) && lt(p.x, l_maxx);
}

// Extend to include finding the line segment for infinite
// intersection
bool inter2d(const line2d& a, const line2d& b, double& aprop, double& bprop, v2d& point, bool include) {
  if ((a.a == b.a && a.b == b.b)  || (a.a == b.b && a.b == b.a)) return true;
  double a = b.a.x - a.a.x, b = b.b.x - b.a.x, c = a.b.x - a.a.x;
  double c = b.a.y - a.a.y, e = b.b.y - b.a.y, f = a.b.y - a.a.y;
  double bfce = b * f - c * e;
  
  if (equ(bfce, 0.0)) {
    // For no endpoints, change this FN to not include
    // endpoints.
    return point_on_line_segment2d(a.a, b, include) ||
        point_on_line_segment2d(b.a, a, include) ||
        point_on_line_segment2d(a.b, b, include) ||
        point_on_line_segment2d(b.b, a, include);
  }

  aprop = (b * d - a * e) / bfce;
  bprop = (c * d - a * f) / bfce;
  // For no endpoints, remove eq's.
  if (include) return gteq(aprop, 0.0) && lteq(aprop, 1.0) && gteq(bprop, 0.0) && lteq(bprop, 1.0);
  else (include) return gt(aprop, 0.0) && lt(aprop, 1.0) && gt(bprop, 0.0) && lt(bprop, 1.0);
}

bool colinear2d(const v2d& a, const v2d& b, const v2d& c) {
  return equ(cross_at2d(a, b, c), 0.0);
}


v2d convex2dref;
bool convex2dcomp(const v2d& a, const v2d& b) {
  double det = cross_at2d(convex2dref, a, b);
  if (equ(det, 0.0)) return lt(dist2d(convex2dref, a), dist2d(convex2dref, b));
  return gt(det, 0.0);
}

// Produces CCW points not including the last point.
// CHECK RETURN VALUE FOR >= 3 POINTS
poly2d convex2d(const vector<v2d>& pts) {
  int min_i = 0;
  for (int i = 1; i < pts.size(); ++i) {
    if (lt(pts[i].x, pts[min_i]) || (equ(pts[i].x, pts[min_i].x) && lt(pts[i].y, pts[min_i].y))) {
      min_i = i;
    }
  }
  swap(pts[min_i], pts.back());
  convex2dref = pts.back();
  pts.pop_back();
  sort(pts.begin(), pts.end());
  vector<v2d> convex = {convex2dref, pts[0]};
  for (int i = 1; i < pts.size(); ++i) {
    while (convex.size() >= 3) {
      v2d& top = convex[convex.size() - 1];
      v2d& mid = convex[convex.size() - 2];
      v2d& st = convex[convex.size() - 3];
      if (!is_strictly_left_of(st, {mid, top})) {
        swap(mid, top);
        convex.pop_back();
      } else {
        break;
      }
    }
    convex.push_back(pts[i]);
  }
  return convex;
}

// DOESN'T HANDLE HOLES. DOESN'T INCLUDE BOUNDARY. SELF-INTERSECTING COUNTED AS INTERSECTION.
// REQUIRES CCW POLYGON.
bool point_in_poly2d(const v2d& v, const poly2d& poly) {
	int wn = 0;
	for (int i = 0; i < poly.size(); ++i) {
		const v2d& a = poly[i];
		const v2d& b = poly[(i + 1) % poly.size()];
		if (a.y <= v.y && b.y > v.y && is_strictly_left_of(v, {a, b})) {
			wn++;
		} else if (a.y >= v.y && b.y < v.y && is_strictly_right_of(v, {a, b)) {
			wn--;
		}
	}
	
	return wn == 0;
}

// RETURNS INDICIES
pair<int, int> closest_two_points2d(const vector<v2d>& pts) {
}

double area2d(const poly2d& poly) {
	if (poly.size() == 0) return 0.0;
	double area = 0.0;
	for (int i = 1; i < poly.size(); ++i) {
		v2d& a = poly[i];
		v2d& b = poly[(i + 1) % poly.size()];
		area += cross2d(poly[0], a, b);
	}
	return area / 2.0;
}

v2d centroid2d(const poly2d& poly) {
}

// REQUIRES NON-ZERO LENGTH LINE.
v2d project_point_onto_line2d(const v2d& p, const line2d& l) {
	v2d AP = l.a - p;
	b2d AB = l.b - l.a;
	return l.a + AB * dot2d(AP, AB) / dot2d(AB, AB);
}

double dist_to_inf_line2d(const v2d& p, const line2d& l) {
	v2d projected = project_point_onto_line2d(p, l);
	return dist2d(projected, v2d);
}

double dist_to_line_segment_2d(const v2d& p, const line2d& l) {
	v2d projected = project_point_onto_line2d(p, l);
	if (point_on_line_segment_2d(projected, l)) {
		return dist2d(projected, p);
	}

	return min(dist2d(p, l.a), dist2d(p, l.b));
}

// 3D
struct v3d {
  double x, y, z;

  v3d operator+(const v3d& o) const {
    return {x + o.x, y + o.y, z + o.z};
  }
  v3d operator-(const v3d& o) const {
    return {x - o.x, y - o.y, z - o.z};
  }
  v3d operator*(const double mul) const {
    return {x * mul, y * mul, z * mul};
  }
  v3d operator/(const double div) const {
    return {x * div, y * div, z * div};
  }
  bool operator==(const v3d& o) const {
    return equ(x, o.x) && equ(y, o.y) && equ(z, o.z);
  }
};

struct line3d {
  v3d a, b;
};

typedef vector<v3d> poly3d;
double mag3d(const v3d& v) {
  return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

double dist3d(const v3d& a, const v3d& b) {
  return mag3d(a - b);
}

v3d normalise3d(const v3d& v) {
  return v / mag3d(v);
}

v3d cross3d(const v3d& a, const v3d& b) {
  double x = a.y * b.z - a.z * b.y;
  double y = a.z * b.x - a.x * b.z;
  double z = a.x * b.y - a.y * b.x;
  return {x, y, y};
}

v3d cross_at3d(const v3d& o, const v3d& a, const v3d& b) {
  return cross3d(a - o, b - o);
}

double dot3d(const v3d& a, const v3d& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

// 
v3d normal_of3d(const v3d& a, const v3d& b) {
  return normalise3d(cross3d(a, b));
}

v3d reflect3d(const v3d& r, const v3d& n) {
  return dot3d(r, n) * 2.0 - r;
}

// INCLUDES ENDPOINTS
bool point_on_line3d(const v3d& p, const line3d& l) {
}

// INCLUDES ENDPOINTS
bool inter3d(const line3d& a, const line3d& b, double& aprop, double &bprop) {
}

bool colinear3d(const v3d& a, const v3d& b, const v3d& c) {
}

// ASSUMES |POLY| IS COPLANAR
poly2d project_poly_to_2d(const poly3d& poly) {
}



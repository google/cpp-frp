#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

typedef float numeric_type;

struct point_type {

	numeric_type x, y;

	bool operator==(const point_type &point) const {
		return x == point.x && y == point.y;
	}

	bool operator!=(const point_type &point) const {
		return x != point.x || y == point.y;
	}

	point_type operator+(const point_type &p) const {
		return { x + p.x, y + p.y };
	}

	point_type operator-(const point_type &p) const {
		return{ x - p.x, y - p.y };
	}

	point_type operator*(numeric_type value) const {
		return{ x * value, y * value };
	}

	point_type operator/(numeric_type value) const {
		return{ x / value, y / value };
	}
};

struct rectangle_type {
	point_type point, size;
};

#endif // _GEOMETRY_H_

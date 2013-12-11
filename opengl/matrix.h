#ifndef MATRIX_H
#define MATRIX_H

struct matrix_t {
	float m[4][4];

	matrix_t() {
		m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
		m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
		m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
		m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
	}

	matrix_t(
		float _00, float _01, float _02, float _03,
		float _10, float _11, float _12, float _13,
		float _20, float _21, float _22, float _23,
		float _30, float _31, float _32, float _33
	) {
		m[0][0] = _00; m[0][1] = _01; m[0][2] = _02; m[0][3] = _03;
		m[1][0] = _10; m[1][1] = _11; m[1][2] = _12; m[1][3] = _13;
		m[2][0] = _20; m[2][1] = _21; m[2][2] = _22; m[2][3] = _23;
		m[3][0] = _30; m[3][1] = _31; m[3][2] = _32; m[3][3] = _33;
	}

	void transform(float ix, float iy, float iz, float *ox, float *oy, float *oz) const {
		if (ox) *ox = ix * m[0][0] + iy * m[1][0] + iz * m[2][0] + m[3][0];
		if (oy) *oy = ix * m[0][1] + iy * m[1][1] + iz * m[2][1] + m[3][1];
		if (oz) *oz = ix * m[0][2] + iy * m[1][2] + iz * m[2][2] + m[3][2];
	}

	static inline matrix_t scale(float x, float y, float z) {
		return matrix_t(
			x, 0.0f, 0.0f, 0.0f,
			0.0f, y, 0.0f, 0.0f,
			0.0f, 0.0f, z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
	}

	static inline matrix_t translate(float x, float y, float z) {
		return matrix_t(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			x,    y,    z,    1.0f
		);
	}
};

inline matrix_t operator *(const matrix_t &a, const matrix_t &b) {
	matrix_t r;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			float f = 0.0f;
			for (int k = 0; k < 4; ++k) {
				f += a.m[i][k] * b.m[k][j];
			}
			r.m[i][j] = f;
		}
	}

	return r;
}

#endif // MATRIX_H

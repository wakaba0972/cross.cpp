#include<iostream>
#include<vector>
#include<cmath>
#include<string>
#include<thread>
#include<windows.h>

using namespace std;

#define WIDTH 70
#define HEIGHT 32
#define DENSITY 30
#define X_OFFSET 20
#define Y_OFFSET 13
#define X_DTHETA 1
#define Y_DTHETA 3
#define MAX_X_THETA 628

const string LUMINANCE = ".,-~:;=!*#$@"; //亮度表

char display[HEIGHT][WIDTH];
short zbuffer[HEIGHT][WIDTH];
float sin_table[MAX_X_THETA];
float cos_table[MAX_X_THETA];

void build_table() {
    for (short i = 0; i < MAX_X_THETA; ++i) {
        sin_table[i] = sin(0.01 * i);
        cos_table[i] = cos(0.01 * i);
    }
}

struct pt3d {
    float y, x, z;
    pt3d(float y = 0, float x = 0, float z = 0) : x(x), y(y), z(z) {}
    pt3d operator-(pt3d p) { return pt3d(y - p.y, x - p.x, z - p.z); }
    pt3d operator+(pt3d p) { return pt3d(y + p.y, x + p.x, z + p.z); }
    pt3d operator*(float k) { return pt3d(y * k, x * k, z * k); }
    pt3d operator/(float k) { return pt3d(y / k, x / k, z / k); }
    pt3d operator/=(float k) { y /= k; x /= k; z /= k; return *this; }
    pt3d operator+=(pt3d p) { y += p.y; x += p.x; z += p.z; return *this; }
    pt3d cross(pt3d p) { return pt3d(y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x); }
    float dot(pt3d p) { return y * p.y + x * p.x + z * p.z; }
    float length() { return sqrt(y * y + x * x + z * z); }
};

class Face {
private:
    pt3d o_vertex[3], vertex[3];

public:
    Face(pt3d a, pt3d b, pt3d c) {
        o_vertex[0] = a;
        o_vertex[1] = b;
        o_vertex[2] = c;
    }

    void draw(pt3d& center, int id) {
        pt3d du = (vertex[1] - vertex[0]) / DENSITY;
        pt3d dv = (vertex[2] - vertex[0]) / DENSITY;

        //計算亮度
        pt3d lu(0, 0, -1);
        pt3d nn = du.cross(dv);
        nn /= nn.length();
        short L = (short)(nn.dot(lu) * 12);
        if (L <= 0) return;

        //遍歷面上每個點
        for (short i = 0; i < DENSITY; i++) {
            pt3d p = center + vertex[0] + du * i;
            for (short j = 0; j < DENSITY; j++) {
                pt3d q = (p + dv * j);
                short qy = q.y + Y_OFFSET;
                short qx = q.x + X_OFFSET;

                if (qy < 0 || qy >= HEIGHT || qx < 0 || qx >= WIDTH) continue;

                //z-buffer
                if (zbuffer[qy][qx] != 0 && q.z * 100 < zbuffer[qy][qx]) continue;
                zbuffer[qy][qx] = q.z * 100;

                display[qy][qx] = LUMINANCE[L];
            }
        }
    }

    void rotateInit() {
        for (short i = 0; i < 3; ++i) {
            vertex[i].x = o_vertex[i].x;
            vertex[i].y = o_vertex[i].y;
            vertex[i].z = o_vertex[i].z;
        }
    }

    void rotateX(int theta) {
        for (short i = 0; i < 3; ++i) {
            float ny = vertex[i].y * cos_table[theta] - vertex[i].z * sin_table[theta];
            float nz = vertex[i].y * sin_table[theta] + vertex[i].z * cos_table[theta];
            vertex[i].y = ny;
            vertex[i].z = nz;
        }
    }

    void rotateY(int theta) {
        for (short i = 0; i < 3; ++i) {
            float nx = vertex[i].x * cos_table[theta] + vertex[i].z * sin_table[theta];
            float nz = vertex[i].z * cos_table[theta] - vertex[i].x * sin_table[theta];
            vertex[i].x = nx;
            vertex[i].z = nz;
        }
    }
};

class Cross {
private:
    short thetaX = 0;
    short thetaY = 0;

    pt3d center = { 3, 10, 5 };

    vector<Face> faces = {
        // 主柱
        Face({-15, -5, 2}, {-15, 5, 2}, {-15, -5, -2}),
        Face({-15, -5, -2}, {-15, 5, -2}, {15, -5, -2}),
        Face({-15, 5, -2}, {-15, 5, 2}, {15, 5, -2}),
        Face({-15, 5, 2}, {-15, -5, 2}, {15, 5, 2}),
        Face({-15, -5, 2}, {-15, -5, -2}, {15, -5, 2}),
        Face({15, -5, -2}, {15, 5, -2}, {15, -5, 2}),

        // 橫梁
        Face({4, -15, 2}, {4, 15, 2}, {4, -15, -2}),
        Face({4, -15, -2}, {4, 15, -2}, {8, -15, -2}),
        Face({4, 15, -2}, {4, 15, 2}, {8, 15, -2}),
        Face({4, 15, 2}, {4, -15, 2}, {8, 15, 2}),
        Face({4, -15, 2}, {4, -15, -2}, {8, -15, 2}),
        Face({8, -15, -2}, {8, 15, -2}, {8, -15, 2}),
    };

    short length = faces.size();

public:
    void draw() {
        rotate();

        //計算每個面
        for (short i = 0; i < length; ++i) faces[i].draw(center, i);

        //渲染
        string s;
        for (short i = 0; i < HEIGHT; i++) {
            for (short j = 0; j < WIDTH; j++) s += display[i][j];
            s += '\n';
        }
        cout << s;
    }

    void rotate() {
        for (Face& f : faces) {
            f.rotateInit();
            f.rotateX(thetaX);
            f.rotateY(thetaY);
        }
        thetaX += X_DTHETA;
        thetaY += Y_DTHETA;

        if (thetaX > MAX_X_THETA) thetaX = 0;
        if (thetaY > MAX_X_THETA) thetaY = 0;
    }
};

int main() {
    cin.tie(0);
    cout.tie(0);

    build_table();
    Cross c;

    while (1) {
        memset(display, ' ', sizeof(display));
        memset(zbuffer, 0, sizeof(zbuffer));

        c.draw();
        Sleep(1);
        system("cls");
    }
}
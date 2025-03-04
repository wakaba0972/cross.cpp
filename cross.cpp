#include<iostream>
#include<vector>
#include<cmath>
#include<string>
#include<windows.h>

using namespace std;

#define WIDTH 80
#define HEIGHT 30
#define DENSITY 30
#define X_OFFSET 20
#define Y_OFFSET 10
#define X_DTHETA 1
#define Y_DTHETA 3
#define MAX_X_THETA 628

const string LUMINANCE = ".,-~:;=!*#$@"; //亮度表

char display[HEIGHT][WIDTH];
short zbuffer[HEIGHT][WIDTH];
float sin_table[MAX_X_THETA];
float cos_table[MAX_X_THETA];

void build_table(){
    for(int i=0; i<MAX_X_THETA; ++i){
        sin_table[i] = sin(0.01 * i);
        cos_table[i] = cos(0.01 * i);
    }
}

struct pt3d{
    float y, x, z;
    pt3d(float y = 0, float x = 0, float z = 0): x(x), y(y), z(z){}
    pt3d operator-(pt3d p) {return pt3d(y - p.y, x - p.x, z - p.z);}
    pt3d operator+(pt3d p) {return pt3d(y + p.y, x + p.x, z + p.z);}
    pt3d operator*(float k) {return pt3d(y * k, x * k, z * k);}
    pt3d operator/(float k) {return pt3d(y / k, x / k, z / k);}
    pt3d operator/=(float k) {y /= k; x /= k; z /= k; return *this;}
    pt3d operator+=(pt3d p) {y += p.y; x += p.x; z += p.z; return *this;}
    pt3d cross(pt3d p) {return pt3d(y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x);}
    float dot(pt3d p) {return y * p.y + x * p.x + z * p.z;}
    float length() {return sqrt(y * y + x * x + z * z);}
};

class face{
    private:
        pt3d o_vertex[3], vertex[3];

    public:
        face(pt3d a, pt3d b, pt3d c) {
            o_vertex[0] = a;
            o_vertex[1] = b;
            o_vertex[2] = c;
        }
        
        void draw(pt3d &center, int id){
            pt3d u = vertex[1] - vertex[0];
            pt3d v = vertex[2] - vertex[0];

            u /= DENSITY;
            v /= DENSITY;

            //計算亮度
            pt3d lu(0, 0, -1);
            pt3d cc = u.cross(v);
            cc /= cc.length();
            short L = (int)(cc.dot(lu)*12);
            if(L <= 0) return;

            //用向量計算面上每個點
            for(int i = 0; i < DENSITY; i++){
                pt3d p = center + vertex[0] + u * i;
                for(int j = 0; j < DENSITY; j++){
                    pt3d q = (p + v * j);
                    int qy = (int)q.y + Y_OFFSET;
                    int qx = (int)q.x + X_OFFSET;

                    //zbuffer
                    //if(q.z < zbuffer[qy][qx]) continue;
                    //zbuffer[qy][qx] = q.z;

                    if(qy >= 0 && qy < HEIGHT && qx >= 0 && qx < WIDTH) {
                        display[qy][qx] = LUMINANCE[L];
                        //display[qy][qx] = id + '0';
                    }
                }
            }
        }

        void rotateInit(){
            for(int i=0; i<3; ++i){
                vertex[i].x = o_vertex[i].x;
                vertex[i].y = o_vertex[i].y;
                vertex[i].z = o_vertex[i].z;
            }
        }

        void rotateX(int theta){
            for(int i=0; i<3; ++i){
                float ny = vertex[i].y * cos_table[theta] - vertex[i].z * sin_table[theta];
                float nz = vertex[i].y * sin_table[theta] + vertex[i].z * cos_table[theta];
                vertex[i].y = ny;
                vertex[i].z = nz;
            }
        }

        void rotateY(int theta){
            for(int i=0; i<3; ++i){
                float nx = vertex[i].x * cos_table[theta] + vertex[i].z * sin_table[theta];
                float nz = vertex[i].z * cos_table[theta] - vertex[i].x * sin_table[theta];
                vertex[i].x = nx;
                vertex[i].z = nz;
            }
        }
};

class Cross{
    private:
        int thetaX = 0;
        int thetaY = 0;

        pt3d center = {3, 10, 5};

        vector<face> faces = {
            // 主柱
            face({-15, -5, 2}, {-15, 5, 2}, {-15, -5, -2}),
            face({-15, -5, -2}, {-15, 5, -2}, {15, -5, -2}),
            face({-15, 5, -2}, {-15, 5, 2}, {15, 5, -2}),
            face({-15, 5, 2}, {-15, -5, 2}, {15, 5, 2}),
            face({-15, -5, 2}, {-15, -5, -2}, {15, -5, 2}),
            face({15, -5, -2}, {15, 5, -2}, {15, -5, 2}),

            // 橫梁
            face({4, -15, 2}, {4, 15, 2}, {4, -15, -2}),
            face({4, -15, -2}, {4, 15, -2}, {8, -15, -2}),
            face({4, 15, -2}, {4, 15, 2}, {8, 15, -2}),
            face({4, 15, 2}, {4, -15, 2}, {8, 15, 2}),
            face({4, -15, 2}, {4, -15, -2}, {8, -15, 2}),
            face({8, -15, -2}, {8, 15, -2}, {8, -15, 2}),
        };

        int length = faces.size();

    public:
        void draw(){
            rotate();

            //計算每個面
            for(int i=0; i<length; ++i) faces[i].draw(center, i);

            //渲染
            string s;
            for(int i = 0; i < HEIGHT; i++){
                for(int j = 0; j < WIDTH; j++) s += display[i][j];
                s += '\n';
            }
            cout << '\n' << s;
        }

        void rotate(){
            for(face &f: faces) {
                f.rotateInit();
                f.rotateX(thetaX);
                f.rotateY(thetaY);
            }
            thetaX += X_DTHETA;
            thetaY += Y_DTHETA;

            if(thetaX >= MAX_X_THETA) thetaX = 0;
            if(thetaY >= MAX_X_THETA) thetaY = 0;
        }
};

int main(){
    cin.tie(0);
    cout.tie(0);

    build_table();
    Cross c;
    
    while(1){
        memset(display, ' ', sizeof(display));
        memset(zbuffer, 0, sizeof(zbuffer));

        c.draw();
        Sleep(1);
        system("cls");
    }

    return 0;
}
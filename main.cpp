#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstdio>
#include <fstream>
#include <cmath>
#include <random>
#include <vector>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

char* filetobuf(const char* file);
void make_vertexShaders();
void make_fragmentShaders();
GLvoid make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid InitBuffer();
bool ReadObj(const char* path);
bool Set_VAO();
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Timer_event(int value);
GLvoid draw_all(int k);
using namespace std;
//--- 필요한 변수 선언
GLfloat rColor = 0.0, gColor = 0.0, bColor = 0.60;
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체
GLuint vbo[2];
GLuint square_vbo[2];
GLuint square_vao;

GLuint normalbuffer;
GLuint line_vao, sp_vao;
GLuint spvbo[2];
std::random_device rd;
std::mt19937 dre(rd());
std::uniform_int_distribution<int> uid{ 1, 5 };
std::uniform_real_distribution<float> speed_urd{ 0.1, 1.0 };

std::vector< glm::vec3 > vertices;
std::vector< glm::vec2 > uvs;
std::vector< glm::vec3 > normals; // 지금은 안쓸거에요.
std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
std::vector< glm::vec3 > temp_vertices;
std::vector< glm::vec2 > temp_uvs;
std::vector< glm::vec3 > temp_normals;
//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
    //--- 윈도우 생성하기
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(700, 700);
    glutCreateWindow("Example1");

    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    glutTimerFunc(100, Timer_event, 1);
    make_shaderProgram();
    //InitBuffer();
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(Keyboard);
    Set_VAO();
    glutReshapeFunc(Reshape);
    glutMainLoop();
}

float camera_y_rad = -60;
bool turn_off = false;
int light_color = 0;
float light_x = 30;
float light_y = 35;
float light_z = 10;
float light_rad = 0;
int animation_num = 0;
int summon_num;
bool y_button = false;
bool Y_button = false;
const int HEIGHT = 15;

class CUBE {
    glm::mat4 TR{ glm::mat4(1.0f) };
    glm::vec3 pos{ 0,0,0 };
    float size{ 1 };
    float speed{ 0 };
    float rad{ 0 };
    float y_rad{ 0 };
    float add_speed{ 0 };
    int wave_i;
    int CUBE_id;
    bool reverse{ false };
    bool flip{ false };
    bool summon{ false };

public:

    CUBE() {}
    float rad_amount{ 0 };

    void draw(unsigned int model, unsigned int color) {
        get_TR();
        glBindVertexArray(square_vao);
        glUniformMatrix4fv(model, 1, GL_FALSE, glm::value_ptr(TR)); //--- modelTransform 변수에 변환 값 적용하기
        glUniform3f(color, 0.0, 0.6, 0);
        glDrawArrays(GL_TRIANGLES, 0, 24);
        glUniform3f(color, 1.0, 0.5, 0);
        glDrawArrays(GL_TRIANGLES, 24, 12);
        glUniform3f(color, 0.0, 0.0, 0.0);
        ex_box_draw();

    }

    void ex_box_draw() {
        glDrawArrays(GL_LINES, 24, 2);
        glDrawArrays(GL_LINES, 25, 2);
        glDrawArrays(GL_LINES, 27, 2);
        glDrawArrays(GL_LINES, 28, 2);
        glDrawArrays(GL_LINES, 6, 2);
        glDrawArrays(GL_LINES, 10, 2);
        glDrawArrays(GL_LINES, 12, 2);
        glDrawArrays(GL_LINES, 0, 2);
    }

    void check_xzpos(int x, int z, int i, int j) {
        speed = speed_urd(dre);
        if (x % 2 == 0) {
            pos.x = (-x / 2 + i) * 2 + 1;
        }
        else {
            pos.x = (-x / 2 + i) * 2;
        }
        wave_i = i;
        if (z % 2 == 0) {
            pos.z = (-z / 2 + j) * 2 + 1;
        }
        else {
            pos.z = (-z / 2 + j) * 2;
        }
        pos.y = 8;
    }

    void init_state(int num) {
        flip = false;
        reverse = false;
        add_speed = 0;
        y_rad = 0;
        size = 0;
        speed = speed_urd(dre);
        pos.y = 0;
        if (num == 2) {
            size = wave_i / 4.0;
            pos.y += wave_i / 4.0;
            speed = 0.2;
        }
        else if (num == 3) {
            size = (CUBE_id / static_cast<float>(summon_num)) * HEIGHT;
            pos.y = (CUBE_id / static_cast<float>(summon_num)) * HEIGHT;
            speed = 0.1;
        }
        else if (num == 4) {

        }
    }

    bool summon_cube() {
        if (!summon) {
            if (pos.y > 0) {
                pos.y -= speed;
                rad += 10;
            }
            else if (pos.y <= 0) {
                size = 1;
                pos.y = 0;
                rad = 0;
                summon = true;
            }
        }
        return summon;
    }

    void animation_one() {
        if (size < HEIGHT && !flip) {
            size += (speed + add_speed);
            pos.y += (speed + add_speed);
            if (size >= HEIGHT) {
                flip = true;
                size = HEIGHT;
                pos.y = HEIGHT;
            }
        }
        else if (size > 0 && flip) {
            size -= (speed + add_speed);
            pos.y -= (speed + add_speed);
            if (size <= 0) {
                speed = speed_urd(dre);
                flip = false;
                size = 0;
                pos.y = 0;
            }
        }
    }

    void animation_two() {
        if (size < HEIGHT && !flip) {
            size += speed + add_speed;
            pos.y += speed + add_speed;
            if (size >= HEIGHT) {
                flip = true;
                size = HEIGHT;
                pos.y = HEIGHT;
            }
        }
        else if (size > 0 && flip) {
            size -= speed + add_speed;
            pos.y -= speed + add_speed;
            if (size <= 0) {
                flip = false;
                size = 0;
                pos.y = 0;
            }
        }
    }

    void animation_three() {
        if (size >= HEIGHT) {
            flip = true;
            size = HEIGHT;
            pos.y = HEIGHT;
        }
        if (size <= 0) {
            flip = false;
            size = 0;
            pos.y = 0;
        }
        if (size < HEIGHT && !flip) {
            size += speed + add_speed;
            pos.y += speed + add_speed;

        }
        else if (size > 0 && flip) {
            size -= speed + add_speed;
            pos.y -= speed + add_speed;

        }
    }

    void animation_four() {
        animation_three();
        y_rad += rad_amount;
    }

    void animation_five() {
        if (size < HEIGHT && !flip && !reverse) {
            size += (speed + add_speed);
            pos.y += (speed + add_speed);
            if (size >= HEIGHT) {
                flip = true;
                size = HEIGHT;
                pos.y = HEIGHT;
            }
        }
        else if (size > 0 && flip && !reverse) {
            size -= (speed + add_speed);
            pos.y += (speed + add_speed);
            if (size <= 0) {
                flip = false;
                reverse = true;
                size = 0;
                pos.y = HEIGHT * 2;
            }
        }
        else if (size < HEIGHT && !flip) {
            size += (speed + add_speed);
            pos.y -= (speed + add_speed);
            if (size >= HEIGHT) {
                speed = speed_urd(dre);
                flip = true;
                size = HEIGHT;
            }
        }
        else if (size > 0 && flip) {
            size -= (speed + add_speed);
            pos.y -= (speed + add_speed);
            if (size <= 0) {
                speed = speed_urd(dre);
                flip = false;
                reverse = false;
                size = 0;
                pos.y = 0;
            }
        }
    }

    glm::mat4 get_TR() {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(pos.x, pos.y, pos.z));
        TR = glm::rotate(TR, glm::radians(rad), glm::vec3(1, 0, 0));
        TR = glm::rotate(TR, glm::radians(rad + y_rad), glm::vec3(0, 1, 0));
        TR = glm::rotate(TR, glm::radians(rad), glm::vec3(0, 0, 1));
        TR = glm::scale(TR, glm::vec3(1, size, 1));
        return TR;
    }

    void set_cube_id() {
        CUBE_id = summon_num;
    }

    void speed_up() {
        if (speed < 0.5)
            speed += 0.01;
    }
    void speed_down() {
        if (speed > 0.011)
            speed -= 0.01;
    }

};

std::vector<CUBE> cubes;
CUBE cube;

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
    //--- 변경된 배경색 설정
    glClearColor(rColor, gColor, bColor, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //--- 렌더링 파이프라인에 세이더 불러오기
    glUseProgram(shaderProgramID);

    glViewport(0, 0, 700, 700);
    draw_all(1);

    glViewport(550, 550, 150, 150);
    draw_all(2);

    glutSwapBuffers(); //--- 화면에 출력하기
}

GLvoid draw_all(int k) {
    unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
    glUniform3f(lightPosLocation, light_x * glm::cos(glm::radians(light_rad)), light_y, light_z * glm::sin(glm::radians(light_rad)));
    unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
    switch (light_color) {
    case 0:
        glUniform3f(lightColorLocation, 1.0, 1.0, 1.0);
        break;
    case 1:
        glUniform3f(lightColorLocation, 0.40, 0.40, 1.0);
        break;
    case 2:
        glUniform3f(lightColorLocation, 1.0, 0.40, 0.70);
        break;
    case 3:
        glUniform3f(lightColorLocation, 0.70, 0.00, 0.0);
        break;
    }

    unsigned int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
    glUniform3f(objColorLocation, 0.50, 0.50, 0.50);
    unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos"); //--- viewPos 값 전달: 카메라 위치
    glUniform3f(viewPosLocation, 0, 0, 0);
    unsigned int ambient = glGetUniformLocation(shaderProgramID, "ambientLight"); //--- viewPos 값 전달: 카메라 위치
    if (turn_off)
        glUniform1f(ambient, 0);
    else
        glUniform1f(ambient, 1);

    unsigned int colorLocation = glGetUniformLocation(shaderProgramID, "colorAttribute");
    unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "transform"); //--- 버텍스 세이더에서모델 변환 위치 가져오기
    unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projectionTransform"); //--- 투영 변환 값 설정

    if (y_button)
        camera_y_rad += 10;
    else if (Y_button)
        camera_y_rad -= 10;

    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 TR_light = glm::mat4(1.0f);
    if (k == 1) {
        projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 150.0f); //--- 투영 공간 설정: fovy, aspect, near, far
        projection = glm::translate(projection, glm::vec3(0.0, 0.0, -120.0));
        projection = glm::rotate(projection, glm::radians(0.f), glm::vec3(0.0, 1.0, 0.0));
        projection = glm::rotate(projection, glm::radians(45.f), glm::vec3(1.0, 0.0, 0.0));
        projection = glm::rotate(projection, glm::radians(camera_y_rad), glm::vec3(0.0, 1.0, 0.0));
    }
    else {
        projection = glm::ortho(-35.f, 30.f, -30.f, 30.f, 0.1f, 70.f);
        projection = glm::rotate(projection, glm::radians(-90.f), glm::vec3(1.0, 0.0, 0.0));
        projection = glm::rotate(projection, glm::radians(10.f), glm::vec3(0.0, 1.0, 0.0));
    }
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);

    glBindVertexArray(square_vao);
    TR_light = glm::translate(TR_light, glm::vec3(light_x * glm::cos(glm::radians(light_rad)), light_y, light_z * glm::sin(glm::radians(light_rad))));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR_light)); //--- modelTransform 변수에 변환 값 적용하기
    glUniform3f(objColorLocation, 1.0, 1.0, 1.0);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    for (CUBE& c : cubes) {
        c.draw(modelLocation, objColorLocation);
    }

    /*for (int i = 0; i < cubes.size(); ++i) {
       cubes[i].draw(modelLocation);
    }*/

}

GLvoid Timer_event(int value) {


    if (!cubes.size()) {
        summon_num = 0;
        std::cout << "행렬: ";
        int w, h;
        cin >> w >> h;
        if (w > 25 || h > 25) {
            std::cout << "값이 큽니다." << std::endl;
            glutTimerFunc(100, Timer_event, 4);
            return;
        }
        /*for (int i = 0; i < h; ++i) {
           if (i % 2 == 0) {
              for (int j = 0; j < w; ++j) {
                 cube.check_xzpos(w, h, i, j);
                 cube.set_cube_id();
                 summon_num += 1;
                 cubes.push_back(cube);
              }
           }
           else {
              summon_num += 10;
              for (int j = 0; j < w; ++j) {
                 cube.check_xzpos(w, h, i, j);
                 cube.set_cube_id();
                 summon_num -= 1;
                 cubes.push_back(cube);
              }
           }
        }*/
        for (int i = 0; i < h; ++i) {
            if (i == 0) {
                for (int j = 0; j < w; ++j) {
                    cube.check_xzpos(w, h, i, j);
                    cube.set_cube_id();
                    summon_num += 1;
                    cube.rad_amount = 10;
                    cubes.push_back(cube);
                }
            }
            else {
                if (i % 2 == 0) {
                    for (int j = 0; j < w; ++j) {
                        cube.check_xzpos(w, h, i, j);
                        cube.set_cube_id();
                        summon_num += 1;
                        cube.rad_amount = 10;
                        cubes.push_back(cube);
                    }
                }
                else {
                    summon_num += w;
                    for (int j = 0; j < w; ++j) {
                        cube.check_xzpos(w, h, i, j);
                        cube.set_cube_id();
                        summon_num -= 1;
                        cube.rad_amount = -10;
                        cubes.push_back(cube);
                    }
                    summon_num += w + 1;
                }
            }
        }
        //cube.trans_pos(0, 0);
        //cubes.push_back(cube);
    }
    int num = 0;
    if (cubes.size()) {

        for (CUBE& c : cubes) {
            if (c.summon_cube()) {
                ++num;
            }
        }
    }

    if (num == cubes.size()) {
        switch (animation_num) {
        case 1:
            for (CUBE& c : cubes) {
                c.animation_one();
            }
            break;
        case 2:
            for (CUBE& c : cubes) {
                c.animation_two();
            }
            break;
        case 3:
            for (CUBE& c : cubes) {
                c.animation_three();
            }
            break;
        case 4:
            for (CUBE& c : cubes) {
                c.animation_four();
            }
            break;
        case 5:
            for (CUBE& c : cubes) {
                c.animation_five();
            }
            break;

        }
    }
    glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
    glutTimerFunc(100, Timer_event, 4);
}


GLvoid Keyboard(unsigned char key, int x, int y) {
    int num = 0;
    switch (key) {
    case '1':
        num = 0;
        if (cubes.size()) {

            for (CUBE& c : cubes) {
                if (c.summon_cube()) {
                    ++num;
                }
            }
        }
        if (num == cubes.size()) {

            for (CUBE& c : cubes) {
                c.init_state(1);
            }
            animation_num = 1;
        }
        break;
    case '2':
        num = 0;
        if (cubes.size()) {

            for (CUBE& c : cubes) {
                if (c.summon_cube()) {
                    ++num;
                }
            }
        }
        if (num == cubes.size()) {

            for (CUBE& c : cubes) {
                c.init_state(2);
            }
            animation_num = 2;
        }
        break;
    case '3':
        num = 0;
        if (cubes.size()) {

            for (CUBE& c : cubes) {
                if (c.summon_cube()) {
                    ++num;
                }
            }
        }
        if (num == cubes.size()) {

            for (CUBE& c : cubes) {
                c.init_state(3);
            }
            animation_num = 3;
        }
        break;
    case '4':
        num = 0;
        if (cubes.size()) {

            for (CUBE& c : cubes) {
                if (c.summon_cube()) {
                    ++num;
                }
            }
        }
        if (num == cubes.size()) {

            for (CUBE& c : cubes) {
                c.init_state(3);
            }
            animation_num = 4;
        }
        break;
    case '5':
        num = 0;
        if (cubes.size()) {

            for (CUBE& c : cubes) {
                if (c.summon_cube()) {
                    ++num;
                }
            }
        }
        if (num == cubes.size()) {

            for (CUBE& c : cubes) {
                c.init_state(5);
            }
            animation_num = 5;
        }
        break;
    case 'r':
        animation_num = 0;
        cubes.clear();
        break;
    case 'y':
        Y_button = false;
        if (y_button)
            y_button = false;
        else
            y_button = true;
        break;
    case 'Y':
        if (Y_button)
            Y_button = false;
        else
            Y_button = true;
        y_button = false;
        break;
    case 't':
        if (turn_off)
            turn_off = false;
        else
            turn_off = true;
        break;
    case 'c':
        switch (light_color) {
        case 0:
            light_color++;
            break;
        case 1:
            light_color++;
            break;
        case 2:
            light_color++;
            break;
        case 3:
            light_color = 0;
            break;
        }
        break;
    case 'q':
        glutLeaveMainLoop();
        break;
    case 'l':
        light_rad += 10;
    case '+':
        if (cubes.size()) {
            for (CUBE& c : cubes) {
                c.speed_up();
            }
        }
        break;
    case '-':
        if (cubes.size()) {

            for (CUBE& c : cubes) {
                c.speed_down();
            }
        }
        break;
    }


}



GLfloat line[6][3]{ {-1, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1} };
GLfloat line_color[6][3]{};

bool Set_VAO() {
    glGenVertexArrays(1, &line_vao);
    glBindVertexArray(line_vao);

    glGenBuffers(2, vbo);
    //버퍼 오브젝트를 바인드 한다.
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    //버퍼 오브젝트의 데이터를 생성
    glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

    //위치 가져오기 함수
    GLint lineAttribute = glGetAttribLocation(shaderProgramID, "positionAttribute");
    //버퍼 오브젝트를 바인드 한다.
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    //버텍스 속성 데이터의 배열을 정의
    glVertexAttribPointer(lineAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //버텍스 속성 배열을 사용하도록 한다.
    glEnableVertexAttribArray(lineAttribute);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    //버퍼 오브젝트의 데이터를 생성
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_color), line, GL_STATIC_DRAW);

    //위치 가져오기 함수
    GLint line_Attribute = glGetAttribLocation(shaderProgramID, "colorAttribute");
    //버퍼 오브젝트를 바인드 한다.
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    //버텍스 속성 데이터의 배열을 정의
    glVertexAttribPointer(line_Attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //버텍스 속성 배열을 사용하도록 한다.
    glEnableVertexAttribArray(line_Attribute);



    // 육면체
    ReadObj("ncube.obj");
    glGenVertexArrays(1, &square_vao);
    glBindVertexArray(square_vao);
    glGenBuffers(2, square_vbo);
    //버퍼 오브젝트를 바인드 한다.
    glBindBuffer(GL_ARRAY_BUFFER, square_vbo[0]);
    //버퍼 오브젝트의 데이터를 생성
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    //glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, square_vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, square_vbo[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    return true;
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
}

GLint result;
GLchar errorLog[512];

void make_vertexShaders()
{
    GLchar* vertexSource;
    vertexSource = filetobuf("phongvertex.glsl");
    //--- 버텍스 세이더 객체 만들기
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    //--- 세이더 코드를 세이더 객체에 넣기
    glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
    //--- 버텍스 세이더 컴파일하기
    glCompileShader(vertexShader);
    //--- 컴파일이 제대로 되지 않은 경우: 에러 체크
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

void make_fragmentShaders()
{
    GLchar* fragmentSource;
    fragmentSource = filetobuf("phongfragment.glsl");
    //--- 프래그먼트 세이더 객체 만들기
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    //--- 세이더 코드를 세이더 객체에 넣기
    glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
    //--- 프래그먼트 세이더 컴파일
    glCompileShader(fragmentShader);
    //--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

GLvoid make_shaderProgram()
{
    GLuint shaderID;
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    //-- shader Program
    shaderProgramID = glCreateProgram();
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);
    //--- 세이더 삭제하기
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    //--- Shader Program 사용하기
    glUseProgram(shaderProgramID);

}




char* filetobuf(const char* file)
{
    FILE* fptr;
    long length;
    char* buf;
    fptr = fopen(file, "rb"); // Open file for reading 
    if (!fptr) // Return NULL on failure 
        return NULL;
    fseek(fptr, 0, SEEK_END); // Seek to the end of the file 
    length = ftell(fptr); // Find out how many bytes into the file we are 
    buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator 
    fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file 
    fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer 
    fclose(fptr); // Close the file 
    buf[length] = 0; // Null terminator 
    return buf; // Return the buffer 
}


bool ReadObj(const char* path) {
    temp_vertices.clear();
    temp_uvs.clear();
    temp_normals.clear();
    vertices.clear();
    uvs.clear();
    normals.clear();
    vertexIndices.clear();
    normalIndices.clear();
    uvIndices.clear();
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        printf("파일 경로 확인\n");
        getchar();
        return false;
    }
    while (1) {
        char lineHeader[500];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break;
        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            if (matches != 9) {
                printf("못읽");
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            uvIndices.push_back(uvIndex[0]);
            normalIndices.push_back(normalIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            uvIndices.push_back(uvIndex[1]);
            normalIndices.push_back(normalIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[2]);
        }
        else {
            char sbuffer[1000];
            fgets(sbuffer, 1000, file);
        }
    }

    for (unsigned int i = 0; i < vertexIndices.size(); ++i) {
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];

        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        glm::vec2 uv = temp_uvs[uvIndex - 1];
        glm::vec3 normal = temp_normals[normalIndex - 1];

        vertices.push_back(vertex);
        uvs.push_back(uv);
        normals.push_back(normal);

    }
    return true;
}

#ifndef H_OCEAN
#define H_OCEAN
#include "ShaderProgram.h"
#include "common.h"
#include "SOIL/SOIL.h"
#include <vector>
#include <GLFW/glfw3.h>

class ClassOcean{
    GLuint vboVertices, vboIndices, vboTexCoords;
    GLuint water_texture;
    int numIndices = 0;
    float Ocean[257][257];
    // std::vector<GLfloat> vertices; //вектор атрибута координат вершин
    // std::vector<unsigned int> indices; //вектор атрибута нормалей к вершинам
    // std::vector<GLfloat> texcoords; //вектор атрибут текстурных координат вершин
public:
    void load_texture(){
        glGenTextures(1, &water_texture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, water_texture);
        int width, height;
        unsigned char* image = SOIL_load_image("../textures/water.png", &width, &height, 0, SOIL_LOAD_RGB);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
        SOIL_free_image_data(image);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    int createTriStrip(int rows, int cols, float size, GLuint &vao) {
        numIndices = 2 * cols * (rows - 1) + rows - 1;

        std::vector<GLfloat> vertices_vec; //вектор атрибута координат вершин
        vertices_vec.reserve(rows * cols * 3);

        std::vector<GLfloat> normals_vec; //вектор атрибута нормалей к вершинам
        normals_vec.reserve(rows * cols * 3);

        std::vector<GLfloat>texcoords_vec; //вектор атрибут текстурных координат вершин
        texcoords_vec.reserve(rows * cols * 2);

        std::vector<float3> normals_vec_tmp(rows * cols, float3(0.0f, 0.0f,0.0f)); //временный вектор нормалей, используемый для расчетов

      std::vector<int3> faces; //вектор граней (треугольников), каждая грань - три
                               //индекса вершин, её составляющих; используется для
                               //удобства расчета нормалей
      faces.reserve(numIndices / 3);

      std::vector<GLuint> indices_vec; //вектор индексов вершин для передачи шейдерной программе
      indices_vec.reserve(numIndices);

      // Срез уровеня моря
    	for (uint x = 0; x < size; ++x)
    		for (uint y = 0; y < size; ++y)
        	    Ocean[x][y] = 0.0f;

    	for (int z = 0; z < rows; ++z){
    	 	for (int x = 0; x < cols; ++x){
          //вычисляем координаты каждой из вершин
          float xx = x;
          float zz = z;
          GLfloat timer = glfwGetTime();
          float coord = (rows/2 - x)*(rows/2 -x) + (rows/2 -z)*(rows/2-z);
          float yy = sinf(timer + sqrt(coord)/1.3) + 0.16;

          vertices_vec.push_back(xx);
          vertices_vec.push_back(yy);
          vertices_vec.push_back(zz);

          texcoords_vec.push_back(x / float(cols - 1)); // вычисляем первую
          // текстурную координату u,
          // для плоскости это просто
          // относительное положение
          // вершины
          texcoords_vec.push_back(z / float(rows - 1)); // аналогично вычисляем вторую текстурную координату v
        }
      }

      // primitive restart - специальный индекс, который обозначает конец строки из
      // треугольников в triangle_strip
      //после этого индекса формирование треугольников из массива индексов начнется
      //заново - будут взяты следующие 3 индекса для первого треугольника
      //и далее каждый последующий индекс будет добавлять один новый треугольник
      //пока снова не встретится primitive restart index

      int primRestart = cols * rows;

      for (int x = 0; x < cols - 1; ++x) {
        for (int z = 0; z < rows - 1; ++z) {
          int offset = x * cols + z;

          //каждую итерацию добавляем по два треугольника, которые вместе формируют
          //четырехугольник
          if (z == 0) //если мы в начале строки треугольников, нам нужны первые
                      //четыре индекса
          {
            indices_vec.push_back(offset + 0);
            indices_vec.push_back(offset + rows);
            indices_vec.push_back(offset + 1);
            indices_vec.push_back(offset + rows + 1);
          } else // иначе нам достаточно двух индексов, чтобы добавить два
                 // треугольника
          {
            indices_vec.push_back(offset + 1);
            indices_vec.push_back(offset + rows + 1);

            if (z == rows - 2)
              indices_vec.push_back(primRestart); // если мы дошли до конца строки,
            // вставляем primRestart, чтобы
            // обозначить переход на следующую
            // строку
          }
        }
      }

      ///////////////////////
      //формируем вектор граней(треугольников) по 3 индекса на каждый
      int currFace = 1;
      for (int i = 0; i < indices_vec.size() - 2; ++i) {
        int3 face;

        int index0 = indices_vec.at(i);
        int index1 = indices_vec.at(i + 1);
        int index2 = indices_vec.at(i + 2);

        if (index0 != primRestart && index1 != primRestart &&
            index2 != primRestart) {
          if (currFace % 2 != 0) //если это нечетный треугольник, то индексы и так в
          //правильном порядке обхода - против часовой
          //стрелки
          {
            face.x = indices_vec.at(i);
            face.y = indices_vec.at(i + 1);
            face.z = indices_vec.at(i + 2);

            currFace++;
          } else //если треугольник четный, то нужно поменять местами 2-й и 3-й
                 //индекс;
          {      //при отрисовке opengl делает это за нас, но при расчете нормалей нам
                 //нужно это сделать самостоятельно
            face.x = indices_vec.at(i);
            face.y = indices_vec.at(i + 2);
            face.z = indices_vec.at(i + 1);

            currFace++;
          }
          faces.push_back(face);
        }
      }

      ///////////////////////
      //расчет нормалей
      for (int i = 0; i < faces.size(); ++i) {
        //получаем из вектора вершин координаты каждой из вершин одного треугольника
        float3 A(vertices_vec.at(3 * faces.at(i).x + 0),
                 vertices_vec.at(3 * faces.at(i).x + 1),
                 vertices_vec.at(3 * faces.at(i).x + 2));
        float3 B(vertices_vec.at(3 * faces.at(i).y + 0),
                 vertices_vec.at(3 * faces.at(i).y + 1),
                 vertices_vec.at(3 * faces.at(i).y + 2));
        float3 C(vertices_vec.at(3 * faces.at(i).z + 0),
                 vertices_vec.at(3 * faces.at(i).z + 1),
                 vertices_vec.at(3 * faces.at(i).z + 2));

        //получаем векторы для ребер треугольника из каждой из 3-х вершин
        float3 edge1A(normalize(B - A));
        float3 edge2A(normalize(C - A));

        float3 edge1B(normalize(A - B));
        float3 edge2B(normalize(C - B));

        float3 edge1C(normalize(A - C));
        float3 edge2C(normalize(B - C));

        //нормаль к треугольнику - векторное произведение любой пары векторов из
        //одной вершины
        float3 face_normal = cross(edge1A, edge2A);

        //простой подход: нормаль к вершине = средняя по треугольникам, к которым
        //принадлежит вершина
        normals_vec_tmp.at(faces.at(i).x) += face_normal;
        normals_vec_tmp.at(faces.at(i).y) += face_normal;
        normals_vec_tmp.at(faces.at(i).z) += face_normal;
      }

      //нормализуем векторы нормалей и записываем их в вектор из GLFloat, который
      //будет передан в шейдерную программу
      for (int i = 0; i < normals_vec_tmp.size(); ++i) {
        float3 N = normalize(normals_vec_tmp.at(i));

        normals_vec.push_back(N.x);
        normals_vec.push_back(N.y);
        normals_vec.push_back(N.z);
      }

      GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;

      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vboVertices);
      glGenBuffers(1, &vboIndices);
      glGenBuffers(1, &vboNormals);
      glGenBuffers(1, &vboTexCoords);

      glBindVertexArray(vao);
      GL_CHECK_ERRORS;
      {

        //передаем в шейдерную программу атрибут координат вершин
        glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
        GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, vertices_vec.size() * sizeof(GL_FLOAT),
                     &vertices_vec[0], GL_STREAM_DRAW);
        GL_CHECK_ERRORS;
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT),
                              (GLvoid *)0);
        GL_CHECK_ERRORS;
        glEnableVertexAttribArray(0);
        GL_CHECK_ERRORS;

        //передаем в шейдерную программу атрибут нормалей
        glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
        GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, normals_vec.size() * sizeof(GL_FLOAT),
                     &normals_vec[0], GL_STREAM_DRAW);
        GL_CHECK_ERRORS;
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT),
                              (GLvoid *)0);
        GL_CHECK_ERRORS;
        glEnableVertexAttribArray(1);
        GL_CHECK_ERRORS;

        //передаем в шейдерную программу атрибут текстурных координат
        glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
        GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, texcoords_vec.size() * sizeof(GL_FLOAT),
                     &texcoords_vec[0], GL_STREAM_DRAW);
        GL_CHECK_ERRORS;
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT),
                              (GLvoid *)0);
        GL_CHECK_ERRORS;
        glEnableVertexAttribArray(2);
        GL_CHECK_ERRORS;

        //передаем в шейдерную программу индексы
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
        GL_CHECK_ERRORS;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_vec.size() * sizeof(GLuint),
                     &indices_vec[0], GL_STREAM_DRAW);
        GL_CHECK_ERRORS;

        glEnable(GL_PRIMITIVE_RESTART);
        GL_CHECK_ERRORS;
        glPrimitiveRestartIndex(primRestart);
        GL_CHECK_ERRORS;
      }
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glBindVertexArray(0);

      return numIndices;
    }

public:
    ClassOcean(){}

    void Draw(GLuint vao, const ShaderProgram& shader, const float4x4& projection, const float4x4& view){
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        shader.StartUseShader();
        shader.SetUniform("projection", projection);
        shader.SetUniform("view", view);

        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, water_texture);
		shader.SetUniform("water_texture", 2);

        glDrawElements(GL_TRIANGLE_STRIP, numIndices, GL_UNSIGNED_INT, nullptr);GL_CHECK_ERRORS;
        glBindVertexArray(0);GL_CHECK_ERRORS;
    }

};

#endif

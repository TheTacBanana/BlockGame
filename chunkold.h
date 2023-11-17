#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <map>
#include <iostream>
#include <thread>

#include "PerlinNoise.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"

class Chunk{
    public:
        float noisescale = 0.025;
        int octaves = 4;
        float persistence = 0.9f;
        float threshold = 0.4;

        int xCoord, zCoord;

        // 0 - Ungenerated
        // 1 - Generating
        // 2 - Generated
        // 3 - OpenGL Buffers Allocated
        // 4 - Deletion in Progress
        unsigned int chunkState;

        Chunk(int xIn, int zIn, int cXS, int cYS, int cZS){
            xCoord = xIn; zCoord = zIn;
            chunkXSize = cXS; chunkYSize = cYS; chunkZSize = cZS;
            chunkState = 0;

            vertexData = new std::vector<glm::vec3>();
            indicesData = new std::vector<unsigned int>();
        }

        ~Chunk(){
            chunkState = 4;

            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);

            if (internalData != nullptr) { delete[] internalData; }

            if (vertexData != nullptr){ vertexData->clear(); delete vertexData;}
            if (indicesData != nullptr){ indicesData->clear(); delete indicesData;}
        }

        void StartAsyncGeneration(){
            if (threadedProcess == nullptr && chunkState == 0){
                threadedProcess = new std::thread (&Chunk::Generate, this);
                threadedProcess->detach();
            }
            else {
                threadedProcess = nullptr;
            }
        }

        bool CanDeleteObject(){
            if (threadedProcess != nullptr && (chunkState == 0 || chunkState == 2 || chunkState == 3)){ //} || chunkState == 3 || threadedProcess == nullptr){
                return true;
            }
            return false;
        }

        void Generate(){
            chunkState = 1;
            GenerateInternalData();
            GenerateMeshData();
            chunkState = 2;
        }

        void GenerateInternalData(){
            const siv::PerlinNoise::seed_type seed = 0;
            const siv::PerlinNoise perlin{ seed };

            internalData = new unsigned int[chunkXSize * chunkYSize * chunkZSize];
            for (size_t i = 0; i < chunkXSize * chunkYSize * chunkZSize; i++) { internalData[i] = 0; }

            for (int x = 0; x < chunkXSize; x++) {
                for (int z = 0; z < chunkZSize; z++) {
                    float n = perlin.octave2D_01((xCoord * chunkXSize + x) * noisescale, (zCoord * chunkZSize + z) * noisescale, octaves, 0.5f);
                    if (n < 0.1) { n = 0.1; }
                    if (n > 1.0) { n = 1.0; }
                    for (size_t y = 0; y < (int)(n * chunkYSize); y++) {
                        int index = x + chunkXSize * chunkZSize * y + chunkZSize * z;
                        if (internalData != nullptr && index < chunkXSize * chunkYSize * chunkZSize){
                            internalData[index] = 1;
                        }
                    }
                }
            }
        } 

        void GenerateMeshData(){
            if (vertexData != nullptr){ vertexData->clear(); delete vertexData; }
            vertexData = new std::vector<glm::vec3>();
            if (indicesData != nullptr){ indicesData->clear(); delete indicesData; }
            indicesData = new std::vector<unsigned int>();

            for (size_t z = 0; z < chunkZSize; z++){
                for (size_t y = 0; y < chunkYSize; y++){
                    for (size_t x = 0; x < chunkXSize; x++){
                        if (GetAt(x, y, z) != 0) {
                            GenerateVoxel(glm::vec3(x, y, z));
                        }
                    }
                }
            }
        }

        void GenerateVoxel(glm::vec3 pos){
            if (GetAt(pos.x + 1, pos.y, pos.z) == 0){
                GenerateTri(pos + ozz, pos + ooz, pos + ooo);
                GenerateTri(pos + ozz, pos + ozo, pos + ooo);
            }
            if (GetAt(pos.x, pos.y + 1, pos.z) == 0){
                GenerateTri(pos + zoz, pos + ooz, pos + ooo);
                GenerateTri(pos + zoz, pos + zoo, pos + ooo);
            }
            if (GetAt(pos.x, pos.y, pos.z + 1) == 0){
                GenerateTri(pos + zzo, pos + ozo, pos + ooo);
                GenerateTri(pos + zzo, pos + zoo, pos + ooo);
            }
            if (GetAt(pos.x - 1, pos.y, pos.z) == 0){
                GenerateTri(pos, pos + zoz, pos + zoo);
                GenerateTri(pos, pos + zzo, pos + zoo);
            }
            if (GetAt(pos.x, pos.y - 1, pos.z) == 0){
                GenerateTri(pos, pos + ozz, pos + ozo);
                GenerateTri(pos, pos + zzo, pos + ozo);
            }
            if (GetAt(pos.x, pos.y, pos.z - 1) == 0){
                GenerateTri(pos, pos + ozz, pos + ooz);
                GenerateTri(pos, pos + zoz, pos + ooz);
            }
        }

        void GenerateTri(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3){
            unsigned int v1Index = AddVertex(v1);
            unsigned int v2Index = AddVertex(v2);
            unsigned int v3Index = AddVertex(v3);

            PushTri(v1Index, v2Index, v3Index);
        }

        void PushTri(unsigned int x, unsigned int y, unsigned int z){
            if (indicesData != nullptr){
                indicesData->push_back(x);
                indicesData->push_back(y);
                indicesData->push_back(z);
            }
        }

        unsigned int AddVertex(glm::vec3 vert){
            if (vertexData != nullptr){
                int result = FindVertex(vert);
                if (result != -1){
                    return (unsigned int) result;
                }
                unsigned int index = vertexData->size();
                vertexData->push_back(vert);
                return index;
            }
            return 0;
        }

        int FindVertex(glm::vec3 vert){
            if (vertexData != nullptr){
                for (size_t i = 0; i < vertexData->size(); i++){
                    if (vert == vertexData->at(i)){
                        return i;
                    }
                }
            }
            return -1;
        }

        unsigned int GetAt(unsigned int x, unsigned int y, unsigned int z){
            if (y >= chunkYSize){ return 0; }
            if (x < 0 || x >= chunkXSize || y < 0 || z < 0 || z >= chunkZSize){
                return 1;
            }
            if (internalData != nullptr){
                return internalData[x + chunkXSize * chunkZSize * y + chunkZSize * z];
            }
            return 0;
        }

        void SetAt(unsigned int x, unsigned int y, unsigned int z, unsigned int val){
            int index = x + chunkXSize * chunkZSize * y + chunkZSize * z;
            if (internalData != nullptr && index >= 0 && index < chunkXSize * chunkYSize * chunkZSize){
                internalData[index] = val;
            }
        }

        void SetupMesh(){
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertexData->size() * sizeof(glm::vec3), &vertexData->at(0), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesData->size() * sizeof(unsigned int), &indicesData->at(0), GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

            glBindBuffer(GL_ARRAY_BUFFER, 0); 

            glBindVertexArray(0); 

            chunkState = 3;
        }

        void Draw(){
            if (indicesData != nullptr){
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indicesData->size()), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }


    private:
        // Chunk Sizes
        int chunkXSize, chunkYSize, chunkZSize;

        std::thread* threadedProcess = nullptr;

        unsigned int* internalData;

        std::vector<glm::vec3>* vertexData;
        std::vector<unsigned int>* indicesData;

        // Rendering
        unsigned int VBO, VAO, EBO;

        // Mesh Generation
        glm::vec3 ozz = glm::vec3(1,0,0);
        glm::vec3 ooz = glm::vec3(1,1,0);
        glm::vec3 ooo = glm::vec3(1,1,1);
        glm::vec3 ozo = glm::vec3(1,0,1);
        glm::vec3 zoz = glm::vec3(0,1,0);
        glm::vec3 zoo = glm::vec3(0,1,1);
        glm::vec3 zzo = glm::vec3(0,0,1);

};

#endif
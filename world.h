#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"
#include "shader.h"
#include "camera.h"

#include <vector>
#include <thread>
#include <map>
#include <cmath>

class World{
    public:
        std::map< std::pair<int, int>, Chunk*> chunkMap;
        int renderDistance;

        static const int chunkXSize = 16, chunkYSize = 32, chunkZSize = 16;

        World(int rDist){
            renderDistance = rDist;
        }

        void Draw(Shader& shader, Camera* cam){
            int camXCoord = cam->position.x / chunkXSize;
            int camZCoord = cam->position.z / chunkZSize;

            RemoveUnloadedFromMap(camXCoord, camZCoord);

            //glm::vec3 camDirection = glm::normalize(glm::vec3(cam->forward.x, 0, cam->forward.z));

            for (int z = camZCoord - renderDistance; z < camZCoord + renderDistance + 1; z++) {
                for (int x = camXCoord - renderDistance; x < camXCoord + renderDistance + 1; x++) {
                    //glm::vec3 toChunk = glm::normalize(glm::vec3(x - camXCoord, 0, z - camZCoord));

                    //float dot = glm::dot(camDirection, toChunk);
                    //float angle = std::acos(dot);

                    //if (glm::degrees(angle) > (cam->fov)) {continue;}

                    Chunk* ch = IndexChunks(x, z);
                    
                    if (ch->chunkState == 0){
                        ch->StartAsyncGeneration();
                    }
                    else if (ch->chunkState == 2){
                        ch->SetupMesh();
                    } 
                    else if (ch->chunkState == 3){
                        glm::mat4 model = glm::mat4(1.0f);
                        model = glm::translate(model, glm::vec3(ch->xCoord * chunkXSize,0,ch->zCoord * chunkZSize));
                        shader.setMat4("model", model);

                        ch->Draw();
                    }
                }
            }    
        }

        void RemoveUnloadedFromMap(int camX, int camZ){
            std::map< std::pair<int, int>, Chunk* >::iterator iter;
            std::vector<std::pair<int, int>> store;

            Chunk* ch;
            for (iter = chunkMap.begin(); iter != chunkMap.end(); iter++) {
                ch = iter->second;

                if (std::abs(camX - ch->xCoord) > renderDistance + 1 || 
                        std::abs(camZ - ch->zCoord) > renderDistance + 1){
                    store.push_back(iter->first);
                }
            }

            for (size_t i = 0; i < store.size(); i++) {
                Chunk* ch = chunkMap[store.at(i)];
                if (ch->CanDeleteObject()){
                    chunkMap.erase(store.at(i));
                    delete ch;
                }
            }
        }

        Chunk* IndexChunks(int xCoord, int zCoord){
            std::pair<int, int> pair (xCoord, zCoord);

            Chunk* ch = chunkMap[pair];
            if (ch != nullptr){
                return ch;
            } else {
                ch = new Chunk(xCoord, zCoord, chunkXSize, chunkYSize, chunkZSize);
                chunkMap[pair] = ch;
                return chunkMap[pair];
            }
        }

        /*
        void DestroyBlock(Camera* cam, float range){
            for (float i = 0; i < range; i += 0.1){
                glm::vec3 blockFloat = cam->position + (float)i * (glm::normalize(cam->forward));
                glm::vec3 blockInt = glm::vec3((int)std::floor(blockFloat.x) % chunkXSize, 
                                               (int)std::floor(blockFloat.y),
                                               (int)std::floor(blockFloat.z) % chunkZSize);
                glm::vec2 chunkIndex = glm::vec2(std::floor(blockFloat.x / (float)chunkXSize), std::floor(blockFloat.z / (float)chunkZSize));
                
                Chunk* ch = IndexChunks(chunkIndex.x, chunkIndex.y);

                unsigned int block = ch->GetAt(blockInt.x, blockInt.y, blockInt.z);

                if (block == 1){
                    ch->SetAt(blockInt.x, blockInt.y, blockInt.z, 0);
                    ch->RemoveMesh();
                    ch->GenerateMeshData();
                    ch->SetupMesh();
                    break;
                }
            }
        }
        */
};

#endif
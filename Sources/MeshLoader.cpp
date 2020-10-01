#include "MeshLoader.h"

#include <iostream>
#include <fstream>
#include <exception>
#include <ios>

using namespace std;

void MeshLoader::loadOFF (const std::string & filename, std::shared_ptr<Mesh> meshPtr) {
	std::cout << " > Start loading mesh <" << filename << ">" << std::endl;
    meshPtr->clear ();
	ifstream in (filename.c_str ());
    if (!in)
        throw std::ios_base::failure ("[Mesh Loader][loadOFF] Cannot open " + filename);
	string offString;
    unsigned int sizeV, sizeT, tmp;
    in >> offString >> sizeV >> sizeT >> tmp;
    auto & P = meshPtr->vertexPositions ();
    auto & T = meshPtr->triangleIndices ();
    P.resize (sizeV);
    T.resize (sizeT);
    size_t tracker = (sizeV + sizeT)/20;
    std::cout << " > [" << std::flush;
    for (unsigned int i = 0; i < sizeV; i++) {
    	if (i % tracker == 0)
    		std::cout << "-" << std::flush;
        in >> P[i][0] >> P[i][1] >> P[i][2];
    }
    int s;
    for (unsigned int i = 0; i < sizeT; i++) {
    	if ((sizeV + i) % tracker == 0)
    		std::cout << "-" << std::flush;
        in >> s;
        for (unsigned int j = 0; j < 3; j++)
            in >> T[i][j];
    }
    std::cout << "]" << std::endl;
    in.close ();
    meshPtr->vertexNormals ().resize (P.size (), glm::vec3 (0.f, 0.f, 1.f));
    meshPtr->vertexTexCoords ().resize (P.size (), glm::vec2 (0.f, 0.f));
    meshPtr->recomputePerVertexNormals ();
		meshPtr->computePlanarParameterization();
    std::cout << " > Mesh <" << filename << "> loaded" <<  std::endl;
}

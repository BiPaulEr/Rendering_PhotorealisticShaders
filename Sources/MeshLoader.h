#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include <string>
#include <memory>

#include "Mesh.h"

namespace MeshLoader {

/// Loads an OFF mesh file. See https://en.wikipedia.org/wiki/OFF_(file_format)
void loadOFF (const std::string & filename, std::shared_ptr<Mesh> meshPtr);

}

#endif // MESH_LOADER_H
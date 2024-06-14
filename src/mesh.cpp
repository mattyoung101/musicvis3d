// Copyright 2024 Matt Young.
// SPDX-License-Identifier: ISC
#include "cosc/mesh.hpp"
#include "glad/gl.h"
#include <spdlog/spdlog.h>
#include <string>

void cosc::Mesh::setupMesh() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), &verts[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) 0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, norm));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

void cosc::Mesh::draw(cosc::Shader &shader) const {
    // FIXME: Remove this block fully if not required by a future bar-refraction shader
    //
    // unsigned int diffuseNr = 1;
    // unsigned int specularNr = 1;
    //
    // for (unsigned int i = 0; i < textures.size(); i++) {
    //     // activate the texture unit
    //     glActiveTexture(GL_TEXTURE0 + i);
    //
    //     // retrieve texture number
    //     std::string number;
    //     auto name = textures[i].type;
    //     if (name == "texture_diffuse") {
    //         number = std::to_string(diffuseNr++);
    //     } else if (name == "texture_specular") {
    //         number = std::to_string(specularNr++);
    //     } else {
    //         SPDLOG_WARN("Unhandled texture name: {}", name);
    //     }
    //
    //     std::string shaderName = "material.";
    //     shaderName += name;
    //     shaderName += number;
    //
    //     SPDLOG_DEBUG("Handle texture!!!");
    //
    //     shader.setInt(shaderName.c_str(), i);
    //     glBindTexture(GL_TEXTURE_2D, textures[i].id);
    // }
    // glActiveTexture(GL_TEXTURE0);

    // draw mesh
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

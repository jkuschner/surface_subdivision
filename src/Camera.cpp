#include <iostream>
#include "Camera.h"
#include <math.h>

// HW2: You can add more helper functions if you want!

glm::mat3 rotation(const float degrees,const glm::vec3 axis){
    const float angle = degrees * M_PI/180.0f; // convert to radians
    const glm::mat3 aCross = glm::mat3(0, axis.z, -axis.y, -axis.z, 0, axis.x, axis.y, -axis.x, 0);

    glm::mat3 res = glm::cos(angle) * glm::mat3(1,0,0,0,1,0,0,0,1);
    res += (1 - glm::cos(angle)) * glm::outerProduct(axis, axis);
    res += glm::sin(angle) * aCross;

    return res;
    // HW2: implement a 3D rotation matrix for the given angle and axis.
}

void Camera::rotateRight(const float degrees){
    // HW2: Update the class members "eye," "up"
    eye = rotation(degrees, up) * eye;
    //Step 1: compute z
    glm::vec3 z = eye - target;
    z = z / glm::length(eye - target);

    //Step 2: ensure u is orthogonal to z
    up = up - glm::dot(up, z) * z;
    up = up / glm::length(up);
}
void Camera::rotateUp(const float degrees){
    // HW2: Update the class members "eye," "up"
    glm::vec3 axis = glm::cross(up, target - eye);
    axis = axis / glm::length(axis);
    eye = rotation(degrees, axis) * eye;

    //Step 1: compute z
    glm::vec3 z = eye - target;
    z = z / glm::length(eye - target);

    //Step 2: ensure u is orthogonal to z
    up = up - glm::dot(up, z) * z;
    up = up / glm::length(up);
}
void Camera::computeMatrices(){
    // Note that glm matrix column majored.
    // That is, A[i] is the ith column of A,
    // and A_{ij} in math notation is A[j][i] in glm notation.
    
    // HW2: Update the class member "view" for the view matrix using "eye," "target," "up."
    //Step 1: compute z
    glm::vec3 z = eye - target;
    z = z / glm::length(eye - target);

    //Step 2: ensure u is orthogonal to z
    up = up - glm::dot(up, z) * z;
    up = up / glm::length(up);

    //Step 3: compute x
    glm::vec3 x = glm::cross(up, z);

    //Step 4: Camera matrix and inverse
    glm::mat4 camera = glm::mat4(x.x, x.y, x.z, 0, up.x, up.y, up.z, 0, z.x, z.y, z.z, 0, eye.x, eye.y, eye.z, 1);
    view = glm::inverse(camera);

    /* Check view matrix
    std::cout << "View matrix = " << std::endl;
    std::cout << "[ " << view[0][0]
    << " " << view[1][0]
    << " " << view[2][0]
    << " " << view[3][0]  << std::endl
    << " " << view[0][1]
    << " " << view[1][1]
    << " " << view[2][1]
    << " " << view[3][1]  << std::endl
    << " " << view[0][2]
    << " " << view[1][2]
    << " " << view[2][2]
    << " " << view[3][2]  << std::endl
    << " " << view[0][3]
    << " " << view[1][3]
    << " " << view[2][3]
    << " " << view[3][3] << " ]." << std::endl;
    */
    
    // HW2: Update the class member "proj" for the perspective matrix using the class members "aspect," "fovy," "near," "far."
    float fovy_rad = fovy * M_PI/180.0f; // remember to convert degrees to radians.
    proj = glm::mat4(1.0f / (aspect * glm::tan(fovy_rad / 2.0f)), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / glm::tan(fovy_rad / 2.0f), 0.0f, 0.0f, 0.0f, 0.0f, -(far + near) / (far - near), -1.0f, 0.0f, 0.0f, -(2*far*near) / (far - near), 0.0f);
    
}

void Camera::reset(){
    eye = eye_default;// position of the eye
    target = target_default;  // look at target
    up = up_default;      // up vector
    fovy = fovy_default;  // field of view in degrees
    aspect = aspect_default; // aspect ratio
    near = near_default; // near clipping distance
    far = far_default; // far clipping distance
}

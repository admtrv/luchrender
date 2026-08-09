/*
 * Transform.h
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <algorithm>
#include <vector>

namespace BulletRender {
namespace scene {

// reference frame of operation
enum class Space {
    Local,
    World
};

// affine transform with parent-child hierarchy, M = T * R * S
class Transform {
public:
    Transform() = default;
    ~Transform();

    // parent and children reference each other by address, copying would alias them
    Transform(const Transform&) = delete;
    Transform& operator=(const Transform&) = delete;

    // hierarchy

    void setParent(Transform* parent, bool keepWorld = true);   // nullptr detaches, keepWorld preserves the current world pose
    Transform* getParent() const;

    void addChild(Transform* child, bool keepWorld = true);     // same as child->setParent(this)
    const std::vector<Transform*>& getChildren() const;

    // local pose

    void setLocalPose(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl);

    void setLocalPosition(const glm::vec3& pos);
    glm::vec3 getLocalPosition() const;

    void setLocalRotation(const glm::quat& rot);
    glm::quat getLocalRotation() const;

    void setLocalScale(const glm::vec3& scl);
    void setLocalScale(float scl);
    glm::vec3 getLocalScale() const;

    // world pose

    void setPosition(const glm::vec3& pos);
    glm::vec3 getPosition() const;

    void setRotation(const glm::quat& rot);
    glm::quat getRotation() const;

    // matrices

    void setLocalMatrix(const glm::mat4& mat);
    const glm::mat4& getLocalMatrix() const;

    void setMatrix(const glm::mat4& mat);
    const glm::mat4& getMatrix() const;

    glm::mat3 getNormalMatrix() const;          // inverse-transpose of the upper 3x3, survives non-uniform scale

    // operations

    void translate(const glm::vec3& delta, Space space = Space::Local);

    void rotate(const glm::quat& rot, Space space = Space::Local);
    void rotate(const glm::vec3& axis, float angleRad, Space space = Space::Local);

    void scale(const glm::vec3& factor);
    void scale(float factor);

    void reset();

    // local axes in world space
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;
    glm::vec3 getForward() const;

private:
    void invalidateLocal();     // marks local matrix and whole subtree as outdated
    void invalidateWorld();     // marks world matrices of this node and of subtree as outdated

    void updateLocal() const;
    void updateWorld() const;

    void detachFromParent();
    bool isAncestorOf(const Transform& node) const;

    glm::vec3 m_position{0.0f};
    glm::quat m_rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 m_scale{1.0f};

    Transform* m_parent = nullptr;
    std::vector<Transform*> m_children;

    mutable glm::mat4 m_local{1.0f};
    mutable glm::mat4 m_world{1.0f};

    mutable bool m_localDirty = false;
    mutable bool m_worldDirty = false;
};

} // namespace scene
} // namespace BulletRender

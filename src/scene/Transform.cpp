/*
 * Transform.cpp
 */

#include "Transform.h"

namespace BulletRender {
namespace scene {

constexpr float SCALE_EPSILON = 1e-8f;      // scale below this is degenerate
constexpr float LENGTH2_EPSILON = 1e-8f;    // squared length below this carries no direction

// affine matrix -> translation, rotation, scale
static void decompose(const glm::mat4& mat, glm::vec3& outPos, glm::quat& outRot, glm::vec3& outScale)
{
    outPos = glm::vec3(mat[3]);

    const glm::vec3 axisX = glm::vec3(mat[0]);
    const glm::vec3 axisY = glm::vec3(mat[1]);
    const glm::vec3 axisZ = glm::vec3(mat[2]);

    outScale = glm::vec3(glm::length(axisX), glm::length(axisY), glm::length(axisZ));

    // degenerate axes cannot be normalized, previous rotation stays
    if (outScale.x < SCALE_EPSILON || outScale.y < SCALE_EPSILON || outScale.z < SCALE_EPSILON)
    {
        return;
    }

    const glm::mat3 basis(axisX / outScale.x, axisY / outScale.y, axisZ / outScale.z);
    outRot = glm::normalize(glm::quat_cast(basis));
}

Transform::~Transform()
{
    // orphaned children become roots and keep their world pose
    for (Transform* child : m_children)
    {
        child->m_parent = nullptr;
        child->setLocalMatrix(child->getMatrix());
    }

    m_children.clear();
    detachFromParent();
}

// hierarchy

void Transform::setParent(Transform* parent, bool keepWorld)
{
    if (parent == m_parent || parent == this)
    {
        return;
    }

    // attaching under own descendant would close cycle
    if (parent != nullptr && isAncestorOf(*parent))
    {
        return;
    }

    const glm::mat4 world = getMatrix();

    detachFromParent();

    m_parent = parent;
    if (m_parent != nullptr)
    {
        m_parent->m_children.push_back(this);
    }

    if (keepWorld)
    {
        setMatrix(world);
    }
    else
    {
        invalidateWorld();
    }
}

Transform* Transform::getParent() const
{
    return m_parent;
}

void Transform::addChild(Transform* child, bool keepWorld)
{
    if (child != nullptr)
    {
        child->setParent(this, keepWorld);
    }
}

const std::vector<Transform*>& Transform::getChildren() const
{
    return m_children;
}

bool Transform::isAncestorOf(const Transform& node) const
{
    for (const Transform* it = node.m_parent; it != nullptr; it = it->m_parent)
    {
        if (it == this)
        {
            return true;
        }
    }

    return false;
}

void Transform::detachFromParent()
{
    if (m_parent == nullptr)
    {
        return;
    }

    std::vector<Transform*>& siblings = m_parent->m_children;
    siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());

    m_parent = nullptr;
}

// local pose

void Transform::setLocalPosition(const glm::vec3& pos)
{
    m_position = pos;
    invalidateLocal();
}

glm::vec3 Transform::getLocalPosition() const
{
    return m_position;
}

void Transform::setLocalRotation(const glm::quat& rot)
{
    m_rotation = glm::normalize(rot);
    invalidateLocal();
}

glm::quat Transform::getLocalRotation() const
{
    return m_rotation;
}

void Transform::setLocalScale(const glm::vec3& scl)
{
    m_scale = scl;
    invalidateLocal();
}

void Transform::setLocalScale(float scl)
{
    setLocalScale(glm::vec3(scl));
}

glm::vec3 Transform::getLocalScale() const
{
    return m_scale;
}

void Transform::setLocalPose(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl)
{
    m_position = pos;
    m_rotation = glm::normalize(rot);
    m_scale = scl;
    invalidateLocal();
}

// world pose

void Transform::setPosition(const glm::vec3& pos)
{
    if (m_parent == nullptr)
    {
        setLocalPosition(pos);
        return;
    }

    setLocalPosition(glm::vec3(glm::inverse(m_parent->getMatrix()) * glm::vec4(pos, 1.0f)));
}

glm::vec3 Transform::getPosition() const
{
    return glm::vec3(getMatrix()[3]);
}

void Transform::setRotation(const glm::quat& rot)
{
    setLocalRotation(m_parent != nullptr ? glm::inverse(m_parent->getRotation()) * rot : rot);
}

glm::quat Transform::getRotation() const
{
    return m_parent != nullptr ? m_parent->getRotation() * m_rotation : m_rotation;
}

// matrices

const glm::mat4& Transform::getLocalMatrix() const
{
    updateLocal();
    return m_local;
}

const glm::mat4& Transform::getMatrix() const
{
    updateWorld();
    return m_world;
}

glm::mat3 Transform::getNormalMatrix() const
{
    return glm::inverseTranspose(glm::mat3(getMatrix()));
}

void Transform::setLocalMatrix(const glm::mat4& mat)
{
    decompose(mat, m_position, m_rotation, m_scale);
    invalidateLocal();
}

void Transform::setMatrix(const glm::mat4& mat)
{
    setLocalMatrix(m_parent != nullptr ? glm::inverse(m_parent->getMatrix()) * mat : mat);
}

void Transform::updateLocal() const
{
    if (!m_localDirty)
    {
        return;
    }

    // T * R * S built directly, cheaper than multiplying three full matrices
    m_local = glm::mat4_cast(m_rotation);
    m_local[0] *= m_scale.x;
    m_local[1] *= m_scale.y;
    m_local[2] *= m_scale.z;
    m_local[3] = glm::vec4(m_position, 1.0f);

    m_localDirty = false;
}

void Transform::updateWorld() const
{
    if (!m_worldDirty && !m_localDirty)
    {
        return;
    }

    updateLocal();
    m_world = m_parent != nullptr ? m_parent->getMatrix() * m_local : m_local;

    m_worldDirty = false;
}

void Transform::invalidateLocal()
{
    m_localDirty = true;
    invalidateWorld();
}

void Transform::invalidateWorld()
{
    // subtree below dirty node is dirty too, no need to walk it again
    if (m_worldDirty)
    {
        return;
    }

    m_worldDirty = true;

    for (Transform* child : m_children)
    {
        child->invalidateWorld();
    }
}

// operations

void Transform::translate(const glm::vec3& delta, Space space)
{
    if (space == Space::Local)
    {
        m_position += m_rotation * delta;
    }
    else if (m_parent != nullptr)
    {
        m_position += glm::vec3(glm::inverse(m_parent->getMatrix()) * glm::vec4(delta, 0.0f));
    }
    else
    {
        m_position += delta;
    }

    invalidateLocal();
}

void Transform::rotate(const glm::quat& rot, Space space)
{
    if (space == Space::Local)
    {
        m_rotation = glm::normalize(m_rotation * rot);
    }
    else
    {
        // brought into parent space, so the node spins around world axes
        const glm::quat parentRot = m_parent != nullptr ? m_parent->getRotation() : glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        m_rotation = glm::normalize(glm::inverse(parentRot) * rot * parentRot * m_rotation);
    }

    invalidateLocal();
}

void Transform::rotate(const glm::vec3& axis, float angleRad, Space space)
{
    if (glm::length2(axis) < LENGTH2_EPSILON)
    {
        return;
    }

    rotate(glm::angleAxis(angleRad, glm::normalize(axis)), space);
}

void Transform::scale(const glm::vec3& factor)
{
    m_scale *= factor;
    invalidateLocal();
}

void Transform::scale(float factor)
{
    scale(glm::vec3(factor));
}

void Transform::reset()
{
    setLocalPose(glm::vec3(0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
}

// local axes in world space

glm::vec3 Transform::getRight() const
{
    return glm::normalize(glm::vec3(getMatrix()[0]));
}

glm::vec3 Transform::getUp() const
{
    return glm::normalize(glm::vec3(getMatrix()[1]));
}

glm::vec3 Transform::getForward() const
{
    return -glm::normalize(glm::vec3(getMatrix()[2]));
}

} // namespace scene
} // namespace BulletRender

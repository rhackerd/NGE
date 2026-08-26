#include "camera.h"
#include <cglm/cam.h>
#include <cglm/euler.h>
#include <cglm/mat4.h>
#include <cglm/quat.h>
#include <cglm/vec3.h>
#include <numbers>

namespace Nova::GE {
    void Camera::init(VmaAllocator allocator, DescriptorMan& descMan) {
        auto ci = CreateInfo::Buffer::Builder()
            .setAllocator(allocator)
            .setSize(sizeof(CameraData))
            .asUniform()
            .build();
        m_buffer.init(ci);
        m_dirty = true;
    }

    void Camera::shutdown() {
        if (m_buffer.isValid())
            m_buffer.shutdown();
    };

    void Camera::setPerspective(float fovDeg, float aspect, float nearP, float farP) {
        m_projType = ProjType::Perspective;
        m_fovDeg = fovDeg;
        m_aspect = aspect;
        m_nearP = nearP;
        m_farP = farP;
        m_dirty = true;
    }

    void Camera::setOrthographic(float left, float right, float bottom, float top, float nearP, float farP) {
        m_projType = ProjType::Orthographic;
        m_left = left;
        m_right = right;
        m_bottom = bottom;
        m_top = top;
        m_nearP = nearP;
        m_farP = farP;
        m_dirty = true;
    }

    void Camera::getForward(vec3 out) {
        vec3 base = {0.0f, 0.0f, -1.0f}; // forward convention (matches rebuildView's fallback)
        glm_quat_rotatev(m_rotation, base, out);
    }

    void Camera::getRight(vec3 out) {
        vec3 base = {1.0f, 0.0f, 0.0f};
        glm_quat_rotatev(m_rotation, base, out);
    }

    void Camera::getUp(vec3 out) {
        vec3 base = {0.0f, 1.0f, 0.0f};
        glm_quat_rotatev(m_rotation, base, out);
    }

    void Camera::setAspect(float aspect) {
        m_aspect = aspect;
        m_dirty = true;
    };

    void Camera::setTarget(vec3& target) {
        glm_vec3_copy(target, m_target);
        m_dirty = true;
    };

    void Camera::setPosition(vec3& pos) {
        glm_vec3_copy(pos, m_pos);
        m_dirty = true;
    };

    void Camera::setUp(vec3& up) {
        glm_vec3_copy(up, m_up);
        m_dirty = true;
    };

    void Camera::setRotation(versor& rot) {
        glm_quat_copy(rot, m_rotation);
        m_dirty = true;
    };

    void Camera::move(vec3& delta) {
        glm_vec3_add(m_pos, delta, m_pos);
        m_dirty = true;
    };

    void Camera::rotate(vec3& deltaEulerRad) {
        // Build a delta quaternion from pitch/yaw/roll (radians) and compose it
        versor deltaQuat;
        glm_euler_xyz_quat((float*)deltaEulerRad, deltaQuat);
        glm_quat_mul(m_rotation, deltaQuat, m_rotation);
        glm_quat_normalize(m_rotation);
        m_dirty = true;
    };

    void Camera::update() {
        if (!m_dirty) return;

        rebuildView();

        // Projection
        if (m_projType == ProjType::Perspective) {
            float fovRad = m_fovDeg * (std::numbers::pi / 180.0f);
            glm_perspective(fovRad, m_aspect, m_nearP, m_farP, m_data.proj);

            m_data.proj[1][1] *= -1.0f; // flip y for Vulkan
        } else {
            mat4 ortho;
            glm_ortho(m_left, m_right, m_bottom, m_top, m_nearP, m_farP, ortho);
            
            glm_mat4_copy(ortho, m_data.proj);
            m_data.proj[1][1] *= -1.0f;
        }

        m_buffer.upload(&m_data, sizeof(CameraData));
        m_dirty = false;
    }

    void Camera::rebuildView() {
        bool hasRotation = !(
            m_rotation[0] == 0.0f &&
            m_rotation[1] == 0.0f &&
            m_rotation[2] == 0.0f &&
            m_rotation[3] == 1.0f
        );

        if (hasRotation) {
            mat4 t, r;
            glm_translate_make(t, (vec3){ -m_pos[0], -m_pos[1], -m_pos[2] });
            glm_quat_mat4(m_rotation, r);
            glm_mat4_transpose(r);
            glm_mat4_mul(r, t, m_data.view); // r * t
        } else {
            vec3 target = { m_pos[0], m_pos[1], m_pos[2] - 1.0f };
            if (!(m_target[0] == 0.0f && m_target[1] == 0.0f && m_target[2] == 0.0f))
                glm_vec3_copy(m_target, target);

            glm_lookat(m_pos, target, m_up, m_data.view);
        }
    }
};
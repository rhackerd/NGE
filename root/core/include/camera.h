#pragma once

#include "buffer.h"
#include "descMan.h"
#include "system.h"
#include <cglm/vec3.h>

// TODO: modernize
// ========================================
// Metadata
//
// State    : #modern@nge3
// Origin   : @nge2
//
// Desc     : A short high level wrapper for camera, is still kinda low level, but can be nicely used for low level and is used by High level wrapper
// Info     : Properly fix the alignas
// ========================================

namespace Nova::GE {

    #if defined(__AVX__)
        #define NOVA_MAT4_ALIGN 32
    #else
        #define NOVA_MAT4_ALIGN 16
    #endif

    struct alignas(NOVA_MAT4_ALIGN) CameraData {
        mat4 view;
        mat4 proj;
    };

    class Camera {
        public:
            Camera() = default;
            ~Camera() = default;

        public:
            void init(VmaAllocator allocator, DescriptorMan& descMan);
            void shutdown();

            void setPerspective(float fovDeg, float aspect, float nearP, float farP);
            void setOrthographic(float left, float right, float bottom, float top, float nearP, float farP);
            void setAspect(float aspect);

            void setPosition(vec3& pos);
            void setPosition(float x,float y,float z) {m_pos[0] = x; m_pos[1] = y; m_pos[2] = z; m_dirty = true;}
            void setTarget(vec3& target);
            void setTarget(float x,float y,float z) {m_target[0] = x; m_target[1] = y; m_target[2] = z; m_dirty = true;}
            void setUp(vec3& up);
            void setUp(float x,float y,float z) {m_up[0] = x; m_up[1] = y; m_up[2] = z; m_dirty = true;}
            void setRotation(versor& rot);

            void move(vec3& delta);
            void rotate(vec3& deltaEulerRad);

            void update();

            const CameraData&      getData()        const { return m_data; }
            vk::Buffer             getBuffer()      const { return m_buffer.getBuffer(); }
            size_t                 getDescOffset()  const { return m_offset; }
            bool                   isValid()        const { return m_buffer.isValid(); }

            void        getPosition(vec3 out) { glm_vec3_copy(m_pos, out); }
            void        getForward(vec3 out);
            void        getRight(vec3 out);
            void        getUp(vec3 out);
            SetHandle   getHandle() { return handle; }

            void initDescriptor(DescriptorMan& man, SetHandle& setLayout) {
                handle = man.allocateSet(setLayout.layout, setLayout.setIndex);
                man.writeUBO(handle, 0, m_buffer.getBuffer(), sizeof(CameraData));
            };

        private:
            void rebuildView();

        private:
            Buffer              m_buffer;
            size_t              m_offset    = 0;
            CameraData          m_data;

            vec3    m_pos       = {0.0f, 0.0f, 0.0f};
            vec3    m_target    = {0.0f, 0.0f, 0.0f};
            vec3    m_up        = {0.0f, 1.0f, 0.0f};
            versor  m_rotation  = {0.0f, 0.0f, 0.0f, 1.0f};

            enum class ProjType {Perspective, Orthographic} m_projType = ProjType::Perspective;
            float               m_fovDeg    = 60.0f;
            float               m_aspect    = 16.0f / 9.0f;
            float               m_nearP     = 0.1f;
            float               m_farP      = 1000.0f;
            float               m_left      = -1.0f;
            float               m_right     = 1.0f;
            float               m_top       = 1.0f;
            float               m_bottom    = -1.0f;

            SetHandle           handle;

            bool m_dirty = true;
    };
};
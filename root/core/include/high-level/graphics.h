#pragma once

#include "device.h"
#include "descMan.h"
#include "swapchain.h"
#include "system.h"

namespace Nova::Graphics {
    class Graphics {
    public:
        Graphics() {};
        Graphics(SDL_Window* window) { init(window); }
        ~Graphics() { shutdown(); }
    
    public:
        // The function suppose that SDL is already initiated
        bool init(SDL_Window* window); // window isn't created here to let the user have more control
        void shutdown();

        Nova::GE::Device&      getDevice()      { return m_device; }
        Nova::GE::Swapchain&   getSwapchain()   { return m_swapchain; }
        Nova::GE::DescriptorMan& getDescMan()   { return m_device.getDescriptorManager(); }
        SDL_Window* getWindow() const {return m_window; }

    private:
        Nova::GE::System    m_system;
        Nova::GE::Device    m_device;
        Nova::GE::Swapchain m_swapchain;
        SDL_Window*         m_window;
    };
}
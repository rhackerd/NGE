#pragma once


// The design

// GPUF - FrameFactory
// CPUF - CommandBufferFactory



// GPUSlot -> Sync struct with {
// FrameResources resource;
// GE::Semaphore imageAvailable;
// GE::Semaphore renderFinished;
// std::atomic_bool inUse = false;
// }

// --- FrameResource ---
// FrameResource is a fresh copy of data like meshes, verticies, textures, etc.. - anything that is needed to render a frame
// It is update every time a resource updates but some another Thread
// If a resource is in used, it is flagged for update and after execution will be updated


// GPUJob -> CommandBuffer and one GPUSlot

// The workflow

// CPUF generates a command buffer
// CPUF submits the command buffer to the GPUF
// GPUF appends command to GPUCommands;
// GPUF on update GPUF checks if GPUCommands is not empty,
//      If not make a GPUJob, which will take on GPUSlot and one CommandBuffer from GPUCommands
//      
// GPUF will flag GPUJob for execution
// GPUF will execute GPUJob
// GPUF will return a FrameResource to Presenter
// FrameResource can be used for VR, Window - just anything that can present or want to show the frame
// Done


// TODO: Move to an .md file
// ========================================
// Metadata
//
// State    : #legacy@nge3
// Origin   : @nge2
//
// Desc     : Renderer overview
// Info     : Needs to be moved into an .md file later because this does not implement anything
// ========================================
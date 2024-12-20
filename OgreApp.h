#pragma once

#include "framework.h"
#include "mewizards.h"
#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreInput.h>
#include <OgreRTShaderSystem.h>
// #include <OgreMeshManager2.h>
// #include <OgreMesh2.h>
#include <OgreMeshManager.h>
#include <iostream>

// Include necessary headers for GLTF loading
#include <OgreCodec.h>
#include <OgrePrerequisites.h>
#include <OgreFileSystemLayer.h>
#include <OgreComponents.h>

class OgreApplication : public OgreBites::ApplicationContext, public OgreBites::InputListener
{
public:
    OgreApplication(HWND hWnd);
    virtual ~OgreApplication();

    void setup();
    bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    bool keyPressed(const OgreBites::KeyboardEvent& evt);
    bool keyReleased(const OgreBites::KeyboardEvent& evt);
    bool mouseMoved(const OgreBites::MouseMotionEvent& evt);
    bool mousePressed(const OgreBites::MouseButtonEvent& evt);
    bool mouseReleased(const OgreBites::MouseButtonEvent& evt);
    void shutdown();

private:
    HWND mHWnd;
    Ogre::SceneNode* mModelNode = nullptr;
    Ogre::SceneNode* mCameraNode = nullptr;
    Ogre::Real mRotX = 0;
    Ogre::Real mRotY = 0;
    bool mDragging = false;
    Ogre::Vector2 mDragStart;
    Ogre::String m_resourcePath;
    Ogre::FileSystemLayer* m_fileSystemLayer;

public:
    void LoadGltf(const std::string& path);
};
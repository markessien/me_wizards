
#include "OgreApp.h"

OgreApplication::OgreApplication(HWND hWnd) : OgreBites::ApplicationContext("Mewizards")
{
    mHWnd = hWnd;
    m_fileSystemLayer = new Ogre::FileSystemLayer("mewizards");
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string exeDir = exePath;
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        exeDir = exeDir.substr(0, lastSlash);
    }
    m_resourcePath = exeDir + "\\";
}

OgreApplication::~OgreApplication()
{
    if (mModelNode)
        mModelNode->getCreator()->destroySceneNode(mModelNode);

    if (mCameraNode)
        mCameraNode->getCreator()->destroySceneNode(mCameraNode);

    delete m_fileSystemLayer;
}

void OgreApplication::shutdown()
{
    OgreBites::ApplicationContext::shutdown();
}

void OgreApplication::setup()
{
    // do not forget to call the base first
    OgreBites::ApplicationContext::setup();
    addInputListener(this);

    // get a pointer to the already created root
    Ogre::Root* root = getRoot();
    Ogre::SceneManager* scnMgr = root->createSceneManager();

    // register our scene with the RTSS
    Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    // -- tutorial section start --
    //! [turnlights]
    scnMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
    //! [turnlights]

    //! [newlight]
    //Ogre::Light* light = scnMgr->createLight("MainLight");
    //Ogre::SceneNode* lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    //lightNode->attachObject(light);
    //! [newlight]

    //! [lightpos]
    //lightNode->setPosition(20, 80, 50);
    //! [lightpos]

    //! [camera]
    mCameraNode = scnMgr->getRootSceneNode()->createChildSceneNode();

    // create the camera
    Ogre::Camera* cam = scnMgr->createCamera("myCam");
    mCameraNode->attachObject(cam);
    mCameraNode->setPosition(0, 0, 15);
    cam->setNearClipDistance(5);
    cam->setFarClipDistance(10000);
    cam->setAutoAspectRatio(true);

    // and tell it to render into the main window
    getRenderWindow()->addViewport(cam);
    //! [camera]

    // cam->lookAt(Ogre::Vector3(0, 0, 0));

    // Set default values for mRotX and mRotY based on the initial camera position
    Ogre::Vector3 camPos = mCameraNode->getPosition();
    Ogre::Vector3 lookAtPos(0, 0, 0);
    Ogre::Vector3 dir = camPos - lookAtPos;

    mRotY = Ogre::Math::ATan2(dir.x, dir.z).valueDegrees();
    mRotX = -Ogre::Math::ASin(dir.y / dir.length()).valueDegrees();

    // Ensure the gltf codec plugin is loaded.
    Ogre::Root::getSingleton().loadPlugin("Codec_GLTF");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(m_resourcePath, "FileSystem", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();


    // Load the GLB model (call this after Ogre has been initialized and before the main loop starts)
    LoadGltf("character.glb");

}

bool OgreApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    bool ret = OgreBites::ApplicationContext::frameRenderingQueued(evt);

    //if (!processUnbufferedInput(evt))
    //    return false;

    return ret;
}

bool OgreApplication::keyPressed(const OgreBites::KeyboardEvent& evt)
{
    if (evt.keysym.sym == OgreBites::SDLK_ESCAPE)
    {
        getRoot()->queueEndRendering();
    }
    return true;
}

bool OgreApplication::keyReleased(const OgreBites::KeyboardEvent& evt)
{
    return true;
}

bool OgreApplication::mouseMoved(const OgreBites::MouseMotionEvent& evt)
{
    if (mDragging)
    {
        Ogre::Vector2 currentPos(evt.x, evt.y);
        Ogre::Vector2 delta = currentPos - mDragStart;
        mDragStart = currentPos;

        mRotY -= delta.x * 0.5f;
        mRotX -= delta.y * 0.5f;

        mRotX = std::max(-89.0f, std::min(89.0f, mRotX));

        Ogre::Quaternion yRot(Ogre::Degree(mRotY), Ogre::Vector3::UNIT_Y);
        Ogre::Quaternion xRot(Ogre::Degree(mRotX), Ogre::Vector3::UNIT_X);

        // Combine rotations: local X axis rotation, then global Y
        mCameraNode->setOrientation(yRot * xRot);
        mCameraNode->setPosition(yRot * xRot * Ogre::Vector3(0, 0, 15));
    }
    return true;
}

bool OgreApplication::mousePressed(const OgreBites::MouseButtonEvent& evt)
{
    if (evt.button == OgreBites::BUTTON_LEFT)
    {
        mDragging = true;
        mDragStart = Ogre::Vector2(evt.x, evt.y);
    }
    return true;
}

bool OgreApplication::mouseReleased(const OgreBites::MouseButtonEvent& evt)
{
    if (evt.button == OgreBites::BUTTON_LEFT)
    {
        mDragging = false;
    }
    return true;
}

void OgreApplication::LoadGltf(const std::string& path)
{
    Ogre::Root* root = getRoot();
    Ogre::SceneManager* scnMgr = root->getSceneManager("Default SceneManager");
    if (!scnMgr) {
        std::cerr << "SceneManager not found!" << std::endl;
        return;
    }
    if (mModelNode) {
        mModelNode->getCreator()->destroySceneNode(mModelNode);
        mModelNode = nullptr;
    }

    Ogre::MeshPtr meshV1;
    try {
        meshV1 = Ogre::MeshManager::getSingleton().load(
            path, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
            Ogre::HardwareBuffer::HBU_STATIC, Ogre::HardwareBuffer::HBU_STATIC);
    }
    catch (const Ogre::Exception& e) {
        std::cerr << "Failed to load mesh: " << e.getFullDescription() << std::endl;
        return;
    }

    // Create a v2 mesh to hold the submeshes
    Ogre::MeshPtr meshV2 = Ogre::MeshManager::getSingleton().createManual(path + "_v2", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    // Convert each v1 submesh to v2 and add it to the v2 mesh
    for (unsigned short i = 0; i < meshV1->getNumSubMeshes(); ++i) {
        Ogre::SubMesh* subMeshV1 = meshV1->getSubMesh(i);
        Ogre::SubMesh* subMeshV2 = meshV2->createSubMesh();
        // subMeshV2->cloneV1SubMesh(subMeshV1, true, true, meshV2.get());
    }

    // Set the bounds for the v2 mesh (optional but recommended)
    meshV2->_setBounds(meshV1->getBounds());
    meshV2->_setBoundingSphereRadius(meshV1->getBoundingSphereRadius());

    // Create an entity from the v2 mesh
    Ogre::Entity* entity = scnMgr->createEntity(meshV2);

    // Create a scene node and attach the entity
    mModelNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    mModelNode->attachObject(entity);

    Ogre::Real scaleFactor = 4.0f;
    mModelNode->setScale(scaleFactor, scaleFactor, scaleFactor);

    // Get the dimensions of the window
    RECT rect;
    GetClientRect(mHWnd, &rect);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    // Center the model
    mModelNode->setPosition(-mModelNode->getAttachedObject(0)->getBoundingBox().getCenter().x, -mModelNode->getAttachedObject(0)->getBoundingBox().getCenter().y, -mModelNode->getAttachedObject(0)->getBoundingBox().getCenter().z);
    mModelNode->setPosition(mModelNode->getPosition().x, mModelNode->getPosition().y, mModelNode->getPosition().z);

}
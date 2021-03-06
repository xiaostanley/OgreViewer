
#include "OgreMain.h"

using namespace Ogre;

COgreMain::COgreMain(void)
	: mRoot(NULL),
	mWindow(NULL),
	mFSLayer(NULL),
	mSceneMgr(NULL),
	mOverlaySystem(NULL),
	inputManager(NULL),
	mKeyboard(NULL),
	mMouse(NULL),
	cameraForward(false),
	cameraBackward(false),
	cameraLeft(false),
	cameraRight(false),
	cameraLeftTurn(false),
	cameraRightTurn(false),
	cameraSpeed(40.0f),
	boundaryVisble(true)
{
}

COgreMain::COgreMain(const Ogre::String& filePath, const Ogre::String& meshName, const Ogre::String& textureName)
	: mRoot(NULL),
	mWindow(NULL),
	mFSLayer(NULL),
	mSceneMgr(NULL),
	mOverlaySystem(NULL),
	inputManager(NULL),
	mKeyboard(NULL),
	mMouse(NULL),
	cameraForward(false),
	cameraBackward(false),
	cameraLeft(false),
	cameraRight(false),
	cameraLeftTurn(false),
	cameraRightTurn(false),
	cameraSpeed(40.0f),
	boundaryVisble(true)
{
	this->filePath = filePath;
	this->meshName = meshName;
	this->textureName = textureName;
}

COgreMain::~COgreMain(void)
{
}

void COgreMain::run(void)
{
	//创建场景
	createScene();

	//启动渲染
	mRoot->startRendering();

	//销毁场景
	destroyScene();
}

void COgreMain::createScene(void)
{
	mFSLayer = OGRE_NEW_T(Ogre::FileSystemLayer, Ogre::MEMCATEGORY_GENERAL)(OGRE_VERSION_NAME);

	createRoot();
	createWindow();
	createInput();
	loadResources();

	createSceneManager();
	createContent();
}

void COgreMain::destroyScene(void)
{
	destroyContent();
	destroySceneManager();

	unloadResources();
	destroyInput();
	destroyWindow();
	destroyRoot();

	OGRE_DELETE_T(mFSLayer, FileSystemLayer, Ogre::MEMCATEGORY_GENERAL);
}

bool COgreMain::frameRenderingQueued(const Ogre::FrameEvent & evt)
{
	//判断主窗口是否关闭
	if (mWindow->isClosed())
		return false;

	//获取键鼠输入
	mKeyboard->capture();
	mMouse->capture();

	if (mKeyboard->isKeyDown(OIS::KC_ESCAPE))
		return false;

	//视点控制
	if (cameraForward || cameraBackward || cameraLeft || cameraRight)
	{
		float heightCam = mainCameraView.getCameraNode()->_getDerivedPosition().y;
		cameraSpeed = 40.f + Ogre::Math::Abs(heightCam);

		//float cameraSpeed = 2.0f;// 2.0f
		float moveX = 0.0f;
		float moveY = 0.0f;
		float moveZ = 0.0f;
		if (cameraForward) moveZ = -cameraSpeed;
		if (cameraBackward) moveZ = cameraSpeed;
		if (cameraLeft) moveX = -cameraSpeed;
		if (cameraRight) moveX = cameraSpeed;

		mainCameraView.getCameraNode()->translate(moveX*evt.timeSinceLastFrame, 0.0f, moveZ*evt.timeSinceLastFrame, Ogre::Node::TS_LOCAL);
	}
	if (cameraLeftTurn || cameraRightTurn)
	{
		float aglYaw = 0.f;
		if (cameraLeftTurn)		aglYaw = -1.1f;
		if (cameraRightTurn)	aglYaw = 1.1f;

		mainCameraView.getCameraNode()->yaw(Ogre::Degree(aglYaw), Ogre::Node::TS_WORLD);
	}

	//mainCameraView.getCameraNode()->yaw(Ogre::Degree(-1.0f * (float)e.state.X.rel), Ogre::Node::TS_WORLD);
	//mainCameraView.getCamera()->lookAt(Ogre::Vector3::ZERO);

	// 显示帧率
	Ogre::OverlayElement* textArea = OverlayManager::getSingleton().getOverlayElement("FPS_TextArea");
	textArea->setCaption(Ogre::DisplayString(L"FPS:      ") + Ogre::DisplayString(Ogre::StringConverter::toString(mWindow->getBestFPS())));
	textArea = OverlayManager::getSingleton().getOverlayElement("Batch_TextArea");
	textArea->setCaption(Ogre::DisplayString(L"Batch:    ") + Ogre::DisplayString(Ogre::StringConverter::toString(mWindow->getBatchCount())));
	textArea = OverlayManager::getSingleton().getOverlayElement("Triangle_TextArea");
	textArea->setCaption(Ogre::DisplayString(L"Triangle: ") + Ogre::DisplayString(Ogre::StringConverter::toString(mWindow->getTriangleCount())));

	return true;
}

bool COgreMain::frameStarted(const Ogre::FrameEvent & evt)
{
	//获取输入
	mKeyboard->capture();
	mMouse->capture();

	//按KC_ESCAPE键退出
	if (mKeyboard->isKeyDown(OIS::KC_ESCAPE))
		return false;

	return true;
}

bool COgreMain::frameEnded(const Ogre::FrameEvent & evt)
{
	return true;
}

void COgreMain::createRoot(void)
{
	Ogre::String pluginsPath = "";
	pluginsPath = mFSLayer->getConfigFilePath("plugins.cfg");

	mRoot = NULL;
	mRoot = OGRE_NEW Ogre::Root(pluginsPath, "", "ogre.log");

#ifdef _GL_RENDER_SYSTEM_
	Ogre::RenderSystem* rsys = mRoot->getRenderSystemByName("OpenGL Rendering Subsystem");
#else
	Ogre::RenderSystem* rsys = mRoot->getRenderSystemByName("Direct3D9 Rendering Subsystem");
#endif
	mRoot->setRenderSystem(rsys);

	mRoot->initialise(false);
}

void COgreMain::destroyRoot(void)
{
	if (mRoot)
	{
		mRoot->saveConfig();

		OGRE_DELETE mRoot;
		mRoot = NULL;
	}
}

void COgreMain::createWindow(void)
{
	//create RenderWindow
	Ogre::NameValuePairList params;
	params["Colour Depth"] = "32";
	params["gamma"] = "Yes";
	params["FSAA"] = "4";
	params["vsync"] = "true";

	mWindow = mRoot->createRenderWindow("OgreMain", 1280, 760, false, &params);
	mRoot->addFrameListener(this);
}

void COgreMain::createInput(void)
{
	OIS::ParamList pl;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;

	//tell OIS about the Ogre window
	mWindow->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

	//setup the manager, keyboard and mouse to handle input
	inputManager = OIS::InputManager::createInputSystem(pl);
	mKeyboard = static_cast<OIS::Keyboard*>(inputManager->createInputObject(OIS::OISKeyboard, true));
	mMouse = static_cast<OIS::Mouse*>(inputManager->createInputObject(OIS::OISMouse, true));

	//tell OIS about the window's dimensions
	unsigned int width, height, depth;
	int top, left;
	mWindow->getMetrics(width, height, depth, left, top);
	const OIS::MouseState &ms = mMouse->getMouseState();
	ms.width = width;
	ms.height = height;

	//input events
	mKeyboard->setEventCallback(this);
	mMouse->setEventCallback(this);
}

void COgreMain::destroyInput(void)
{
	inputManager->destroyInputObject(mMouse);
	mMouse = NULL;
	inputManager->destroyInputObject(mKeyboard);
	mKeyboard = NULL;
	OIS::InputManager::destroyInputSystem(inputManager);
	inputManager = NULL;
}

void COgreMain::destroyWindow(void)
{
	mWindow = NULL;
}

void COgreMain::loadResources(void)
{
	Ogre::String resourcePath = "";
	resourcePath = mFSLayer->getConfigFilePath("resources.cfg");

	Ogre::ConfigFile config;
	config.load(resourcePath);

	Ogre::ConfigFile::SectionIterator seci = config.getSectionIterator();
	Ogre::String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);
		}
	}

	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	// 增加资源路径
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(filePath + "com", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(filePath + "modtex", "FileSystem");
	//Ogre::ResourceGroupManager::getSingleton().addResourceLocation(filePath + "sim", "FileSystem");
}

void COgreMain::unloadResources(void)
{
	Ogre::ResourceGroupManager::ResourceManagerIterator seci =
		Ogre::ResourceGroupManager::getSingleton().getResourceManagerIterator();
	while (seci.hasMoreElements())
	{
		seci.getNext()->unloadUnreferencedResources();
	}
}

void COgreMain::createSceneManager(void)
{
	//场景管理器
	mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);

	//平面管理器
	mOverlaySystem = new Ogre::OverlaySystem();
	mSceneMgr->addRenderQueueListener(mOverlaySystem);
}

void COgreMain::destroySceneManager(void)
{
	//删除平面管理器
	delete mOverlaySystem;
	mOverlaySystem = NULL;

	//删除场景管理器
	mRoot->destroySceneManager(mSceneMgr);
	mSceneMgr = NULL;
}

void COgreMain::createContent(void)
{
	// 显示坐标轴
	Ogre::Entity* axis = mSceneMgr->createEntity("axis", "Axis.mesh");
	Ogre::SceneNode* nodeAxis = mSceneMgr->getRootSceneNode()->createChildSceneNode("nodeAxis");
	nodeAxis->attachObject(axis);
	nodeAxis->setScale(50.f, 50.f, 50.f);

	// 创建摄像头
	mainCameraView.createCameraNode(mSceneMgr, "MainCamera", 10.f, 99999 * 6.0f);	// 10 10000

	// 设置主要摄像头
	Ogre::Vector3 mainCameraPos;
	mainCameraPos.x = 0.f;
	mainCameraPos.y = 0.f;
	mainCameraPos.z = 0.f;
	mainCameraView.getCameraNode()->setPosition(mainCameraPos);

	// 创建视口
	mainCameraView.setViewportRect(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1, Ogre::ColourValue(0.0f, 0.0f, 0.0f), true);
	mainCameraView.createViewport(mWindow);

	///////////////////////////////////////////////////////////////////////////////////////////////

	// 加载字体
	Ogre::FontPtr font = Ogre::FontManager::getSingleton().create("SimHeiFont", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	font->setSource("simhei.ttf");
	font->setType(FT_TRUETYPE);
	font->setTrueTypeSize(32);
	font->setTrueTypeResolution(96);
	font->addCodePointRange(Ogre::Font::CodePointRange(33, 166));
	font->load();

	// 创建overlay
	mainOverlay = Ogre::OverlayManager::getSingleton().create("mainOverlay");
	mainOverlay->setZOrder(254);
	mainOverlay->show();

	// 创建OverlayContainer
	Ogre::OverlayContainer* ovContainer = static_cast<Ogre::OverlayContainer*>(Ogre::OverlayManager::getSingleton().createOverlayElement("BorderPanel", "Container"));
	ovContainer->setMetricsMode(GMM_RELATIVE);
	ovContainer->setPosition(0.f, 0.f);
	ovContainer->setWidth(0.25f);
	ovContainer->setHeight(0.14f);
	ovContainer->setMaterialName("Core/StatsBlockCenter");
	mainOverlay->add2D(ovContainer);

	// 创建文字显示区域
	Ogre::OverlayElement* textArea = OverlayManager::getSingleton().createOverlayElement("TextArea", "FPS_TextArea");
	textArea->setMetricsMode(GMM_RELATIVE);
	textArea->setPosition(0.01f, 0.01f);
	textArea->setWidth(0.2f);
	textArea->setHeight(0.1f);
	textArea->setParameter("font_name", "SimHeiFont");
	textArea->setParameter("char_height", Ogre::StringConverter::toString(Ogre::Real(0.05f)));
	textArea->setParameter("horz_align", "left");
	textArea->setColour(Ogre::ColourValue(1.f, 1.f, 1.f));
	textArea->setCaption(Ogre::DisplayString("FPS"));
	ovContainer->addChild(textArea);

	textArea = OverlayManager::getSingleton().createOverlayElement("TextArea", "Batch_TextArea");
	textArea->setMetricsMode(GMM_RELATIVE);
	textArea->setPosition(0.01f, 0.05f);
	textArea->setWidth(0.2f);
	textArea->setHeight(0.1f);
	textArea->setParameter("font_name", "SimHeiFont");
	textArea->setParameter("char_height", Ogre::StringConverter::toString(Ogre::Real(0.05f)));
	textArea->setParameter("horz_align", "left");
	textArea->setColour(Ogre::ColourValue(1.f, 1.f, 1.f));
	textArea->setCaption(Ogre::DisplayString("Batch"));
	ovContainer->addChild(textArea);

	textArea = OverlayManager::getSingleton().createOverlayElement("TextArea", "Triangle_TextArea");
	textArea->setMetricsMode(GMM_RELATIVE);
	textArea->setPosition(0.01f, 0.09f);
	textArea->setWidth(0.2f);
	textArea->setHeight(0.1f);
	textArea->setParameter("font_name", "SimHeiFont");
	textArea->setParameter("char_height", Ogre::StringConverter::toString(Ogre::Real(0.05f)));
	textArea->setParameter("horz_align", "left");
	textArea->setColour(Ogre::ColourValue(1.f, 1.f, 1.f));
	textArea->setCaption(Ogre::DisplayString("Triangle"));
	ovContainer->addChild(textArea);

	///////////////////////////////////////////////////////////////////////////////////////////////

	//设置阴影及环境光
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.05f, 0.05f, 0.05f));

	//太阳光（平行光）
	Ogre::Light* sunlight = mSceneMgr->createLight("sunlight");
	sunlight->setType(Ogre::Light::LT_DIRECTIONAL);

	// 创建材质
	MaterialPtr materialPtr = MaterialManager::getSingleton().create("TerrainBase", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Pass* pass = materialPtr->getTechnique(0)->getPass(0);
	pass->setLightingEnabled(false);
	TextureUnitState* tex = pass->createTextureUnitState(textureName, 0);
	tex->setTextureName(textureName);

	// 显示地形模型
	Entity* entTerra = mSceneMgr->createEntity("entTerra", meshName);
	entTerra->setMaterial(materialPtr);

	entTerra->getSubEntity(1)->setVisible(boundaryVisble);
	entTerra->getSubEntity(2)->setVisible(boundaryVisble);
	entTerra->getSubEntity(3)->setVisible(boundaryVisble);
	entTerra->getSubEntity(4)->setVisible(boundaryVisble);
	entTerra->getSubEntity(5)->setVisible(!boundaryVisble);
	entTerra->getSubEntity(6)->setVisible(!boundaryVisble);
	entTerra->getSubEntity(7)->setVisible(!boundaryVisble);
	entTerra->getSubEntity(8)->setVisible(!boundaryVisble);
	SceneNode* nodeTerraBase = mSceneMgr->createSceneNode("nodeTerraBase");
	nodeTerraBase->attachObject(entTerra);
	nodeTerraBase->setPosition(-entTerra->getBoundingBox().getCenter());
	SceneNode* nodeTerra = mSceneMgr->getRootSceneNode()->createChildSceneNode("nodeTerra");
	nodeTerra->addChild(nodeTerraBase);
	nodeTerra->setScale(0.06f, 0.12f, 0.06f);
}

void COgreMain::destroyContent(void)
{
	mainCameraView.destroyViewport(mWindow);
	mainCameraView.destroyCameraNode(mSceneMgr);
}

bool COgreMain::keyPressed(const OIS::KeyEvent & e)
{
	//渲染模式切换
	if (e.key == OIS::KC_R)
	{
		Ogre::PolygonMode pm;
		pm = mainCameraView.getCamera()->getPolygonMode();

		if (pm == Ogre::PM_SOLID)
			mainCameraView.getCamera()->setPolygonMode(Ogre::PM_WIREFRAME);
		else if (pm == Ogre::PM_WIREFRAME)
			mainCameraView.getCamera()->setPolygonMode(Ogre::PM_POINTS);
		else
			mainCameraView.getCamera()->setPolygonMode(Ogre::PM_SOLID);
	}

	//视点控制
	if (e.key == OIS::KC_W)
	{
		cameraForward = true;
	}
	else if (e.key == OIS::KC_S)
	{
		cameraBackward = true;
	}
	else if (e.key == OIS::KC_A)
	{
		cameraLeft = true;
	}
	else if (e.key == OIS::KC_D)
	{
		cameraRight = true;
	}
	else if (e.key == OIS::KC_LEFT)
	{
		cameraLeftTurn = true;
	}
	else if (e.key == OIS::KC_RIGHT)
	{
		cameraRightTurn = true;
	}

	// 切换FPS界面显示
	if (e.key == OIS::KC_Y)
	{
		if (mainOverlay->isVisible())
			mainOverlay->hide();
		else
			mainOverlay->show();
	}

	// 切换边界
	if (e.key == OIS::KC_P)
	{
		Entity* entTerra = mSceneMgr->getEntity("entTerra");
		boundaryVisble = !boundaryVisble;
		entTerra->getSubEntity(1)->setVisible(boundaryVisble);
		entTerra->getSubEntity(2)->setVisible(boundaryVisble);
		entTerra->getSubEntity(3)->setVisible(boundaryVisble);
		entTerra->getSubEntity(4)->setVisible(boundaryVisble);
		entTerra->getSubEntity(5)->setVisible(!boundaryVisble);
		entTerra->getSubEntity(6)->setVisible(!boundaryVisble);
		entTerra->getSubEntity(7)->setVisible(!boundaryVisble);
		entTerra->getSubEntity(8)->setVisible(!boundaryVisble);
	}

	return true;
}

bool COgreMain::keyReleased(const OIS::KeyEvent & e)
{
	//视点控制
	if (e.key == OIS::KC_W)
	{
		cameraForward = false;
	}
	else if (e.key == OIS::KC_S)
	{
		cameraBackward = false;
	}
	else if (e.key == OIS::KC_A)
	{
		cameraLeft = false;
	}
	else if (e.key == OIS::KC_D)
	{
		cameraRight = false;
	}
	else if (e.key == OIS::KC_LEFT)
	{
		cameraLeftTurn = false;
	}
	else if (e.key == OIS::KC_RIGHT)
	{
		cameraRightTurn = false;
	}

	return true;
}

bool COgreMain::mouseMoved(const OIS::MouseEvent & e)
{
	//视点控制
	mainCameraView.getCameraNode()->yaw(Ogre::Degree(-1.0f * (float)e.state.X.rel), Ogre::Node::TS_WORLD);
	mainCameraView.getCameraNode()->pitch(Ogre::Degree(-1.0f * (float)e.state.Y.rel), Ogre::Node::TS_LOCAL);

	return true;
}

bool COgreMain::mousePressed(const OIS::MouseEvent & e, OIS::MouseButtonID id)
{
	return true;
}

bool COgreMain::mouseReleased(const OIS::MouseEvent & e, OIS::MouseButtonID id)
{
	return true;
}


#include "AssetManager.h"

#include "GlobalEnum.h"
#include "Globals.h"
#include "ScreenObject.h"

#include "cocos2d.h"
USING_NS_CC;

#include <sstream>

#define FAIL_RETURN_FALSE  { assert(false); return false; }

bool AssetManager::LoadSprites(const char* spriteListFile)
{
	//  Grab the XML Wrapper instance and load the file
	XMLWrapper* xmlWrapper = XMLWrapper::Get_Instance();
	const RapidXML_Doc* xmlDoc = xmlWrapper->LoadXMLFile(spriteListFile);

	//  Find the root and entry nodes
	const RapidXML_Node* rootNode = xmlDoc->first_node("SpriteList");
	const RapidXML_Node* entryNode = rootNode->first_node("Sprite");

	//  Define some helper variables
	const char* gameFlag = NULL;
	const RapidXML_Attribute* entryAttribute1 = NULL;
	const RapidXML_Attribute* entryAttribute2 = NULL;
	const RapidXML_Attribute* entryAttribute3 = NULL;
	const RapidXML_Attribute* entryAttribute4 = NULL;
	const RapidXML_Attribute* entryAttribute5 = NULL;
	const RapidXML_Attribute* entryAttribute6 = NULL;
	const RapidXML_Attribute* entryAttribute7 = NULL;
	const RapidXML_Node* spriteFrameNode = NULL;

	//  Cycle through the entries in the list
	do {
		gameFlag = ((entryAttribute1 = entryNode->first_attribute("gameflag")) != NULL) ? entryAttribute1->value() : "";
		if (GameFlagActiveCheck(gameFlag))
		{
			//  Grab the name, ensure there is one
			entryAttribute1 = entryNode->first_attribute("name");
			if (entryAttribute1 == NULL) FAIL_RETURN_FALSE;

			//  Add the data to the proper list, ensuring it doesn't already exist
			if (SpriteList.find(entryAttribute1->value()) != SpriteList.end()) FAIL_RETURN_FALSE;
			SpriteData* newSprite = new SpriteData(entryAttribute1->value());
			SpriteList[entryAttribute1->value()] = newSprite;

			//  Grab the file, ensure there is one
			entryAttribute2 = entryNode->first_attribute("file");
			if ((entryAttribute2 != NULL)) newSprite->AddImageFrame(FileUtils::getInstance()->fullPathForFilename(entryAttribute2->value()));
			else
			{
				//  Grab the list flag, if there is one
				entryAttribute3 = entryNode->first_attribute("framecount");
				if (entryAttribute3 != NULL)
				{
					entryAttribute4 = entryNode->first_attribute("framerate");
					if (entryAttribute4 == NULL) FAIL_RETURN_FALSE;
					newSprite->SetFrameRate(atoi(entryAttribute4->value()));

					entryAttribute5 = entryNode->first_attribute("width");
					if (entryAttribute5 == NULL) FAIL_RETURN_FALSE;

					entryAttribute6 = entryNode->first_attribute("height");
					if (entryAttribute6 == NULL) FAIL_RETURN_FALSE;

					newSprite->setWidth(atoi(entryAttribute5->value()));
					newSprite->setHeight(atoi(entryAttribute6->value()));

					spriteFrameNode = entryNode->first_node("Image");
					do {
						entryAttribute7 = spriteFrameNode->first_attribute("file");
						if (entryAttribute7 == NULL) FAIL_RETURN_FALSE;
						newSprite->AddImageFrame(entryAttribute7->value());
					} while ((spriteFrameNode = spriteFrameNode->next_sibling("Image")) != NULL);
				}
			}
		}
	} while	((entryNode = entryNode->next_sibling("Sprite")) != NULL);

	return true;
}

bool AssetManager::LoadFonts(const char* fontListFile)
{
	//  Grab the XML Wrapper instance and load the file
	XMLWrapper* xmlWrapper = XMLWrapper::Get_Instance();
	const RapidXML_Doc* xmlDoc = xmlWrapper->LoadXMLFile(fontListFile);

	//  Find the root and entry nodes
	const RapidXML_Node* rootNode = xmlDoc->first_node("FontList");
	const RapidXML_Node* entryNode = rootNode->first_node("Font");

	//  Define some helper variables
	const char* gameFlag = NULL;
	const RapidXML_Attribute* entryAttribute1 = NULL;
	const RapidXML_Attribute* entryAttribute2 = NULL;

	//  Cycle through the entries in the list
	do {
		gameFlag = ((entryAttribute1 = entryNode->first_attribute("gameflag")) != NULL) ? entryAttribute1->value() : "";
		if (GameFlagActiveCheck(gameFlag))
		{
			//  Grab the file, ensure there is one
			entryAttribute1 = entryNode->first_attribute("file");
			if (entryAttribute1 == NULL) FAIL_RETURN_FALSE;

			//  Grab the name, ensure there is one
			entryAttribute2 = entryNode->first_attribute("name");
			if (entryAttribute2 == NULL) FAIL_RETURN_FALSE;

			//  Add the data to the proper list, ensuring it doesn't already exist
			std::string fontIdentifier = "Root.UI_Basic.Fonts." + std::string(entryAttribute2->value());
			if (FontList.find(fontIdentifier) != FontList.end()) FAIL_RETURN_FALSE;
			FontList[fontIdentifier] = entryAttribute1->value();
		}
	} while ((entryNode = entryNode->next_sibling("Font")) != NULL);

	return true;
}

bool AssetManager::LoadAnimations(const char* animationListFile)
{
	//  Grab the XML Wrapper instance and load the file
	XMLWrapper* xmlWrapper = XMLWrapper::Get_Instance();
	const RapidXML_Doc* xmlDoc = xmlWrapper->LoadXMLFile(animationListFile);

	//  Find the root and entry nodes
	const RapidXML_Node* rootNode = xmlDoc->first_node("AnimationList");
	const RapidXML_Node* entryNode = rootNode->first_node("Animation");

	//  Define some helper variables
	const char* gameFlag = NULL;
	const RapidXML_Attribute* entryAttribute1 = NULL;
	const RapidXML_Attribute* entryAttribute2 = NULL;
	const RapidXML_Attribute* entryAttribute3 = NULL;
	const RapidXML_Attribute* entryAttribute4 = NULL;
	const RapidXML_Attribute* entryAttribute5 = NULL;
	const RapidXML_Attribute* entryAttribute6 = NULL;
	const RapidXML_Attribute* entryAttribute7 = NULL;
	const RapidXML_Attribute* entryAttribute8 = NULL;
	float t = 0.0f;
	float x = 0.0f;
	float y = 0.0f;
	float sx = 0.0f;
	float sy = 0.0f;
	float r = 0.0f;
	float a = 0.0f;
	const RapidXML_Node* keyframeNode = NULL;

	//  Cycle through the entries in the list
	do {
		gameFlag = ((entryAttribute1 = entryNode->first_attribute("gameflag")) != NULL) ? entryAttribute1->value() : "";
		if (GameFlagActiveCheck(gameFlag))
		{
			//  Grab the name, ensure there is one
			entryAttribute1 = entryNode->first_attribute("name");
			if (entryAttribute1 == NULL) FAIL_RETURN_FALSE;

			if (AnimationList.find(entryAttribute1->value()) != AnimationList.end()) FAIL_RETURN_FALSE;
			BasicAnimation* newAnim = new BasicAnimation(entryAttribute1->value());
			AnimationList[entryAttribute1->value()] = newAnim;

			float lastTime = 0.0f;
			float currentX = 0.0f;
			float currentY = 0.0f;
			float lastScaleX = 0.0f;
			float lastScaleY = 0.0f;

			//  Cycle through the entries in the secondary list
			keyframeNode = entryNode->first_node("Keyframe");
			do {
				//  Grab the time, ensure there is one
				entryAttribute2 = keyframeNode->first_attribute("t");
				if (entryAttribute2 == NULL) FAIL_RETURN_FALSE;
				t = atof(entryAttribute2->value()) - lastTime;
				lastTime += t;

				//  Grab the X position, ensure there is one
				entryAttribute3 = keyframeNode->first_attribute("x");
				if (entryAttribute3 == NULL) FAIL_RETURN_FALSE;
				x = atof(entryAttribute3->value()) - currentX;
				currentX += x;

				//  Grab the Y position, ensure there is one
				entryAttribute4 = keyframeNode->first_attribute("y");
				if (entryAttribute4 == NULL) FAIL_RETURN_FALSE;
				y = -atof(entryAttribute4->value()) - currentY;
				currentY += y;

				//  Grab the X Scale, ensure there is one
				entryAttribute5 = keyframeNode->first_attribute("sx");
				if (entryAttribute5 == NULL) FAIL_RETURN_FALSE;
				sx = 1.0f + atof(entryAttribute5->value());

				//  Grab the Y Scale, ensure there is one
				entryAttribute6 = keyframeNode->first_attribute("sy");
				if (entryAttribute6 == NULL) FAIL_RETURN_FALSE;
				sy = 1.0f + atof(entryAttribute6->value());

				//  Grab the rotation, ensure there is one
				entryAttribute7 = keyframeNode->first_attribute("r");
				if (entryAttribute7 == NULL) FAIL_RETURN_FALSE;
				r = atof(entryAttribute7->value());

				//  Grab the alpha, ensure there is one
				entryAttribute8 = keyframeNode->first_attribute("a");
				if (entryAttribute8 == NULL) FAIL_RETURN_FALSE;
				a = atof(entryAttribute8->value());

				newAnim->AddKeyframe(BasicAnimation::BasicKeyframe(t, x, y, sx, sy, r, a));
			} while ((keyframeNode = keyframeNode->next_sibling("Keyframe")) != NULL);
		}
	} while ((entryNode = entryNode->next_sibling("Animation")) != NULL);

	return true;
}

bool AssetManager::LoadParticles(const char* particleListFile)
{
	//  Grab the XML Wrapper instance and load the file
	XMLWrapper* xmlWrapper = XMLWrapper::Get_Instance();
	const RapidXML_Doc* xmlDoc = xmlWrapper->LoadXMLFile(particleListFile);

	//  Find the root and entry nodes
	const RapidXML_Node* rootNode = xmlDoc->first_node("ParticleList");
	const RapidXML_Node* entryNode = rootNode->first_node("Particle");

	//  Define some helper variables
	const char* gameFlag = NULL;
	const RapidXML_Attribute* entryAttribute1 = NULL;
	const RapidXML_Attribute* entryAttribute2 = NULL;
	const RapidXML_Attribute* entryAttribute3 = NULL;
	const RapidXML_Attribute* entryAttribute4 = NULL;
	const RapidXML_Attribute* entryAttribute5 = NULL;
	const RapidXML_Attribute* entryAttribute6 = NULL;
	const RapidXML_Attribute* entryAttribute7 = NULL;
	const RapidXML_Attribute* entryAttribute8 = NULL;
	const RapidXML_Attribute* entryAttribute9 = NULL;
	const RapidXML_Attribute* entryAttribute10 = NULL;
	const RapidXML_Attribute* entryAttribute11 = NULL;
	const RapidXML_Attribute* entryAttribute12 = NULL;
	const RapidXML_Attribute* entryAttribute13 = NULL;
	const RapidXML_Attribute* entryAttribute14 = NULL;
	const RapidXML_Attribute* entryAttribute15 = NULL;
	const RapidXML_Attribute* entryAttribute16 = NULL;
	const RapidXML_Attribute* entryAttribute17 = NULL;
	const RapidXML_Attribute* entryAttribute18 = NULL;
	const RapidXML_Attribute* entryAttribute19 = NULL;
	const RapidXML_Attribute* entryAttribute20 = NULL;
	const RapidXML_Attribute* entryAttribute21 = NULL;
	const RapidXML_Attribute* entryAttribute22 = NULL;
	const RapidXML_Attribute* entryAttribute23 = NULL;
	const RapidXML_Attribute* entryAttribute24 = NULL;
	const RapidXML_Attribute* entryAttribute25 = NULL;
	const RapidXML_Attribute* entryAttribute26 = NULL;
	const RapidXML_Attribute* entryAttribute27 = NULL;
	const RapidXML_Attribute* entryAttribute28 = NULL;
	const RapidXML_Attribute* entryAttribute29 = NULL;
	const RapidXML_Attribute* entryAttribute30 = NULL;
	const RapidXML_Attribute* entryAttribute31 = NULL;
	const RapidXML_Attribute* entryAttribute32 = NULL;
	const RapidXML_Attribute* entryAttribute33 = NULL;
	const RapidXML_Attribute* entryAttribute34 = NULL;
	const RapidXML_Attribute* entryAttribute35 = NULL;
	const RapidXML_Attribute* entryAttribute36 = NULL;
	const RapidXML_Attribute* entryAttribute37 = NULL;
	const RapidXML_Attribute* entryAttribute38 = NULL;

	//  Cycle through the entries in the list
	do {
		gameFlag = ((entryAttribute1 = entryNode->first_attribute("gameflag")) != NULL) ? entryAttribute1->value() : "";
		if (GameFlagActiveCheck(gameFlag))
		{
			//  Grab the file, ensure there is one
			entryAttribute1 = entryNode->first_attribute("file");
			if (entryAttribute1 == NULL) FAIL_RETURN_FALSE;

			//  Grab the name, ensure there is one
			entryAttribute2 = entryNode->first_attribute("duration");
			if (entryAttribute2 == NULL) FAIL_RETURN_FALSE;

			//  Grab the gravity X and Y, ensure they exist
			entryAttribute3 = entryNode->first_attribute("gravity_x");
			if (entryAttribute3 == NULL) FAIL_RETURN_FALSE;
			entryAttribute4 = entryNode->first_attribute("gravity_y");
			if (entryAttribute4 == NULL) FAIL_RETURN_FALSE;

			//  Grab the speed and it's variance, ensure they exist
			entryAttribute5 = entryNode->first_attribute("speed");
			if (entryAttribute5 == NULL) FAIL_RETURN_FALSE;
			entryAttribute6 = entryNode->first_attribute("speedvar");
			if (entryAttribute6 == NULL) FAIL_RETURN_FALSE;

			//  Grab the angle and it's variance, ensure they exist
			entryAttribute7 = entryNode->first_attribute("angle");
			if (entryAttribute7 == NULL) FAIL_RETURN_FALSE;
			entryAttribute8 = entryNode->first_attribute("anglevar");
			if (entryAttribute8 == NULL) FAIL_RETURN_FALSE;

			//  Grab the lifetime and it's variance, ensure they exist
			entryAttribute9 = entryNode->first_attribute("life");
			if (entryAttribute9 == NULL) FAIL_RETURN_FALSE;
			entryAttribute10 = entryNode->first_attribute("lifevar");
			if (entryAttribute10 == NULL) FAIL_RETURN_FALSE;

			//  Grab the count, ensure there is one
			entryAttribute11 = entryNode->first_attribute("count");
			if (entryAttribute11 == NULL) FAIL_RETURN_FALSE;

			//  Grab the name, ensure there is one
			entryAttribute12 = entryNode->first_attribute("name");
			if (entryAttribute12 == NULL) FAIL_RETURN_FALSE;

			//  Grab the start color and it's variance, ensure they exist
			entryAttribute13 = entryNode->first_attribute("startR");
			if (entryAttribute13 == NULL) FAIL_RETURN_FALSE;
			entryAttribute14 = entryNode->first_attribute("startRvar");
			if (entryAttribute14 == NULL) FAIL_RETURN_FALSE;
			entryAttribute15 = entryNode->first_attribute("startG");
			if (entryAttribute15 == NULL) FAIL_RETURN_FALSE;
			entryAttribute16 = entryNode->first_attribute("startGvar");
			if (entryAttribute16 == NULL) FAIL_RETURN_FALSE;
			entryAttribute17 = entryNode->first_attribute("startB");
			if (entryAttribute17 == NULL) FAIL_RETURN_FALSE;
			entryAttribute18 = entryNode->first_attribute("startBvar");
			if (entryAttribute18 == NULL) FAIL_RETURN_FALSE;
			entryAttribute19 = entryNode->first_attribute("startA");
			if (entryAttribute19 == NULL) FAIL_RETURN_FALSE;
			entryAttribute20 = entryNode->first_attribute("startAvar");
			if (entryAttribute20 == NULL) FAIL_RETURN_FALSE;

			//  Grab the end color and it's variance, ensure they exist
			entryAttribute21 = entryNode->first_attribute("endR");
			if (entryAttribute21 == NULL) FAIL_RETURN_FALSE;
			entryAttribute22 = entryNode->first_attribute("endRvar");
			if (entryAttribute22 == NULL) FAIL_RETURN_FALSE;
			entryAttribute23 = entryNode->first_attribute("endG");
			if (entryAttribute23 == NULL) FAIL_RETURN_FALSE;
			entryAttribute24 = entryNode->first_attribute("endGvar");
			if (entryAttribute24 == NULL) FAIL_RETURN_FALSE;
			entryAttribute25 = entryNode->first_attribute("endB");
			if (entryAttribute25 == NULL) FAIL_RETURN_FALSE;
			entryAttribute26 = entryNode->first_attribute("endBvar");
			if (entryAttribute26 == NULL) FAIL_RETURN_FALSE;
			entryAttribute27 = entryNode->first_attribute("endA");
			if (entryAttribute27 == NULL) FAIL_RETURN_FALSE;
			entryAttribute28 = entryNode->first_attribute("endAvar");
			if (entryAttribute28 == NULL) FAIL_RETURN_FALSE;

			//  Grab the start size and it's variance, ensure they exist
			entryAttribute29 = entryNode->first_attribute("startSize");
			if (entryAttribute29 == NULL) FAIL_RETURN_FALSE;
			entryAttribute30 = entryNode->first_attribute("startSizeVar");
			if (entryAttribute30 == NULL) FAIL_RETURN_FALSE;

			//  Grab the end size and it's variance, ensure they exist
			entryAttribute31 = entryNode->first_attribute("endSize");
			if (entryAttribute31 == NULL) FAIL_RETURN_FALSE;
			entryAttribute32 = entryNode->first_attribute("endSizeVar");
			if (entryAttribute32 == NULL) FAIL_RETURN_FALSE;

			//  Grab the start spin and it's variance, ensure they exist
			entryAttribute33 = entryNode->first_attribute("startSpin");
			if (entryAttribute33 == NULL) FAIL_RETURN_FALSE;
			entryAttribute34 = entryNode->first_attribute("startSpinVar");
			if (entryAttribute34 == NULL) FAIL_RETURN_FALSE;

			//  Grab the end spin and it's variance, ensure they exist
			entryAttribute35 = entryNode->first_attribute("endSpin");
			if (entryAttribute35 == NULL) FAIL_RETURN_FALSE;
			entryAttribute36 = entryNode->first_attribute("endSpinVar");
			if (entryAttribute36 == NULL) FAIL_RETURN_FALSE;

			//  Grab the width and height, ensure they exist
			entryAttribute37 = entryNode->first_attribute("width");
			if (entryAttribute37 == NULL) FAIL_RETURN_FALSE;
			entryAttribute38 = entryNode->first_attribute("height");
			if (entryAttribute38 == NULL) FAIL_RETURN_FALSE;

			//  Add the data to the proper list, ensuring it doesn't already exist
			std::string particleIdentifier = "Root.Animations." + std::string(entryAttribute12->value());
			if (ParticleList.find(particleIdentifier) != ParticleList.end()) FAIL_RETURN_FALSE;
			ParticleList[particleIdentifier] = new ParticleInfo();

			//  Set all particle info data
            std::string nonTemporaryString;
            char* nonTemporaryChars1;
            char* nonTemporaryChars2;
            char* nonTemporaryChars3;
            char* nonTemporaryChars4;
            Color4F nonTemporaryColor;
			ParticleList[particleIdentifier]->setTextureFile(nonTemporaryString = entryAttribute1->value());
			ParticleList[particleIdentifier]->setDuration(atof(entryAttribute2->value()));
			ParticleList[particleIdentifier]->setGravity(Vec2(atof(entryAttribute3->value()), atof(entryAttribute4->value())));
			ParticleList[particleIdentifier]->setSpeed(atof(entryAttribute5->value()));
			ParticleList[particleIdentifier]->setSpeedVariance(atof(entryAttribute6->value()));
			ParticleList[particleIdentifier]->setAngle(atof(entryAttribute7->value()));
			ParticleList[particleIdentifier]->setAngleVariance(atof(entryAttribute8->value()));
			ParticleList[particleIdentifier]->setLifetime(atof(entryAttribute9->value()));
			ParticleList[particleIdentifier]->setLifetimeVariance(atof(entryAttribute10->value()));
			ParticleList[particleIdentifier]->setCount(atoi(entryAttribute11->value()));
			ParticleList[particleIdentifier]->setName(nonTemporaryString = entryAttribute12->value());
			ParticleList[particleIdentifier]->setStartColor(nonTemporaryColor = Color4F(atof(nonTemporaryChars1 = entryAttribute13->value()), atof(nonTemporaryChars2 = entryAttribute15->value()), atof(nonTemporaryChars3 = entryAttribute17->value()), atof(nonTemporaryChars4 = entryAttribute19->value())));
			ParticleList[particleIdentifier]->setStartColorVariance(nonTemporaryColor = Color4F(atof(nonTemporaryChars1 = entryAttribute14->value()), atof(nonTemporaryChars2 = entryAttribute16->value()), atof(nonTemporaryChars3 = entryAttribute18->value()), atof(nonTemporaryChars4 = entryAttribute20->value())));
			ParticleList[particleIdentifier]->setEndColor(nonTemporaryColor = Color4F(atof(nonTemporaryChars1 = entryAttribute21->value()), atof(nonTemporaryChars2 = entryAttribute23->value()), atof(nonTemporaryChars3 = entryAttribute25->value()), atof(nonTemporaryChars4 = entryAttribute27->value())));
			ParticleList[particleIdentifier]->setEndColorVariance(nonTemporaryColor = Color4F(atof(nonTemporaryChars1 = entryAttribute22->value()), atof(nonTemporaryChars2 = entryAttribute24->value()), atof(nonTemporaryChars3 = entryAttribute26->value()), atof(nonTemporaryChars4 = entryAttribute28->value())));
			ParticleList[particleIdentifier]->setStartSize(atof(entryAttribute29->value()));
			ParticleList[particleIdentifier]->setStartSizeVariance(atof(entryAttribute30->value()));
			ParticleList[particleIdentifier]->setEndSize(atof(entryAttribute31->value()));
			ParticleList[particleIdentifier]->setEndSizeVariance(atof(entryAttribute32->value()));
			ParticleList[particleIdentifier]->setStartSpin(atof(entryAttribute33->value()));
			ParticleList[particleIdentifier]->setStartSpinVariance(atof(entryAttribute34->value()));
			ParticleList[particleIdentifier]->setEndSpin(atof(entryAttribute35->value()));
			ParticleList[particleIdentifier]->setEndSpinVariance(atof(entryAttribute36->value()));
			ParticleList[particleIdentifier]->setArea(Vec2(atof(entryAttribute37->value()), atof(entryAttribute38->value())));
		}
	} while ((entryNode = entryNode->next_sibling("Particle")) != NULL);

	return true;
}

bool AssetManager::LoadLayout(const char* layoutFile, cocos2d::Node* sceneLayer)
{
	//  Grab the XML Wrapper instance and load the file
	XMLWrapper* xmlWrapper = XMLWrapper::Get_Instance();
	const RapidXML_Doc* xmlDoc = xmlWrapper->LoadXMLFile(layoutFile);

	//  Find the root and entry nodes
	const RapidXML_Node* rootNode = xmlDoc->first_node("Layout");
	const RapidXML_Node* entryNode = rootNode->first_node();

	bool childrenLoaded = true;
	do {
		childrenLoaded &= LoadLayoutNode("Root.", sceneLayer, entryNode);
	} while (childrenLoaded && (entryNode = entryNode->next_sibling()) != NULL);

	sceneLayer->setCascadeOpacityEnabled(true);

	return childrenLoaded;
}

bool AssetManager::LoadLayoutNode(std::string lookup, cocos2d::Node* parent, const RapidXML_Node* entryNode)
{
	if (entryNode == NULL) FAIL_RETURN_FALSE;

	const char* nodeType = NULL;
	const char* gameFlag = NULL;
	std::string newLookup = lookup;
	cocos2d::Node* newNode = NULL;
	const RapidXML_Attribute* entryAttribute1 = NULL;
	const RapidXML_Attribute* entryAttribute2 = NULL;
	const RapidXML_Attribute* entryAttribute3 = NULL;
	const RapidXML_Attribute* entryAttribute4 = NULL;
	const RapidXML_Attribute* entryAttribute5 = NULL;
	const RapidXML_Attribute* entryAttribute6 = NULL;
	const RapidXML_Attribute* entryAttribute7 = NULL;
	const RapidXML_Attribute* entryAttribute8 = NULL;
	const RapidXML_Attribute* entryAttribute9 = NULL;

	gameFlag = ((entryAttribute1 = entryNode->first_attribute("gameflag")) != NULL) ? entryAttribute1->value() : "";
	if (!GameFlagActiveCheck(gameFlag)) return true;

	entryAttribute2 = entryNode->first_attribute("name");
	if (entryAttribute2 == NULL) FAIL_RETURN_FALSE;
	newLookup += entryAttribute2->value();

	nodeType = entryNode->name();
	if		(strcmp(nodeType, "Node") == 0)			newNode = cocos2d::Node::create();
	else if (strcmp(nodeType, "Text") == 0)
	{
		entryAttribute3 = entryNode->first_attribute("text");
		if (entryAttribute3 == NULL) FAIL_RETURN_FALSE;

		entryAttribute4 = entryNode->first_attribute("asset");
		if (entryAttribute4 == NULL) FAIL_RETURN_FALSE;

		if (FontList.find(entryAttribute4->value()) == FontList.end()) FAIL_RETURN_FALSE;

		newNode = cocos2d::Label::createWithBMFont(FontList[entryAttribute4->value()], entryAttribute3->value());
	}
	else if (strcmp(nodeType, "Object") == 0)
	{
		entryAttribute3 = entryNode->first_attribute("asset");
		if (entryAttribute3 == NULL) FAIL_RETURN_FALSE;

		std::unordered_map<std::string, SpriteData*>::const_iterator iter = SpriteList.find(entryAttribute3->value());
		if (iter == SpriteList.end()) FAIL_RETURN_FALSE;

		SpriteData* basicSprite = (*iter).second;
		newNode = ScreenObject::create(basicSprite);
	}
	else if (strcmp(nodeType, "Particle") == 0)
	{
		entryAttribute3 = entryNode->first_attribute("asset");
		if (entryAttribute3 == NULL) FAIL_RETURN_FALSE;

		std::unordered_map<std::string, ParticleInfo*>::const_iterator iter = ParticleList.find(entryAttribute3->value());
		if (iter == ParticleList.end()) FAIL_RETURN_FALSE;

		ParticleInfo* basicParticle = (*iter).second;
		newNode = ScreenParticle::create(basicParticle, entryAttribute3->value());
    }
	else if (strcmp(nodeType, "Button") == 0)
	{
		entryAttribute3 = entryNode->first_attribute("asset");
		if (entryAttribute3 == NULL) FAIL_RETURN_FALSE;

		std::unordered_map<std::string, SpriteData*>::const_iterator iter = SpriteList.find(entryAttribute3->value());
		if (iter == SpriteList.end()) FAIL_RETURN_FALSE;

		SpriteData* basicSprite = (*iter).second;
		newNode = ScreenButton::create(basicSprite);
		((ScreenButton*)(newNode))->setZoomScale(0.0f);
	}
	else if (strcmp(nodeType, "Reference") == 0)
	{
		entryAttribute3 = entryNode->first_attribute("ref");
		if (entryAttribute3 == NULL) FAIL_RETURN_FALSE;

		std::unordered_map<std::string, cocos2d::Node*>::iterator iter = LayoutNodeList.find(entryAttribute3->value());
		if (iter == LayoutNodeList.end()) FAIL_RETURN_FALSE;

		newNode = cloneLayoutNode((*iter).second, newLookup.c_str());
		if (newNode == NULL) FAIL_RETURN_FALSE;
	}

	Vec2 position(newNode->getPositionX(), newNode->getPositionY());
	Vec2 scale(newNode->getScaleX(), newNode->getScaleY());
	
	newNode->setCascadeOpacityEnabled(true);
	parent->addChild(newNode, -parent->getChildrenCount(), entryAttribute2->value());
	if (strcmp(nodeType, "Reference") != 0) newNode->setTag(NodeTypeList[nodeType]);

	if (strcmp(nodeType, "Particle") == 0) ((ScreenParticle*)(newNode))->stopSystem();

	entryAttribute3 = entryNode->first_attribute("hide");

	entryAttribute4 = entryNode->first_attribute("x");
	if (entryAttribute4 != NULL) position.x = atof(entryAttribute4->value());

	entryAttribute5 = entryNode->first_attribute("y");
	if (entryAttribute5 != NULL) position.y = atof(entryAttribute5->value());

	entryAttribute6 = entryNode->first_attribute("sx");
	if (entryAttribute6 != NULL) scale.x = atof(entryAttribute6->value());

	entryAttribute7 = entryNode->first_attribute("sy");
	if (entryAttribute7 != NULL) scale.y = atof(entryAttribute7->value());

	if (strcmp(nodeType, "Text") == 0)
	{
		scale.x *= scale.x;
		scale.y *= scale.y;
	}

	LayoutNodeList[newLookup] = newNode;
	newNode->setVisible(entryAttribute3 ? (strcmp(entryAttribute3->value(), "0") == 0) : true);
	newNode->setPosition(position.x, position.y);
	newNode->setScale(scale.x, scale.y);

	entryAttribute8 = entryNode->first_attribute("alpha");
	if (entryAttribute8 != NULL) newNode->setOpacity(GLubyte(255 * atof(entryAttribute8->value())));

	entryAttribute9 = entryNode->first_attribute("a");
	if (entryAttribute9 != NULL)
	{
		if		(strcmp(entryAttribute9->value(), "tl") == 0) newNode->setAnchorPoint(Vec2(0.0f, 1.0f));
		else if (strcmp(entryAttribute9->value(), "tc") == 0) newNode->setAnchorPoint(Vec2(0.5f, 1.0f));
		else if (strcmp(entryAttribute9->value(), "tr") == 0) newNode->setAnchorPoint(Vec2(1.0f, 1.0f));
		else if (strcmp(entryAttribute9->value(), "cl") == 0) newNode->setAnchorPoint(Vec2(0.0f, 0.5f));
		else if (strcmp(entryAttribute9->value(), "cc") == 0) newNode->setAnchorPoint(Vec2(0.5f, 0.5f));
		else if (strcmp(entryAttribute9->value(), "cr") == 0) newNode->setAnchorPoint(Vec2(1.0f, 0.5f));
		else if (strcmp(entryAttribute9->value(), "bl") == 0) newNode->setAnchorPoint(Vec2(0.0f, 0.0f));
		else if (strcmp(entryAttribute9->value(), "bc") == 0) newNode->setAnchorPoint(Vec2(0.5f, 0.0f));
		else if (strcmp(entryAttribute9->value(), "br") == 0) newNode->setAnchorPoint(Vec2(1.0f, 0.0f));
	}
	else newNode->setAnchorPoint(Vec2(0.0f, 1.0f));
	
	bool childrenLoaded = true;
	const RapidXML_Node* childNode = entryNode->first_node();
	if (childNode != NULL)
	{
		do {
			childrenLoaded &= LoadLayoutNode(newLookup + ".", newNode, childNode);
		} while (childrenLoaded && (childNode = childNode->next_sibling()) != NULL);
	}

	return childrenLoaded;
}

cocos2d::Node* AssetManager::cloneLayoutNode(const cocos2d::Node* referenceNode, const char* path)
{
	cocos2d::Node* newNode = NULL;
	const int nodeType = referenceNode->getTag();
	const RapidXML_Attribute* entryAttribute1 = NULL;

	if		(nodeType == NODETYPE_NODE)			newNode = cocos2d::Node::create();
	else if (nodeType == NODETYPE_TEXT)			newNode = cocos2d::Label::create();
	else if (nodeType == NODETYPE_OBJECT)
	{
		ScreenObject* referenceSprite = (ScreenObject*)(referenceNode);
		newNode = ScreenObject::createWithSpriteFrame(referenceSprite->getSpriteFrame());
	}
	else if (nodeType == NODETYPE_BUTTON)
	{
		ScreenButton* referenceButton = (ScreenButton*)(referenceNode);
		newNode = ScreenButton::create(referenceButton->getSpriteName());
		((ScreenButton*)(newNode))->setZoomScale(0.0f);
	}
	else if (nodeType == NODETYPE_PARTICLE)
	{
		ScreenParticle* referenceParticle = (ScreenParticle*)(referenceNode);
		newNode = ScreenParticle::create(referenceParticle->getParticleName());
	}
	else assert(false);

	newNode->setTag(nodeType);

	newNode->setName(referenceNode->getName());
	newNode->setVisible(referenceNode->isVisible());
	newNode->setPosition(referenceNode->getPosition());
	newNode->setScaleX(referenceNode->getScaleX());
	newNode->setScaleY(referenceNode->getScaleY());
	newNode->setOpacity(referenceNode->getOpacity());
	newNode->setAnchorPoint(referenceNode->getAnchorPoint());

	if (nodeType == NODETYPE_TEXT)
	{
		cocos2d::Label* newLabel = (cocos2d::Label*)(newNode);
		cocos2d::Label* referenceLabel = (cocos2d::Label*)(referenceNode);
		newLabel->setString(referenceLabel->getString());
		newLabel->setBMFontFilePath(referenceLabel->getBMFontFilePath());
	}

	if (nodeType == NODETYPE_NODE)
	{
		const cocos2d::Vector<cocos2d::Node*>& childList = referenceNode->getChildren();
		for (cocos2d::Vector<cocos2d::Node*>::const_iterator iter = childList.begin(); iter != childList.end(); ++iter)
		{
			std::string objPath(std::string(path) + "." + (*iter)->getName());
			newNode->setCascadeOpacityEnabled(true);
			cocos2d::Node* childNode = cloneLayoutNode((*iter), objPath.c_str());
			newNode->addChild(childNode, -newNode->getChildrenCount(), childNode->getName());
			if (childNode->getTag() == NODETYPE_PARTICLE) ((ScreenParticle*)(childNode))->stopSystem();
			LayoutNodeList[objPath] = childNode;
		}
	}

	return newNode;
}

void AssetManager::ReleaseAllAssets()
{
	while (SpriteList.empty() == false)
	{
		delete SpriteList.begin()->second;
		SpriteList.erase(SpriteList.begin());
	}

	FontList.clear();

	while (AnimationList.empty() == false)
	{
		delete AnimationList.begin()->second;
		AnimationList.erase(AnimationList.begin());
	}

	LayoutNodeList.clear();
}


SpriteData* AssetManager::getSpriteByName(const char* name)
{
	std::unordered_map<std::string, SpriteData*>::iterator iter = SpriteList.find(name);
	if (iter == SpriteList.end()) return NULL;

	return (*iter).second;
}

std::string& AssetManager::getFontFileByName(const char* name)
{
    std::string nonTemporaryString;
	std::unordered_map<std::string, std::string>::iterator iter = FontList.find(name);
	if (iter == FontList.end()) { assert(false); return (nonTemporaryString = ""); }

	return (*iter).second;
}

BasicAnimation* AssetManager::getAnimationByName(const char* name)
{
	std::unordered_map<std::string, BasicAnimation*>::iterator iter = AnimationList.find(name);
	if (iter == AnimationList.end()) { assert(false); return NULL; }

	return (*iter).second;
}

ParticleInfo* AssetManager::getParticleByName(const char* name)
{
	std::unordered_map<std::string, ParticleInfo*>::iterator iter = ParticleList.find(name);
	if (iter == ParticleList.end()) { assert(false); return NULL; }

	return (*iter).second;
}

Node* AssetManager::getLayoutNodeByName(const char* name)
{
	std::unordered_map<std::string, cocos2d::Node*>::iterator iter = LayoutNodeList.find(name);
	if (iter == LayoutNodeList.end()) return NULL;

	return (*iter).second;
}

void AssetManager::addAnimLink(Node* objectNode, AnimationLink* animLink)
{
	removeAnimLink(objectNode);
	AnimationLinkList[objectNode] = animLink;
}

void AssetManager::removeAnimLink(Node* objectNode)
{
	std::unordered_map<Node*, AnimationLink*>::iterator iter = AnimationLinkList.find(objectNode);
	if (iter != AnimationLinkList.end()) {
		delete (*iter).second;
		AnimationLinkList.erase(iter);
	}
}

AnimationLink* AssetManager::getAnimationLinkByObject(Node* objectNode)
{
	std::unordered_map<Node*, AnimationLink*>::iterator iter = AnimationLinkList.find(objectNode);
	if (iter == AnimationLinkList.end()) return NULL;

	return (*iter).second;
}

bool AssetManager::GameFlagActiveCheck(const char* gameFlagList)
{
	if (strlen(gameFlagList) == 0) return true;

	std::stringstream gameFlagStream(gameFlagList);
	int flagIter = -1;
	gameFlagStream >> flagIter;
	while (gameFlagStream)
	{
		if (IS_GAME_ACTIVE(flagIter)) return true;
		gameFlagStream >> flagIter;
	}

	return false;
}
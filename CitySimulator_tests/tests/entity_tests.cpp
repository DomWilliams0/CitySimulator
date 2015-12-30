#include "test_helpers.hpp"
#include "services.hpp"

struct EntityTests : public ::testing::Test
{
	virtual void SetUp() override
	{
		Locator::provide(SERVICE_CONFIG, new ConfigService("data/test_reference_config.json",
		                                                   "data/test_config.json"));
		Locator::provide(SERVICE_INPUT, new InputService);
		Locator::provide(SERVICE_RENDER, new RenderService(nullptr));
		Locator::provide(SERVICE_ANIMATION, new AnimationService);
		Locator::provide(SERVICE_ENTITY, new EntityService);
		Locator::locate<AnimationService>()->processQueuedSprites();
	}

	virtual void TearDown() override
	{
	}
};

TEST_F(EntityTests, EntityLifeCycle)
{
	EntityService *es = Locator::locate<EntityService>();

	EXPECT_EQ(es->getEntityCount(), 0);
	EntityID e = es->createEntity();
	EXPECT_FALSE(es->isAlive(e)); // no components = dead

	EXPECT_FALSE(es->hasComponent(e, COMPONENT_INPUT));
	es->addAIInputComponent(e);
	EXPECT_TRUE(es->hasComponent(e, COMPONENT_INPUT));
	EXPECT_TRUE(es->isAlive(e));
	EXPECT_EQ(es->getEntityCount(), 1);

	InputComponent *input = es->getComponent<InputComponent>(e, COMPONENT_INPUT);
	BaseComponent *other = es->getComponentOfType(e, COMPONENT_INPUT);
	EXPECT_EQ(input, other);

	es->killEntity(e);
	EXPECT_FALSE(es->isAlive(e));

	EXPECT_EQ(es->getEntityCount(), 0);
}

TEST_F(EntityTests, Sprite)
{
	AnimationService *as = Locator::locate<AnimationService>();

	Animation *anim = nullptr;
	EXPECT_NO_THROW(anim = as->getAnimation(ENTITY_HUMAN, "Test Man"));
	ASSERT_NE(anim, nullptr);

	EXPECT_NO_THROW(Animator(anim, 0.25f));
}

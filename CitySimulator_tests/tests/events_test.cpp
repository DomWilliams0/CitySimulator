#include "test_helpers.hpp"


struct EventsTest : public ::testing::Test
{
	virtual void SetUp() override
	{
		es = new EventService;
		Locator::provide(SERVICE_EVENT, es);
	}

	EventService *es;
	
};

struct InputKeyListener : public EventListener
{

	virtual void onEvent(const Event &event) override
	{
		if (event.type != EVENT_RAW_INPUT_KEY)
			error("InputKeyListener received event that wasn't an input event");
	}
};

struct ErrorOnEventListener : public EventListener
{
	virtual void onEvent(const Event &event) override
	{
		error("FailOnEventListener's onEvent was called");
	}
};

TEST_F(EventsTest, RegisterAndUnregister)
{
	ErrorOnEventListener l;
	auto key(sf::Keyboard::K);

	es->callRawInputKeyEvent(key, true);
	EXPECT_NO_THROW(es->processQueue());

	es->registerListener(&l, EVENT_RAW_INPUT_KEY);
	es->callRawInputKeyEvent(key, true);
	EXPECT_ANY_THROW(es->processQueue());

	es->unregisterListener(&l, EVENT_RAW_INPUT_KEY);
	es->callRawInputKeyEvent(key, true);
	EXPECT_NO_THROW(es->processQueue());
}

TEST_F(EventsTest, SpecificEvents)
{
	InputKeyListener ikl;
	es->registerListener(&ikl, EVENT_RAW_INPUT_KEY);

	Event badEvent;
	badEvent.type = EVENT_UNKNOWN;

	es->callRawInputKeyEvent(sf::Keyboard::K, true);
	es->callEvent(badEvent);
	EXPECT_NO_THROW(es->processQueue());

	es->registerListener(&ikl, EVENT_UNKNOWN);
	es->callEvent(badEvent);
	EXPECT_ANY_THROW(es->processQueue());
}
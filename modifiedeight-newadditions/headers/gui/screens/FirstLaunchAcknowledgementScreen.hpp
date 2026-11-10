#pragma once
#include <gui/Screen.hpp>
#include <memory>
struct Button;
struct Label;
struct TextBox;
struct NinePatchLayer;

struct FirstLaunchAcknowledgementScreen : Screen {
	std::shared_ptr<Button> headerButton;
	std::shared_ptr<Button> confirmButton;
	std::shared_ptr<Label> messageLabel;
	std::shared_ptr<Label> feedbackLabel;
	std::shared_ptr<NinePatchLayer> panelLayer;

	FirstLaunchAcknowledgementScreen();

	virtual ~FirstLaunchAcknowledgementScreen();
	virtual void render(int32_t, int32_t, float);
	virtual void init();
	virtual void setupPositions();
	virtual bool_t handleBackEvent(bool_t);
	virtual void buttonClicked(Button*);
};

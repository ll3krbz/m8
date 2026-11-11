#include <gui/screens/FirstLaunchAcknowledgementScreen.hpp>
#include <Minecraft.hpp>
#include <gui/elements/Label.hpp>
#include <gui/elements/TextBox.hpp>
#include <gui/NinePatchFactory.hpp>
#include <fstream>
#include <gui/buttons/Touch_TButton.hpp>
#include <gui/buttons/Touch_THeader.hpp>
#include <gui/NinePatchLayer.hpp>
#include <rendering/Tesselator.hpp>
#include <util/IntRectangle.hpp>
#include <fstream>
#include <string.h>

FirstLaunchAcknowledgementScreen::FirstLaunchAcknowledgementScreen() {
}

FirstLaunchAcknowledgementScreen::~FirstLaunchAcknowledgementScreen() {
}

void FirstLaunchAcknowledgementScreen::render(int32_t a2, int32_t a3, float a4) {
	if (this->supppressedBySubWindow()) {
		this->renderBackground(0);
	} else {
		this->renderMenuBackground(a4);
		this->panelLayer->draw(Tesselator::instance, 5, this->headerButton->height + 5);
	}
	Screen::render(a2, a3, a4);
}

void FirstLaunchAcknowledgementScreen::init() {
	this->headerButton = std::shared_ptr<Button>(new Touch::THeader(0, "READ THIS"));
	this->confirmButton = std::shared_ptr<Button>(new Touch::TButton(1, "I confirm", this->minecraft));
	this->messageLabel = std::shared_ptr<Label>(new Label(
		"Before starting the game, please confirm that you have read and acknowledge what you have read:\n\n"
		"- This client was created by enthusiast developers on their own initiative.\n"
		"We derive no personal benefit from its development, and we are under no obligation to you.\n"
		"- We don't guarantee stable client performance,\n"
		"or that it will be free of bugs, glitches, or other issues.\n"
		"- This client is not affiliated with Mojang AB and Microsoft.\n"
		"\n\n"
		"Join us on Discord: discord.gg/4fv4RrTav4.",
		this->minecraft, -1, 0, 0, 0, 1));
	this->feedbackLabel = std::shared_ptr<Label>(new Label("", this->minecraft, -1, 0, 0, 0, 1));
	NinePatchFactory factory(this->minecraft->texturesPtr, "gui/spritesheet.png");
	this->panelLayer = std::shared_ptr<NinePatchLayer>(factory.createSymmetrical(IntRectangle{34, 43, 14, 14}, 3, 3, 32, 32));
	this->buttons.emplace_back(this->headerButton.get());
	this->buttons.emplace_back(this->confirmButton.get());
	this->elements.emplace_back(this->messageLabel.get());
	this->elements.emplace_back(this->feedbackLabel.get());
}

void FirstLaunchAcknowledgementScreen::setupPositions() {
	bool_t compact = this->height < 260;
	int32_t panelTop = compact ? 28 : 34;
	int32_t panelLeft = 10;
	int32_t panelWidth = this->width - 20;
	int32_t textWidth = panelWidth - (compact ? 16 : 32);
	this->headerButton->posX = 0;
	this->headerButton->posY = 0;
	this->headerButton->width = this->width;
	this->headerButton->height = 26;
	int32_t panelHeight = compact ? this->height - panelTop - 42 : 170;
	this->panelLayer->setSize((float)panelWidth, (float)panelHeight);
	this->messageLabel->width = textWidth;
	this->messageLabel->height = panelHeight - 42;
	this->messageLabel->field_38 = true;
	this->messageLabel->posX = panelLeft + (compact ? 8 : 16);
	this->messageLabel->posY = panelTop + 8;
	this->messageLabel->setupPositions();
	this->feedbackLabel->width = panelWidth - 32;
	this->feedbackLabel->posX = panelLeft + 16;
	this->feedbackLabel->posY = panelTop + panelHeight + 2;
	this->confirmButton->width = 64;
	this->confirmButton->height = 18;
	this->confirmButton->posX = this->width - this->confirmButton->width - 4;
	this->confirmButton->posY = 4;
}

bool_t FirstLaunchAcknowledgementScreen::handleBackEvent(bool_t a2) {
	if (!a2) {
		bool_t handledByElement = true;
		for (auto&& element : this->elements) {
			if (element->suppressOtherGUI()) {
				handledByElement = false;
				element->backPressed(this->minecraft, 0);
			}
		}
		if (handledByElement) {
			this->minecraft->quit();
		}
	}
	return 1;
}

void FirstLaunchAcknowledgementScreen::buttonClicked(Button* button) {
	if (button != this->confirmButton.get()) return;

	this->minecraft->options.firstLaunchAcknowledged = 1;
	this->minecraft->options.save();
	this->minecraft->screenChooser.setScreen(START_MENU_SCREEN);
}

#include <item/BowlFoodItem.hpp>
#include <level/Level.hpp>
#include <entity/Player.hpp>

BowlFoodItem::BowlFoodItem(int32_t a2, int32_t a3)
	: FoodItem(a2, a3, 0.6, 0) {
	this->setMaxStackSize(1);
}
BowlFoodItem::~BowlFoodItem() {
}
ItemInstance* BowlFoodItem::use(ItemInstance* a2, Level* a3, Player* a4) {
	a4->startUsingItem(ItemInstance(*a2), this->getMaxUseDuration());
	return a2;
}
ItemInstance BowlFoodItem::useTimeDepleted(ItemInstance* a3, Level* a4, Player* player) {
	player->heal(this->healAmount);
	a4->playSound(player, "random.burp", 0.5f, a4->random.nextFloat() * 0.1f + 0.9f);
	*a3 = ItemInstance(Item::bowl);
	return ItemInstance(*a3);
}

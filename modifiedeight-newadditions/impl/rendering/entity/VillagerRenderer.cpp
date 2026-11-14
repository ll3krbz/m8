#include <rendering/entity/VillagerRenderer.hpp>
#include <entity/Villager.hpp>
#include <entity/Player.hpp>
#include <rendering/EntityRenderDispatcher.hpp>
#include <rendering/ItemInHandRenderer.hpp>
#include <rendering/TileRenderer.hpp>
#include <level/Level.hpp>
#include <tile/Tile.hpp>

VillagerRenderer::VillagerRenderer(VillagerModel* model, float shadow)
	: MobRenderer(model, shadow) {
}

VillagerRenderer::~VillagerRenderer() {
}

void VillagerRenderer::additionalRendering(Mob* a2, float a3) {
	if (!a2 || a2->getEntityTypeId() != 120) return;
	Villager* v = (Villager*)a2;
	if (v->tradeCount <= 0) return;
	Player* p = a2->level ? a2->level->getNearestPlayer(a2, 6.0f) : nullptr;
	if (!p) return;
	ItemInstance* held = p->getCarriedItem();
	int32_t showTradeIdx = -1;
	if (held && !held->isNull() && held->count > 0) {
		for (int32_t i = 0; i < v->tradeCount; ++i) {
			if (v->trades[i].inputId == held->getId()) {
				showTradeIdx = i;
				break;
			}
		}
	}
	if (showTradeIdx < 0 && held && !held->isNull() && held->count > 0) {
		int32_t hid = held->getId();
		if (hid == 264 || hid == 265 || hid == 266 || hid == 388 || hid == 263 || hid == 296) {
			showTradeIdx = 0;
		}
	}
	if (showTradeIdx >= 0) {
		VillagerTrade& tr = v->trades[showTradeIdx];
		ItemInstance showInst(tr.outputId, tr.outputCount, tr.outputMeta);
		glPushMatrix();
		if (this->model) {
			((VillagerModel*)this->model)->arms.translateTo(0.0625f);
		}
		if (showInst.tileClass != nullptr && TileRenderer::canRender(showInst.tileClass->getRenderShape())) {
			glTranslatef(0.0f, 0.14f, -0.14f);
			glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
			glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
			glScalef(0.36f, 0.36f, 0.36f);
		} else {
			glTranslatef(0.18f, 0.30f, 0.02f);
			glScalef(0.38f, 0.38f, 0.38f);
			glRotatef(60.0f, 0.0f, 0.0f, 1.0f);
			glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
			glRotatef(20.0f, 0.0f, 0.0f, 1.0f);
		}
		if (EntityRenderer::entityRenderDispatcher && EntityRenderer::entityRenderDispatcher->itemInHandRenderer) {
			EntityRenderer::entityRenderDispatcher->itemInHandRenderer->renderItem(a2, &showInst);
		}
		glPopMatrix();
	}
}

#import <Foundation/Foundation.h>
#import <GameController/GameController.h>

#include <input/Gamepad.hpp>

static GCController *s_pad = nil;
static BOOL s_menuTap = NO;

void GamepadMFi_poll(void) {
	@autoreleasepool {
		GCController *pad = nil;
		for (GCController *c in [GCController controllers]) {
			if (c.extendedGamepad) {
				pad = c;
				break;
			}
		}
		if (!pad) {
			s_pad = nil;
			s_menuTap = NO;
			Gamepad::setConnected(0);
			return;
		}
		if (s_pad != pad) {
			s_pad = pad;
			pad.controllerPausedHandler = ^(GCController *controller) {
				(void)controller;
				s_menuTap = YES;
			};
		}
		GCExtendedGamepad *gp = pad.extendedGamepad;
		Gamepad::setConnected(1);
		Gamepad::feedButton(GP_BTN_A, gp.buttonA.pressed != 0, GP_BTN_A);
		Gamepad::feedButton(GP_BTN_B, gp.buttonB.pressed != 0, GP_BTN_B);
		Gamepad::feedButton(GP_BTN_X, gp.buttonX.pressed != 0, GP_BTN_X);
		Gamepad::feedButton(GP_BTN_Y, gp.buttonY.pressed != 0, GP_BTN_Y);
		Gamepad::feedButton(GP_BTN_LB, gp.leftShoulder.pressed != 0, GP_BTN_LB);
		Gamepad::feedButton(GP_BTN_RB, gp.rightShoulder.pressed != 0, GP_BTN_RB);
		BOOL l3 = NO, r3 = NO;
		if ([gp respondsToSelector:@selector(leftThumbstickButton)]) {
			GCControllerButtonInput *btn = (GCControllerButtonInput *)[gp valueForKey:@"leftThumbstickButton"];
			if (btn) l3 = btn.pressed != 0;
		}
		if ([gp respondsToSelector:@selector(rightThumbstickButton)]) {
			GCControllerButtonInput *btn = (GCControllerButtonInput *)[gp valueForKey:@"rightThumbstickButton"];
			if (btn) r3 = btn.pressed != 0;
		}
		Gamepad::feedButton(GP_BTN_L3, l3, GP_BTN_L3);
		Gamepad::feedButton(GP_BTN_R3, r3, GP_BTN_R3);
		Gamepad::feedAxis(GP_AXIS_LX, gp.leftThumbstick.xAxis.value, 0);
		Gamepad::feedAxis(GP_AXIS_LY, -gp.leftThumbstick.yAxis.value, 1);
		Gamepad::feedAxis(GP_AXIS_RX, gp.rightThumbstick.xAxis.value, 2);
		Gamepad::feedAxis(GP_AXIS_RY, -gp.rightThumbstick.yAxis.value, 3);
		Gamepad::feedAxis(GP_AXIS_LT, gp.leftTrigger.value, 4);
		Gamepad::feedAxis(GP_AXIS_RT, gp.rightTrigger.value, 5);
		Gamepad::feedHat(gp.dpad.up.pressed != 0, gp.dpad.down.pressed != 0,
			gp.dpad.left.pressed != 0, gp.dpad.right.pressed != 0);
		Gamepad::syncDerivedButtons();
		if (s_menuTap) {
			s_menuTap = NO;
			Gamepad::feedButton(GP_BTN_START, 1, GP_BTN_START);
		} else {
			Gamepad::feedButton(GP_BTN_START, 0, GP_BTN_START);
		}
	}
}

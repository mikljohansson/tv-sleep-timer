#ifndef _TVT_CONFIG_H_
#define _TVT_CONFIG_H_

#define TVT_IR_LED_PIN          3
#define TVT_STATUS_LED_PIN      9
#define TVT_SHUTDOWN_PIN        4
#define TVT_BUTTON_PIN          2
#define TVT_ONBOARD_LED_PIN     LED_BUILTIN

#define TVT_SLEEP_TIMER_SECONDS     1800ll
#define TVT_WARNING_BLINK_SECONDS   120ll

#define TVT_STATUS_WARNING_BRIGHTNESS  128
#define TVT_STATUS_WARNING_INTERVAL    1000ll

extern volatile int buttonPressed;

#endif /* _TVT_CONFIG_H_ */
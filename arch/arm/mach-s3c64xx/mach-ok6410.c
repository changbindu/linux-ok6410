/* linux/arch/arm/mach-s3c64xx/mach-smdk6410.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/serial_core.h>
#include <linux/serial_s3c.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/smsc911x.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>
#include <linux/pwm_backlight.h>
#include <linux/platform_data/s3c-hsotg.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#ifdef CONFIG_DM9000
#include <linux/dm9000.h>
#endif

#ifdef CONFIG_SMDK6410_WM1190_EV1
#include <linux/mfd/wm8350/core.h>
#include <linux/mfd/wm8350/pmic.h>
#endif

#ifdef CONFIG_SMDK6410_WM1192_EV1
#include <linux/mfd/wm831x/core.h>
#include <linux/mfd/wm831x/pdata.h>
#endif

#include <video/platform_lcd.h>
#include <video/samsung_fimd.h>
#include <media/gpio-ir-recv.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/map.h>

#include <asm/irq.h>
#include <asm/mach-types.h>

#include <mach/regs-gpio.h>
#include <mach/gpio-samsung.h>

#include <linux/platform_data/ata-samsung_cf.h>
#include <linux/platform_data/i2c-s3c2410.h>
#include <plat/fb.h>
#include <plat/gpio-cfg.h>

#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/adc.h>
#include <linux/platform_data/touchscreen-s3c2410.h>
#include <plat/keypad.h>
#include <linux/gpio_keys.h>
#include <plat/backlight.h>
#include <plat/samsung-time.h>
#include <plat/usb-phy.h>


#include <linux/platform_data/mtd-nand-s3c2410.h>

#include "common.h"
#include "regs-modem.h"
#include "regs-srom.h"
#include "regs-sys.h"

#define UCON S3C2410_UCON_DEFAULT | S3C2410_UCON_UCLK
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE

static struct s3c2410_uartcfg ok6410_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[3] = {
		.hwport	     = 3,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
};

static struct mtd_partition ok6410_nand_part[] = {
	[0] = {
		.name		= "uboot",
		.size		= SZ_1M,
		.offset		= 0,
		.mask_flags	= MTD_CAP_NANDFLASH,
	},
	[1] = {
		.name		= "kernel",
		.size		= 5 * SZ_1M,
		.offset		= MTDPART_OFS_APPEND,
		.mask_flags	= MTD_CAP_NANDFLASH,
	},
	[2] = {
		.name		= "rootfs",
		.size		= MTDPART_SIZ_FULL,
		.offset		= MTDPART_OFS_APPEND,
	},
};

static struct s3c2410_nand_set ok6410_nand_sets[] = {
	[0] = {
		.name		= "nand",
		.nr_chips	= 1,
		.disable_ecc = 1,
		.nr_partitions	= ARRAY_SIZE(ok6410_nand_part),
		.partitions	= ok6410_nand_part,
	},
};

static struct s3c2410_platform_nand ok6410_nand_info = {
	.tacls		= 25,
	.twrph0		= 55,
	.twrph1		= 40,
	.nr_sets	= ARRAY_SIZE(ok6410_nand_sets),
	.sets		= ok6410_nand_sets,
};

#ifdef CONFIG_DM9000

#define S3C64XX_PA_DM9000	(0x18000000)
#define S3C64XX_SZ_DM9000	SZ_1M
#define S3C64XX_VA_DM9000	S3C_ADDR(0x03b00300)
static struct resource dm9000_resources[] = {
	[0] = {
		.start		= S3C64XX_PA_DM9000,
		.end		= S3C64XX_PA_DM9000 + 3,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= S3C64XX_PA_DM9000 + 4,
		.end		= S3C64XX_PA_DM9000 + S3C64XX_SZ_DM9000 - 1, 
		.flags		= IORESOURCE_MEM,
	},
	[2] = {
		.start		= IRQ_EINT(7),
		.end		= IRQ_EINT(7),
		.flags		= IORESOURCE_IRQ | IRQF_TRIGGER_HIGH,
	},
};

static struct dm9000_plat_data dm9000_setup = {
	.flags			= DM9000_PLATF_16BITONLY,
	.dev_addr		= { 0x08, 0x90, 0x00, 0xa0, 0x90, 0x90 },
};

static struct platform_device s3c_device_dm9000 = {
	.name			= "dm9000",
	.id			= 0,
	.num_resources	= ARRAY_SIZE(dm9000_resources),
	.resource		= dm9000_resources,
	.dev			= {
		.platform_data = &dm9000_setup,
	}
};
#endif

static struct gpio_led ok6410_leds[] = {
	[0]= {
		.name = "LED1",
		.gpio = S3C64XX_GPM(0),
	},
	[1]= {
		.name = "LED2",
		.gpio = S3C64XX_GPM(1),
	},
	[2]= {
		.name = "LED3",
		.gpio = S3C64XX_GPM(2),
	},
	[3]= {
		.name = "LED4",
		.gpio = S3C64XX_GPM(3),
	},
};

static struct gpio_led_platform_data ok6410_gpio_led_pdata = {
	.num_leds	= ARRAY_SIZE(ok6410_leds),
	.leds		=ok6410_leds,
};

static struct platform_device ok6410_device_led = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data = &ok6410_gpio_led_pdata,
	},
};

/* gpio buttons */
static struct gpio_keys_button gpio_buttons[] = {
	{
		.gpio		= S3C64XX_GPN(0),
		.code		= KEY_UP,
		.desc		= "BUTTON1",
		.active_low	= 1,
		.wakeup		= 0,
	},
	{
		.gpio		= S3C64XX_GPN(1),
		.code		= KEY_DOWN,
		.desc		= "BUTTON2",
		.active_low	= 1,
		.wakeup		= 0,
	},
	{
		.gpio		= S3C64XX_GPN(2),
		.code		= KEY_LEFT,
		.desc		= "BUTTON3",
		.active_low	= 1,
		.wakeup		= 0,
	},
	{
		.gpio		= S3C64XX_GPN(3),
		.code		= KEY_RIGHT,
		.desc		= "BUTTON4",
		.active_low	= 1,
		.wakeup		= 0,
	},
	{
		.gpio		= S3C64XX_GPN(4),
		.code		= KEY_ENTER,
		.desc		= "BUTTON5",
		.active_low	= 1,
		.wakeup		= 0,
	},
	{
		.gpio		= S3C64XX_GPN(5),
		.code		= KEY_ESC,
		.desc		= "BUTTON6",
		.active_low	= 1,
		.wakeup		= 0,
	}
};

static struct gpio_keys_platform_data gpio_button_data = {
	.buttons	= gpio_buttons,
	.nbuttons	= ARRAY_SIZE(gpio_buttons),
};

static struct platform_device ok6410_gpio_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.num_resources	= 0,
	.dev		= {
		.platform_data	= &gpio_button_data,
	}
};

/**
 * gpio_ir_recv need a pin that could triger IRQ, but the pin connected to ir
 * receiver on ok6410 is GPE1 which doesn't have that function. So this ir device
 * actually not work yet. We may consider using other pin like GPMx instead.
 */
static struct gpio_ir_recv_platform_data ok6410_gpio_ir_recv_data = {
	.gpio_nr	= S3C64XX_GPE(1),
	.active_low	= false,
};

static struct platform_device ok6410_gpio_ir_recv = {
	.name			= "gpio-rc-recv",
	.num_resources		= 0,
	.dev.platform_data	= &ok6410_gpio_ir_recv_data,
};

/* framebuffer and LCD setup. */

/* GPF15 = LCD backlight control
 * GPF13 => Panel power
 * GPN5 = LCD nRESET signal
 * PWM_TOUT1 => backlight brightness
 */

static void ok6410_lcd_power_set(struct plat_lcd_data *pd,
				   unsigned int power)
{
	if (power) {
		gpio_direction_output(S3C64XX_GPF(13), 1);

		/* fire nRESET on power up */
		gpio_direction_output(S3C64XX_GPN(5), 0);
		msleep(10);
		gpio_direction_output(S3C64XX_GPN(5), 1);
		msleep(1);
	} else {
		gpio_direction_output(S3C64XX_GPF(13), 0);
	}
}

static struct plat_lcd_data ok6410_lcd_power_data = {
	.set_power	= ok6410_lcd_power_set,
};

static struct platform_device ok6410_lcd_powerdev = {
	.name			= "platform-lcd",
	.dev.parent		= &s3c_device_fb.dev,
	.dev.platform_data	= &ok6410_lcd_power_data,
};

/**
 * lcd types
 */

/* this is to ensure we use win0 */
#ifdef CONFIG_FB_S3C_TFT480272
/* 4.3 inch 480x272 TFT LCD */
static struct s3c_fb_pd_win ok6410_fb_win0 = {
	.max_bpp	= 32, /* max bits per pixel */
	.default_bpp	= 16,
	.xres		= 480,
	.yres		= 272,
	.virtual_y	= 480 * 2,
	.virtual_x	= 272,
};
static struct fb_videomode ok6410_lcd_timing = {
	.left_margin	= 2, /* horizon back porch(thb) */
	.right_margin	= 2, /* horizon front porch(thf) */
	.upper_margin	= 2, /* vertical back porch(tvb) */
	.lower_margin	= 2, /* vertical front porch(tvf) */
	.hsync_len	= 41, /* horizon pulse width(thp) */
	.vsync_len	= 10, /* vertical pulse width(tvp) */
	.xres		= 480, /* horizon pixel  x resolition(thd) */
	.yres		= 272, /* line cnt       y resolution(tvd) */
};
#elif defined(CONFIG_FB_S3C_TFT800480)
/* 7 inch 800x480 TFT LCD */
static struct s3c_fb_pd_win ok6410_fb_win0 = {
	.max_bpp	= 32,
	.default_bpp	= 16,
	.xres		= 800,
	.yres		= 480,
	.virtual_y	= 800 * 2,
	.virtual_x	= 480,
};
static struct fb_videomode ok6410_lcd_timing = {
	.left_margin	= 40,
	.right_margin	= 40,
	.upper_margin	= 29,
	.lower_margin	= 13,
	.hsync_len	= 48,
	.vsync_len	= 3,
	.xres		= 800,
	.yres		= 480,
};
#elif defined(CONFIG_FB_S3C_T240320)
/* 3.5 inch 240X320 Toppoly LCD */
static struct s3c_fb_pd_win ok6410_fb_win0 = {
	.max_bpp	= 32,
	.default_bpp	= 16,
	.xres		= 320,
	.yres		= 240,
	.virtual_y	= 320 * 2,
	.virtual_x	= 240,
};
static struct fb_videomode ok6410_lcd_timing = {
	.left_margin	= 68,
	.right_margin	= 40,
	.upper_margin	= 18,
	.lower_margin	= 5,
	.hsync_len	= 48,
	.vsync_len	= 15,
	.xres		= 320,
	.yres		= 240,
};
#elif defined(CONFIG_FB_S3C_X240320)
/* 3.5 inch 240X320 LCD(ACX502BMU) */
static struct s3c_fb_pd_win ok6410_fb_win0 = {
	.max_bpp	= 32,
	.default_bpp	= 16,
	.xres		= 320,
	.yres		= 240,
	.virtual_y	= 320 * 2,
	.virtual_x	= 240,
};
static struct fb_videomode ok6410_lcd_timing = {
	.left_margin	= 68,
	.right_margin	= 40,
	.upper_margin	= 18,
	.lower_margin	= 5,
	.hsync_len	= 48,
	.vsync_len	= 15,
	.xres		= 320,
	.yres		= 240,
};
#elif defined(CONFIG_FB_S3C_TFT640480)
/* 8 inch 640X480 L80 LCD */
static struct s3c_fb_pd_win ok6410_fb_win0 = {
	.max_bpp	= 32,
	.default_bpp	= 16,
	.xres		= 800,
	.yres		= 600,
	.virtual_y	= 800 * 2,
	.virtual_x	= 600,
};
static struct fb_videomode ok6410_lcd_timing = {
	.left_margin	= 73,
	.right_margin	= 55,
	.upper_margin	= 32,
	.lower_margin	= 36,
	.hsync_len	= 119,
	.vsync_len	= 5,
	.xres		= 800,
	.yres		= 600,
};
#elif defined(CONFIG_FB_S3C_VGA1024768)
/* VGA 1024x768 */
static struct s3c_fb_pd_win ok6410_fb_win0 = {
	.max_bpp	= 32,
	.default_bpp	= 16,
	.xres		= 1024,
	.yres		= 768,
	.virtual_y	= 1024 * 2,
	.virtual_x	= 768,
};
static struct fb_videomode ok6410_lcd_timing = {
	.left_margin	= 40,
	.right_margin	= 40,
	.upper_margin	= 32,
	.lower_margin	= 36,
	.hsync_len	= 119,
	.vsync_len	= 5,
	.xres		= 1024,
	.yres		= 768,
};
#elif defined(CONFIG_FB_S3C_EZVGA800600)
/* EZVGA 800x600 */
static struct s3c_fb_pd_win ok6410_fb_win0 = {
	.max_bpp	= 32,
	.default_bpp	= 16,
	.xres		= 800,
	.yres		= 600,
	.virtual_y	= 800 * 2,
	.virtual_x	= 600,
};
static struct fb_videomode ok6410_lcd_timing = {
	.left_margin	= 73,
	.right_margin	= 55,
	.upper_margin	= 32,
	.lower_margin	= 36,
	.hsync_len	= 119,
	.vsync_len	= 5,
	.xres		= 800,
	.yres		= 600,
};
#endif

/* 405566 clocks per frame => 60Hz refresh requires 24333960Hz clock */
static struct s3c_fb_platdata ok6410_lcd_pdata __initdata = {
	.setup_gpio	= s3c64xx_fb_gpio_setup_24bpp,
	.vtiming	= &ok6410_lcd_timing,
	.win[0]		= &ok6410_fb_win0,
	.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
	.vidcon1	= VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC,
};

#ifdef CONFIG_REGULATOR
static struct regulator_consumer_supply ok6410_b_pwr_5v_consumers[] __initdata = {
	REGULATOR_SUPPLY("PVDD", "0-001b"),
	REGULATOR_SUPPLY("AVDD", "0-001b"),
};

static struct regulator_init_data ok6410_b_pwr_5v_data = {
	.constraints = {
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(ok6410_b_pwr_5v_consumers),
	.consumer_supplies = ok6410_b_pwr_5v_consumers,
};

static struct fixed_voltage_config ok6410_b_pwr_5v_pdata = {
	.supply_name = "B_PWR_5V",
	.microvolts = 5000000,
	.init_data = &ok6410_b_pwr_5v_data,
	.gpio = -EINVAL,
};

static struct platform_device ok6410_b_pwr_5v = {
	.name          = "reg-fixed-voltage",
	.id            = -1,
	.dev = {
		.platform_data = &ok6410_b_pwr_5v_pdata,
	},
};
#endif

static struct s3c_ide_platdata ok6410_ide_pdata __initdata = {
	.setup_gpio	= s3c64xx_ide_setup_gpio,
};

static uint32_t ok6410_keymap[] __initdata = {
	/* KEY(row, col, keycode) */
	KEY(0, 3, KEY_1), KEY(0, 4, KEY_2), KEY(0, 5, KEY_3),
	KEY(0, 6, KEY_4), KEY(0, 7, KEY_5),
	KEY(1, 3, KEY_A), KEY(1, 4, KEY_B), KEY(1, 5, KEY_C),
	KEY(1, 6, KEY_D), KEY(1, 7, KEY_E)
};

static struct matrix_keymap_data ok6410_keymap_data __initdata = {
	.keymap		= ok6410_keymap,
	.keymap_size	= ARRAY_SIZE(ok6410_keymap),
};

static struct samsung_keypad_platdata ok6410_keypad_data __initdata = {
	.keymap_data	= &ok6410_keymap_data,
	.rows		= 2,
	.cols		= 8,
};

static struct map_desc ok6410_iodesc[] = {};

static struct platform_device *ok6410_devices[] __initdata = {
#ifdef CONFIG_SMDK6410_SD_CH0
	&s3c_device_hsmmc0,
#endif
#ifdef CONFIG_SMDK6410_SD_CH1
	&s3c_device_hsmmc1,
#endif
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_fb,
	&s3c_device_ohci,
	&samsung_device_pwm,
	&s3c_device_usb_hsotg,
	&s3c64xx_device_iisv4,
	&ok6410_device_led,
	&ok6410_gpio_button_device,
	&ok6410_gpio_ir_recv,
	&samsung_device_keypad,

#ifdef CONFIG_REGULATOR
	&ok6410_b_pwr_5v,
#endif
	&ok6410_lcd_powerdev,

	&s3c_device_nand,
#ifdef CONFIG_DM9000
	&s3c_device_dm9000,
#endif
	&s3c_device_adc,
	&s3c_device_cfcon,
	&s3c_device_rtc,
	&s3c_device_ts,
	&s3c_device_wdt,
};

#ifdef CONFIG_REGULATOR
/* ARM core */
static struct regulator_consumer_supply ok6410_vddarm_consumers[] = {
	REGULATOR_SUPPLY("vddarm", NULL),
};

/* VDDARM, BUCK1 on J5 */
static struct regulator_init_data ok6410_vddarm = {
	.constraints = {
		.name = "PVDD_ARM",
		.min_uV = 1000000,
		.max_uV = 1300000,
		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
	},
	.num_consumer_supplies = ARRAY_SIZE(ok6410_vddarm_consumers),
	.consumer_supplies = ok6410_vddarm_consumers,
};

/* VDD_INT, BUCK2 on J5 */
static struct regulator_init_data ok6410_vddint = {
	.constraints = {
		.name = "PVDD_INT",
		.min_uV = 1000000,
		.max_uV = 1200000,
		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
	},
};

/* VDD_HI, LDO3 on J5 */
static struct regulator_init_data ok6410_vddhi = {
	.constraints = {
		.name = "PVDD_HI",
		.always_on = 1,
	},
};

/* VDD_PLL, LDO2 on J5 */
static struct regulator_init_data ok6410_vddpll = {
	.constraints = {
		.name = "PVDD_PLL",
		.always_on = 1,
	},
};

/* VDD_UH_MMC, LDO5 on J5 */
static struct regulator_init_data ok6410_vdduh_mmc = {
	.constraints = {
		.name = "PVDD_UH+PVDD_MMC",
		.always_on = 1,
	},
};

/* VCCM3BT, LDO8 on J5 */
static struct regulator_init_data ok6410_vccmc3bt = {
	.constraints = {
		.name = "PVCCM3BT",
		.always_on = 1,
	},
};

/* VCCM2MTV, LDO11 on J5 */
static struct regulator_init_data ok6410_vccm2mtv = {
	.constraints = {
		.name = "PVCCM2MTV",
		.always_on = 1,
	},
};

/* VDD_LCD, LDO12 on J5 */
static struct regulator_init_data ok6410_vddlcd = {
	.constraints = {
		.name = "PVDD_LCD",
		.always_on = 1,
	},
};

/* VDD_OTGI, LDO9 on J5 */
static struct regulator_init_data ok6410_vddotgi = {
	.constraints = {
		.name = "PVDD_OTGI",
		.always_on = 1,
	},
};

/* VDD_OTG, LDO14 on J5 */
static struct regulator_init_data ok6410_vddotg = {
	.constraints = {
		.name = "PVDD_OTG",
		.always_on = 1,
	},
};

/* VDD_ALIVE, LDO15 on J5 */
static struct regulator_init_data ok6410_vddalive = {
	.constraints = {
		.name = "PVDD_ALIVE",
		.always_on = 1,
	},
};

/* VDD_AUDIO, VLDO_AUDIO on J5 */
static struct regulator_init_data ok6410_vddaudio = {
	.constraints = {
		.name = "PVDD_AUDIO",
		.always_on = 1,
	},
};
#endif

#ifdef CONFIG_SMDK6410_WM1190_EV1
/* S3C64xx internal logic & PLL */
static struct regulator_init_data wm8350_dcdc1_data = {
	.constraints = {
		.name = "PVDD_INT+PVDD_PLL",
		.min_uV = 1200000,
		.max_uV = 1200000,
		.always_on = 1,
		.apply_uV = 1,
	},
};

/* Memory */
static struct regulator_init_data wm8350_dcdc3_data = {
	.constraints = {
		.name = "PVDD_MEM",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.always_on = 1,
		.state_mem = {
			 .uV = 1800000,
			 .mode = REGULATOR_MODE_NORMAL,
			 .enabled = 1,
		},
		.initial_state = PM_SUSPEND_MEM,
	},
};

/* USB, EXT, PCM, ADC/DAC, USB, MMC */
static struct regulator_consumer_supply wm8350_dcdc4_consumers[] = {
	REGULATOR_SUPPLY("DVDD", "0-001b"),
};

static struct regulator_init_data wm8350_dcdc4_data = {
	.constraints = {
		.name = "PVDD_HI+PVDD_EXT+PVDD_SYS+PVCCM2MTV",
		.min_uV = 3000000,
		.max_uV = 3000000,
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(wm8350_dcdc4_consumers),
	.consumer_supplies = wm8350_dcdc4_consumers,
};

/* OTGi/1190-EV1 HPVDD & AVDD */
static struct regulator_init_data wm8350_ldo4_data = {
	.constraints = {
		.name = "PVDD_OTGI+HPVDD+AVDD",
		.min_uV = 1200000,
		.max_uV = 1200000,
		.apply_uV = 1,
		.always_on = 1,
	},
};

static struct {
	int regulator;
	struct regulator_init_data *initdata;
} wm1190_regulators[] = {
	{ WM8350_DCDC_1, &wm8350_dcdc1_data },
	{ WM8350_DCDC_3, &wm8350_dcdc3_data },
	{ WM8350_DCDC_4, &wm8350_dcdc4_data },
	{ WM8350_DCDC_6, &ok6410_vddarm },
	{ WM8350_LDO_1, &ok6410_vddalive },
	{ WM8350_LDO_2, &ok6410_vddotg },
	{ WM8350_LDO_3, &ok6410_vddlcd },
	{ WM8350_LDO_4, &wm8350_ldo4_data },
};

static int __init ok6410_wm8350_init(struct wm8350 *wm8350)
{
	int i;

	/* Configure the IRQ line */
	s3c_gpio_setpull(S3C64XX_GPN(12), S3C_GPIO_PULL_UP);

	/* Instantiate the regulators */
	for (i = 0; i < ARRAY_SIZE(wm1190_regulators); i++)
		wm8350_register_regulator(wm8350,
					  wm1190_regulators[i].regulator,
					  wm1190_regulators[i].initdata);

	return 0;
}

static struct wm8350_platform_data __initdata ok6410_wm8350_pdata = {
	.init = ok6410_wm8350_init,
	.irq_high = 1,
	.irq_base = IRQ_BOARD_START,
};
#endif

#ifdef CONFIG_SMDK6410_WM1192_EV1
static struct gpio_led wm1192_pmic_leds[] = {
	{
		.name = "PMIC:red:power",
		.gpio = GPIO_BOARD_START + 3,
		.default_state = LEDS_GPIO_DEFSTATE_ON,
	},
};

static struct gpio_led_platform_data wm1192_pmic_led = {
	.num_leds = ARRAY_SIZE(wm1192_pmic_leds),
	.leds = wm1192_pmic_leds,
};

static struct platform_device wm1192_pmic_led_dev = {
	.name          = "leds-gpio",
	.id            = -1,
	.dev = {
		.platform_data = &wm1192_pmic_led,
	},
};

static int wm1192_pre_init(struct wm831x *wm831x)
{
	int ret;

	/* Configure the IRQ line */
	s3c_gpio_setpull(S3C64XX_GPN(12), S3C_GPIO_PULL_UP);

	ret = platform_device_register(&wm1192_pmic_led_dev);
	if (ret != 0)
		dev_err(wm831x->dev, "Failed to add PMIC LED: %d\n", ret);

	return 0;
}

static struct wm831x_backlight_pdata wm1192_backlight_pdata = {
	.isink = 1,
	.max_uA = 27554,
};

static struct regulator_init_data wm1192_dcdc3 = {
	.constraints = {
		.name = "PVDD_MEM+PVDD_GPS",
		.always_on = 1,
	},
};

static struct regulator_consumer_supply wm1192_ldo1_consumers[] = {
	REGULATOR_SUPPLY("DVDD", "0-001b"),   /* WM8580 */
};

static struct regulator_init_data wm1192_ldo1 = {
	.constraints = {
		.name = "PVDD_LCD+PVDD_EXT",
		.always_on = 1,
	},
	.consumer_supplies = wm1192_ldo1_consumers,
	.num_consumer_supplies = ARRAY_SIZE(wm1192_ldo1_consumers),
};

static struct wm831x_status_pdata wm1192_led7_pdata = {
	.name = "LED7:green:",
};

static struct wm831x_status_pdata wm1192_led8_pdata = {
	.name = "LED8:green:",
};

static struct wm831x_pdata ok6410_wm1192_pdata = {
	.pre_init = wm1192_pre_init,

	.backlight = &wm1192_backlight_pdata,
	.dcdc = {
		&ok6410_vddarm,  /* DCDC1 */
		&ok6410_vddint,  /* DCDC2 */
		&wm1192_dcdc3,
	},
	.gpio_base = GPIO_BOARD_START,
	.ldo = {
		 &wm1192_ldo1,        /* LDO1 */
		 &ok6410_vdduh_mmc, /* LDO2 */
		 NULL,                /* LDO3 NC */
		 &ok6410_vddotgi,   /* LDO4 */
		 &ok6410_vddotg,    /* LDO5 */
		 &ok6410_vddhi,     /* LDO6 */
		 &ok6410_vddaudio,  /* LDO7 */
		 &ok6410_vccm2mtv,  /* LDO8 */
		 &ok6410_vddpll,    /* LDO9 */
		 &ok6410_vccmc3bt,  /* LDO10 */
		 &ok6410_vddalive,  /* LDO11 */
	},
	.status = {
		&wm1192_led7_pdata,
		&wm1192_led8_pdata,
	},
};
#endif

static struct i2c_board_info i2c_devs0[] __initdata = {
	{ I2C_BOARD_INFO("24c08", 0x50), },
	{ I2C_BOARD_INFO("wm8580", 0x1b), },

#ifdef CONFIG_SMDK6410_WM1192_EV1
	{ I2C_BOARD_INFO("wm8312", 0x34),
	  .platform_data = &ok6410_wm1192_pdata,
	  .irq = S3C_EINT(12),
	},
#endif

#ifdef CONFIG_SMDK6410_WM1190_EV1
	{ I2C_BOARD_INFO("wm8350", 0x1a),
	  .platform_data = &ok6410_wm8350_pdata,
	  .irq = S3C_EINT(12),
	},
#endif
};

static struct i2c_board_info i2c_devs1[] __initdata = {
	{ I2C_BOARD_INFO("24c128", 0x57), },	/* Samsung S524AD0XD1 */
};

/* LCD Backlight data */
static struct samsung_bl_gpio_info ok6410_bl_gpio_info = {
	.no = S3C64XX_GPF(15),
	.func = S3C_GPIO_SFN(2),
};

static struct platform_pwm_backlight_data ok6410_bl_data = {
	.pwm_id = 1,
};

static int ok6410_usb_phy_init(struct platform_device *pdev, int type)
{
	return 0;
}

static int ok6410_usb_phy_exit(struct platform_device *pdev, int type)
{
	return 0;
}

static struct s3c_hsotg_plat ok6410_hsotg_pdata = {
	/* replace phy init/exit NOP functions, so keep usb phy clock enabled
	 * which is needed by USB 1.0 host controller.
	 */
	.phy_init = ok6410_usb_phy_init,
	.phy_exit = ok6410_usb_phy_exit,
};

static void __init ok6410_map_io(void)
{
	u32 tmp;

	s3c64xx_init_io(ok6410_iodesc, ARRAY_SIZE(ok6410_iodesc));
	s3c64xx_set_xtal_freq(12000000);
	s3c24xx_init_uarts(ok6410_uartcfgs, ARRAY_SIZE(ok6410_uartcfgs));
	samsung_set_timer_source(SAMSUNG_PWM3, SAMSUNG_PWM4);

	/* set the LCD type */

	tmp = __raw_readl(S3C64XX_SPCON);
	tmp &= ~S3C64XX_SPCON_LCD_SEL_MASK;
	tmp |= S3C64XX_SPCON_LCD_SEL_RGB;
	__raw_writel(tmp, S3C64XX_SPCON);

	/* remove the lcd bypass */
	tmp = __raw_readl(S3C64XX_MODEM_MIFPCON);
	tmp &= ~MIFPCON_LCD_BYPASS;
	__raw_writel(tmp, S3C64XX_MODEM_MIFPCON);
}

static void __init ok6410_machine_init(void)
{
	u32 cs1;

	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);
	s3c_fb_set_platdata(&ok6410_lcd_pdata);
	s3c_hsotg_set_platdata(&ok6410_hsotg_pdata);

	samsung_keypad_set_platdata(&ok6410_keypad_data);

	s3c_nand_set_platdata(&ok6410_nand_info);
	s3c24xx_ts_set_platdata(NULL);

	/* configure nCS1 width to 16 bits */

	cs1 = __raw_readl(S3C64XX_SROM_BW) &
		    ~(S3C64XX_SROM_BW__CS_MASK << S3C64XX_SROM_BW__NCS1__SHIFT);
	cs1 |= ((1 << S3C64XX_SROM_BW__DATAWIDTH__SHIFT) |
		(1 << S3C64XX_SROM_BW__WAITENABLE__SHIFT) |
		(1 << S3C64XX_SROM_BW__BYTEENABLE__SHIFT)) <<
						   S3C64XX_SROM_BW__NCS1__SHIFT;
	__raw_writel(cs1, S3C64XX_SROM_BW);

	/* set timing for nCS1 suitable for ethernet chip */

	__raw_writel((0 << S3C64XX_SROM_BCX__PMC__SHIFT) |
		     (6 << S3C64XX_SROM_BCX__TACP__SHIFT) |
		     (4 << S3C64XX_SROM_BCX__TCAH__SHIFT) |
		     (1 << S3C64XX_SROM_BCX__TCOH__SHIFT) |
		     (0xe << S3C64XX_SROM_BCX__TACC__SHIFT) |
		     (4 << S3C64XX_SROM_BCX__TCOS__SHIFT) |
		     (0 << S3C64XX_SROM_BCX__TACS__SHIFT), S3C64XX_SROM_BC1);

	gpio_request(S3C64XX_GPN(5), "LCD power");
	gpio_request(S3C64XX_GPF(13), "LCD power");

	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));

	s3c_ide_set_platdata(&ok6410_ide_pdata);

	samsung_bl_set(&ok6410_bl_gpio_info, &ok6410_bl_data);

	platform_add_devices(ok6410_devices, ARRAY_SIZE(ok6410_devices));

	/* enable USB phy clock */
	s5p_usb_phy_init(&s3c_device_usb_hsotg, USB_PHY_TYPE_DEVICE);
}

MACHINE_START(OK6410, "OK6410")
	/* Maintainer: Ben Dooks <ben-linux@fluff.org> */
	.atag_offset	= 0x100,

	.init_irq	= s3c6410_init_irq,
	.map_io		= ok6410_map_io,
	.init_machine	= ok6410_machine_init,
	.init_time	= samsung_timer_init,
	.restart	= s3c64xx_restart,
MACHINE_END

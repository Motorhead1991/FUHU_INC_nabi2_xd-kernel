/*
 * arch/arm/mach-tegra/board-kai-sdhci.c
 *
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2012 NVIDIA Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/resource.h>
#include <linux/platform_device.h>
#if 1 //CL2N
#include <linux/wlan_plat.h>  
#endif
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/mmc/host.h>
#if 0 //Kai
#include <linux/wl12xx.h>
#endif

#include <asm/mach-types.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/sdhci.h>
#include <mach/io_dpd.h>

#include "gpio-names.h"
#include "board.h"
#include "board-kai.h"

#define KAI_SD_CD	TEGRA_GPIO_PI5
#if 0 //Kai
#define KAI_WLAN_EN	TEGRA_GPIO_PD3
#define KAI_WLAN_IRQ	TEGRA_GPIO_PV1
#else //CL2N
#define KAI_WLAN_PWR	TEGRA_GPIO_PD4
#define KAI_WLAN_WOW	TEGRA_GPIO_PW3
#endif

static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;
#if 1 //CL2N
static int kai_wifi_reset(int on);
#endif
static int kai_wifi_power(int power_on);
static int kai_wifi_set_carddetect(int val);

static int kai_wifi_status_register(
		void (*callback)(int card_present, void *dev_id),
		void *dev_id)
{
	if (wifi_status_cb)
		return -EAGAIN;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

#if 0 //Kai
static struct wl12xx_platform_data kai_wlan_data __initdata = {
	.irq = TEGRA_GPIO_TO_IRQ(KAI_WLAN_IRQ),
	.board_ref_clock = WL12XX_REFCLOCK_26,
	.board_tcxo_clock = 1,
	.set_power = kai_wifi_power,
	.set_carddetect = kai_wifi_set_carddetect,
};
#else //CL2N
static struct wifi_platform_data kai_wifi_control = {
	.set_power	= kai_wifi_power,
	.set_reset	= kai_wifi_reset,
	.set_carddetect	= kai_wifi_set_carddetect,
};

static struct resource wifi_resource[] = {
	[0] = {
		.name	= "bcmdhd_wlan_irq",
		.start	= TEGRA_GPIO_TO_IRQ(KAI_WLAN_WOW),
		.end	= TEGRA_GPIO_TO_IRQ(KAI_WLAN_WOW),
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE,
	},
};

static struct platform_device kai_wifi_device = {
	.name		= "bcmdhd_wlan",
	.id		= 1,
	.num_resources	= 1,
	.resource	= wifi_resource,
	.dev		= {
		.platform_data = &kai_wifi_control,
	},
};
#endif

static struct resource sdhci_resource0[] = {
	[0] = {
		.start	= INT_SDMMC1,
		.end	= INT_SDMMC1,
		.flags	= IORESOURCE_IRQ,
	},
	[1] = {
		.start	= TEGRA_SDMMC1_BASE,
		.end	= TEGRA_SDMMC1_BASE + TEGRA_SDMMC1_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct resource sdhci_resource2[] = {
	[0] = {
		.start	= INT_SDMMC3,
		.end	= INT_SDMMC3,
		.flags	= IORESOURCE_IRQ,
	},
	[1] = {
		.start	= TEGRA_SDMMC3_BASE,
		.end	= TEGRA_SDMMC3_BASE + TEGRA_SDMMC3_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct resource sdhci_resource3[] = {
	[0] = {
		.start	= INT_SDMMC4,
		.end	= INT_SDMMC4,
		.flags	= IORESOURCE_IRQ,
	},
	[1] = {
		.start	= TEGRA_SDMMC4_BASE,
		.end	= TEGRA_SDMMC4_BASE + TEGRA_SDMMC4_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
};


static struct tegra_sdhci_platform_data tegra_sdhci_platform_data2 = {
	.mmc_data = {
		.register_status_notify	= kai_wifi_status_register,
		.built_in = 0,
		.ocr_mask = MMC_OCR_1V8_MASK,
	},
#ifndef CONFIG_MMC_EMBEDDED_SDIO
	.pm_flags = MMC_PM_KEEP_POWER,
#endif
	.cd_gpio = -1,
	.wp_gpio = -1,
	.power_gpio = -1,
	.tap_delay = 0x0F,
	.ddr_clk_limit = 41000000,
/*	.is_voltage_switch_supported = false,
	.vdd_rail_name = NULL,
	.slot_rail_name = NULL,
	.vdd_max_uv = -1,
	.vdd_min_uv = -1,
	.max_clk = 0,
	.is_8bit_supported = false, */
	/* .max_clk = 25000000, */
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data0 = {
	.cd_gpio = KAI_SD_CD,
	.wp_gpio = -1,
	.power_gpio = -1,
	.tap_delay = 0x0F,
	.ddr_clk_limit = 52000000, /* &*&*&*SJ_20120220, To adjust microSDHC UHS-I memory card DDR50 mode clcok limit from 41MHz to 52Mhz. */
/*	.is_voltage_switch_supported = true,
	.vdd_rail_name = "vddio_sdmmc1",
	.slot_rail_name = "vddio_sd_slot",
	.vdd_max_uv = 3320000,
	.vdd_min_uv = 3280000,
	.max_clk = 208000000,
	.is_8bit_supported = false, */
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data3 = {
	.cd_gpio = -1,
	.wp_gpio = -1,
	.power_gpio = -1,
	.is_8bit = 1,
	.tap_delay = 0x0F,
	.ddr_clk_limit = 41000000,
	.mmc_data = {
		.built_in = 1,
	}
/*	.is_voltage_switch_supported = false,
	.vdd_rail_name = NULL,
	.slot_rail_name = NULL,
	.vdd_max_uv = -1,
	.vdd_min_uv = -1,
	.max_clk = 48000000,
	.is_8bit_supported = true, */
};

static struct platform_device tegra_sdhci_device0 = {
	.name		= "sdhci-tegra",
	.id		= 0,
	.resource	= sdhci_resource0,
	.num_resources	= ARRAY_SIZE(sdhci_resource0),
	.dev = {
		.platform_data = &tegra_sdhci_platform_data0,
	},
};

static struct platform_device tegra_sdhci_device2 = {
	.name		= "sdhci-tegra",
	.id		= 2,
	.resource	= sdhci_resource2,
	.num_resources	= ARRAY_SIZE(sdhci_resource2),
	.dev = {
		.platform_data = &tegra_sdhci_platform_data2,
	},
};

static struct platform_device tegra_sdhci_device3 = {
	.name		= "sdhci-tegra",
	.id		= 3,
	.resource	= sdhci_resource3,
	.num_resources	= ARRAY_SIZE(sdhci_resource3),
	.dev = {
		.platform_data = &tegra_sdhci_platform_data3,
	},
};

static int kai_wifi_set_carddetect(int val)
{
	pr_debug("%s: %d\n", __func__, val);
	if (wifi_status_cb)
		wifi_status_cb(val, wifi_status_cb_devid);
	else
	pr_warning("%s: Nobody to notify\n", __func__);
	return 0;
}

static int kai_wifi_power(int power_on)
{
#if 0 //KAI
	struct tegra_io_dpd *sd_dpd;
	pr_err("Powering %s wifi\n", (power_on ? "on" : "off"));

	/*
	 * FIXME : we need to revisit IO DPD code
	 * on how should multiple pins under DPD get controlled
	 *
	 * kai GPIO WLAN enable is part of SDMMC3 pin group
	 */
	sd_dpd = tegra_io_dpd_get(&tegra_sdhci_device2.dev);
	if (sd_dpd) {
		mutex_lock(&sd_dpd->delay_lock);
		tegra_io_dpd_disable(sd_dpd);
		mutex_unlock(&sd_dpd->delay_lock);
	}
	if (power_on) {
		gpio_set_value(KAI_WLAN_EN, 1);
		mdelay(15);
		gpio_set_value(KAI_WLAN_EN, 0);
		mdelay(1);
		gpio_set_value(KAI_WLAN_EN, 1);
		mdelay(70);
	} else {
		gpio_set_value(KAI_WLAN_EN, 0);
	}
	if (sd_dpd) {
		mutex_lock(&sd_dpd->delay_lock);
		tegra_io_dpd_enable(sd_dpd);
		mutex_unlock(&sd_dpd->delay_lock);
	}
#else //CL2N
    pr_err("Powering %s wifi\n", (power_on ? "on" : "off"));
	gpio_set_value(KAI_WLAN_PWR, power_on);
	mdelay(100);
#endif
	return 0;
}

#if 1 //CL2N
static int kai_wifi_reset(int on)
{
	pr_debug("%s: do nothing\n", __func__);
	return 0;
}
#endif

#ifdef CONFIG_TEGRA_PREPOWER_WIFI
static int __init kai_wifi_prepower(void)
{
	if (!machine_is_kai())
		return 0;

	kai_wifi_power(1);

	return 0;
}

subsys_initcall_sync(kai_wifi_prepower);
#endif

static int __init kai_wifi_init(void)
{
	int rc;
#if 0 //KAI
	rc = gpio_request(KAI_WLAN_EN, "wl12xx");
	if (rc)
		pr_err("WLAN_EN gpio request failed:%d\n", rc);

	rc = gpio_request(KAI_WLAN_IRQ, "wl12xx");
	if (rc)
		pr_err("WLAN_IRQ gpio request failed:%d\n", rc);

	rc = gpio_direction_output(KAI_WLAN_EN, 0);
	if (rc)
		pr_err("WLAN_EN gpio direction configuration failed:%d\n", rc);

	rc = gpio_direction_input(KAI_WLAN_IRQ);
	if (rc)
		pr_err("WLAN_IRQ gpio direction configuration failed:%d\n", rc);

	if (wl12xx_set_platform_data(&kai_wlan_data))
		pr_err("Error setting wl12xx data\n");
#else //CL2N
	rc = gpio_request(KAI_WLAN_PWR, "wlan_power");
	if (rc)
		pr_err("WLAN_PWR gpio request failed:%d\n", rc);
	
	rc = gpio_request(KAI_WLAN_WOW, "bcmsdh_sdmmc");
	if (rc)
		pr_err("WLAN_WOW gpio request failed:%d\n", rc);

	tegra_gpio_enable(KAI_WLAN_PWR);
	tegra_gpio_enable(KAI_WLAN_WOW);

	rc = gpio_direction_output(KAI_WLAN_PWR, 0);
	if (rc)
		pr_err("WLAN_PWR gpio direction configuration failed:%d\n", rc);
	
	rc = gpio_direction_input(KAI_WLAN_WOW);
	if (rc)
		pr_err("WLAN_WOW gpio direction configuration failed:%d\n", rc);

	platform_device_register(&kai_wifi_device);
#endif
	return 0;
}

int __init kai_sdhci_init(void)
{
	platform_device_register(&tegra_sdhci_device3);
	platform_device_register(&tegra_sdhci_device2);
	platform_device_register(&tegra_sdhci_device0);

	kai_wifi_init();
	return 0;
}

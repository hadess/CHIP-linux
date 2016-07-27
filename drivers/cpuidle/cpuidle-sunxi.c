/*
 * CPU idle support for Allwinner SoCs
 *
 * Copyright (C) 2016 Antoine Tenart <antoine.tenart@free-electrons.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/cpuidle.h>
#include <linux/cpu_pm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/psci.h>

#include <asm/cpuidle.h>
#include <asm/suspend.h>

#include <uapi/linux/psci.h>

#define SUNXI_IDLE_PARAM				\
	((0 << PSCI_0_2_POWER_STATE_ID_SHIFT) |		\
	 (0 << PSCI_0_2_POWER_STATE_AFFL_SHIFT) |	\
	 (PSCI_POWER_STATE_TYPE_POWER_DOWN << PSCI_0_2_POWER_STATE_TYPE_SHIFT))

static int sunxi_psci_suspend(unsigned long val)
{
	return psci_ops.cpu_suspend(SUNXI_IDLE_PARAM, __pa(cpu_resume));
}

static int sunxi_enter_idle(struct cpuidle_device *dev,
			    struct cpuidle_driver *drv, int index)
{
	cpu_pm_enter();
	cpu_suspend(0, sunxi_psci_suspend);
	cpu_pm_exit();

	return index;
}

static struct cpuidle_driver sunxi_idle_driver = {
	.name		= "sunxi-idle",
	.owner		= THIS_MODULE,
	.states[0]	= ARM_CPUIDLE_WFI_STATE,
	.states[1]	= {
		.enter			= sunxi_enter_idle,
		.exit_latency		= 10,
		.target_residency	= 10000,
		.name			= "DDR SR",
		.desc			= "WFI and DDR Self Refresh",
	},
	.state_count	= 2,
};

static int sunxi_cpuidle_probe(struct platform_device *dev)
{
	return cpuidle_register(&sunxi_idle_driver, NULL);
}

static struct platform_driver sunxi_cpuidle_driver = {
	.driver	= {
		.name = "cpuidle-sunxi",
	},
	.probe	= sunxi_cpuidle_probe,
};
builtin_platform_driver(sunxi_cpuidle_driver);

MODULE_AUTHOR("Antoine Tenart <antoine.tenart@free-electrons.com>");
MODULE_DESCRIPTION("CPU idle driver for Allwinner SoCs");
MODULE_LICENSE("GPL");

// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

//#define DEBUG
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/thermal.h>
#include <linux/cpufreq.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include "cpucore_cooling.h"
#include <linux/cpumask.h>
#include "thermal_core.h"

static int cpucore_get_max_state(struct thermal_cooling_device *cdev,
			      unsigned long *max)
{
	struct cpucore_cooling_device *cpucore_cdev = cdev->devdata;

	*max = cpucore_cdev->cpunum;

	return 0;
}

static int cpucore_get_cur_state(struct thermal_cooling_device *cdev,
			      unsigned long *state)
{
	struct cpucore_cooling_device *cpucore_cdev = cdev->devdata;

	*state = cpucore_cdev->setstep;

	return 0;
}

static int calculate_hotstep(struct thermal_cooling_device *cdev,
			      unsigned long state)
{
	struct cpucore_cooling_device *cpucore_cdev = cdev->devdata;
	int i, cpu, ret = 0;

	if (WARN_ON(state > cpucore_cdev->cpunum))
		return -EINVAL;

	if (state > cpucore_cdev->setstep)
		cpucore_cdev->mode = CPU_UNPLUG;
	else if (state < cpucore_cdev->setstep)
		cpucore_cdev->mode = CPU_PLUG;
	else
		return 0;

	cpucore_cdev->setstep = state;

	switch (cpucore_cdev->mode) {
	case CPU_PLUG:
		cpu = cpucore_cdev->cores_num[state];
		for (i = 0; i < cpucore_cdev->cluster_num; i++) {
			if (!cpumask_weight(cpucore_cdev->offline[i]))
				continue;
			if (!(cpumask_test_cpu(cpu, cpucore_cdev->offline[i]) && !cpu_online(cpu)))
				continue;
			pr_debug("[%s]online cpu%d\n", cdev->type, cpu);
			ret = add_cpu(cpu);
			if (ret)
				pr_err("add cpu%d failed!!!\n", cpu);
			cpumask_clear_cpu(cpu, cpucore_cdev->offline[i]);
			cpumask_set_cpu(cpu, cpucore_cdev->online[i]);
			break;
		}
		break;
	case CPU_UNPLUG:
		cpu = cpucore_cdev->cores_num[state - 1];
		for (i = 0; i < cpucore_cdev->cluster_num; i++) {
			if (!cpumask_weight(cpucore_cdev->online[i]))
				continue;
			if (!(cpumask_test_cpu(cpu, cpucore_cdev->online[i]) && cpu_online(cpu)))
				continue;
			if (cpu == nr_cpu_ids)
				continue;
			pr_debug("[%s]offline cpu%d\n", cdev->type, cpu);
			ret = remove_cpu(cpu);
			if (ret)
				pr_err("cpu%d remove failed!!!\n", cpu);
			cpumask_set_cpu(cpu, cpucore_cdev->offline[i]);
			cpumask_clear_cpu(cpu, cpucore_cdev->online[i]);
			break;
		}
		break;
	default:
		break;
	}

	return 0;
}

static int cpucore_set_cur_state(struct thermal_cooling_device *cdev,
			      unsigned long state)
{
	int ret = 0;

	ret = calculate_hotstep(cdev, state);

	return ret;
}

static const struct thermal_cooling_device_ops cpucore_cooling_ops = {
	.get_max_state = cpucore_get_max_state,
	.get_cur_state = cpucore_get_cur_state,
	.set_cur_state = cpucore_set_cur_state,
};

static int setup_cooling_params(struct cpucore_cooling_device *cdev,
	struct device_node *np)
{
	int i, j, cpu, offset;

	cdev->cpunum = 0;
	cdev->setstep = 0;
	cdev->mode = CPU_MODE_MAX;

	cdev->cluster_num = of_property_count_u32_elems(np, "cluster_core_num");
	if (cdev->cluster_num < 1)
		return -EINVAL;
	cdev->cpu_cores_num = of_property_count_u32_elems(np, "cluster_cores");
	if (cdev->cpu_cores_num < 1)
		return -EINVAL;
	cdev->cluster_core_num = devm_kcalloc(cdev->dev, cdev->cluster_num,
					      sizeof(u32), GFP_KERNEL);
	if (!cdev->cluster_core_num)
		return -ENOMEM;
	cdev->cores_num = devm_kcalloc(cdev->dev, cdev->cpu_cores_num,
				       sizeof(u32), GFP_KERNEL);
	if (!cdev->cores_num)
		return -ENOMEM;
	cdev->online = devm_kcalloc(cdev->dev, cdev->cluster_num,
				    sizeof(cpumask_var_t), GFP_KERNEL);
	if (!cdev->online)
		return -ENOMEM;
	cdev->offline = devm_kcalloc(cdev->dev, cdev->cluster_num,
				     sizeof(cpumask_var_t), GFP_KERNEL);
	if (!cdev->offline)
		return -ENOMEM;
	if (of_property_read_u32_array(np, "cluster_core_num",
		cdev->cluster_core_num, cdev->cluster_num))
		return -EINVAL;
	for (i = 0; i < cdev->cluster_num; i++) {
		offset = i == 0 ? 0 : cdev->cluster_core_num[i - 1];
		for (j = 0; j < cdev->cluster_core_num[i]; j++) {
			if (!of_property_read_u32_index(np, "cluster_cores", j + offset,
				&cpu)) {
				cdev->cores_num[j + offset] = cpu;
				cpumask_set_cpu(cpu, cdev->online[i]);
				cdev->cpunum++;
			}
		}
		pr_debug("cluster%d[%d %*pbl]\n", i, cdev->cluster_core_num[i],
			cpumask_pr_args(cdev->online[i]));
	}

	return 0;
}

void cpucore_cooling_unregister(struct thermal_cooling_device *cdev)
{
	thermal_cooling_device_unregister(cdev);
}

static int cpucore_cooling_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct thermal_cooling_device *cool_dev;
	struct cpucore_cooling_device *cpucore_cdev;
	int ret;

	cpucore_cdev = devm_kzalloc(dev, sizeof(*cpucore_cdev), GFP_KERNEL);
	if (!cpucore_cdev)
		return -ENOMEM;

	cpucore_cdev->dev = dev;
	if (setup_cooling_params(cpucore_cdev, pdev->dev.of_node))
		return -EINVAL;
	cpucore_cdev->setstep = 0;
	cool_dev = thermal_of_cooling_device_register(pdev->dev.of_node, "thermal-cpucore",
						      cpucore_cdev, &cpucore_cooling_ops);
	if (IS_ERR(cool_dev)) {
		ret = PTR_ERR(cool_dev);
		dev_err(&pdev->dev, "Failed to register cpucore cooling device: %d\n", ret);
		return ret;
	}

	cpucore_cdev->cool_dev = cool_dev;
	dev_set_drvdata(dev, cpucore_cdev);
	dev_dbg(dev, "cpucore cooling device probe done\n");

	return 0;
}

static void cpucore_cooling_remove(struct platform_device *pdev)
{
}

static const struct of_device_id cpucore_of_match[] = {
	{
		.compatible = "amlogic, cpucore-cooling",
	},
	{},
};

struct platform_driver cpucore_cooling_platdrv = {
	.driver = {
		.name           = "cpucore-cooling",
		.owner          = THIS_MODULE,
		.of_match_table = cpucore_of_match,
	},
	.probe  = cpucore_cooling_probe,
	.remove = cpucore_cooling_remove,

};
